typedef struct {
    uint8_t current_theme;
} settings_t;

int snake_settings_save_current_theme(uint8_t current_theme);
uint8_t snake_settings_get_current_theme(void);