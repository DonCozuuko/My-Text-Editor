#include <raylib.h>
#include "special_characters.h"

void handleSpecialChars(int *key) {
    if (*key == KEY_ONE) {
        *key = 33;  // '!'
    } else if (*key == KEY_TWO) {
        *key = 64;  // '@'
    } else if (*key == KEY_THREE) {
        *key = 35;  // '#'
    } else if (*key == KEY_FOUR) {
        *key = 36;  // '$'
    } else if (*key == KEY_FIVE) {
        *key = 37;  // '%'
    } else if (*key == KEY_SIX) {
        *key = 94;  // '^'
    } else if (*key == KEY_SEVEN) {
        *key = 38;  // '&'
    } else if (*key == KEY_EIGHT) {
        *key = 42;  // '*'
    } else if (*key == KEY_NINE) {
        *key = 40;  // '('
    } else if (*key == KEY_ZERO) {
        *key = 41;  // ')'
    } else if (*key == KEY_MINUS) {
        *key = 95;  // '_'
    } else if (*key == KEY_EQUAL) {
        *key = 43;  // '+'
    } else if (*key == KEY_LEFT_BRACKET) {
        *key = 123;
    } else if (*key == KEY_RIGHT_BRACKET) {
        *key = 125;
    } else if (*key == KEY_COMMA) {
        *key = 60;  // '<'
    } else if (*key == KEY_PERIOD) {
        *key = 62;  // '>'
    } else if (*key == KEY_SEMICOLON) {
        *key = 58;  // ':'
    } else if (*key == KEY_APOSTROPHE) {
        *key = 34;  // '"'
    } else if (*key == KEY_SLASH) {
        *key = 63;  // '?'
    } else if (*key == KEY_BACKSLASH) {
        *key = 124; // '|'
    } else if (*key == KEY_GRAVE) {
        *key = 126; // '~'
    }
}
