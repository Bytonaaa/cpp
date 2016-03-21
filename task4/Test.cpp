#include <iostream>
#include "format.h"

using namespace std;

template <class... Args>
void test(Args... args) {
    printf("%d\n", sizeof...(args));
}

int main() {
    //printf("%-#+010d %0+-10d\n", 12, 12);
    std::cout << format("s: %*.*u d: %d g: %d, float: %f, char: %c, char string: %s, std string: %s", (unsigned)111, 222, (unsigned)UINT32_MAX, 3434, 43534, 435345.34, '#', "Suka nah", std::string("+100500"));
    char *test = (char*)"sdfsdfds";
    std::cout << format("%s", nullptr);
}