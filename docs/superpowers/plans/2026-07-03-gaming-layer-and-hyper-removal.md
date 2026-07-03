# Gaming Layer and Hyper Key Removal Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the home-row `hyper` key with a standard return key (`&kp RET`), remove the `hyper` macro, define a low-latency `GAMING` layer on layer index 7, configure toggle triggers on the `MEDIA` and `GAMING` layers, and deactivate combos on the `GAMING` layer.

**Architecture:** We will modify the ZMK Devicetree configuration in `config/sofle.keymap`. By setting `#define GAMING 7` and mapping keys to standard behaviors with zero tap-holds or macros, we create a latency-free gaming layer. Combos are explicitly restricted to layers 0-6 to prevent accidental activations during gameplay.

**Tech Stack:** ZMK Firmware, Devicetree (dts), Keymap Configuration.

## Global Constraints
* Maintain strict devicetree syntax to prevent compiler aborts.
* Do not stage or commit any temporary agent or superpower files; only commit `config/sofle.keymap` and the design/plan documentation files.

---

### Task 1: Remove Hyper Macro and Add Return Key

**Files:**
- Modify: `config/sofle.keymap:165-174`, `config/sofle.keymap:193-193`

**Interfaces:**
- Consumes: Existing `config/sofle.keymap`
- Produces: Updated `BASE` layer mapping `&kp RET` on the home-row pinky position; complete removal of the `hyper` macro block.

- [ ] **Step 1: Edit the keymap to remove the `hyper` macro and replace `&hyper` with `&kp RET`**

Apply the following modifications to `config/sofle.keymap`:
1. Remove the entire `hyper: hyper { ... };` macro inside `macros`.
2. Update the `base_layer`'s 3rd row binding to replace `&hyper` with `&kp RET`.

```dts
// Remove:
    macros {
        hyper: hyper {
            compatible = "zmk,behavior-macro";
            #binding-cells = <0>;
            bindings =
                <&macro_press &kp LGUI &kp LALT &kp LCTRL &kp LSHFT>,
                <&macro_pause_for_release>,
                <&macro_release &kp LGUI &kp LALT &kp LCTRL &kp LSHFT>;
        };
    };

// Replace BASE layer line 193:
&mt LCTRL ESC  &mt LGUI A  &mt LALT S  &mt LCTRL D   &mt LSHFT F   &kp G                                        &kp H       &mt RSHFT J   &mt RCTRL K   &mt RALT L   &mt RGUI SQT &kp RET
```

- [ ] **Step 2: Commit Task 1 changes**

Run:
```bash
git add config/sofle.keymap
git commit -m "feat: replace hyper key with return and remove hyper macro"
```

---

### Task 2: Configure GAMING Layer Definitions and Media Layer Toggle

**Files:**
- Modify: `config/sofle.keymap:23-32` (layers definition block), `config/sofle.keymap:223-233` (media layer block)

**Interfaces:**
- Consumes: Task 1 output
- Produces: `GAMING` layer index definition; toggle key `&tog GAMING` placed at the left home-row inner-index position (G key) on the `MEDIA` layer.

- [ ] **Step 1: Add `#define GAMING 7` to layer definitions**

Add the definition:
```dts
#define BASE  0
#define NAV   1
#define NUM   2
#define MEDIA 3
#define SYM   4
#define FUN   5
#define MOUSE 6
#define GAMING 7
```

- [ ] **Step 2: Update the `media_layer` to place `&tog GAMING`**

Replace the left-hand 3rd row (home row) in `media_layer` to map `&tog GAMING` to the inner-index position:
```dts
&none          &kp LGUI    &kp LALT    &kp LCTRL     &kp LSHFT     &tog GAMING                                  &ext_power EP_TOG &kp C_PREV   &kp C_VOL_DN   &kp C_VOL_UP &kp C_NEXT   &none
```

- [ ] **Step 3: Commit Task 2 changes**

Run:
```bash
git add config/sofle.keymap
git commit -m "feat: define GAMING layer index and add toggle key to media layer"
```

---

### Task 3: Restrict Combos to Layers 0-6

**Files:**
- Modify: `config/sofle.keymap:55-163` (combos section)

**Interfaces:**
- Consumes: Task 2 output
- Produces: Combos modified with `layers = <0 1 2 3 4 5 6>;` properties so that they are deactivated in the `GAMING` layer.

- [ ] **Step 1: Add `layers` restriction to all combo definitions**

For every combo defined in the `combos` block, add:
```dts
layers = <0 1 2 3 4 5 6>;
```
Example:
```dts
        combo_caps_word {
            bindings = <&caps_word>;
            key-positions = <28 31>;        // F + J (index home row)
            timeout-ms = <30>;
            require-prior-idle-ms = <150>;
            layers = <0 1 2 3 4 5 6>;
        };
```
Repeat this for `combo_esc`, `combo_bspc`, `combo_del`, `combo_tab`, `combo_enter`, `combo_copy`, `combo_paste`, `combo_cut`, `combo_minus`, `combo_underscore`, `combo_equal`, `combo_grave`, `combo_semi`.

- [ ] **Step 2: Commit Task 3 changes**

Run:
```bash
git add config/sofle.keymap
git commit -m "feat: restrict combos to layers 0-6 to prevent gaming interference"
```

---

### Task 4: Implement GAMING Layer Keymap

**Files:**
- Modify: `config/sofle.keymap` (add GAMING layer to keymap list)

**Interfaces:**
- Consumes: Task 3 output
- Produces: Complete `gaming_layer` definition at the end of the `keymap` block.

- [ ] **Step 1: Append the `gaming_layer` to the keymap block**

At the end of the `keymap` block, add:
```dts
        gaming_layer {
            display-name = "GAMING";
            bindings = <
&kp GRAVE      &kp N1      &kp N2      &kp N3        &kp N4        &kp N5                                       &trans      &trans        &trans        &trans       &trans       &trans
&kp TAB        &kp Q       &kp W       &kp E         &kp R         &kp T                                        &trans      &trans        &trans        &trans       &trans       &trans
&kp LCTRL      &kp A       &kp S       &kp D         &kp F         &kp G                                        &trans      &trans        &trans        &trans       &trans       &trans
&kp LSHFT      &kp Z       &kp X       &kp C         &kp V         &kp B          &kp C_MUTE     &trans         &trans      &trans        &trans        &trans       &trans       &trans
                                       &kp LGUI      &kp LALT      &kp ESC        &kp SPACE      &tog GAMING    &trans      &trans        &trans        &trans       &trans
            >;
        };
```

- [ ] **Step 2: Commit Task 4 changes**

Run:
```bash
git add config/sofle.keymap
git commit -m "feat: add dedicated gaming layer with low latency bindings"
```
