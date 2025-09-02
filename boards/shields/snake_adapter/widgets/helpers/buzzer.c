/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifdef CONFIG_USE_BUZZER

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_buzzer, LOG_LEVEL_DBG);

#include <stdlib.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include "settings.h"
#include "buzzer.h"
#include "pwm.h"

#define BUZZER_STACK 1024

#define FUNKYTOWN_NOTES 13
#define MARIO_NOTES	37
#define GOLIOTH_NOTES 21
#define MEGALOVANIA_NOTES 22
#define ONEUP_NOTES 6
#define COIN_NOTES 2
#define BEEP_NOTES 1
#define COIN_CHORDS_COUNT 2
#define CRAZY_COIN_NOTES 48

static const struct pwm_dt_spec sBuzzer = PWM_DT_SPEC_GET(DT_CHOSEN(zephyr_buzzer));

SongName song = funkytown;

static const int coin_chord1[] = { B5 };
static const int coin_chord2[] = { E6 };

static Chord coin_chords[COIN_CHORDS_COUNT] = {
	{.note_count = 2, .duration = EIGTH, .notes = coin_chord1},
	{.note_count = 2, .duration = HALF, .notes = coin_chord2},
};

static Sound coin_song[COIN_NOTES] = {
    {.note = B6, .duration = EIGTH},
    {.note = E7, .duration = HALF},
};

static Sound crazy_coin_song[CRAZY_COIN_NOTES] = {
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},
    {.note = B5, .duration = SIXTY_FOURTH},
    {.note = B6, .duration = SIXTY_FOURTH},

    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
    {.note = E6, .duration = SIXTY_FOURTH},
    {.note = E7, .duration = SIXTY_FOURTH},
};

static Sound reversed_coin_song[COIN_NOTES] = {
    {.note = E7, .duration = EIGTH},
    {.note = B6, .duration = HALF},
};

static Sound beep_song[BEEP_NOTES] = {
    {.note = 1000, .duration = 100},
};

static Sound oneup_song[ONEUP_NOTES] = {
    {.note = E5, .duration = DOTTED_EIGTH},
    {.note = G5, .duration = DOTTED_EIGTH},
    {.note = E6, .duration = DOTTED_EIGTH},
    {.note = C6, .duration = DOTTED_EIGTH},
    {.note = D6, .duration = DOTTED_EIGTH},
    {.note = G6, .duration = HALF},
};

static Sound megalovania_song[MEGALOVANIA_NOTES] = {
    {.note = D4, .duration = EIGTH},
    {.note = D4, .duration = EIGTH},
    {.note = A4, .duration = EIGTH},
    {.note = D5, .duration = SIXTEENTH + EIGTH}, // dotted EIGTH
    {.note = REST, .duration = SIXTEENTH},
    {.note = Db5, .duration = SIXTEENTH},
    {.note = C5, .duration = SIXTEENTH},
    {.note = Db5, .duration = SIXTEENTH},
    {.note = C5, .duration = EIGTH},
    {.note = A4, .duration = EIGTH},
    {.note = A4, .duration = EIGTH},

    // Repeat the same pattern once
    {.note = D4, .duration = EIGTH},
    {.note = D4, .duration = EIGTH},
    {.note = A4, .duration = EIGTH},
    {.note = D5, .duration = SIXTEENTH + EIGTH}, // dotted EIGTH
    {.note = REST, .duration = SIXTEENTH},
    {.note = Db5, .duration = SIXTEENTH},
    {.note = C5, .duration = SIXTEENTH},
    {.note = Db5, .duration = SIXTEENTH},
    {.note = C5, .duration = EIGTH},
    {.note = A4, .duration = EIGTH},
    {.note = A4, .duration = EIGTH},
};

static Sound funkytown_song[FUNKYTOWN_NOTES] = {
	{.note = C5, .duration = QUARTER},
	{.note = REST, .duration = EIGTH},
	{.note = C5, .duration = QUARTER},
	{.note = Bb4, .duration = QUARTER},
	{.note = C5, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = G4, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = G4, .duration = QUARTER},
	{.note = C5, .duration = QUARTER},
	{.note = F5, .duration = QUARTER},
	{.note = E5, .duration = QUARTER},
	{.note = C5, .duration = QUARTER}};

