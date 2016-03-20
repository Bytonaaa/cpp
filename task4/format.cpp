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

            while (*next != '\0') {
                if (int flags = checkForFlags()) {
                    fmt.flags |= flags;
                    continue;
                } else
                    break;
            }

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
    int checkForFlags() {
        switch (*next) {
            case '0':
                return 1;
            case '+':
                return 2;
            case '-':
                return 4;
            case '#':
                return 8;

            default:
                return 0;
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
                break;
            case 'F':
                fmt.spec = F;
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

template<>
std::string sprint<int>(Format const *fmt, int arg) {
    if (fmt->spec == d)
        return std::to_string(arg);
    else
        throw std::invalid_argument("Invalid argument");
}

template<>
std::string sprint<double>(Format const *fmt, double arg) {
    if (fmt->spec == f || fmt->spec == F)
        return std::to_string(arg);
    else
        throw std::invalid_argument("Invalid argument");
}

template<>
std::string sprint<float>(Format const *fmt, float arg) {
    if (fmt->spec == f || fmt->spec == F)
        return std::to_string(arg);
    else
        throw std::invalid_argument("Invalid argument");
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
            throw std::out_of_range("<null>");//TODO
    }
}