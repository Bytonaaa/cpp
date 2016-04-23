#include <iostream>
#include "lazy_string.h"

using namespace std;

lazy_string::operator std::string() {
    return ref->str.substr(start, sz);
};


lazy_string::lazy_string(const std::string &str) {
    ref = new l_str(str);
    start = 0;
    sz = str.size();
}

lazy_string::lazy_string() {
    lazy_string("");
}

lazy_string::lazy_string(size_t start, size_t sz, l_str *ref) {
    this->start = start;
    this->sz = sz;
    this->ref = ref;
    ref[0]++;
}

lazy_string::~lazy_string() {
    ref[0]--;
}


size_t lazy_string::size() const {
    return sz;
}

size_t lazy_string::length() const {
    return sz;
}


lazy_string lazy_string::substr(size_t pos, size_t len) {
    if (pos >= sz)
        throw std::out_of_range("lazy_string");
    return lazy_string(start + pos, pos + len > sz ? (sz - pos) : len, ref);
}

lazy_string::custom_char lazy_string::at(size_t i) {
    if (i >= sz)
        throw std::out_of_range("lazy_string");
    return custom_char(ref->str.at(start + i), this, i);
}

lazy_string::custom_char lazy_string::operator[](size_t i) {
    return custom_char(ref->str[start + i], this, i);
}

lazy_string &lazy_string::operator=(const lazy_string &str) {
    this->start = str.start;
    this->sz = str.sz;
    this->ref = str.ref;
    this->ref[0]++;

    return *this;
}


std::istream &operator>>(std::istream &is, lazy_string &ls) {
    auto ref = new lazy_string::l_str();
    is >> ref->str;

    ls.ref[0]--;
    ls.ref = ref;

    ls.start = 0;
    ls.sz = ref->str.size();

    return is;
}

std::ostream &operator<<(std::ostream &os, lazy_string &ls) {
    for (int i = 0; i < ls.sz; i++)
        os << ls.ref->str[ls.start + i];//os << ls[i];
    return os;
}


lazy_string::l_str::l_str(const std::string &str) {
    this->str = str;
    this->count = 1;
}

lazy_string::l_str::l_str() {
    count = 1;
}


void lazy_string::l_str::operator++(int) {
    mtx.lock();
    count++;
    mtx.unlock();
}

void lazy_string::l_str::operator--(int) {
    mtx.lock();
    count--;
    if (count == 0) {
        delete this;
    }
    mtx.unlock();//TODO
}


lazy_string::custom_char::custom_char(char c, lazy_string *ls, size_t index) {
    this->c = c;
    this->ls = ls;
    this->index = index;
}

lazy_string::custom_char &lazy_string::custom_char::operator=(char c) {
    if (ls->ref->count > 1) {
        auto nref = new l_str(ls->ref->str.substr(ls->start, ls->sz));
        ls->ref[0]--;
        ls->ref = nref;
        ls->start = 0;
        //ls->sz = ls->ref->str.size()
    }
    ls->ref->str[ls->start + index] = c;
    return *this;
}

lazy_string::custom_char::operator char() {
    return c;
}

