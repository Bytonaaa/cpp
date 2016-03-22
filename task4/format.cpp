#include "format.h"

class FormatParser {
public:
    const char *next;

    FormatParser(const char *format) : next(format) { };

    void operator()(Format &fmt) {
        if (*next == '%') {
            next++;

            if (*next == '%') {
                fmt.str = "%";
                return;
            }
            if (*next == '\0')
                throw std::invalid_argument("Unexpected end of format string");


            checkForFlags(fmt);

            if (isdigit(*next))
                fmt.width = readNumber();
            else if (*next == '*')
                next++, fmt.width = WP_READ;

            if (*next == '.') {
                next++;

                if (isdigit(*next))
                    fmt.precision = readNumber();
                else if (*next == '*')
                    next++, fmt.precision = WP_READ;
                else
                    throw std::invalid_argument("Invalid format");
            }

            checkForLength(fmt);
            checkForSpec(fmt);

        } else {
            while (*next != '%' && *next != '\0') {
                fmt.str.push_back(*next);
                next++;
            }
        }
    }

private:
    void checkForFlags(Format &fmt) {
        bool ok = true;
        while (ok) {
            switch (*next) {
                case '0':
                    fmt.zero = true;
                    break;
                case '+':
                    fmt.plus = true;
                    break;
                case '-':
                    fmt.minus = true;
                    break;
                case '#':
                    fmt.sharp = true;
                    break;

                case '\0':
                    throw std::invalid_argument("Invalid format");

                default:
                    ok = false;
                    next--;
            }
            next++;
        }
    }

    int readNumber() {
        int num = 0;
        while (isdigit(*next)) {
            num *= 10;
            if (num < 0)
                throw std::invalid_argument("Invalid format: overflow");

            num += *next - '0';
            next++;
        }
        return num;
    }

    void checkForLength(Format &fmt) {
        switch (*next) {
            case 'l':
                if (next[1] == 'l')
                    next++, fmt.length = ll;
                else
                    fmt.length = l;
                break;
            case 'h':
                if (next[1] == 'h')
                    next++, fmt.length = hh;
                else
                    fmt.length = h;
                break;
            case 'j':
                fmt.length = f;
                break;
            case 'z':
                fmt.length = z;
                break;
            case 't':
                fmt.length = t;
                break;
            case 'L':
                fmt.length = L;

            case '\0':
                throw std::invalid_argument("Unexpected end of format");
            default:
                next--;
                break;
        }
        next++;
    }

    void checkForSpec(Format &fmt) {
        switch (*next) {
            case 'd':
                fmt.spec = d;
                break;
            case 'u':
                fmt.spec = u;
                break;
            case 'o':
                fmt.spec = o;
                break;
            case 'x':
                fmt.spec = x;
                break;
            case 'X':
                fmt.spec = X;
                break;
            case 'f':
                fmt.spec = f;
            case 'F':
                fmt.spec = fmt.spec == f ? f : F;
                if (fmt.precision == -1)
                    fmt.precision = 6;
                break;
            case 'e':
                fmt.spec = e;
                break;
            case 'E':
                fmt.spec = E;
                break;
            case 'g':
                fmt.spec = g;
                break;
            case 'G':
                fmt.spec = G;
                break;
            case 'a':
                fmt.spec = a;
                break;
            case 'A':
                fmt.spec = A;
                break;
            case 'c':
                fmt.spec = c;
                break;
            case 's':
                fmt.spec = s;
                break;
            case 'p':
                fmt.spec = p;
                break;
            case 'n': //WTF is this???
                fmt.spec = n;
                break;
            case '\0':
                throw std::invalid_argument("Unexpected end of format");
            default:
                throw std::invalid_argument("Invalid format");
        }
        next++;
    }
};

void parse(std::vector<Format> &fmt, const char *format) {
    FormatParser formatParser(format);
    while (*formatParser.next) {
        fmt.push_back(Format());
        formatParser(fmt.back());
    }
}

std::string commonFormatter(Format const *fmt, std::string str) {
    std::string result;

    if (fmt->minus) {
        result = str;

        while (fmt->width > result.size())
            result.push_back(' ');

    } else {
        if (fmt->width > str.size()) {
            size_t i = (size_t) fmt->width - str.size();
            char c = fmt->zero ? '0' : ' ';

            while (i--)
                result.push_back(c);
        }
        result += str;
    }
    return result;
}

std::string sprintFloat(Format const *fmt, double d) {
    union {
        double a;
        struct {
            unsigned s : 1;
            unsigned p : 11;
            unsigned long long m : 52;
        };
    } Float, tmp;
    Float.a = d;
    std::string result;

    return std::to_string(d);
}

std::string sprintChar(Format const *fmt, char c) {
    std::string s;
    s.push_back(c);
    return s;
}

template<>
std::string sprint(Format const *fmt, int arg) {
    if (fmt->spec == d)
        return std::to_string(arg);
    else
        throw std::invalid_argument("Invalid argument: int found");
}

template<>
std::string sprint(Format const *fmt, unsigned int arg) {
    if (fmt->spec == u)
        return std::to_string(arg);
    else
        throw std::invalid_argument("Invalid argument: unsigned int found");
}

template<>
std::string sprint(Format const *fmt, double arg) {
    if (fmt->spec == f || fmt->spec == F)
        return sprintFloat(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: double found");
}

template<>
std::string sprint(Format const *fmt, float arg) {
    if (fmt->spec == f || fmt->spec == F)
        return sprintFloat(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: float found");
}

template<>
std::string sprint<std::string>(Format const *fmt, std::string arg) {
    if (fmt->spec == s)
        return arg;
    else
        throw std::invalid_argument("Invalid argument: std::string found");
}

template<>
std::string sprint(Format const *fmt, const char *arg) {
    if (arg == NULL)
        //throw std::invalid_argument("Invalid argument: null pointer found");
        return std::string("(null)");

    if (fmt->spec == s)
        return sprint<std::string>(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: char* found");
}

template<>
std::string sprint(Format const *fmt, char *arg) {
    return sprint(fmt, (const char *) arg);
}

template<>
std::string sprint(Format const *fmt, char arg) {
    if (fmt->spec == c)
        return sprintChar(fmt, arg);
    else
        throw std::invalid_argument("Invalid argument: char found");
}

template<>
std::string sprint(Format const *fmt, long long arg) {
    if (fmt->spec == d && fmt->length == ll)
        return std::to_string(arg);
    else
        throw std::invalid_argument("Invalid argument: long long found");
}

template<>
std::string sprint(Format const *fmt, unsigned long long arg) {
    if (fmt->spec == u && fmt->length == ll)
        return std::to_string(arg);
    else
        throw std::invalid_argument("");
}

template<>
std::string sprint(Format const *fmt, std::nullptr_t arg) {
    throw std::invalid_argument("Invalid argument: nullptr found");
}

std::string format(std::string const &format) {
    return format;
}

void gen(Format *fmt, unsigned long size, std::string &str) {
    if (size) {
        if (fmt->str.size()) {
            str += fmt->str;
            gen(fmt + 1, size - 1, str);
        } else
            throw std::out_of_range("I need more arguments!!1!");//TODO
    }
}

template<>
int checkForInt<int>(int arg) {
    return arg;
}

template<>
int checkForInt<size_t>(size_t arg) {
    return (int) arg;
}

template<>
int checkForInt<unsigned int>(unsigned int arg) {
    return (int) arg;
}