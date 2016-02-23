#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#ifdef __linux__
#include <stdint.h>
#endif

//
//
//
//
//
//
//
//
//
//
//              DO NOT READ THIS SHITCODE, PLEASE !!!!!!!!11!!
//
//
//
//
//
//
//
//
//
//

#define EOC ((uint32_t)0xFFFFFFF8)  //End of chain
#define NEXT_ID UINT64_MAX

#ifndef __x86_64__
#error "Only x86-64 processors are supported"
#endif
#ifdef _WIN32
#error "Windows must die!"
#endif
#if !(defined(__APPLE__) || defined(__linux__))
#warning "This shit was only tested on Linux and OS X. Use it at own risk!"
#endif

FILE *file;
uint32_t sizeOfDirectory;
uint32_t entriesUsed;

uint32_t tableBuf[1024], curTableId;


typedef struct Entry {
    uint64_t id;
    char *name;
    char *number;
} Entry;


void readHeader(void) {
    fseek(file, 0x0, SEEK_SET);
    fread(tableBuf, sizeof(uint32_t), 1024, file);
    curTableId = 0;

    fseek(file, 0x1040, SEEK_SET);
    fread(&sizeOfDirectory, sizeof(uint32_t), 1, file);
    fread(&entriesUsed, sizeof(uint32_t), 1, file);

    //Signature check
    if (tableBuf[0] != 0x746e6f43) {
        printf("Incorrect file\n");
        exit(1);
    }
}

void flushCurTable(void) {
    fseek(file, 0x5000 * curTableId, SEEK_SET);
    fwrite(tableBuf, 4096, 1, file);
}

void writeHeader(void) {
    flushCurTable();

    fseek(file, 0x1040, SEEK_SET);

    fwrite(&sizeOfDirectory, sizeof(uint32_t), 1, file);
    fwrite(&entriesUsed, sizeof(uint32_t), 1, file);
}

//loads another table into buffer if it is necessary
void changeTable(uint32_t block) {
    if (curTableId != (block >> 10)) {
        flushCurTable();

        curTableId = (block >> 10);

        long tablePos = (long) 0x5000 * curTableId;

        //fsetpos(file, &tablePos);
        fseek(file, tablePos, SEEK_SET);
        fread(tableBuf, sizeof(uint32_t), 1024, file);
    }
}

void writeTable(uint32_t block, uint32_t value) {
    changeTable(block);

    tableBuf[block & 0x3FF] = value;

    //long posOfRecord = (long) 0x5000 * curTableId + (block & 0x3FF) * 4;
    ////fsetpos(file, &posOfRecord);
    //fseek(file, posOfRecord, SEEK_SET);
    //fwrite(&value, sizeof(uint32_t), 1, file);
}

void createTable(uint32_t table) {
    flushCurTable();
    memset(tableBuf, 0, 4096);

    curTableId = table;

    tableBuf[0] = 0x746e6f43;       //Signature "Cont"
    tableBuf[1] = 1020;             //Empty blocks in the table
    tableBuf[2] = 4;                //Next block to be written

    fseek(file, 0x5000 * table, SEEK_SET);
    fwrite(tableBuf, 4096, 1, file);

    changeTable(0);
    tableBuf[3]++;
}

//returns the next block in the chain
uint32_t getNextBlock(uint32_t block) {
    if (block == EOC) {
        return EOC;
    }

    changeTable(block);

    block &= 0x3FF;
    return tableBuf[block];
}

uint32_t getBlockFromMainListById(uint64_t id) {
    uint64_t offset = 0x10 + 0x8 * (id - 1);
    uint64_t len = offset >> 4;

    uint32_t block = 0x4;
    while (len--) {
        block = getNextBlock(block);
    }
    return block;
}

uint32_t findEmptyBlock(void) {
    uint32_t table = 0;
    uint32_t block = 0;

    changeTable(0);
    uint32_t numOfTables = tableBuf[3];

    while (block == 0) {
        if (table == numOfTables) {
            createTable(table);
            numOfTables++;
        }

        changeTable(0x5000 * table++);

        if (tableBuf[1]) {  //if any empty blocks are present in the table
            for (int i = 4; i < 1024; i++) {
                if (tableBuf[i] == 0) {
                    tableBuf[i] = EOC;
                    block = (curTableId << 10) + i;
                    tableBuf[1]--;
                    break;
                }
            }
        }
    }

    return block;
}


