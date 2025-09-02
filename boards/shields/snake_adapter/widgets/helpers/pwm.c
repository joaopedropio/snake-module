/*
 * Copyright (c) 2024 Golioth, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_buzzer, LOG_LEVEL_DBG);

#include <stdlib.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include "pwm.h"

static const struct pwm_dt_spec pwm_buzzer = PWM_DT_SPEC_GET(DT_CHOSEN(zephyr_buzzer));

void stop_pwm() {
    pwm_set_pulse_dt(&pwm_buzzer, 0);
}

void play_song(Sound sounds[], int notes_count) {
    // for (int i = 0; i < notes_count; i++) {
    //     if (i == 0) {
    //         //play_slide(C4, C5, WHOLE);
    //         play_dual_note_simulated(C4, C5, WHOLE);
    //     } else {
    //         play_sound_with_vibrato(sounds[i], 10.0f, 5.0f);
    //     }
    // }
    for (int i = 0; i < notes_count; i++) {
        play_sound(sounds[i]);
    }
    
    // Stop the sound after done
	stop_pwm();
}

/*

#define SUCCESS_NOTES 2
static Sound success_sound[SUCCESS_NOTES] = {
    {.note = E5, .duration = SIXTEENTH},
    {.note = C5, .duration = EIGTH}
};
//Optional: Use play_dual_note_simulated(C5, E5, EIGTH) for a harmonic "ding."

#define CANCEL_NOTES 2
static Sound cancel_sound[CANCEL_NOTES] = {
    {.note = Bb4, .duration = SIXTEENTH},
    {.note = G4, .duration = EIGTH}
};

#define CHARGING_NOTES 4
static Sound charging_sound[CHARGING_NOTES] = {
    {.note = A4, .duration = SIXTEENTH},
    {.note = C5, .duration = SIXTEENTH},
    {.note = E5, .duration = EIGTH},
    {.note = G5, .duration = QUARTER}
};
// Can also be played with slight play_slide_with_vibrato transitions between steps.

#define BATTERY_FULL_NOTES 5
static Sound battery_full_sound[BATTERY_FULL_NOTES] = {
    {.note = G4, .duration = SIXTEENTH},
    {.note = C5, .duration = SIXTEENTH},
    {.note = E5, .duration = SIXTEENTH},
    {.note = G5, .duration = SIXTEENTH},
    {.note = C6, .duration = QUARTER}
};

#define WARNING_NOTES 2
static Sound warning_sound[WARNING_NOTES] = {
    {.note = F4, .duration = EIGTH},
    {.note = A4, .duration = QUARTER}
};

#define CRITICAL_ERROR_NOTES 4
static Sound critical_error_sound[CRITICAL_ERROR_NOTES] = {
    {.note = C5, .duration = SIXTEENTH},
    {.note = Gb4, .duration = SIXTEENTH},
    {.note = E4, .duration = EIGTH},
    {.note = C3, .duration = HALF}
};
// Could also be: play_slide_with_vibrato(C5, C3, WHOLE, 12.0f, 4.0f);

#define WIFI_SEARCH_NOTES 3
static Sound wifi_search_sound[WIFI_SEARCH_NOTES] = {
    {.note = G4, .duration = SIXTEENTH},
    {.note = A4, .duration = SIXTEENTH},
    {.note = Bb4, .duration = SIXTEENTH}
};
//Play this in a loop while searching, optionally increasing pitch subtly on each loop.

#define WIFI_CONNECTED_NOTES 3
static Sound wifi_connected_sound[WIFI_CONNECTED_NOTES] = {
    {.note = D5, .duration = SIXTEENTH},
    {.note = F5, .duration = SIXTEENTH},
    {.note = A5, .duration = EIGTH}
};

#define LOW_BATTERY_NOTES 3
static Sound low_battery_sound[LOW_BATTERY_NOTES] = {
    {.note = C4, .duration = SIXTEENTH},
    {.note = REST, .duration = SIXTEENTH},
    {.note = C4, .duration = QUARTER}
};
// Could add a vibrato here to make it "shaky" like a dying battery :play_sound_with_vibrato((Sound){.note = C4, .duration = QUARTER}, 12.0f, 4.0f);


// #####################################################################################

typedef enum {
    SOUND_THEME_CHANGED,
    SOUND_CONNECTED,
    SOUND_DISCONNECTED,
    SOUND_STARTUP,
    SOUND_SHUTDOWN,
    SOUND_SUCCESS,
    SOUND_CANCEL,
    SOUND_CHARGING,
    SOUND_BATTERY_FULL,
    SOUND_WARNING,
    SOUND_CRITICAL_ERROR,
    SOUND_WIFI_SEARCH,
    SOUND_WIFI_CONNECTED,
    SOUND_LOW_BATTERY,
    SOUND_NOTIFICATION
} SoundType;

 case SOUND_SUCCESS:
    play_song(success_sound, SUCCESS_NOTES);
    break;

    
// #####################################################################################
SOUND MODULATION TECHNIQUES
1. Tremolo (Volume Modulation)

Definition: Rapid oscillation of amplitude (volume), not pitch.

Effect: A "pulsing" or stuttering sound — useful for alerts or warnings.

Implementation Idea:

Simulate by toggling between pwm_set_dt() and pwm_set_pulse_dt(0) at fast intervals.
void play_tremolo(int freq, int duration_ms, float rate_hz) {
    uint32_t elapsed_ms = 0;
    uint32_t step_ms = 10;

    float period_ns = 1e9 / freq;
    float pulse_ns = period_ns / 2;

    while (elapsed_ms < duration_ms) {
        float t = elapsed_ms / 1000.0f;
        float tremolo = (sinf(2 * PI * rate_hz * t) + 1.0f) / 2.0f;

        // Use tremolo as duty cycle modulator
        uint32_t modulated_pulse = (uint32_t)(pulse_ns * tremolo);
        pwm_set_dt(&pwm_buzzer, period_ns, modulated_pulse);

        k_msleep(step_ms);
        elapsed_ms += step_ms;
    }

    stop_pwm();
}

2. Pitch Drop / Falloff

Definition: A nonlinear (usually exponential) pitch drop — like "dying robot" or retro UI.

Usage: Power-down sounds, failure tones.

play_slide_exponential(C6, A3, HALF); // already implemented!

play_slide_with_vibrato(C6, A2, WHOLE, 20.0f, 5.0f);




3. Trill (Rapid Alternating Between Two Notes)

Definition: Alternates between two close notes rapidly.

Effect: Expressive or attention-catching. Very musical.

Usage: Notifications, playful UI actions.
void play_trill(int note1, int note2, int duration_ms, int rate_hz) {
    uint32_t elapsed_ms = 0;
    uint32_t switch_interval_ms = 1000 / (rate_hz * 2);

    while (elapsed_ms < duration_ms) {
        pwm_set_dt(&pwm_buzzer, PWM_HZ(note1), PWM_HZ(note1) / 2);
        k_msleep(switch_interval_ms);
        elapsed_ms += switch_interval_ms;

        pwm_set_dt(&pwm_buzzer, PWM_HZ(note2), PWM_HZ(note2) / 2);
        k_msleep(switch_interval_ms);
        elapsed_ms += switch_interval_ms;
    }

    stop_pwm();
}

4. Glissando (Smooth Continuous Slide Through Notes)

Definition: Continuous sweep through a series of notes, not just start → end.

Usage: Menu scroll, drag actions, startup tones.

void play_glissando(int start_note, int end_note, int duration_ms) {
    int steps = abs(end_note - start_note);
    if (steps == 0) return;

    int step_duration = duration_ms / steps;
    int direction = (end_note > start_note) ? 1 : -1;

    for (int n = start_note; n != end_note; n += direction) {
        pwm_set_dt(&pwm_buzzer, PWM_HZ(n), PWM_HZ(n) / 2);
        k_msleep(step_duration);
    }

    stop_pwm();
}

6. Bit-Crushed / Retro Effect

Definition: Make it sound like old-school 8-bit tones

Implementation: Intentionally step through frequencies in jagged intervals or use square wave burst patterns.
void play_bitcrushed_ramp(int start_note, int end_note, int duration_ms) {
    int steps = 8;
    int step_duration = duration_ms / steps;

    for (int i = 0; i < steps; ++i) {
        int freq = start_note + ((end_note - start_note) * i) / steps;
        pwm_set_dt(&pwm_buzzer, PWM_HZ(freq), PWM_HZ(freq) / 2);
        k_msleep(step_duration);
    }

    stop_pwm();
}

7. Cold Boot Effect (Rising+Glitch)

Start with a slow vibrato ramp and end with a bright tone.
play_slide_with_vibrato(C4, C6, WHOLE, 20.0f, 1.0f);
play_trill(C6, E6, 200, 10);

Technique Combinations
| Technique Combo       | Description             | Example Use            |
| --------------------- | ----------------------- | ---------------------- |
| Slide + Vibrato       | Natural pitch motion    | Startup, UI confirm    |
| Trill + Slide         | Expressive with urgency | Warning, toggle effect |
| Tremolo + Slide Down  | Distress or dying tone  | Low battery, shutdown  |
| Bit-crushed Glissando | Retro or playful sound  | Easter egg, game mode  |
| Trill + Vibrato       | Very musical, energetic | Theme change, success  |


*/


