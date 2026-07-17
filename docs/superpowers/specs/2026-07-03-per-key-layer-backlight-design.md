# Spec: Per-Key Layer-Specific Backlight Mapping for Sofle Keyboard
**Date:** 2026-07-03

## 1. Objectives
* Implement custom per-key RGB backlighting on the Sofle split keyboard using standard upstream ZMK.
* When the global backlight is on, shifting layers will highlight only the mapped keys of the active layer.
* Unmapped keys (those with `&trans` or `&none`) will go completely dark (**Complete Isolation**).
* When the global backlight is toggled off, no layer-shifting highlights should occur (**Off is Off**).

## 2. Color Palettes
We define the following layer colors:
* **BASE:** Off-white (`RGB(240, 240, 220)`)
* **NAV:** Pale Blue (`RGB(100, 150, 255)`)
* **NUM:** Pale Yellow (`RGB(240, 240, 100)`)
* **MEDIA:** Pale Green (`RGB(100, 240, 100)`)
* **SYM:** Pale Red (`RGB(240, 100, 100)`)
* **FUN:** Pale Orange (`RGB(240, 150, 50)`)
* **GAMING:** Amber (`RGB(255, 120, 0)`)

## 3. Architecture & Implementation Plan

### 3.1 Listening to Layer State Events
We will write custom C-code inside the `zmk-config` repository under `config/boards/shields/sofle/src/rgb_layer_colors.c`.
This code subscribes to:
* `zmk_layer_state_changed`
* `zmk_underglow_state_changed` (or query ZMK's current underglow state using native API `zmk_rgb_underglow_get_state`).

### 3.2 Key-to-LED Matrix Mapping
Since Sofle has a split design with a sequential WS2812 chain (`chain-length = <36>` consisting of 6 underglow + 30 per-key LEDs per half), we map the physical matrix keys to LED strip indices.
We define an array `bool is_key_mapped[NUM_LAYERS][30]` for the keys of the left and right halves on each layer.

### 3.3 Driving the LED Strip
When a layer changes, if underglow is enabled, we update the led strip's colors. For each key:
* If the key is mapped on the current layer, light up with the designated color.
* Else, set to `RGB(0, 0, 0)`.

## 4. Verification Plan
* Ensure `config/boards/shields/sofle/src/rgb_layer_colors.c` compiles without any errors.
* Verify that build outputs binary successfully in GitHub Actions.