//returns NULL if it reached to the end of chain
void *readBlock(uint32_t block) {
    static uint32_t data[4];

    if (block == EOC) {
        return NULL;
    }

    changeTable(block);

    long pos = (long) 0x5000 * curTableId + 0x1000 + (block & 0x3FF) * 0x10;
    fseek(file, pos, SEEK_SET);
    fread(data, sizeof(uint32_t), 4, file);

    return data;
}

size_t writeBlock(uint32_t block, void *data) {
    if (block == EOC) {
        return 0;
    }

    changeTable(block);

    long pos = (long) 0x5000 * curTableId + 0x1000 + (block & 0x3FF) * 0x10;
    fseek(file, pos, SEEK_SET);

    return fwrite(data, 16, 1, file);
}

void deleteBlock(uint32_t block) {
    writeTable(block, 0);
    tableBuf[0x1]++;    //contains number of free blocks in the table

    long pos = (long) 0x5000 * curTableId + 0x4;
    fseek(file, pos, SEEK_SET);
    fwrite(tableBuf + 0x1, sizeof(uint32_t), 1, file);
}

//
char *readString(uint32_t block) {
    char *str, *ptr, *data;

    if (block == 0) {
        return NULL;
    }

    data = (char *) readBlock(block);
    uint32_t len = *(uint32_t *) data;

    str = ptr = (char *) malloc(len + 1);

    for (int i = 0; i < 12 && len; i++) { //the first block contains only 12 chars
        *(ptr++) = data[4 + i];
        len--;
    }

    while (len) { //0 - 1 == UINT32_MAX
        block = getNextBlock(block);
        data = (char *) readBlock(block);

        if (data == NULL) { //if something went wrong
            printf("FATAL ERROR: file is corrupted\n");
            exit(1);
        }

        for (int i = 0; i < 16 && len; i++) {
            *(ptr++) = data[i];
            len--;
        }
    }

    *ptr = '\0';
    return str;
}

uint32_t writeString(char *str) {
    char data[16];
    uint32_t len = (uint32_t) strlen(str);
    uint32_t firstBlock = findEmptyBlock();
    uint32_t lastBlock, block;

    *(uint32_t *) data = len;
    for (int i = 0; i < 12 && len; i++) {
        data[4 + i] = *(str++);
        len--;
    }

    writeBlock(firstBlock, data);

    lastBlock = firstBlock;
    while (len) {
        //writeTable(lastBlock, EOC);    //we must write smth to table before finding the next block
        block = findEmptyBlock();
        writeTable(lastBlock, block);

        for (int i = 0; i < 16 && len; i++) {
            data[i] = *(str++);
            len--;
        }

        writeBlock(block, data);
        lastBlock = block;
    }

    writeTable(lastBlock, EOC);
    return firstBlock;
}

//this destroys the chain of string
void deleteString(uint32_t block) {
    while (block != EOC) {
        uint32_t nextBlock = getNextBlock(block);
        deleteBlock(block);
        block = nextBlock;
    }
}

//returns NULL if contacts are empty
//returns NULL if the list are ended
//returns entry->id == 0, if it is a deleted entry
Entry *getById(uint64_t id) {
    static Entry entry;
    static uint64_t lastId;
    static uint32_t lastBlock;
    uint32_t block, *data;

    if (entry.name) {
        free(entry.name);
        free(entry.number);

        entry.id = 0;
        entry.name = NULL;
        entry.number = NULL;
    }

    if (id == NEXT_ID) {
        id = lastId + 1;

        if (id > sizeOfDirectory) {
            return NULL;
        }

        if (id & 1) {
            block = getNextBlock(lastBlock);
        } else {
            block = lastBlock;
        }
    } else {
        block = getBlockFromMainListById(id);
    }

    data = (uint32_t *) readBlock(block);

    if (data == NULL) {
        puts("FATAL: file is corrupted");
        exit(1);
    }

    //if 'id' is odd, we read by using indexes 0 and 1, if not - we use indexes 2 and 3
    //we need it because we store in one block two contact records
    uint32_t nameBlock = data[0 + (((id ^ 1) & 1) << 1)];
    uint32_t numberBlock = data[1 + (((id ^ 1) & 1) << 1)];

    entry.name = readString(nameBlock);
    entry.number = readString(numberBlock);

    if (entry.name) {
        entry.id = id;
    } else {
        entry.id = 0;
    }

    lastId = id;
    lastBlock = block;
    return &entry;
}

