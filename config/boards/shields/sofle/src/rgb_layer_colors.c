#define DT_DRV_COMPAT zmk_custom_led_strip

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zmk/rgb_underglow.h>

#define NUM_LAYERS 8
#define PER_KEY_LEDS 30
#define TOTAL_LEDS 36 // 6 underglow + 30 per-key

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

static const uint8_t layer_mapped_keys[NUM_LAYERS][PER_KEY_LEDS] = {
    [0] = { [0 ... 29] = 1 },
    [1] = {
        [6] = 1, [7] = 1, [8] = 1, [9] = 1, [10] = 1,
        [18] = 1, [19] = 1, [20] = 1, [21] = 1, [22] = 1
    },
    [2] = {
        [1] = 1, [2] = 1, [3] = 1, [4] = 1, [13] = 1, [14] = 1, [15] = 1, [16] = 1,
        [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    [3] = {
        [18] = 1, [19] = 1, [20] = 1, [21] = 1, [22] = 1,
        [24] = 1, [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    [4] = {
        [1] = 1, [2] = 1, [3] = 1, [4] = 1, [13] = 1, [14] = 1, [15] = 1, [16] = 1,
        [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    [5] = {
        [1] = 1, [2] = 1, [3] = 1, [4] = 1, [13] = 1, [14] = 1, [15] = 1, [16] = 1,
        [25] = 1, [26] = 1, [27] = 1, [28] = 1
    },
    [6] = {
        [18] = 1, [19] = 1, [20] = 1, [21] = 1
    },
    [7] = {
        [0] = 1, [1] = 1, [2] = 1, [3] = 1, [4] = 1,
        [12] = 1, [14] = 1, [15] = 1,
        [24] = 1, [25] = 1, [26] = 1, [27] = 1,
        [18] = 1, [19] = 1, [20] = 1,
        [23] = 1
    }
};

static uint16_t rgb_to_hue(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    uint8_t min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    if (max == min) return 0;
    
    int hue = 0;
    int delta = max - min;
    if (max == r) {
        hue = 60 * (g - b) / delta;
    } else if (max == g) {
        hue = 60 * (b - r) / delta + 120;
    } else {
        hue = 60 * (r - g) / delta + 240;
    }
    if (hue < 0) hue += 360;
    return (uint16_t)hue;
}

struct custom_led_strip_config {
    const struct device *real_strip;
    uint32_t chain_length;
};

static int custom_led_strip_update_rgb(const struct device *dev, const struct led_rgb *pixels, size_t num_pixels) {
    const struct custom_led_strip_config *config = dev->config;
    
    if (num_pixels == 0) {
        return led_strip_update_rgb(config->real_strip, pixels, num_pixels);
    }
    
    // Check if underglow is globally disabled
    bool all_dark = true;
    for (size_t i = 0; i < num_pixels; i++) {
        if (pixels[i].r != 0 || pixels[i].g != 0 || pixels[i].b != 0) {
            all_dark = false;
            break;
        }
    }
    
    if (all_dark) {
        return led_strip_update_rgb(config->real_strip, pixels, num_pixels);
    }
    
    // Check if animation (varying colors across strip)
    bool is_animation = false;
    for (size_t i = 1; i < num_pixels; i++) {
        if (pixels[i].r != pixels[0].r || pixels[i].g != pixels[0].g || pixels[i].b != pixels[0].b) {
            is_animation = true;
            break;
        }
    }
    
    if (is_animation) {
        return led_strip_update_rgb(config->real_strip, pixels, num_pixels);
    }
    
    // Decode active layer
    uint16_t hue = rgb_to_hue(pixels[0].r, pixels[0].g, pixels[0].b);
    uint8_t max = pixels[0].r > pixels[0].g ? (pixels[0].r > pixels[0].b ? pixels[0].r : pixels[0].b) : (pixels[0].g > pixels[0].b ? pixels[0].g : pixels[0].b);
    uint8_t min = pixels[0].r < pixels[0].g ? (pixels[0].r < pixels[0].b ? pixels[0].r : pixels[0].b) : (pixels[0].g < pixels[0].b ? pixels[0].g : pixels[0].b);
    
    uint8_t layer = 0;
    if (max - min < 15) {
        layer = 0;
    } else if (hue >= 10 && hue < 35) {
        layer = 7;
    } else if (hue >= 35 && hue < 52) {
        layer = 5;
    } else if (hue >= 52 && hue < 90) {
        layer = 2;
    } else if (hue >= 90 && hue < 160) {
        layer = 3;
    } else if (hue >= 160 && hue < 280) {
        layer = 1;
    } else {
        layer = 4;
    }
    
    struct led_rgb temp_pixels[TOTAL_LEDS] = {0};
    struct led_rgb layer_color = layer_colors[layer];
    
    // Underglow (0-5) stays dark for isolation
    for (int i = 0; i < 6; i++) {
        temp_pixels[i].r = 0;
        temp_pixels[i].g = 0;
        temp_pixels[i].b = 0;
    }
    
    // Per-key highlights (6-35)
    for (int i = 0; i < 30; i++) {
        int led_idx = i + 6;
        if (layer_mapped_keys[layer][i]) {
            temp_pixels[led_idx] = layer_color;
        } else {
            temp_pixels[led_idx].r = 0;
            temp_pixels[led_idx].g = 0;
            temp_pixels[led_idx].b = 0;
        }
    }
    
    return led_strip_update_rgb(config->real_strip, temp_pixels, num_pixels);
}

static const struct led_strip_driver_api custom_led_strip_api = {
    .update_rgb = custom_led_strip_update_rgb,
};

static int custom_led_strip_init(const struct device *dev) {
    const struct custom_led_strip_config *config = dev->config;
    if (!device_is_ready(config->real_strip)) {
        return -ENODEV;
    }
    return 0;
}

#define CUSTOM_LED_STRIP_INIT_INST(inst) \
    static const struct custom_led_strip_config custom_led_strip_config_##inst = { \
        .real_strip = DEVICE_DT_GET(DT_INST_PHANDLE(inst, real_strip)), \
        .chain_length = DT_INST_PROP(inst, chain_length), \
    }; \
    static struct { int dummy; } custom_led_strip_data_##inst; \
    DEVICE_DT_INST_DEFINE(inst, \
                          custom_led_strip_init, \
                          NULL, \
                          &custom_led_strip_data_##inst, \
                          &custom_led_strip_config_##inst, \
                          POST_KERNEL, \
                          95, \
                          &custom_led_strip_api);

DT_INST_FOREACH_STATUS_OKAY(CUSTOM_LED_STRIP_INIT_INST)

// Central-only active layer event subscription and global underglow color mapping
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/activity.h>
#include <zmk/keymap.h>

static int rgb_layer_listener_cb(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);
    if (ev) {
        uint8_t layer = zmk_keymap_highest_layer_active();
        struct zmk_led_hsb hsb;
        if (layer == 0) {
            hsb.h = 0; hsb.s = 0; hsb.b = 100;
        } else if (layer == 1) {
            hsb.h = 220; hsb.s = 100; hsb.b = 100;
        } else if (layer == 2) {
            hsb.h = 60; hsb.s = 100; hsb.b = 100;
        } else if (layer == 3) {
            hsb.h = 120; hsb.s = 100; hsb.b = 100;
        } else if (layer == 4) {
            hsb.h = 350; hsb.s = 100; hsb.b = 100;
        } else if (layer == 5) {
            hsb.h = 45; hsb.s = 100; hsb.b = 100;
        } else if (layer == 6) {
            hsb.h = 0; hsb.s = 0; hsb.b = 0;
        } else if (layer == 7) {
            hsb.h = 20; hsb.s = 100; hsb.b = 100;
        }
        
        bool underglow_on;
        if (zmk_rgb_underglow_get_state(&underglow_on) == 0 && underglow_on) {
            zmk_rgb_underglow_set_hsb(&hsb);
        }
    }
    return 0;
}

ZMK_LISTENER(rgb_layer_listener, rgb_layer_listener_cb);
ZMK_SUBSCRIPTION(rgb_layer_listener, zmk_layer_state_changed);

#endif
