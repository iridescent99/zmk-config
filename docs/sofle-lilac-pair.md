# Sofle Choc Pro BT: matched lilac firmware

Use both files from this ZIP. Both are built from the commit in SOURCE_COMMIT.txt:

| Half | File | Screen |
|---|---|---|
| Left (USB/central) | `sofle_choc_pro_left_display_lilac.uf2` | Large latest key, battery, connection and layer |
| Right (wireless/peripheral) | `sofle_choc_pro_right_lilac.uf2` | Original artwork and connection indicator |

Both halves start with solid lilac lighting. This replaces saved RGB-off/colour
preferences at boot and enables LED power. Colour is HSV 280°, 35%, 70%; ZMK
scales brightness by the existing board cap of 30% (maximum channel duty 21%).
Physical colour depends on the LEDs, switches and keycaps. Normal lighting
controls and five-minute idle shutdown still work. A reboot restores lilac.

Neither image includes split-repair or diagnostic code. No `FIX DONE`, `KEY 0/1`,
or security diagnostic text remains. There is no bond/settings reset: the
recently repaired split pairing and existing computer pairings are preserved.
Studio, encoders, key bindings, Bluetooth security, USB and split transport are
unchanged. Only RGB/power preferences are intentionally updated.

## Flash both on Linux

1. Extract this ZIP. Optionally run `sha256sum -c SHA256SUMS` inside it.
2. Switch both halves off and disconnect their USB cables.
3. Connect the right half to Linux by USB. Double-press its recessed reset button
   underneath near USB. Copy **only** `sofle_choc_pro_right_lilac.uf2` onto the
   **KEEBART** drive. Wait for automatic reboot, then unplug it and leave it off.
4. Connect the left half, double-press reset, and copy **only**
   `sofle_choc_pro_left_display_lilac.uf2` onto KEEBART. Wait for reboot.
5. Turn both halves on. Leave USB connected only to the left. Wait ten seconds.
6. Press Q and then J in a text editor: both should type, with the newest letter
   visible on the left. Hold a key: it should stay visible until one second after
   release. Check lilac lighting on both halves and turn/click both encoders.
7. Check typing through the already paired Mac profile without forgetting it.

If using a terminal, replace the source and mount paths with the actual paths:

```sh
cp -- /path/to/correct-half.uf2 /media/"$USER"/KEEBART/
sync
```

The bootloader normally ejects itself after accepting the file. Double-pressing
reset enters the bootloader; it does not clear pairing settings. Do not use a
settings-reset image or the earlier split-repair package for this update.

## Source and verification

Feature branch: `feature/sofle-live-keypress-display`, repository
https://github.com/iridescent99/zmk-config . Targets `sofle_choc_pro_left` and
`sofle_choc_pro_right`, shield `sharp_mip`, original Keebart ZMK v0.3 build flow.
The optional `CONFIG_SOFLE_LILAC_LIGHTING` settings-commit callback applies the
colour through public ZMK APIs after persisted settings load; it never accesses
Bluetooth or Studio settings. `CONFIG_SOFLE_KEYPRESS_DISPLAY` is left-only.

Automated tests cover the key state machine, actual renderer with pinned LVGL
and a simulated display/clock, and the lighting callback including saved-off and
failure paths. CI builds both targets and checks UF2 application bounds. These
checks do not constitute physical keyboard testing or measurement of LED colour,
radio behaviour, input latency or battery consumption.

To return to the original screens and ordinary saved lighting behaviour, use
`sofle_choc_pro_bt_left.uf2` and `sofle_choc_pro_bt_right.uf2` from the same run's
full `firmware` artifact. Normal application flashing retains pairings; the last
saved lilac lighting preference remains adjustable through the stock bindings.
