#ifndef TASK6_LAZY_STRING_H
#define TASK6_LAZY_STRING_H

#include <string>
#include <istream>
#include <ostream>
#include <mutex>

class lazy_string {
public:
    struct custom_char {
        friend class lazy_string;

        operator char();
        custom_char &operator=(char);

    private:
        custom_char(char, lazy_string *, size_t);

        char c;
        size_t index;
        lazy_string *ls;
    };

    operator std::string();
    lazy_string();
    lazy_string(const std::string &str);
    ~lazy_string();
    size_t size() const;
    size_t length() const;
    lazy_string substr(size_t pos = 0, size_t len = std::string::npos);
    custom_char at(size_t);
    custom_char operator[](size_t);
    lazy_string &operator=(const lazy_string &str);

    friend std::istream &operator>>(std::istream &is, lazy_string &ls);
    friend std::ostream &operator<<(std::ostream &os, lazy_string &ls);

private:
    size_t start, sz;
    struct l_str {
        std::string str;
        size_t count;

        std::mutex mtx;

        l_str(const std::string &str);
        l_str();

        void operator++(int);
        void operator--(int);
    } *ref;
    lazy_string(size_t start, size_t sz, l_str *ref);
};

#endif //TASK6_LAZY_STRING_H
