#ifndef __VAR_PRINTF__
#define __VAR_PRINTF__

#include <ostream>

void safe_fprintf(std::ostream &out, const char *s)
{
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%') {
                ++s;
            }
            else {
                throw "invalid format string: missing arguments";
            }
        }
        out << *s++;
    }
}

template<typename T, typename... Args>
void safe_fprintf(std::ostream &out, const char *s, T& value, Args... args)
{
    while (*s) {
        if (*s == '%') {
            if (*(s + 1) == '%') {
                ++s;
            }
            else {
                out << value;
                safe_fprintf(out, s + 1, args...); // call even when *s == 0 to detect extra arguments
                return;
            }
        }
        out << *s++;
    }
    throw "extra arguments provided to printf";
}

#endif