uint64_t findEmptyId(void) {
    if (sizeOfDirectory == entriesUsed) {
        if (sizeOfDirectory & 1) {
            sizeOfDirectory++;
            entriesUsed++;

            return sizeOfDirectory;
        } else {
            uint32_t block = 0x4;
            for (; ;) {
                uint32_t nextBlock = getNextBlock(block);
                if (nextBlock == EOC) {
                    break;
                }
                block = nextBlock;
            }

            uint32_t newBlock = findEmptyBlock();
            writeTable(block, newBlock);

            sizeOfDirectory++;
            entriesUsed++;

            return sizeOfDirectory;
        }
    } else {
        uint64_t id = 0;
        while (getById(++id)->id);

        entriesUsed++;
        return id;
    }
}

void stringToLower(char *str) {
    while (*str) {
        *str = (char) tolower(*str);
        str++;
    }
}

void extractCleanNumber(char *str) {
    char *ptr = str;

    while (*ptr) {
        if (isdigit(*(ptr++))) {
            *(str++) = *(ptr - 1);
        }
    }

    *str = '\0';
}

void findByNumber(const char *number) {
    Entry *entry = getById(1);

    while (entry) {
        if (entry->id) {
            char *str = (char *) malloc(strlen(entry->number) + 1);
            strcpy(str, entry->number);
            extractCleanNumber(str);

            if (!strcmp(str, number)) {
                printf("%ld %s %s\n", entry->id, entry->name, entry->number);
                fflush(stdout);
            }

            free(str);
        }

        entry = getById(NEXT_ID);
    }
}

void findByName(char *name) {
    stringToLower(name);
    Entry *entry = getById(1);

    while (entry) {
        if (entry->id) {
            char *str = (char *) malloc(strlen(entry->name) + 1);
            strcpy(str, entry->name);
            stringToLower(str);

            if (strstr(str, name)) {
                printf("%ld %s %s\n", entry->id, entry->name, entry->number);
                fflush(stdout);
            }

            free(str);
        }

        entry = getById(NEXT_ID);
    }
}

void find(char *str) {
    if (!strcmp(str, "-a")) {
        Entry *entry = getById(1);

        while (entry) {
            if (entry->id)
                printf("%ld %s %s\n", entry->id, entry->name, entry->number);
            entry = getById(NEXT_ID);
        }

        return;
    }


    if (isalpha(*str)) {
        findByName(str);
    } else {
        findByNumber(str);
    }
}

void delete(uint64_t id) {
    uint32_t block = getBlockFromMainListById(id);

    if (block != EOC) {   //if contact is present
        uint32_t *data = (uint32_t *) readBlock(block);
        uint32_t new_data[4];
        memcpy(new_data, data, 16);

        data += id & 1 ? 0 : 2;
        uint32_t nameBlock = data[0];
        uint32_t numberBlock = data[1];

        *(new_data + 0 + (id & 1 ? 0 : 2)) = 0;
        *(new_data + 1 + (id & 1 ? 0 : 2)) = 0;

        /*int ic = (((int) id ^ 1) & 1);
        uint32_t nameBlock = data[ic * 2 + 0];
        uint32_t numberBlock = data[ic * 2 + 1];

        new_data[ic * 2 + 0] = 0;
        new_data[ic * 2 + 1] = 0;
        new_data[!ic * 2 + 0] = data[!ic * 2 + 0];
        new_data[!ic * 2 + 1] = data[!ic * 2 + 1];*/

        if (nameBlock != 0) {
            deleteString(nameBlock);
            deleteString(numberBlock);

            writeBlock(block, new_data);

            entriesUsed--;
        }
    }
}

