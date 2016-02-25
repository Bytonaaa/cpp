#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

//Sorry for my bad english

#ifdef __linux__    //CLion's reformat removes this include on Mac

#include <stdint.h>

#endif

///////////////////////////////////////////////////
#define EOC ((uint32_t) 0xFFFFFFF8)  //End of chain

#define FILE_SIGNATURE 0x746e6f43

#define READ_BUFFER_SIZE ((size_t) 4096)
#define TABLE_SIZE ((size_t) 4096)
#define CHUNK_SIZE ((size_t) 0x5000)

#define SIZE_OF_ROOT_OFFSET 0x1040
#define SIZE_OF_TABLE_ENTRY sizeof(uint32_t)
#define SIZE_OF_CONTACT_ENTRY sizeof(uint32_t)

#define BLOCKS_PER_TABLE 1024
#define BLOCK_BITS 10
#define FREE_BLOCKS_IN_NEW_TABLE 1020
#define FREE_BLOCKS_IN_NEW_FILE 1018
#define MAX_BLOCK 0x3FF

#define SIGNATURE_BLOCK 0x0
#define FREE_BLOCKS_BLOCK 0x1
#define NUMBER_OF_TABLES_BLOCK 0x3
#define ROOT_BLOCK 0x4

#define BLOCK_SIZE 16

#define ERROR_INCORRECT_INPUT "error: Incorrect input"
#define ERROR_NO_COMMAND "error: command not found"
///////////////////////////////////////////////////

#ifndef __x86_64__
#error "Only x86-64 processors are supported"
#endif
#ifdef _WIN32
#error "Windows must die!"
#endif
#if !(defined(__APPLE__) || defined(__linux__))
#warning "It was only tested on Linux and OS X. Use it at own risk!"
#endif

FILE *file;
uint32_t sizeOfRoot;
uint32_t entriesUsed;

uint32_t tableBuf[BLOCKS_PER_TABLE];
uint32_t curTableId;


typedef struct Contact {
    uint64_t id;
    char *name;
    char *number;
} Contact;


void readHeader(void) {
    rewind(file);
    fread(tableBuf, TABLE_SIZE, 1, file);
    curTableId = 0;

    fseek(file, SIZE_OF_ROOT_OFFSET, SEEK_SET);
    fread(&sizeOfRoot, sizeof(uint32_t), 1, file);
    fread(&entriesUsed, sizeof(uint32_t), 1, file);

    //Signature check
    if (tableBuf[SIGNATURE_BLOCK] != FILE_SIGNATURE) {
        printf("Incorrect file\n");
        exit(1);
    }
}

//Save table to file
void flushCurTable(void) {
    fseek(file, (long) curTableId * CHUNK_SIZE, SEEK_SET);
    fwrite(tableBuf, TABLE_SIZE, 1, file);
}

//Before fclose, use this procedure
void writeHeader(void) {
    flushCurTable();

    fseek(file, SIZE_OF_ROOT_OFFSET, SEEK_SET);

    fwrite(&sizeOfRoot, sizeof(uint32_t), 1, file);
    fwrite(&entriesUsed, sizeof(uint32_t), 1, file);
}

//loads another table into buffer if it is necessary
void changeTable(uint32_t block) {
    if (curTableId != (block >> BLOCK_BITS)) {
        flushCurTable();

        curTableId = (block >> BLOCK_BITS);

        long tablePos = (long) curTableId * CHUNK_SIZE;

        fseek(file, tablePos, SEEK_SET);
        fread(tableBuf, TABLE_SIZE, 1, file);
    }
}

//writes to table by number of block
void writeTable(uint32_t block, uint32_t value) {
    changeTable(block);

    tableBuf[block & MAX_BLOCK] = value;
}

//TODO: Possible bug
void createTable(uint32_t table) {
    flushCurTable();
    memset(tableBuf, 0, TABLE_SIZE);

    curTableId = table;

    tableBuf[SIGNATURE_BLOCK] = FILE_SIGNATURE;
    tableBuf[FREE_BLOCKS_BLOCK] = FREE_BLOCKS_IN_NEW_TABLE;

    fseek(file, (long) table * CHUNK_SIZE, SEEK_SET);
    fwrite(tableBuf, TABLE_SIZE, 1, file);

    changeTable(0);
    tableBuf[NUMBER_OF_TABLES_BLOCK]++;
}