static Sound mario_song[MARIO_NOTES] = {
	{.note = E6, .duration = QUARTER},
	{.note = REST, .duration = EIGTH},
	{.note = E6, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = E6, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = C6, .duration = QUARTER},
	{.note = E6, .duration = HALF},
	{.note = G6, .duration = HALF},
	{.note = REST, .duration = QUARTER},
	{.note = G4, .duration = WHOLE},
	{.note = REST, .duration = WHOLE},
	/* break in sound */
	{.note = C6, .duration = HALF},
	{.note = REST, .duration = QUARTER},
	{.note = G5, .duration = HALF},
	{.note = REST, .duration = QUARTER},
	{.note = E5, .duration = HALF},
	{.note = REST, .duration = QUARTER},
	{.note = A5, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = B5, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = Bb5, .duration = QUARTER},
	{.note = A5, .duration = HALF},
	{.note = G5, .duration = QUARTER},
	{.note = E6, .duration = QUARTER},
	{.note = G6, .duration = QUARTER},
	{.note = A6, .duration = HALF},
	{.note = F6, .duration = QUARTER},
	{.note = G6, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = E6, .duration = QUARTER},
	{.note = REST, .duration = QUARTER},
	{.note = C6, .duration = QUARTER},
	{.note = D6, .duration = QUARTER},
	{.note = B5, .duration = QUARTER}};

	

/* Thread plays song on buzzer */

K_SEM_DEFINE(buzzer_initialized_sem, 0, 1); /* Wait until buzzer is ready */

extern void buzzer_thread(void *d0, void *d1, void *d2)
{
	/* Block until buzzer is available */
	k_sem_take(&buzzer_initialized_sem, K_FOREVER);
	while (1) {
		if (!snake_settings_get_mute()) {
			switch (song) {
			case beep: play_song(beep_song, BEEP_NOTES); break;
			case funkytown: play_song(funkytown_song, FUNKYTOWN_NOTES); break;
			case mario: play_song(mario_song, MARIO_NOTES); break;
			case megalovania: play_song(megalovania_song, MEGALOVANIA_NOTES); break;
			case oneup: play_song(oneup_song, ONEUP_NOTES); break;
			case coin: play_song(coin_song, COIN_NOTES); break;
			case reversed_coin: play_song(reversed_coin_song, COIN_NOTES); break;
			case crazy_coin: play_song(crazy_coin_song, CRAZY_COIN_NOTES); break;
			case coin_polyphonic: play_chords(coin_chords, COIN_CHORDS_COUNT); break;
            case theme_change_song: run_theme_change_song(); break;
            case connected_song: run_connected_song(); break;
            case disconnected_song: run_disconnected_song(); break;
            case error_song: run_error_song(); break;
            case notification_song: run_notification_song(); break;
            case startup_song: run_startup_song(); break;
            case powerd_down_song: run_powerd_down_song(); break;
			default: break;
			}
		}

		/* Sleep thread until awoken externally */
		k_sleep(K_FOREVER);
	}
}

K_THREAD_DEFINE(buzzer_tid, BUZZER_STACK, buzzer_thread, NULL, NULL, NULL, K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);

int app_buzzer_init(void)
{
	if (!device_is_ready(sBuzzer.dev)) {
		return -ENODEV;
	}
	k_sem_give(&buzzer_initialized_sem);
	return 0;
}

void play_once(SongName song_name) {
	song = song_name;
	k_wakeup(buzzer_tid);
}

void play_theme_change_song(void) {
    song = theme_change_song;
    k_wakeup(buzzer_tid);
}

void play_connected_song(void) {
    song = connected_song;
    k_wakeup(buzzer_tid);
}

void play_disconnected_song(void) {
    song = disconnected_song;
    k_wakeup(buzzer_tid);
}

void play_error_song(void) {
    song = error_song;
    k_wakeup(buzzer_tid);
}

void play_notification_song(void) {
    song = notification_song;
    k_wakeup(buzzer_tid);
}

void play_startup_song(void) {
    song = startup_song;
    k_wakeup(buzzer_tid);
}

void play_powerd_down_song(void) {
    song = powerd_down_song;
    k_wakeup(buzzer_tid);
}


// #####################################################################################

#define THEME_CHANGED_NOTES 3
static Sound theme_changed_sound[THEME_CHANGED_NOTES] = {
    {.note = C5, .duration = EIGTH},
    {.note = E5, .duration = EIGTH},
    {.note = G5, .duration = QUARTER}
};

