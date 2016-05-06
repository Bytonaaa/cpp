#ifndef TASK6_LAZY_STRING_H
#define TASK6_LAZY_STRING_H

#include <string>
#include <istream>
#include <ostream>
#include <memory>

class lazy_string {
public:
    struct ls_char {
        friend class lazy_string;
        
        operator char() const;
        ls_char &operator=(char);

    private:
        ls_char(lazy_string *, size_t);

        const size_t index;
        lazy_string *const ls;
    };

    operator std::string();
    lazy_string();
    lazy_string(const std::string &str);
    size_t size() const;
    size_t length() const;
    lazy_string substr(size_t pos = 0, size_t len = std::string::npos) const;
    ls_char at(size_t);
    ls_char operator[](size_t);
    lazy_string &operator=(const lazy_string &str);

    friend std::istream &operator>>(std::istream &is, lazy_string &ls);
    friend std::ostream &operator<<(std::ostream &os, lazy_string &ls);

private:
    size_t start, sz;
    std::shared_ptr<std::string> ref;
    lazy_string(size_t start, size_t sz, std::shared_ptr<std::string> ref);
};

#endif //TASK6_LAZY_STRING_H
