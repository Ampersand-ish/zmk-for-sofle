#include <zephyr/kernel.h>
#include <zephyr/device.h>

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

#include <zephyr/drivers/led_strip.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/activity.h>
#include <zmk/rgb_underglow.h>
#include <zmk/keymap.h>

#define NUM_LAYERS 8
#define PER_KEY_LEDS 30
#define TOTAL_LEDS 36 // 6 underglow + 30 per-key

// Layer Color definition using the standard struct led_rgb from led_strip.h

static const struct led_rgb layer_colors[NUM_LAYERS] = {
    [0] = {240, 240, 220}, // BASE: Off-white
    [1] = {100, 150, 255}, // NAV: Pale Blue
    [2] = {240, 240, 100}, // NUM: Pale Yellow
    [3] = {100, 240, 100}, // MEDIA: Pale Green
    [4] = {240, 100, 100}, // SYM: Pale Red
    [5] = {240, 150, 50},  // FUN: Pale Orange
    [6] = {0, 0, 0},       // MOUSE: Disabled/Dark
    [7] = {255, 120, 0}    // GAMING: Amber
};

// Active key mapping lookup (1 = highlighted, 0 = isolated/dark)
// Maps corresponding physical keys (30 keys per half)
static const uint8_t layer_mapped_keys[NUM_LAYERS][PER_KEY_LEDS] = {
    // BASE (all active)
    [0] = { [0 ... 29] = 1 },
    // NAV (vim keys, navigation cluster only)
    [1] = {
        [6] = 1, [7] = 1, [8] = 1, [9] = 1, [10] = 1, // Arrows and caps
        [18] = 1, [19] = 1, [20] = 1, [21] = 1, [22] = 1
    },
    // NUM (numpad only)
    [2] = {
        [1] = 1, [2] = 1, [3] = 1, [4] = 1, [13] = 1, [14] = 1, [15] = 1, [16] = 1,
        [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    // MEDIA (volume, brightness only)
    [3] = {
        [18] = 1, [19] = 1, [20] = 1, [21] = 1, [22] = 1,
        [24] = 1, [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    // SYM (symbols only)
    [4] = {
        [1] = 1, [2] = 1, [3] = 1, [4] = 1, [13] = 1, [14] = 1, [15] = 1, [16] = 1,
        [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    // FUN (Function keys F1-F12 only)
    [5] = {
        [1] = 1, [2] = 1, [3] = 1, [4] = 1, [13] = 1, [14] = 1, [15] = 1, [16] = 1,
        [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    // MOUSE (mouse movements only)
    [6] = {
        [18] = 1, [19] = 1, [20] = 1, [21] = 1
    },
    // GAMING (WASD, shift, ctrl, space, numbers only)
    [7] = {
        [0] = 1, [1] = 1, [2] = 1, [3] = 1, [4] = 1, // Numbers
        [12] = 1, [14] = 1, [15] = 1,                 // Q, W, E, R
        [24] = 1, [25] = 1, [26] = 1, [27] = 1,       // LCTRL, A, S, D
        [18] = 1, [19] = 1, [20] = 1,                 // LSHIFT, Z, X
        [23] = 1                                      // SPACE
    }
};

static void update_layer_backlight(uint8_t layer) {
    const struct device *led_strip = DEVICE_DT_GET(DT_CHOSEN(zmk_underglow));
    if (!device_is_ready(led_strip)) {
        return;
    }

    // Check if underglow is globally enabled (Off is Off)
    #if IS_ENABLED(CONFIG_ZMK_RGB_UNDERGLOW)
    bool underglow_on;
    if (zmk_rgb_underglow_get_state(&underglow_on) < 0 || !underglow_on) {
        // If backlight is toggled off, keep all LEDs off
        struct led_rgb pixels[TOTAL_LEDS] = {0};
        led_strip_update_rgb(led_strip, pixels, TOTAL_LEDS);
        return;
    }
    #endif

    struct led_rgb pixels[TOTAL_LEDS] = {0};
    struct led_rgb color = layer_colors[layer];

    // Underglow LEDs (0-5) remain dark or solid background
    for (int i = 0; i < 6; i++) {
        pixels[i].r = 0;
        pixels[i].g = 0;
        pixels[i].b = 0;
    }

    // Per-key LEDs (6-35) map to key active state
    for (int i = 0; i < PER_KEY_LEDS; i++) {
        int led_idx = i + 6;
        if (layer_mapped_keys[layer][i]) {
            pixels[led_idx] = color;
        } else {
            pixels[led_idx].r = 0;
            pixels[led_idx].g = 0;
            pixels[led_idx].b = 0;
        }
    }

    led_strip_update_rgb(led_strip, pixels, TOTAL_LEDS);
}

static int rgb_layer_listener_cb(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);
    if (ev) {
        update_layer_backlight(zmk_keymap_highest_layer_active());
    }
    return 0;
}

ZMK_LISTENER(rgb_layer_listener, rgb_layer_listener_cb);
ZMK_SUBSCRIPTION(rgb_layer_listener, zmk_layer_state_changed);

#endif
