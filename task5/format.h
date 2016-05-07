#ifndef TASK4_FORMAT_H
#define TASK4_FORMAT_H

#include <string>
#include <stdexcept>
#include <vector>
#include <typeinfo>
#include <iostream>
#include <cxxabi.h>

#define DEFAULT_PRECISION (-1)  //default value
#define WP_READ (-2)    // useful if we want to read a width or a precision value from the arguments list

namespace formatImpl {
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

    std::string sprintPointer(Format const *fmt, void *arg);

    const char *demangle(const char *mangledName);

    template <typename T>
    std::string sprintAuto(Format const *fmt, typename std::enable_if<std::is_pointer<T>::value, T>::type arg) {
        std::string str;
        if (arg == nullptr)
            str = "nullptr<";
        else
            str = "ptr<";

        str += demangle(typeid(T).name());
        str[str.size() - 1] = '>';

        if (arg != nullptr)
            str += "(" + sprintPointer(fmt, (void *) arg) + ")";

        return str;
    }

    template <typename T>
    std::string sprintAuto(Format const *fmt, typename std::enable_if<std::is_same<T, std::nullptr_t>::value, T>::type arg) {
        return "nullptr";
    }

    template <typename T>
    std::string sprintAuto(Format const *fmt, typename std::enable_if<(std::is_array<T>::value &&
            std::is_same<typename std::remove_all_extents<T>::type, char>::value) ||
            std::is_same<T, std::string>::value, T>::type const &arg) {
        return arg;
    }

    template <typename T, size_t sz>
    size_t getSizeOfArray(T(&)[sz]) { return sz; }

    template <typename T>
    std::string sprintAuto(Format const *fmt, typename std::enable_if<std::is_array<T>::value &&
            !std::is_same<typename std::remove_all_extents<T>::type, char>::value, T>::type const &arg) {
        std::cout << demangle(typeid(T).name()) << std::endl;
        std::string result = "[" + std::to_string(arg[0]);
        for (int i = 1; i < getSizeOfArray(arg); i++)
            result += ", " + std::to_string(arg[i]);
        result += "]";

        return result;
    }

    template <typename T,
            typename = typename std::enable_if<!std::is_pointer<T>::value>::type,
            typename = typename std::enable_if<!std::is_array<T>::value>::type,
            typename = typename std::enable_if<!std::is_same<T, std::string>::value>::type,
            typename = typename std::enable_if<!std::is_same<T, std::nullptr_t>::value>::type
    >
    struct is_convertible_to_string {
        static std::string convert(T t) {
            return std::to_string(t);
        }
        enum {
            value = true,
        };
        typedef T type;
    };

    template <typename T>
    std::string sprintAuto(Format const *fmt, typename std::enable_if<is_convertible_to_string<T>::value, T>::type const &arg) {
        return is_convertible_to_string<T>::convert(arg);
    }

    template <typename T>
    std::string sprintAuto(Format const *fmt, T const & arg) {
        std::string ex = "Can not convert type ";
        ex += demangle(typeid(T).name());
        throw std::invalid_argument(ex);
    }

    template <typename T>
    std::string sprint(Format const *fmt, typename std::enable_if<std::is_pointer<T>::value, T>::type arg) {
        if (fmt->type == p)
            return sprintPointer(fmt, (void*) arg);

        throw std::invalid_argument(std::string("Invalid argument pointer ") + demangle(typeid(T).name()) + " " + demangle(typeid(fmt->type).name()));
    }

    template <typename T>
    std::string sprint(Format const *fmt, typename std::enable_if<std::is_array<T>::value, T>::type const &arg) {
        throw std::invalid_argument("Invalid argument, or this feature is not implemented.");
    }

    template<typename T>
    std::string sprint(Format const *fmt, T arg) {
        throw std::invalid_argument("Invalid argument, or this feature is not implemented.");
    }

    template<typename T>
    int checkForInt(const typename std::enable_if<std::is_integral<T>::value, T>::type &arg) {
        if (arg < 0)
            throw std::invalid_argument("Invalid argument: illegal value " + std::to_string(arg));
        return (int) arg;
    }

    template <typename T>
    int checkForInt(const typename std::enable_if<!std::is_integral<T>::value, T>::type &) {
        throw std::invalid_argument("Invalid format: integral type expected");
    }

    void formatImplementation(Format *fmt, unsigned long size, std::string &str);   //base

    template<typename T, typename... Args>
    void formatImplementation(Format *format, unsigned long tokensLeft, std::string &str, const T &curArgument, const Args&... args) {
        if (tokensLeft) {
            if (format->str.size()) {
                str += format->str;
                formatImplementation(format + 1, tokensLeft - 1, str, curArgument, args...);
            } else {
                if (format->width == WP_READ) {
                    format->width = checkForInt<T>(curArgument);
                } else if (format->precision == WP_READ) {
                    format->precision = checkForInt<T>(curArgument);
                } else {
                    if (format->type == automatic)
                        str += sprintAuto<T>(format, curArgument);
                    else
                        str += sprint<T>(format, curArgument);
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
std::string format(std::string const &formatString, const Args&... args) {
    std::string str;
    std::vector<formatImpl::Format> fmt;

    formatImpl::parseFormatString(fmt, formatString.c_str()); //generates tokens from format string

    formatImpl::formatImplementation(fmt.data(), fmt.size(), str, args...);

    return str;
}

#endif //TASK4_FORMAT_H