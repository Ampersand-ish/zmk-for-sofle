# Design Spec: Autoshift Integration for Combos and Base Layer Symbols & Numbers
**Date:** 2026-07-19

## 1. Objectives
* Add an intuitive and low-latency Autoshift capability to the Sofle keyboard's vertical symbol combos (Tier 3), allowing tap actions to output standard symbols and hold actions to output their shifted counterparts.
* Consolidate the symbol combos by removing the redundant `combo_underscore` combo, since `combo_minus` with Autoshift will now cover both minus (`-`) and underscore (`_`).
* Integrate Autoshift into all symbol/punctuation and numeric keys on the `BASE` layer so that holding any base layer symbol or number key outputs its shifted version (including numbers shifting to their corresponding symbol characters, e.g., holding `1` outputs `!`).
* Replace the modifier-tap `&mt RGUI SQT` key on the right-hand home row with an autoshifted quote key `&as DQT SQT` (tap for `'`, hold for `"`), prioritizing convenient quote punctuation over the right GUI modifier.

## 2. Approach
Since ZMK does not feature a built-in native "Auto Shift" behavior, we implement this as a custom **Hold-Tap** behavior:
1. **Behavior Definition:** Add a custom `auto_shift` behavior with the alias `as`. It utilizes the `tap-preferred` flavor so that rapid typing does not trigger accidental shifted characters, and a `quick-tap-ms` of `0` to prevent accidental hold triggers on double-tap.
2. **Combos Optimization:** Apply the `&as` behavior to the Tier 3 vertical symbol combos. Remove the `combo_underscore` definition completely from `config/sofle.keymap` to keep the keymap clean and optimized.
3. **Base Layer Mapping:** Replace all standard punctuation/symbol keys and all numeric keys on the `BASE` layer with their autoshifted equivalents using `&as`.

## 3. Detailed Specifications

### 3.1 Behavior Configuration
Add the custom hold-tap configuration to the `behaviors` block of `config/sofle.keymap`:
```devicetree
        as: auto_shift {
            compatible = "zmk,behavior-hold-tap";
            label = "AUTO_SHIFT";
            #binding-cells = <2>;
            tapping-term-ms = <200>;
            quick-tap-ms = <0>;
            flavor = "tap-preferred";
            bindings = <&kp>, <&kp>;
        };
```

### 3.2 Combos Configuration (Tier 3: Vertical Symbol Combos)
Update the symbol combos in the `combos` block of `config/sofle.keymap` by removing `combo_underscore` and using `&as` bindings on the rest:
```devicetree
        // -- Tier 3: Vertical Symbol Combos --

        combo_minus {
            bindings = <&as UNDER MINUS>;
            key-positions = <31 45>;        // J + M (right index vertical)
            timeout-ms = <40>;
            require-prior-idle-ms = <100>;
            layers = <0 1 2 3 4 5 6>;
        };

        combo_equal {
            bindings = <&as PLUS EQUAL>;
            key-positions = <28 40>;        // F + V (left index vertical)
            timeout-ms = <40>;
            require-prior-idle-ms = <100>;
            layers = <0 1 2 3 4 5 6>;
        };

        combo_grave {
            bindings = <&as TILDE GRAVE>;
            key-positions = <26 38>;        // S + X (left ring vertical)
            timeout-ms = <40>;
            require-prior-idle-ms = <100>;
            layers = <0 1 2 3 4 5 6>;
        };

        combo_semi {
            bindings = <&as COLON SEMI>;
            key-positions = <33 34>;        // L + ' (right ring+pinky home)
            timeout-ms = <30>;
            require-prior-idle-ms = <150>;
            layers = <0 1 2 3 4 5 6>;
        };
```

### 3.3 Base Layer Configuration
Replace the top row, home-row quote key, and bottom-row symbol keys in the `base_layer` of the `keymap` block in `config/sofle.keymap` with their corresponding `as` definitions:
```devicetree
        base_layer {
            display-name = "BASE";
            bindings = <
&as TILDE GRAVE &as EXCL N1 &as AT N2 &as HASH N3   &as DLLR N4   &as PRCNT N5                                 &as CARET N6 &as AMPS N7   &as ASTRK N8  &as LPAR N9  &as RPAR N0  &as PLUS EQUAL
&kp TAB         &kp Q       &kp W     &kp E         &kp R         &kp T                                        &kp Y        &kp U         &kp I         &kp O        &kp P        &kp BSPC
&mt LCTRL ESC   &mt LGUI A  &mt LALT S &mt LCTRL D  &mt LSHFT F   &kp G                                        &kp H        &mt RSHFT J   &mt RCTRL K   &mt RALT L   &as DQT SQT  &kp RET
&kp LSHFT       &kp Z       &kp X     &kp C         &kp V         &kp B         &kp C_MUTE     &kp DEL         &kp N        &kp M         &as LT COMMA  &as GT DOT   &as QMARK FSLH &kp ESC
                                      &kp LGUI      &kp LALT      &lt MEDIA ESC &lt NAV SPACE  &lt MOUSE TAB   &lt SYM SPACE &lt NUM BSPC &lt FUN DEL   &kp RALT     &kp RGUI
            >;

            sensor-bindings = <&inc_dec_kp C_VOL_DN C_VOL_UP &inc_dec_kp PG_UP PG_DN>;
        };
```

## 4. Verification Plan
1. **DeviceTree Syntax Verification:** Check that `config/sofle.keymap` compiles successfully using the build script or Makefile.
2. **Visual Inspection:** Verify that all key bindings compile cleanly and that the keymap layout matches standard expected mappings.
