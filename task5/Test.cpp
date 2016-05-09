#include <iostream>
#include <forward_list>
#include "format.h"

using namespace std;

int main() {
    std::string s ("%d subject %3f has %%%0+3.100500lli%d a submarine as a subsequence");

    cout << format("%f azaz", (float) 1.0f) << endl;

    cout << "///////////////////////////" << endl;

    std::regex_iterator<std::string::const_iterator> rit ( s.begin(), s.end(), format_impl::regex );
    std::regex_iterator<std::string::const_iterator> rend;

    while (rit != rend) {
        for (auto x: *rit)
            std::cout << "/" << x;
        ++rit;
        cout << endl;
    }
    cout << rit->str() << endl;
//    int b[10], *a, si;
//    //std::cout << typeid(b).name() << " " << typeid(a).name() << " " << typeid(si).name() << endl;
//    std::cout << format("%@", nullptr);
//    std::cout << format("%@ %@ %*d\n", 0.0, (void*)34, 45, 4);
//
//    printf("%+a, %a, %p\n", 0., 2e-320, (void*)34);
//    std::cout << format("%a\n", 2e-323);
//    string s = format("%@\n", b);
//    std::cout << s << endl;
//    std::cout << format("%@\n", "string");
}