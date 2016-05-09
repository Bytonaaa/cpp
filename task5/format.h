#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <stdexcept>
#include <vector>
#include <typeinfo>
#include <iostream>
#include <cxxabi.h>
#include <regex>
#include <type_traits>

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

    template<typename T> struct remove_all_const : std::remove_const<T> {};

    template<typename T> struct remove_all_const<T*> {
        typedef typename remove_all_const<T>::type *type;
    };

    template<typename T> struct remove_all_const<T * const> {
        typedef typename remove_all_const<T>::type *type;
    };

    const char *demangle(const char *mangledName);

    template <typename T>
    std::string sprint(std::string &fmt, typename std::enable_if<!std::is_same<std::string, T>::value, T>::type &arg) {
        std::string result((size_t) snprintf(NULL, 0, fmt.c_str(), arg), '\0');
        snprintf(const_cast<char *>(result.c_str()), result.size() + 1, fmt.c_str(), arg);
        return result;
    }

    template <typename T>
    std::string sprint(std::string &fmt, typename std::enable_if<std::is_same<std::string, T>::value, T>::type &arg) {
        return arg;
    }

    template <typename T>
    bool check_type(FormatSpec len, FormatSpec type, T &) {
        if (type == p)
            return true;
        if (type == s && (std::is_same<T, char *>::value || std::is_same<T, std::string>::value))
            return true;

        throw std::invalid_argument(std::string("Invalid argument type ") + demangle(typeid(T).name()));
    }

    template <typename T>
    std::string print_arg(std::string fmt, FormatSpec len, FormatSpec type, T &arg) {
        if (check_type(len, type, arg)) {
            return sprint<T>(fmt, arg);
        } else
            throw std::invalid_argument("Invalid format");
    }

    template<typename T>
    int read_int(T &arg, typename std::enable_if<std::is_integral<T>::value>::type* = 0) {
        return (int) arg;
    }

    template<typename T>
    int read_int(T &arg, typename std::enable_if<!std::is_integral<T>::value>::type* = 0) {
        throw std::invalid_argument(std::string("Invalid argument, expected integral type found ") + demangle(typeid(T).name()));
    }

    template<typename T>
    bool read_format(std::string &s, std::string &str, T &arg) {
        std::smatch match;
        std::regex_search(s, match, regex);

        if (match[RIT_WIDTH] == "*") {
            s.replace((size_t) match.position(RIT_WIDTH), 1, std::to_string(read_int(arg)));
            return true;
        }

        if (match[RIT_PRECISION] == "*") {
            s.replace((size_t) match.position(RIT_PRECISION), 1, std::to_string(read_int(arg)));
            return true;
        }

        FormatSpec len = (FormatSpec) match[RIT_LENGTH].str()[0];
        FormatSpec type = (FormatSpec) match[RIT_SPECIFIER].str()[0];

        if (match[RIT_LENGTH].str() == "ll")
            len = ll;
        if (match[RIT_LENGTH].str() == "hh")
            len = hh;

        str += print_arg(match[RIT_STRING], len, type, arg);

        return false;
    };

    static void format_implementation(std::string &s, std::regex_iterator<std::string::const_iterator> &rit,
                                      std::string &str) {
        if (rit == rend)
            return;

        if ((*rit)[RIT_STRING] == "%")
            throw std::invalid_argument("Invalid format");

        if ((*rit)[RIT_SPECIFIER] == "") {
            str.append((*rit)[RIT_STRING]);
        } else if ((*rit)[RIT_SPECIFIER] == "%") {
            str.push_back('%');
        }
        format_implementation(s, ++rit, str);
    }

    template<typename T, typename... Args>
    void format_implementation(std::string &s, std::regex_iterator<std::string::const_iterator> &rit, std::string &str,
                               const T &arg, const Args &... args) {
        typedef typename remove_all_const<T>::type TT;

        if ((*rit)[RIT_STRING] == "%")
            throw std::invalid_argument(std::string("Invalid format ") + (*(++rit))[0].str());

        if ((*rit)[RIT_SPECIFIER] == "") {
            str.append((*rit)[RIT_STRING]);
        } else if ((*rit)[RIT_SPECIFIER] == "%") {
            str.push_back('%');
        } else {
            if (s.empty())
                s = rit->str();

            if (read_format<TT>(s, str, const_cast<TT &>(arg)))
                format_implementation(s, rit, str, args...);
            else {
                s.clear();
                format_implementation(s, ++rit, str, args...);
            }
            return;
        }
        format_implementation(s, ++rit, str, arg, args...);
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
    std::string s, str;
    std::cout << formatString << std::endl;

    std::regex_iterator<std::string::const_iterator> rgx_iterator(formatString.begin(), formatString.end(),
                                                                  format_impl::regex);

    format_impl::format_implementation(s, rgx_iterator, str, args...);

    return str;
}

#endif //TASK4_FORMAT_H