// #####################################################################################
void play_sound(Sound sound) {
    if (sound.note < 10) {
        /* Low frequency notes represent a 'pause' */
        stop_pwm();
        k_msleep(sound.duration);
    } else {
        pwm_set_dt(&pwm_buzzer, PWM_HZ(sound.note), PWM_HZ((sound.note)) / 2);
        k_msleep(sound.duration);
    }
}

void play_slide(int start_freq, int end_freq, int duration_ms) {
    uint32_t elapsed_ms = 0;
    uint32_t step_ms = 10; // Control resolution

    // Linear frequency increment per millisecond
    int freq_delta = end_freq - start_freq;
    float freq_step_per_ms = freq_delta / (float)duration_ms;

    while (elapsed_ms < duration_ms) {
        float t = elapsed_ms;
        float current_freq = start_freq + freq_step_per_ms * t;

        // Calculate period and pulse width in nanoseconds
        uint32_t period_ns = (uint32_t)(1e9 / current_freq);
        uint32_t pulse_ns = period_ns / 2; // 50% duty cycle

        pwm_set_dt(&pwm_buzzer, period_ns, pulse_ns);

        k_msleep(step_ms);
        elapsed_ms += step_ms;
    }

    // Stop the sound after done
    stop_pwm();
}

void play_slide_with_vibrato(int start_freq, int end_freq, int duration_ms, float vibrato_depth_hz, float vibrato_rate_hz) {
    uint32_t elapsed_ms = 0;
    uint32_t step_ms = 10; // Control resolution in ms

    int freq_delta = end_freq - start_freq;

    while (elapsed_ms < duration_ms) {
        float t = elapsed_ms / 1000.0f; // Time in seconds

        // Linear slide
        float base_freq = start_freq + (freq_delta * ((float)elapsed_ms / duration_ms));

        // Vibrato (sine wave modulation)
        float vibrato = sinf(2 * PI * vibrato_rate_hz * t) * vibrato_depth_hz;

        // Final frequency
        float current_freq = base_freq + vibrato;

        // Avoid divide-by-zero and negative frequencies
        if (current_freq < 1.0f) current_freq = 1.0f;

        // Convert frequency to PWM period/pulse
        uint32_t period_ns = (uint32_t)(1e9 / current_freq);
        uint32_t pulse_ns = period_ns / 2; // 50% duty cycle

        pwm_set_dt(&pwm_buzzer, period_ns, pulse_ns);

        k_msleep(step_ms);
        elapsed_ms += step_ms;
    }

    // Stop sound
    stop_pwm();
}

