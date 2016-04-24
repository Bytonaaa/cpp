#include <iostream>
#include "lazy_string.h"

using namespace std;

class test1 {
public:
    int a;
    test1(int a) : a(a) {
        cout << "test1: constructed " << a << endl;
    };

    ~test1() {
        cout << "test1: destructed " << a << endl;
    }

    test1 &operator=(const test1 &that) {
        this->a = that.a;
        return *this;
    }

    void *operator new(size_t sz) {
        cout << "test1: allocated" << endl;
        return malloc(sizeof(test1) * sz);
    }

    void operator delete(void *m) {
        cout << "test1: deleted" << endl;
        free(m);
    }

    void operator--(int) {
        a--;
        if (a == 0)
            delete this;
    }
};

template<typename T>
class pointer {

};

int main() {
    /*string str;
    lazy_string s("lazy_test"), ss;
    ss = s;
    cout << s << " " << ss << endl;
    s[0] = 'e';
    putchar(s[1]);
    cout << s << " " << ss << endl;
    s.at(2) = 's';
    cout << s << " " << ss << endl;*/
    auto s = lazy_string("getget");
    auto ss = lazy_string("get");
    s = ss;
    cout << "......" << endl;
}