//returns the next block in the chain
uint32_t getNextBlock(uint32_t block) {
    if (block == EOC) {
        return EOC;
    }

    changeTable(block);

    return tableBuf[block & MAX_BLOCK];
}

//
uint32_t getBlockFromRootById(uint64_t id) {
    uint64_t offset = 0x10 + 0x8 * (id - 1);
    uint64_t len = offset >> SIZE_OF_CONTACT_ENTRY;

    uint32_t block = ROOT_BLOCK;
    while (len--) {
        block = getNextBlock(block);
    }
    return block;
}

//allocates a free block
uint32_t allocBlock(void) {
    uint32_t table = 0;
    uint32_t block = 0;

    changeTable(0);
    uint32_t numOfTables = tableBuf[NUMBER_OF_TABLES_BLOCK];

    while (block == 0) {
        if (table == numOfTables) {
            createTable(table);
            numOfTables++;
        }

        changeTable(BLOCKS_PER_TABLE * table++);

        if (tableBuf[FREE_BLOCKS_BLOCK]) {  //if any empty blocks are present in the table
            for (int i = 4; i < BLOCKS_PER_TABLE; i++) {
                if (tableBuf[i] == 0) {
                    tableBuf[i] = EOC;
                    block = (curTableId << BLOCK_BITS) + i;
                    tableBuf[FREE_BLOCKS_BLOCK]--;
                    break;
                }
            }
        }
    }

    return block;
}


//returns NULL if it reached to the end of chain
//TODO: Remove static
void *readBlock(uint32_t block) {
    static uint32_t data[BLOCK_SIZE / sizeof(uint32_t)];

    if (block == EOC) {
        return NULL;
    }

    changeTable(block);

    long pos = (long) CHUNK_SIZE * curTableId + TABLE_SIZE + (block & MAX_BLOCK) * BLOCK_SIZE;
    fseek(file, pos, SEEK_SET);
    fread(data, BLOCK_SIZE, 1, file);

    return data;
}

//writes block of data to file
size_t writeBlock(uint32_t block, void *data) {
    if (block == EOC) {
        return 0;
    }

    changeTable(block);

    long pos = (long) CHUNK_SIZE * curTableId + TABLE_SIZE + (block & MAX_BLOCK) * BLOCK_SIZE;
    fseek(file, pos, SEEK_SET);

    return fwrite(data, BLOCK_SIZE, 1, file);
}

//writes NULL to table by number of block
void deleteBlock(uint32_t block) {
    writeTable(block, 0);
    tableBuf[FREE_BLOCKS_BLOCK]++;    //contains number of free blocks in the table
}

