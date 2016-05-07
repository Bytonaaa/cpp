#include <iostream>
#include <forward_list>
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

template <typename T>
void foo(T arg) {
    cout << is_array<T>::value;
}

int main() {
    int b[8], *a, si;
    std::cout << typeid(b).name() << " " << typeid(a).name() << " " << typeid(si).name() << endl;
    std::cout << format("%@ %@ %*d\n", 0.0, (void*)34, 45, 4);
    printf("%+a, %a, %p\n", 0., 2e-320, (void*)34);
    std::cout << format("%a\n", 2e-323);
    std::cout << format<int[8]>("%@\n", b);
    std::vector<int> ta;
    cout << std::is_array<int[3]>::value << endl;
    foo(b);
}