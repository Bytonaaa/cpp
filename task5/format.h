#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <stdexcept>
#include <vector>
#include <typeinfo>
#include <iostream>
#include <cxxabi.h>
#include <regex>

#define DEFAULT_PRECISION (-1)  //default value
#define WP_READ (-2)    // useful if we want to read a width or a precision value from the arguments list\

#define RIT_STRING 0
#define RIT_FLAGS 1
#define RIT_WIDTH 2
#define RIT_DOT 3
#define RIT_PRECISION 4
#define RIT_LENGTH 5
#define RIT_SPECIFIER 6

template<typename... Args>
std::string format(const std::string &formatString, const Args &... args);

namespace format_impl {
    extern std::regex regex;
    extern std::regex_iterator<std::string::const_iterator> rend;

    enum FormatSpec {
        def = '\0',
        automatic = '@',

        d = 'd',
        i = 'i',
        u = 'u',
        o = 'o',
        x = 'x',
        X = 'X',
        f = 'f',
        F = 'F',
        e = 'e',
        E = 'E',
        g = 'g',
        G = 'G',
        a = 'a',
        A = 'A',
        c = 'c',
        s = 's',
        p = 'p',
        n = 'n', //???

        hh = '0',
        h = 'h',
        l = 'l',
        ll = '9',
        j = 'j',
        z = 'z',
        t = 't',
        L = 'L',
    };
/*
 * There are two types of tokens:
 *      1) string (str.size() > 0)
 *      2) format specifier (str.size() == 0)
 */
    struct Format {
        std::string fmt;
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

    struct state {
        int state = 0;
        Format format;

        struct state &clear() {
            state = 0;
            format = Format();
            return *this;
        }
    };

    std::string sprintPointer(Format const *fmt, void *arg);

    const char *demangle(const char *mangledName);

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value, std::string>::type sprint(Format const *fmt, T arg) {
        return sprintPointer(fmt, arg);
    }

    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value, std::string>::type sprint(Format const *fmt, T arg) {
        throw std::invalid_argument("Invalid argument or this type of argument is not supported yet");
    }

    template <typename T>
    bool check_type(FormatSpec len, FormatSpec type, T &) {
        if (type == p)
            return true;

        throw std::invalid_argument("Invalid argument type");
    }

    template <typename T>
    std::string print_arg(Format &fmt, T &arg) {
        if (check_type(fmt.length, fmt.type, arg)) {
            return sprint(&fmt, arg);
        } else
            throw std::invalid_argument("Invalid format");
    }

    template<typename T>
    int read_int(T &arg, typename std::enable_if<std::is_integral<T>::value>::type* = 0) {
        return (int) arg;
    }

    template<typename T>
    int read_int(T &arg, typename std::enable_if<!std::is_integral<T>::value>::type* = 0) {
        throw std::invalid_argument("Invalid argument, expected integral type");
    }

    void read_flags(Format &fmt, const std::string &str);

    template<typename T>
    bool read_format(state &s, std::regex_iterator<std::string::const_iterator> &rit, std::string &str, T &arg) {
        auto &match = *rit;

        if (match[RIT_SPECIFIER] == "%") {
            str.append("%");
            return false;
        }

        if (s.state != 0) {
            if (s.state == RIT_WIDTH)
                goto read_precision;
            if (s.state == RIT_PRECISION)
                goto read_length;
        }

        s.state++;
        if (match[RIT_FLAGS].str().size())
            read_flags(s.format, match[RIT_FLAGS].str());

        s.state++;
        if (match[RIT_WIDTH].str().size()) {
            if (match[RIT_WIDTH] == "*") {
                s.format.width = read_int(arg);
                return true;
            } else
                s.format.width = atoi(match[RIT_WIDTH].str().c_str());
        }

        s.state++;
        read_precision:
        if (match[RIT_PRECISION].str().size()) {
            if (match[RIT_PRECISION] == "*") {
                s.format.precision = read_int(arg);
                return true;
            } else
                s.format.precision = atoi(match[RIT_PRECISION].str().c_str());
        }
        else if (match[RIT_DOT].str().size())
            s.format.precision = 0;
        s.state++;

        read_length:
        s.format.length = (FormatSpec) match[RIT_LENGTH].str()[0];
        s.format.type = (FormatSpec) match[RIT_SPECIFIER].str()[0];

        if (match[RIT_LENGTH].str() == "ll")
            s.format.length = ll;
        if (match[RIT_LENGTH].str() == "hh")
            s.format.length = hh;

        s.format.fmt = match.str();

        str += print_arg(s.format, arg);

        return false;
    };

    static void format_implementation(state &s, std::regex_iterator<std::string::const_iterator> &rit,
                                      std::string &str) {
        if (rit == rend)
            return;

        if ((*rit)[RIT_STRING] == "%")
            throw std::invalid_argument("Invalid format");

        if ((*rit)[RIT_SPECIFIER] == "") {
            str.append((*rit)[RIT_STRING]);
        } else if ((*rit)[RIT_SPECIFIER] == "%") {
            str.append((*rit)[RIT_SPECIFIER]);
        }
        format_implementation(s.clear(), ++rit, str);
    }

    template<typename T, typename... Args>
    void format_implementation(state &s, std::regex_iterator<std::string::const_iterator> &rit, std::string &str,
                               const T &arg, Args &... args) {
        if ((*rit)[RIT_STRING] == "%")
            throw std::invalid_argument("Invalid format");

        if ((*rit)[RIT_SPECIFIER] == "") {
            str.append((*rit)[RIT_STRING]);
            format_implementation(s.clear(), ++rit, str, arg, args...);
        } else {
            if (read_format<T>(s, rit, str, const_cast<T &>(arg)))
                format_implementation(s, rit, str, args...);
            else
                format_implementation(s.clear(), ++rit, str, args...);
        }
    };
}

/**
 * Returns a formatted string using the specified format string and
 * arguments.
 *
 * @param  format
 *         A <a href="http://cplusplus.com/printf">format string</a>
 *
 * @param  args
 *         Arguments referenced by the format specifiers in the format
 *         string.  If there are more arguments than format specifiers, the
 *         extra arguments are ignored.  The number of arguments is
 *         variable and may be zero.
 *
 * @throws  std::invalid_format
 *          If a format string contains an illegal syntax, a format
 *          specifier that is incompatible with the given arguments,
 *          insufficient arguments given the format string, or other
 *          illegal conditions.
 *
 * @throws  std::out_of_range
 *          If the arguments' list contains too few arguments
 *
 * @return  A formatted string
 */

template<typename... Args>
std::string format(std::string const &formatString, const Args &... args) {
    std::string str;

    std::regex_iterator<std::string::const_iterator> rgx_iterator(formatString.begin(), formatString.end(),
                                                                  format_impl::regex);
    format_impl::state state;

    format_impl::format_implementation(state, rgx_iterator, str, args...);

    return str;
}

#endif //TASK4_FORMAT_H