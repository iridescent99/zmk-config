# Sofle Choc Pro BT — full-board gradient and detailed starfield

A matched pair for the **58-key Keebart Sofle Choc Pro BT**, built from one commit.
The left keypress visualiser, encoders, Studio, keymap, split communication and
computer pairings remain unchanged. There is no repair/reset code in this pair.

| Half | File | Screen |
|---|---|---|
| Left | `sofle_choc_pro_left_full_gradient.uf2` | Latest key from either half, layer and status |
| Right | `sofle_choc_pro_right_full_gradient_starfield.uf2` | Detailed drifting starfield, battery and connection |

## Appearance

The lighting forms one stationary **lavender → lilac → pink** progression from
outer left to outer right. Keys in the same physical column have the same colour;
the right half continues the gradient. Thumb keys use their own horizontal
positions. Palette anchors approximate the supplied reference: `#c4a8f0`,
`#dd79e8`, `#e34eb8`. Actual LED colour depends on switches and keycaps.

The right screen uses all 140×68 pixels below its 20-pixel status header:
72 stars at three depths, a fine 85-point galactic ribbon, individually phased
twinkling and two staggered shooting stars with tapered dust trails. Near stars
move faster than distant ones. The monochrome scene runs at 8 frames/second,
pauses on idle/sleep and resumes on right-half activity. It does not react to,
store or transmit keys. The left typing display is unchanged.

## Flash both halves on Linux

1. Extract this ZIP; in its directory run `sha256sum -c SHA256SUMS`.
2. Unplug both halves and switch both off.
3. Connect **right** by USB, double-press its recessed reset button underneath
   near USB, and copy **only** `sofle_choc_pro_right_full_gradient_starfield.uf2`
   onto the **KEEBART** drive. Wait for reboot, unplug it and leave it off.
4. Connect **left**, double-press reset and copy **only**
   `sofle_choc_pro_left_full_gradient.uf2` onto KEEBART. Wait for reboot.
5. Turn both on; leave USB connected to the **left**. Wait ten seconds.

For terminal copying, substitute the mount path shown by your file manager:

```sh
cp -- sofle_choc_pro_right_full_gradient_starfield.uf2 /media/"$USER"/KEEBART/
sync
# After connecting the left half and entering its bootloader:
cp -- sofle_choc_pro_left_full_gradient.uf2 /media/"$USER"/KEEBART/
sync
```

Do not flash `settings_reset`, diagnostics or split-repair images. Do not clear
Bluetooth profiles or forget the Mac pairing. Ordinary UF2 flashing replaces
only application flash. The existing lilac startup code updates RGB/power
preferences only; it does not change identity, bonds or Studio settings.

## Manual checks

- Q and J both type normally and both appear on the left display.
- Compare colours vertically: Q/A/Z should match; U/J/M should match. The
  outer left should be pale lavender and outer right pink. Inner thumb colours
  should meet smoothly. Report the row/keys if any colour jumps backwards.
- Right stars drift smoothly, occasional meteors pass, and battery/connection
  icons remain visible when status changes. Test both encoders and the Mac.
- After five minutes idle, RGB turns off and animation stops; a right key wakes
  that half. Activity follows ZMK's existing peripheral activity state.

## Implementation and limitations

The `sofle_gradient` shield keeps the original WS2812 driver and ZMK's sole RGB
writer. `CONFIG_SOFLE_PHYSICAL_GRADIENT` maps 30 local LEDs to global X coordinates
and interpolates a fixed palette. The geometry is derived from Keebart's
[Bluetooth-model table](https://github.com/Keebart/zmk-config/blob/19d7f37d8339bbd9305ddfb1ddfb49b16ec7f8cd/studio/coordinates_sofle_bt.h).
**Keebart labels that LED order provisional.** Automated tests verify the exact
mapping, but the physical wiring/orientation still needs the visual check above.
The geometry assumes the standard typing arrangement, regardless of desk gap or
rotation. No runtime lighting transport or Studio-fork code was introduced.

Brightness, power toggle and idle-off work. The board's existing 30% maximum
brightness remains; no output channel exceeds ZMK's incoming peak. This variant
fixes hue and saturation to the requested palette, so hue/saturation keys no
longer recolour the LEDs. Other effects can alter brightness; boot restores the
stationary palette. Both halves use the same constants, with no clock or BLE
synchronisation required for a static gradient.

`CONFIG_SOFLE_RIGHT_STARFIELD` is independent and peripheral-only. It renders
into one 9,520-byte static canvas with bounded integer arithmetic, no allocation,
no floating point and one invalidation per frame. All animation and activity
updates execute on ZMK's dedicated display queue. No input listeners or radio
changes are involved. The original artwork, solid-lilac and earlier orbit builds
remain separately available.

Host tests cover all 60 LED positions, same-column equality, the split boundary,
brightness/off at every level, 2,000 starfield frames in both orientations,
buffer bounds and counter overflow. Pinned-LVGL integration tests cover rendering,
header isolation, idle/sleep/wake and the existing left display. CI compiles both
correct Sofle board targets with `sharp_mip sofle_gradient`, and checks the two
UF2 files against application-partition boundaries. Build checks do not establish
physical BLE reliability, latency, battery life, perceived colour or LCD smoothness.
No hardware has been automatically flashed or tested.

## Restore the known-working pair

Reflash both files from the saved `sofle-lilac-matched.zip` with the same procedure:
`sofle_choc_pro_right_lilac.uf2` on right, then
`sofle_choc_pro_left_display_lilac.uf2` on left. This restores solid lilac and the
original right artwork while retaining the left visualiser and pairings.
The user-confirmed pair is from commit
`f86b31f2bf697324fdfe49184f79bfa6cfd3dd75`,
[build 43884438640](https://github.com/iridescent99/zmk-config/actions/runs/43884438640/artifacts/14234945847).
