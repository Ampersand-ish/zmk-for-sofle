# Spec: Gaming Layer and Hyper Key Removal for Sofle Keyboard
**Date:** 2026-07-03

## 1. Objectives
* Remove the `hyper` macro and replace the `&hyper` key binding on the `BASE` layer with standard return (`&kp RET`).
* Add a dedicated, low-latency `GAMING` layer to allow gameplay with no modifier tap-holds, macro delays, or accidental combo activations.
* Provide easy-to-use toggle mechanisms to enter and exit the `GAMING` layer.

## 2. Requirements & Changes

### 2.1 Hyper Key Removal
* **Keymap:** Replace `&hyper` (outer right home-row key, keypos 35) with `&kp RET` (Return).
* **Macros:** Delete the `hyper` macro block entirely from `config/sofle.keymap` to keep the file clean.

### 2.2 GAMING Layer Toggle
* **To Enter:** Add a toggle key `&tog GAMING` on the `MEDIA` layer at the left home-row inner-index position (corresponds to `G` on `BASE`).
* **To Exit:** Place a toggle key `&tog GAMING` on the `GAMING` layer itself on the left thumb cluster, replacing the `&kp TAB` (outermost thumb position next to Space).

### 2.3 GAMING Layer Layout Configuration
* Define `#define GAMING 7` at the top of the keymap.
* Left-hand keys are configured as standard, direct-press keys with zero tap-hold modifiers or layer triggers:
  * **Top Row:** `&kp GRAVE` | `&kp N1` | `&kp N2` | `&kp N3` | `&kp N4` | `&kp N5`
  * **Upper Row:** `&kp TAB` | `&kp Q` | `&kp W` | `&kp E` | `&kp R` | `&kp T`
  * **Home Row:** `&kp LCTRL` | `&kp A` | `&kp S` | `&kp D` | `&kp F` | `&kp G`
  * **Bottom Row:** `&kp LSHIFT` | `&kp Z` | `&kp X` | `&kp C` | `&kp V` | `&kp B` | `&kp C_MUTE`
  * **Thumbs:** `&kp LGUI` | `&kp LALT` | `&kp ESC` | `&kp SPACE` | `&tog GAMING`
* Right-hand keys and all other keys are set to `&trans` so they fall back to the `BASE` layer.

### 2.4 Combo Deactivation in Gaming Layer
To prevent accidental combo triggers during fast game inputs, restrict all existing combos in `config/sofle.keymap` to only run on layers 0 through 6:
* `layers = <0 1 2 3 4 5 6>;`

## 3. Verification Plan
* Validate syntax of `config/sofle.keymap`.
* Verify that the local or CI ZMK build completes successfully.
