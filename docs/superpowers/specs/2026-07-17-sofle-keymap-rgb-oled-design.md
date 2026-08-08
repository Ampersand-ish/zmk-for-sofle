# Design Spec: Custom LED Strip Wrapper, OLED Re-init, and Brightness Module Integration for Sofle Keyboard
**Date:** 2026-07-17

## 1. Objectives
* Implement robust, flicker-free per-key layer-specific RGB backlighting that synchronizes flawlessly across both left and right halves on standard upstream ZMK.
* Implement automatic OLED display re-initialization and recovery when external power is toggled back on (`EP_TOG` / `EP_ON`).
* Integrate the custom `zmk-oled-brightness` module and bind its brightness, contrast, and power controls to the MEDIA layer.
* Change the right-half thumb return key to space: `&lt SYM SPACE`.

## 2. Approach: Custom LED Strip Wrapper (Approach 1)
To prevent standard ZMK underglow animations/states from competing with or overwriting our layer colors, and to sync layer colors to the peripheral half without core ZMK modifications:
1. **Virtual Wrapper Device:** We configure ZMK's underglow chosen node `zmk,underglow` to point to a custom virtual `zmk,custom-led-strip` driver instead of the physical `ws2812` driver.
2. **Layer Synchronization:** On the central half, layer transitions trigger a listener that sets ZMK's global underglow HSB color to a specific unique Hue mapping. This Hue is natively synchronized by ZMK to the peripheral half over BLE.
3. **Write Interception:** On both halves, when ZMK's underglow driver writes to `zmk,underglow`, our custom wrapper intercepts the write, decodes the Hue back to the active layer, constructs the correct per-key isolated LED array, and writes it directly to the physical `ws2812` strip.
4. **Fallback:** If our wrapper receives an active animation or a color not matching the layer mappings, it passes the pixels through unmodified, maintaining compatibility with standard underglow effects.

## 3. Detailed Specifications

### 3.1 Devicetree & Driver

#### Devicetree YAML Binding (`config/dts/bindings/zmk,custom-led-strip.yaml`)
```yaml
description: Custom LED strip wrapper for layer-specific lighting

compatible: "zmk,custom-led-strip"

include: led-strip.yaml

properties:
  real-strip:
    type: phandle
    required: true
```

#### Devicetree Overlay (`config/boards/shields/sofle/boards/nice_nano_nrf52840_zmk.overlay`)
```dts
/ {
    chosen {
        zmk,underglow = &custom_strip;
    };

    custom_strip: custom_strip {
        compatible = "zmk,custom-led-strip";
        real-strip = <&led_strip>;
        chain-length = <36>;
        status = "okay";
    };
};
```

#### Power Domain for OLED SSD1306 (`config/boards/shields/sofle/sofle.dtsi`)
Adding the `power-domain = <&ext_power>;` node ensures modern ZMK power management automatically suspends and re-initializes the SSD1306 display cleanly on external power toggle events.
```dts
&pro_micro_i2c {
    status = "okay";

    oled: ssd1306@3c {
        compatible = "solomon,ssd1306fb";
        reg = <0x3c>;
        width = <128>;
        height = <32>;
        segment-offset = <0>;
        page-offset = <0>;
        display-offset = <0>;
        multiplex-ratio = <31>;
        segment-remap;
        com-invdir;
        com-sequential;
        inversion-on;
        prechargep = <0x22>;
        power-domain = <&ext_power>; // Re-init on EP_TOG!
    };
};
```

### 3.2 West Manifest Module (`config/west.yml`)
We add `zmk-oled-brightness` under `projects`:
```yaml
    - name: zmk-oled-brightness
      url: https://github.com/mctechnology17/zmk-oled-brightness
      revision: main
```

### 3.3 Keyboard Config (`config/sofle.conf`)
Enable the OLED brightness module and behaviors:
```kconfig
# Enable OLED brightness and contrast behaviors
CONFIG_ZMK_OLED_BRIGHTNESS=y
CONFIG_ZMK_OLED_BRIGHTNESS_BEHAVIORS=y
```

### 3.4 Keymap Updates (`config/sofle.keymap`)
* Change the thumb return key in `base_layer` from `&lt SYM RET` to `&lt SYM SPACE`.
* Include the OLED brightness header:
  ```dts
  #include <dt-bindings/zmk-oled-brightness/oled-brightness.h>
  ```
* Bind behaviors to the MEDIA layer (right-half top row):
  ```dts
  &ol_cons ZMK_OLED_CONTRAST_DOWN 16 &ol_cons ZMK_OLED_CONTRAST_UP 16 &ol_cond &ol_brts ZMK_OLED_BRIGHTNESS_DOWN 10 &ol_brts ZMK_OLED_BRIGHTNESS_UP 10 &ol_pwr ZMK_OLED_POWER_TOGGLE
  ```

### 3.5 Custom Wrapper Driver Code (`config/boards/shields/sofle/src/rgb_layer_colors.c`)
The driver implements:
1. `DT_DRV_COMPAT` custom led-strip initialization.
2. An `update_rgb` hook that decodes Hue into layer numbers:
   * Layer 0 (BASE): H=0, S=0 (Grayscale)
   * Layer 1 (NAV): H=220
   * Layer 2 (NUM): H=60
   * Layer 3 (MEDIA): H=120
   * Layer 4 (SYM): H=350
   * Layer 5 (FUN): H=45
   * Layer 6 (MOUSE): H=0, S=0, B=0 (All Dark)
   * Layer 7 (GAMING): H=20
3. A central-only ZMK listener for `zmk_layer_state_changed` that updates the underglow HSB state to match the highest active layer.

## 4. Verification Plan
* Compile the project locally or via GitHub Actions.
* Verify clean compilation on both `sofle_left` (central) and `sofle_right` (peripheral) builds.
* Verify the OLED display successfully resumes and turns on after toggling external power off and on again.
