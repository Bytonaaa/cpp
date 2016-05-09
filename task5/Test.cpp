#include <iostream>
#include <forward_list>
#include "format.h"

using namespace std;

int main() {
    //std::string s ("%d subject %3f has %%%0+3.100500lli%d a submarine as a subsequence");

    cout << format("%d", 1) << format_impl::demangle(typeid(std::string).name()) << endl;

    cout << "///////////////////////////" << endl;
    int b[10], si = 2, *a = &si;
    //std::cout << typeid(b).name() << " " << typeid(a).name() << " " << typeid(si).name() << endl;
    std::cout << format("%@", nullptr) << endl;
    std::cout << format("%@\n", a) << endl;

    printf("%+a, %a, %p\n", 0., 2e-320, (void*)34);
    std::cout << format("%a\n", 2e-323);
    string s = format("%@\n", b);
    std::cout << s << endl;
    std::cout << format("%@\n", "string");
}