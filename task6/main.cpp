#include <iostream>
#include "lazy_string.h"

using namespace std;

int main() {
    string str;
    lazy_string s("lazy_test"), ss;
    ss = s;
    cout << s << " " << ss << endl;
    s[0] = 'e';
    putchar(s[1]);
    cout << s << " " << ss << endl;
    s.at(2) = 's';
    cout << s << " " << ss << endl;
}
