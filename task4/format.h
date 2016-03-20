#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <stdexcept>
#include <vector>
#include <cstdlib>

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
    int flags;
    int width = -1;
    int precision = -1;
    FormatSpec length;
    FormatSpec spec;
};

void parse(std::vector<Format> &fmt, const char *format);

/*template<typename T>
std::string sprint(Format const *fmt, T arg) {
    throw std::invalid_argument("Not implemented stuff");
}*/

template<typename T>
std::string sprint(Format const *fmt, T arg) {
    throw std::invalid_argument("Invalid argument, or this feature is not implemented.");
}

void gen(Format *fmt, unsigned long size, std::string &str);

template<typename T>
void gen(Format *fmt, unsigned long size, std::string &str, T arg) {
    if (size) {
        if (fmt->str.size()) {
            str += fmt->str;
            gen(fmt + 1, size - 1, str, arg);
        } else {
            if (fmt->width == WP_READ || fmt->precision == WP_READ)
                throw std::out_of_range("<null>");//TODO
            else
                str += sprint(fmt, arg);

            gen(fmt + 1, size - 1, str);
        }
    }
}

template<typename T, typename... Args>
void gen(Format *fmt, unsigned long size, std::string &str, T arg, Args... args) {
    if (size) {
        if (fmt->str.size()) {
            str += fmt->str;
            gen(fmt + 1, size - 1, str, arg, args...);
        } else {
            if (fmt->width == WP_READ) {
                fmt->width = arg;   //зачем кидать исключения, если и так компиль заорёт об ошибке?
            } else if (fmt->precision == WP_READ) {
                fmt->precision = arg;
            } else {
                str += sprint(fmt, arg);
                fmt++;
                size--;
            }
            gen(fmt, size, str, args...);
        }
    }
};

std::string format(std::string const &format);

template<typename... Args>
std::string format(std::string const &format, Args... args) {
    std::string str;
    std::vector<Format> fmt;

    parse(fmt, format.c_str());
    gen(fmt.data(), fmt.size(), str, args...);

    return str;
}

#endif //TASK4_FORMAT_H