//parses num string to uint64_t
//
uint64_t ato64(const char *str) {
    uint64_t result = 0;
    while (*str) {
        result *= 10;
        result += (uint64_t) (*(str++) - '0');
    }
    return result;
}

FILE *createFile(const char *name) {
    FILE *fp = fopen(name, "w+b");

    if (fp == NULL) {
        printf("Can't create a file");
        exit(1);
    }

    uint32_t *table = (uint32_t *) calloc(1048, sizeof(uint32_t));
    table[0] = 0x746e6f43;  //Signature "Cont"
    table[1] = 1019;    //Empty blocks in the table
    table[2] = 5;       //Next block to be written
    table[3] = 1;       //Number of tables
    table[4] = 5;       //No comments
    table[5] = EOC;     //End of main list chain
    table[1040] = 1;    //sizeOfDirectory
    fwrite(table, sizeof(uint32_t), 1048, fp);
    free(table);

    return fp;
}

void changeName(uint64_t id, char *name) {
    uint32_t block = getBlockFromMainListById(id);
    uint32_t *data = readBlock(block);

    if (data) {
        uint32_t new_data[4];
        memcpy(new_data, data, 16);

        data += id & 1 ? 0 : 2;
        uint32_t nameBlock = data[0];

        if (nameBlock != 0) {
            deleteString(nameBlock);
            *(new_data + 0 + (id & 1 ? 0 : 2)) = writeString(name);

            writeBlock(block, new_data);
        }
    }
}

void changeNumber(uint64_t id, char *number) {
    uint32_t block = getBlockFromMainListById(id);
    uint32_t *data = readBlock(block);

    if (data) {
        uint32_t new_data[4];
        memcpy(new_data, data, 16);

        data += id & 1 ? 0 : 2;
        uint32_t numberBlock = data[1];

        if (numberBlock != 0) {
            deleteString(numberBlock);
            *(new_data + 1 + (id & 1 ? 0 : 2)) = writeString(number);

            writeBlock(block, new_data);
        }
    }
}

void createEntry(char *name, char *number) {
    uint64_t id = findEmptyId();
    uint32_t nameBlock = writeString(name);
    uint32_t numberBlock = writeString(number);
    uint32_t block = getBlockFromMainListById(id);

    uint32_t *data = readBlock(block);

    *(data + 0 + (id & 1 ? 0 : 2)) = nameBlock;
    *(data + 1 + (id & 1 ? 0 : 2)) = numberBlock;

    writeBlock(block, data);
}

int main(int argc, const char **argv) {
    if (argc < 2)
        return 1;

    file = fopen(argv[1], "r+b");
    if (file == NULL) {
        file = createFile(argv[1]);
    }
    readHeader();
    char *buffer = (char *) malloc((size_t) 0x8000000);
    char *bufferB = (char *) malloc((size_t) 0x8000000);
    for (; ;) {
        scanf("%s", buffer);

        if (!strcmp("exit", buffer)) {
            break;
        } else if (!strcmp("find", buffer)) {
            scanf("%s", buffer);
            find(buffer);
        } else if (!strcmp("change", buffer)) {
            scanf("%s", buffer);
            uint64_t id = ato64(buffer);

            scanf("%s", buffer);
            if (!strcmp("name", buffer)) {
                scanf("%s", buffer);
                changeName(id, buffer);
            } else if (!strcmp("number", buffer)) {
                scanf("%s", buffer);
                changeNumber(id, buffer);
            } else {
                puts("error: Unrecognized command");
            }
        } else if (!strcmp("delete", buffer)) {
            scanf("%s", buffer);
            uint64_t id = ato64(buffer);
            delete(id);
        } else if (!strcmp("create", buffer)) {
            scanf("%s %s", buffer, bufferB);
            createEntry(buffer, bufferB);
        } else {
            puts("error: Command not found");
        }
        fflush(stdout);
    }

    free(buffer);
    writeHeader();
    fclose(file);
}
