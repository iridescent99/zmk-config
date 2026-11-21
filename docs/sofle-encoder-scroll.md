# Sofle right encoder: browser and terminal scrolling

This optional matched pair is based on the LaTeX/display firmware from source
`34a48dc75fa4e3a81d1629fbbafe182b3fe339a5`. It keeps the LaTeX layers, layer/key
display, blue-to-lilac LEDs and right-screen starfield. The single physical
encoder on the right emits ordinary vertical mouse-wheel events:

- Clockwise: scroll down.
- Counter-clockwise: scroll up.
- Encoder press: retains the existing runtime key assignment.

The firmware continues to define both sensor positions because ZMK's split
sensor indices are shared by both halves. The right sensor binding is replaced;
an absent left encoder does not require removing or renumbering either sensor.

The scroll binding is active in Base, Lower, Raise and both LaTeX layers. Higher
layers without an explicit encoder mapping fall through to those bindings.
Ordinary firmware artifacts retain their original media/volume bindings; only
the two `*_scroll.uf2` images enable the `sofle_scroll` shield.

## Files and installation

Download the `sofle-right-encoder-scroll` artifact from this branch's successful
**Build ZMK firmware** run. The ZIP contains:

| Half | Firmware |
| --- | --- |
| Left/central | `sofle_choc_pro_left_scroll.uf2` |
| Right/peripheral | `sofle_choc_pro_right_scroll.uf2` |

Keep the current working firmware ZIP for rollback. Extract the new artifact and
run `sha256sum -c SHA256SUMS` inside the extracted directory.

1. Switch both halves off and unplug USB.
2. Connect **right** by USB and double-press its recessed reset button to mount
   the **KEEBART** bootloader drive. Copy only `sofle_choc_pro_right_scroll.uf2`
   onto it. Wait for reboot, unplug it and leave it switched off.
3. Connect **left**, enter its bootloader the same way, and copy only
   `sofle_choc_pro_left_scroll.uf2`. Wait for reboot.
4. Switch both on. For the first test, keep USB connected to **left** and wait
   ten seconds for the split connection.

These application images do not contain reset/repair code or writes to the
settings/bootloader partitions. Do not restore the stock Studio keymap as part
of this change: that would discard saved runtime typing assignments. Encoder
rotation is compiled separately from the Studio-editable key positions.

Mouse support adds a mouse HID report. USB must re-enumerate after flashing.
Bluetooth hosts may cache the old HID descriptor and need the refresh procedure
described in [ZMK's mouse-emulation documentation](https://zmk.dev/docs/keymaps/behaviors/mouse-emulation).
Start with USB so host Bluetooth caching cannot obscure the first test.

## Use and physical verification

In the browser, put the pointer over the page or desired scrollable panel and
rotate the knob. In Kitty/Zellij, put it over the target pane. The review profile
leaves Zellij mouse handling enabled; Alt+s still enters explicit scrollback
mode, and Esc leaves it. Neovim also enables mouse input in the review profile.

Verify both directions, a single click, quick rotation and stopping immediately.
Check normal typing, the LaTeX toggle, display labels and both halves after the
update. Build and host checks cannot establish physical direction, detent
resolution, host scroll speed or transport reliability. If direction is opposite
to preference, swap the two `MOVE_Y` bindings in `sofle_scroll.overlay` and rebuild.

## Design and validation

The optional shield enables `CONFIG_ZMK_POINTING` and gives the stock ZMK
two-axis input driver a dedicated scroll instance, bound through a sensor
rotation behavior. It uses 125 wheel units/second,
an 8 ms report interval and a 12 ms press/release: nominally one wheel report
per detent, with a short behavior queue. Scheduler timing and host wheel settings
still need physical validation. High-resolution/smooth scrolling is left off.

Only the `sofle_scroll` build changes the mouse timing. No new C behavior,
desktop remapper, input-device access rule or continuous background service is
needed. Existing original artifacts remain usable for rollback.

Host validation preprocesses both source keymaps with the optional shield on
and off, verifies every explicit encoder layer, and proves that the typing and
macro bindings are identical between variants. Existing LaTeX/display checks
also run. CI compiles both actual board targets and validates that the delivered
UF2s write only the application partition.

Sources: [ZMK sensor rotation](https://zmk.dev/docs/keymaps/behaviors/sensor-rotate),
[mouse emulation](https://zmk.dev/docs/keymaps/behaviors/mouse-emulation),
[Studio capabilities](https://zmk.dev/docs/features/studio).