// void play_theme_change_song(void) {
//     play_slide(theme_changed_sound[0], theme_changed_sound[0], EIGTH + EIGTH);
//     play_sound(theme_changed_sound[2]);
    
//     // Stop the sound after done
// 	stop_pwm();
// }
void run_theme_change_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_slide(theme_changed_sound[0].note, theme_changed_sound[1].note, EIGTH + EIGTH);
    play_sound(theme_changed_sound[2]);
    
    // Stop the sound after done
	stop_pwm();
}


// #####################################################################################

#define CONNECTED_NOTES 2
static Sound connected_sound[CONNECTED_NOTES] = {
    {.note = C6, .duration = SIXTEENTH},
    {.note = E6, .duration = EIGTH}
};

void run_connected_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_dual_note_simulated(connected_sound[0].note, connected_sound[1].note, EIGTH);
    
    // Stop the sound after done
	stop_pwm();
}


// #####################################################################################

#define DISCONNECTED_NOTES 3
static Sound disconnected_sound[DISCONNECTED_NOTES] = {
    {.note = E5, .duration = EIGTH},
    {.note = C5, .duration = EIGTH},
    {.note = A4, .duration = QUARTER}
};

void run_disconnected_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_sound(disconnected_sound[0]);
    play_sound(disconnected_sound[1]);
    play_sound(disconnected_sound[2]);
    
    // Stop the sound after done
	stop_pwm();
}


// #####################################################################################

#define ERROR_NOTES 2
static Sound error_sound[ERROR_NOTES] = {
    {.note = C4, .duration = EIGTH},
    {.note = Gb4, .duration = EIGTH}
};

void run_error_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_dual_note_simulated(error_sound[0].note, error_sound[1].note, EIGTH);
    
    // Stop the sound after done
	stop_pwm();
}

// #####################################################################################

#define NOTIFICATION_NOTES 1
static Sound notification_sound[NOTIFICATION_NOTES] = {
    {.note = G5, .duration = QUARTER}
};

void run_notification_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_sound_with_vibrato(error_sound[0], 6.0f, 5.0f);
    
    // Stop the sound after done
	stop_pwm();
}

// #####################################################################################


#define STARTUP_NOTES 6

static Sound startup_sound[STARTUP_NOTES] = {
    {.note = C5, .duration = SIXTEENTH},
    {.note = E5, .duration = SIXTEENTH},
    {.note = G5, .duration = EIGTH},
    {.note = REST, .duration = SIXTEENTH},
    {.note = C6, .duration = QUARTER},   // Bright final tone
    {.note = REST, .duration = EIGTH}
};

void run_startup_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_slide(C4, C5, EIGTH);
    play_sound(startup_sound[1]);
    play_sound(startup_sound[2]);
    play_sound(startup_sound[3]);
    play_sound(startup_sound[4]);
    play_sound(startup_sound[5]);
    
    // Stop the sound after done
	stop_pwm();
}

// #####################################################################################

#define POWER_DOWN_NOTES 5

static Sound power_down_sound[POWER_DOWN_NOTES] = {
    {.note = C6, .duration = SIXTEENTH},
    {.note = G5, .duration = SIXTEENTH},
    {.note = E5, .duration = SIXTEENTH},
    {.note = C5, .duration = EIGTH},
    {.note = A3, .duration = QUARTER}, // soft final low tone
};

void run_powerd_down_song(void) {
    if (snake_settings_get_mute()) {
        return;
    }
    play_sound(power_down_sound[0]);
    play_sound(power_down_sound[1]);
    play_sound(power_down_sound[2]);
    play_sound(power_down_sound[3]);
    play_slide_with_vibrato(C6, A3, HALF, 6.0f, 5.0f);
    
    // Stop the sound after done
	stop_pwm();
}

// #####################################################################################

#else

int app_buzzer_init(void)
{
	return 0;
}

void play_once(SongName song_name)
{
	return;
}

void play_theme_change_song(void) {
    return;
}

void play_connected_song(void) {
    return;
}

void play_disconnected_song(void) {
    return;
}

void play_error_song(void) {
    return;
}

void play_notification_song(void) {
    return;
}

void play_startup_song(void) {
    return;
}

void play_powerd_down_song(void) {
    return;
}


#endif