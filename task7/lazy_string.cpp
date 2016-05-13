#include <iostream>
#include "lazy_string.h"

using namespace std;

lazy_string::operator std::string() {
    return ref->substr(start, sz);
};

lazy_string::lazy_string(const std::string &str) {
    ref = std::make_shared<std::string>(str);
    start = 0;
    sz = str.size();
}

lazy_string::lazy_string() {
    start = 0;
    sz = 0;
}

lazy_string::lazy_string(const lazy_string &ls) {
    *this = ls;
}

size_t lazy_string::size() const {
    return sz;
}

size_t lazy_string::length() const {
    return sz;
}


lazy_string lazy_string::substr(size_t pos, size_t len) {
    mtx.lock();

    if (pos > sz)
        throw std::out_of_range("lazy_string");

    lazy_string result;

    result.ref = ref;
    result.start = start + pos;
    result.sz = pos + len > sz ? (sz - pos) : len;

    mtx.unlock();
    return result;
}

lazy_string::char_ref lazy_string::at(size_t i) {
    if (i >= sz)
        throw std::out_of_range("lazy_string");
    return char_ref(this, i);
}

lazy_string::char_ref lazy_string::operator[](size_t i) {
    return char_ref(this, i);
}

std::istream &operator>>(std::istream &is, lazy_string &ls) {
    ls.mtx.lock();

    auto ref = std::make_shared<std::string>();
    is >> *ref;

    ls.ref = ref;

    ls.start = 0;
    ls.sz = ref->size();

    ls.mtx.unlock();
    return is;
}

std::ostream &operator<<(std::ostream &os, lazy_string &ls) {
    for (int i = 0; i < ls.sz; i++)
        os << (*ls.ref)[ls.start + i]; //os << ls[i];
    return os;
}


lazy_string::char_ref::char_ref(lazy_string *ls, size_t index) : ls(ls), index(index) { }

lazy_string::char_ref &lazy_string::char_ref::operator=(char c) {
    ls->mtx.lock();
    if (ls->ref.use_count() > 1) {
        ls->ref = std::make_shared<std::string>(ls->ref->substr(ls->start, ls->sz));
        ls->start = 0;
        //ls->sz = ls->ref->size();
    }
    (*ls->ref)[ls->start + index] = c;
    ls->mtx.unlock();

    return *this;
}

lazy_string::char_ref::operator char() const {
    return (*ls->ref)[ls->start + index];
}

lazy_string &lazy_string::operator=(const lazy_string &ls) {
    if (this == &ls)
        return *this;

    lazy_string &kostyl = const_cast<lazy_string&>(ls);

    mtx.lock();
    kostyl.mtx.lock();

    ref = ls.ref;
    start = ls.start;
    sz = ls.sz;

    mtx.unlock();
    kostyl.mtx.unlock();

    return *this;
}