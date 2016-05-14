#include <iostream>
#include <memory>
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

void *operator new(size_t sz) {
    void *p = malloc(sz);
    cout << "alloc " << p << " " << sz << endl;
    return p;
}

void operator delete(void *p) {
    cout << "dealloc " << p << endl;
    free(p);
}

template<typename T>
class pointer {

};

void inspect(char *p, size_t sz) {
    cout << p << endl;
    for (int i = 0; i < sz; i++) {
        if (i % 16 == 0)
            cout << endl;
        printf("%.2X ", p[i]);
    }
}

int main() {
    //std::shared_ptr<std::string> sss = std::make_shared<std::string>("test-stirng");
    //cout << *sss << " " << sss << endl;
    /*string str;
    lazy_string s("lazy_test"), ss;
    ss = s;
    cout << s << " " << ss << endl;
    s[0] = 'e';
    putchar(s[1]);
    cout << s << " " << ss << endl;
    s.at(2) = 's';
    cout << s << " " << ss << endl;*/
    cout << "......" << endl;
    auto s = lazy_string("getget");
    auto ss = s.substr(2, 3);
    string sss = "string";
    cout << s << endl << ss << endl << sss;
    s[2] = 'd';
    cout << s << endl << ss << endl;
    const lazy_string hello("hello");
    cout << hello[0] << endl;
}
