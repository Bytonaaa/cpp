#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <stdexcept>
#include <vector>

#define WP_READ (-2)

enum FormatSpec {
    def,
    d,
    u,
    o,
    x,
    X,
    f,
    F,
    e,
    E,
    g,
    G,
    a,
    A,
    c,
    s,
    p,
    n, //???

    hh,
    h,
    l,
    ll,
    j,
    z,
    t,
    L,
};

struct Format {
    std::string str;
    union {
        int flags;
        struct {
            bool zero:1;
            bool plus:1;
            bool minus:1;
            bool sharp:1;
        };
    };
    int width = 0;
    int precision = -1;
    FormatSpec length;
    FormatSpec spec;
};

template<typename T>
std::string sprint(Format const *fmt, T arg) {
    throw std::invalid_argument("Invalid argument, or this feature is not implemented.");
}

/*template<typename T>
void gen(Format *fmt, unsigned long size, std::string &str, T arg) {
    if (size) {
        if (fmt->str.size()) {
            str += fmt->str;
            gen(fmt + 1, size - 1, str, arg);
        } else {
            if (fmt->width == WP_READ || fmt->precision == WP_READ)
                throw std::out_of_range("<null>");
            else
                str += sprint(fmt, arg);

            gen(fmt + 1, size - 1, str);
        }
    }
}*/

template<typename T>
int checkForInt(T arg) {
    throw std::invalid_argument("Invalid argument: int or unsigned int expected");
}

void gen(Format *fmt, unsigned long size, std::string &str);
std::string commonFormatter(Format const *fmt, std::string str);

template<typename T, typename... Args>
void gen(Format *fmt, unsigned long size, std::string &str, T arg, Args... args) {
    if (size) {
        if (fmt->str.size()) {
            str += fmt->str;
            gen(fmt + 1, size - 1, str, arg, args...);
        } else {
            if (fmt->width == WP_READ) {
                fmt->width = checkForInt(arg);
            } else if (fmt->precision == WP_READ) {
                fmt->precision = checkForInt(arg);
            } else {
                str += commonFormatter(fmt, sprint(fmt, arg));
                fmt++;
                size--;
            }
            gen(fmt, size, str, args...);
        }
    }
};

//std::string format(std::string const &format);
void parse(std::vector<Format> &fmt, const char *format);

template<typename... Args>
std::string format(std::string const &fmtstr, Args... args) {
    std::string str;
    std::vector<Format> fmt;

    parse(fmt, fmtstr.c_str());

    gen(fmt.data(), fmt.size(), str, args...);

    return str;
}

#endif //TASK4_FORMAT_H
