#include <iostream>
#include <iomanip>
#include "format.h"

using namespace std;

/*namespace std {
    class String : public string {
    public:
        ~String() {
            std::string::~basic_string();
            cout << "String deleted" << endl;
        }
    };
}*/

int main() {
    printf("%+x\n", 5);
    std::cout << format("%+025d, %u\n", -2345556, (unsigned)5);
    /*printf("%.15o, %.a, %p, %+llu\n", -1, 1.12345678901234, (int*)0x123456789ABCDEF, ULLONG_MAX);
    std::cout << format("d: %.15o, %#.8x, %X, %.g %lu\n", (unsigned)-1, (unsigned)3, UINT_MAX, 13.12345678901234, ULONG_MAX);
    cout << std::hexfloat;
    cout.precision(1);
    cout << std::setprecision(1) << 13.12345678901234 << endl;*/
}