#include <iostream>
#include "format.h"

using namespace std;

int main() {
    printf("%0s %#.8d\n", "", 1);
    std::cout << format("s: %-567894.*u d: %d g: %d, float: %f, char: %c, char string: %s", 222, (unsigned)UINT32_MAX, 3434, 43534, 435345.34, '#', "Suka nah");
    char *test = (char*)"sdfsdfds";
    //std::cout << format("%s", nullptr);
}