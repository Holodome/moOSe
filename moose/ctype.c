#include <ctype.h>
#include <string.h>

int isdigit(int c) {
    return c >= '0' && c <= '9';
}
int isalnum(int c) {
    return isdigit(c) || isalpha(c);
}
int isalpha(int c) {
    return islower(c) || isupper(c);
}
int iscntrl(int c) {
    return (0x00 <= c && c <= 0x1f) || c == 0x7f;
}
int isgraph(int c) {
    return isalnum(c) || ispunct(c);
}
int islower(int c) {
    return 'a' <= c && c <= 'z';
}
int isupper(int c) {
    return 'A' <= c && c <= 'Z';
}
int isxdigit(int c) {
    return isdigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
}
int isspace(int c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' ||
           c == '\v';
}
int ispunct(int c) {
    return strchr("!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~", c) != NULL;
}
int isascii(int c) {
    return c <= 0x7f;
}
int isodigit(int c) {
    return '0' <= c && c <= '8';
}

int toupper(int c) {
    return islower(c) ? c + 'A' - 'a' : c;
};
int tolower(int c) {
    return isupper(c) ? c + 'a' - 'A' : c;
};
int toascii(int c) {
    return c & 0x7f;
}