void play_slide_exponential(float start_freq, float end_freq, uint32_t duration_ms) {
    uint32_t elapsed_ms = 0;
    uint32_t step_ms = 10; // Control resolution

    // Linear frequency increment per millisecond
    float freq_delta = end_freq - start_freq;
    float freq_step_per_ms = freq_delta / (float)duration_ms;

    while (elapsed_ms < duration_ms) {
        float t = elapsed_ms;
        float slide_factor = powf(end_freq / start_freq, 1.0f / duration_ms);
        float current_freq = start_freq * powf(slide_factor, elapsed_ms);

        // Calculate period and pulse width in nanoseconds
        uint32_t period_ns = (uint32_t)(1e9 / current_freq);
        uint32_t pulse_ns = period_ns / 2; // 50% duty cycle

        pwm_set_dt(&pwm_buzzer, period_ns, pulse_ns);

        k_msleep(step_ms);
        elapsed_ms += step_ms;
    }

    // Stop the sound after done
    stop_pwm();
}


void play_sound_with_vibrato(Sound sound, float vibrato_depth_hz, float vibrato_rate_hz) {
    uint32_t elapsed_ms = 0;
    uint32_t step_ms = 10; // control resolution (update every 10ms)

    while (elapsed_ms < sound.duration) {
        float t = elapsed_ms / 1000.0f;  // time in seconds
        // Vibrato effect: modulate frequency as sine wave
        float vibrato = sinf(2 * PI * vibrato_rate_hz * t) * vibrato_depth_hz;
        float current_freq = sound.note + vibrato;

        // Calculate period and pulse width in nanoseconds
        uint32_t period_ns = (uint32_t)(1e9 / current_freq);
        uint32_t pulse_ns = period_ns / 2; // 50% duty cycle

        pwm_set_dt(&pwm_buzzer, period_ns, pulse_ns);

        k_msleep(step_ms);
        elapsed_ms += step_ms;
    }

    // Stop the sound after done
	stop_pwm();
}

void play_dual_note_simulated(int freq1, int freq2, uint32_t duration_ms) {
    uint32_t elapsed_ms = 0;
    uint32_t switch_interval_ms = 5; // Fast enough to trick the ear

    while (elapsed_ms < duration_ms) {
        // Play first note
        uint32_t period1 = (uint32_t)(1e9 / freq1);
        pwm_set_dt(&pwm_buzzer, period1, period1 / 2);
        k_msleep(switch_interval_ms);

        // Play second note
        uint32_t period2 = (uint32_t)(1e9 / freq2);
        pwm_set_dt(&pwm_buzzer, period2, period2 / 2);
        k_msleep(switch_interval_ms);

        elapsed_ms += 2 * switch_interval_ms;
    }

    // Stop sound
    stop_pwm();
}

void play_chords(Chord chords[], int chords_count) {
    for (int i = 0; i < chords_count; i++) {
        Chord chord = chords[i];
        play_dual_note_simulated(chord.notes[0], chord.notes[1], chord.duration);
    }

	stop_pwm();
}