//reads string from file by number of block
char *readString(uint32_t block) {
    char *result, *ptr, *data;

    if (block == 0) {
        return NULL;
    }

    data = (char *) readBlock(block);
    uint32_t len = *(uint32_t *) data;

    result = ptr = (char *) malloc(len + 1);

    for (int i = 0; i < 12 && len; i++) { //the first block contains 12 chars only
        *(ptr++) = data[4 + i];
        len--;
    }

    while (len) {
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
    return result;
}

//creates new string in file and returns blockid
uint32_t writeString(char *str) {
    char data[BLOCK_SIZE];
    uint32_t len = (uint32_t) strlen(str);
    uint32_t firstBlock = allocBlock();
    uint32_t lastBlock, block;

    *(uint32_t *) data = len;
    for (int i = 0; i < 12 && len; i++) {   //first block contains 12 chars only
        data[4 + i] = *(str++);
        len--;
    }

    writeBlock(firstBlock, data);

    lastBlock = firstBlock;
    while (len) {
        block = allocBlock();
        writeTable(lastBlock, block);

        for (int i = 0; i < BLOCK_SIZE && len; i++) {
            data[i] = *(str++);
            len--;
        }

        writeBlock(block, data);
        lastBlock = block;
    }

    writeTable(lastBlock, EOC); //na vsyakii sluchai
    return firstBlock;
}

//destroys the chain of string
void deleteString(uint32_t block) {
    while (block != EOC) {
        uint32_t nextBlock = getNextBlock(block);
        deleteBlock(block);
        block = nextBlock;
    }
}

//returns NULL if contacts are empty
//returns NULL if the list are ended
//returns entry->id == 0, if it is a free entry
//TODO: Remove static
Contact *getById(uint64_t id) {
    static Contact contact;
    uint32_t block, *data;

    if (contact.name) {
        free(contact.name);
        free(contact.number);

        contact.id = 0;
        contact.name = NULL;
        contact.number = NULL;
    }

    block = getBlockFromRootById(id);

    if (id > sizeOfRoot) {
        return NULL;
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

    contact.name = readString(nameBlock);
    contact.number = readString(numberBlock);

    if (contact.name) {
        contact.id = id;
    } else {
        contact.id = 0;
    }

    return &contact;
}

//finds id for a new contact
//code is awful but it works
uint64_t findEmptyId(void) {
    if (sizeOfRoot == entriesUsed) {
        if (sizeOfRoot & 1) {
            sizeOfRoot++;
            entriesUsed++;

            return sizeOfRoot;
        } else {
            uint32_t block = 0x4;
            for (; ;) {
                uint32_t nextBlock = getNextBlock(block);
                if (nextBlock == EOC) {
                    break;
                }
                block = nextBlock;
            }

            uint32_t newBlock = allocBlock();
            writeTable(block, newBlock);

            sizeOfRoot++;
            entriesUsed++;

            return sizeOfRoot;
        }
    } else {
        uint64_t id = 0;
        while (getById(++id)->id);

        entriesUsed++;
        return id;
    }
}

//removes special symbols from number string
char *getTrueNumber(char *str) {
    char *newStr = (char *) malloc(strlen(str) + 1);

    int j = 0;
    for (int i = 0; str[i]; i++) {
        if (isdigit(str[i])) {
            newStr[j++] = str[i];
        }
    }

    newStr[j] = '\0';
    return newStr;
}

//
void findByNumber(const char *number) {
    uint64_t id = 1;

    Contact *contact = getById(id);
    while (contact) {
        if (contact->id) {
            char *str = getTrueNumber(contact->number);

            if (!strcmp(str, number)) {
                printf("%ld %s %s\n", contact->id, contact->name, contact->number);
            }

            free(str);
        }

        contact = getById(++id);
    }
}

//
char *stringToLower(char *str) {
    char *newStr = (char *) malloc(strlen(str) + 1);

    int i = 0;
    for (; str[i]; i++) {
        newStr[i] = (char) tolower(str[i]);
    }

    newStr[i] = '\0';
    return newStr;
}

//
void findByName(char *name) {
    name = stringToLower(name);
    uint64_t id = 1;

    Contact *contact = getById(id);
    while (contact) {
        if (contact->id) {
            char *lower = stringToLower(contact->name);

            if (strstr(lower, name)) {
                printf("%ld %s %s\n", contact->id, contact->name, contact->number);
            }

            free(lower);
        }

        contact = getById(++id);
    }
    free(name);
}

//
void find(char *str) {
    if (isalpha(*str)) {
        findByName(str);
    } else {
        findByNumber(str);
    }
}

//removes contact by id
void delete(uint64_t id) {
    uint32_t block = getBlockFromRootById(id);

    if (block != EOC) {   //if contact is present
        uint32_t *data = (uint32_t *) readBlock(block);
        uint32_t newData[BLOCK_SIZE / sizeof(uint32_t)];
        memcpy(newData, data, BLOCK_SIZE);

        //i dunno how to make this simpler
        //all explanations are given in getById() function
        data += id & 1 ? 0 : 2;
        uint32_t nameBlock = data[0];
        uint32_t numberBlock = data[1];

        *(newData + 0 + (id & 1 ? 0 : 2)) = 0;
        *(newData + 1 + (id & 1 ? 0 : 2)) = 0;

        if (nameBlock != 0) {
            deleteString(nameBlock);
            deleteString(numberBlock);

            writeBlock(block, newData);

            entriesUsed--;
        }
    }
}

//parses num string to uint64_t
//c99 do not support atoll()
uint64_t ato64(const char *str) {
    uint64_t result = 0;
    while (*str) {
        result *= 10;
        result += (uint64_t) (*(str++) - '0');
    }
    return result;
}

//if file does not exist, creates a new file
FILE *createFile(const char *name) {
    FILE *fp = fopen(name, "w+b");

    if (fp == NULL) {
        printf("Can't create a file");
        exit(1);
    }

    uint32_t *table = (uint32_t *) calloc(TABLE_SIZE, 1);
    table[SIGNATURE_BLOCK] = FILE_SIGNATURE;
    table[FREE_BLOCKS_BLOCK] = FREE_BLOCKS_IN_NEW_FILE;
    table[NUMBER_OF_TABLES_BLOCK] = 1;
    table[ROOT_BLOCK] = 5;          //Next block of root list
    table[5] = EOC;                 //End of main list chain
    fwrite(table, TABLE_SIZE, 1, fp);
    free(table);

    sizeOfRoot = 1;
    entriesUsed = 0;

    fseek(fp, SIZE_OF_ROOT_OFFSET, SEEK_SET);
    fwrite(&sizeOfRoot, sizeof(sizeOfRoot), 1, fp);
    fwrite(&entriesUsed, sizeof(entriesUsed), 1, fp);

    return fp;
}

//changes name by id
void changeName(uint64_t id, char *name) {
    uint32_t block = getBlockFromRootById(id);
    uint32_t *data = readBlock(block);

    if (data) {
        uint32_t new_data[BLOCK_SIZE / sizeof(uint32_t)];
        memcpy(new_data, data, BLOCK_SIZE);

        data += id & 1 ? 0 : 2;
        uint32_t nameBlock = data[0];

        if (nameBlock != 0) {
            deleteString(nameBlock);
            *(new_data + 0 + (id & 1 ? 0 : 2)) = writeString(name);

            writeBlock(block, new_data);
        }
    }
}

//changes number by id
void changeNumber(uint64_t id, char *number) {
    uint32_t block = getBlockFromRootById(id);
    uint32_t *data = readBlock(block);

    if (data) {
        uint32_t new_data[BLOCK_SIZE / sizeof(uint32_t)];
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

//creates a new contact
void createContact(char *name, char *number) {
    uint64_t id = findEmptyId();
    uint32_t nameBlock = writeString(name);
    uint32_t numberBlock = writeString(number);
    uint32_t block = getBlockFromRootById(id);

    uint32_t *data = readBlock(block);

    *(data + 0 + (id & 1 ? 0 : 2)) = nameBlock;
    *(data + 1 + (id & 1 ? 0 : 2)) = numberBlock;

    writeBlock(block, data);
    //printf("ok\n");
}

//gets a string from stdin
//TODO:
char *readCommand(void) {
    char *cmd = NULL;
    uint32_t len = 0;

    do {
        len += READ_BUFFER_SIZE;
        cmd = realloc(cmd, len);
        fgets(cmd, READ_BUFFER_SIZE, stdin);
    } while (feof(stdin));

    return cmd;
}

int isNumber(char *str) {
    while (*str) {
        if (!isdigit(*(str++))) {
            return false;
        }
    }
    return true;
}

//TODO: Reach scanf limit of 4K characters;
#define DELIM " "

int main(int argc, const char **argv) {
    if (argc < 2)
        return 1;

    file = fopen(argv[1], "r+b");
    if (file == NULL) {
        file = createFile(argv[1]);
    }
    readHeader();

    for (; ;) {
        char *string = readCommand();

        char *cmd = strtok(string, DELIM);
        if (cmd == NULL) {
            free(string);
            continue;
        }

        if (!strcmp(cmd, "create")) {
            char *name = strtok(NULL, DELIM);
            char *number = strtok(NULL, DELIM);

            createContact(name, number);
        } else if (!strcmp(cmd, "find")) {
            char *str = strtok(NULL, DELIM);

            find(str);
        } else if (!strcmp(cmd, "change")) {
            char *idStr = strtok(NULL, DELIM);

            if (isNumber(idStr)) {
                uint64_t id = ato64(idStr);
                char *subcmd = strtok(NULL, DELIM);
                char *str = strtok(NULL, DELIM);

                if (!strcmp(subcmd, "name")) {
                    changeName(id, str);
                } else if (!strcmp(subcmd, "number")) {
                    changeNumber(id, str);
                } else {
                    puts(ERROR_INCORRECT_INPUT);
                }
            } else {
                puts(ERROR_INCORRECT_INPUT);
            }
        } else if (!strcmp(cmd, "delete")) {
            char *idStr = strtok(NULL, DELIM);

            if (isNumber(idStr)) {
                uint64_t id = ato64(idStr);

                delete(id);
            } else {
                puts(ERROR_INCORRECT_INPUT);
            }
        } else if (!strcmp(cmd, "exit")) {
            free(string);
            break;
        } else {
            puts("error: command not found");
        }

        fflush(stdout);
        free(string);
    }

    writeHeader();
    fclose(file);
}
