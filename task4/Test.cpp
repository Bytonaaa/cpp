#include <iostream>
#include "format.h"
#include <vector>

using namespace std;

template <class... Args>
void test(Args... args) {
    printf("%d\n", sizeof...(args));
}

int main() {
    printf("%+010d 45345\n", 12);
    std::cout << format("s: %*.*d d: %d g: %d, float: %f", 111, 222.34, 333, 3434, 43534, 435345.34);
}