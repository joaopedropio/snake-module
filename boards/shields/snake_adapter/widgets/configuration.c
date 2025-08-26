#include <stdlib.h>
#include "helpers/display.h"
#include "theme.h"

#define HEX_PARSE_ERROR ((uint32_t)-1)

uint32_t hex_string_to_uint(const char *hex_str) {
    if (!hex_str) {
        return HEX_PARSE_ERROR;
    }

    uint32_t result = 0;
    uint8_t i = 0;

    // Optional "0x" or "0X" prefix
    if (hex_str[0] == '0' && (hex_str[1] == 'x' || hex_str[1] == 'X')) {
        i = 2;
    }

    if (hex_str[i + 6] != '\0') {
        // Not rgb hex
        return HEX_PARSE_ERROR;
    }

    if (hex_str[i] == '\0') {
        // Empty string after "0x"
        return HEX_PARSE_ERROR;
    }

    for (; hex_str[i] != '\0'; ++i) {
        char c = hex_str[i];
        uint32_t digit;

        if (isdigit(c)) {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        } else {
            // Invalid character for hex
            return HEX_PARSE_ERROR;
        }

        result = (result << 4) | digit;  // Multiply result by 16 and add digit
    }

    return result;
}

void configure(void) {
    uint32_t color1 = hex_string_to_uint(CONFIG_THEME_COLOR_1);
    uint32_t color2 = hex_string_to_uint(CONFIG_THEME_COLOR_2);
    uint32_t color3 = hex_string_to_uint(CONFIG_THEME_COLOR_3);
    uint32_t color4 = hex_string_to_uint(CONFIG_THEME_COLOR_4);
    if (color1 == HEX_PARSE_ERROR ||
        color2 == HEX_PARSE_ERROR ||
        color3 == HEX_PARSE_ERROR ||
        color4 == HEX_PARSE_ERROR) {
        themes_colors[0][0] = 0x4aedffu;
        themes_colors[0][1] = 0xff8acdu;
        themes_colors[0][2] = 0xb03e80u;
        themes_colors[0][3] = 0xffffffu;
    } else {
        themes_colors[0][0] = color1;
        themes_colors[0][1] = color2;
        themes_colors[0][2] = color3;
        themes_colors[0][3] = color4;
    }
}
