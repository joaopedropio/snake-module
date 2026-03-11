#pragma once

#include <stddef.h>
#include <stdint.h>

struct custom_theme {
    const char *name;

    uint32_t background;
    uint32_t foreground;
    uint32_t accent;
    uint32_t warning;
    uint32_t success;
};

extern const struct custom_theme custom_themes[];
extern const size_t custom_themes_count;