#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <stdexcept>
#include <vector>

#define DEFAULT_PRECISION (-1)  //default value
#define WP_READ (-2)    // useful if we want to read a width or a precision value from the arguments list

enum FormatSpec {
    def,
    automatic,

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
/*
 * There are two types of tokens:
 *      1) string (str.size() > 0)
 *      2) format specifier (str.size() == 0)
 */
struct Format {
    std::string str;
    union {
        int flags = 0;
        struct {
            bool zero:1;    //'0'
            bool plus:1;    //'+'
            bool minus:1;   //'-'
            bool sharp:1;   //'#'
            bool space:1;   //' '
        };
    };
    int width = 0;
    int precision = DEFAULT_PRECISION;
    FormatSpec length;
    FormatSpec type;
};

/*
 * sprint() generates the string with the given format specifier and the given argument
 * If the argument does not match its specifier, it throws an exception
 */
template<typename T>
std::string sprint(Format const *fmt, T arg) {
    throw std::invalid_argument("Invalid argument, or this feature is not implemented.");
}

/*
 * A simple checker for integer value
 * If the argument is not int, size_t or unsigned int this throws an exception below
 *
 * Otherwise returns the argument casted to int (template's specifications are in format.cpp)
 */
template<typename T>
int checkForInt(T arg) {
    throw std::invalid_argument("Invalid format: int, unsigned int or size_t were expected");
}


/*
 * Generates the string with given the format string
 * It works like sprintf()
 * Format specifiers match the format specifiers in sprintf()
 * Full (or not) a format string specification is given on http://cplusplus.com/printf
 *
 * Throws exceptions if:
 *      1) the argument does not match its format specifier
 *      2) too few arguments were given
 *      3) too many arguments were given (it helps to find some mistakes in format string)
 */

//zero argument function
void formatImplementation(Format *fmt, unsigned long size, std::string &str);

template<typename T, typename... Args>
void formatImplementation(Format *format, unsigned long tokensLeft, std::string &str, T curArgument, Args... args) {
    if (tokensLeft) {
        if (format->str.size()) {
            str += format->str;
            formatImplementation(format + 1, tokensLeft - 1, str, curArgument, args...);
        } else {
            if (format->width == WP_READ) {
                format->width = checkForInt(curArgument);
            } else if (format->precision == WP_READ) {
                format->precision = checkForInt(curArgument);
            } else {
                str += sprint(format, curArgument);
                format++;
                tokensLeft--;
            }
            formatImplementation(format, tokensLeft, str, args...);
        }
    } else if (sizeof...(args)) {
        throw std::invalid_argument("Too many arguments are given");
    }
};

void parseFormatString(std::vector<Format> &fmt, const char *format);

template<typename... Args>
std::string format(std::string const &formatString, Args... args) {
    std::string str;
    std::vector<Format> fmt;

    parseFormatString(fmt, formatString.c_str()); //generates tokens from format string

    formatImplementation(fmt.data(), fmt.size(), str, args...);

    return str;
}

#endif //TASK4_FORMAT_H
