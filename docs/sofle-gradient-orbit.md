# Sofle Choc Pro BT: static gradient + right-screen orbit

This matched pair builds on the working lilac firmware. The left keypress display,
key bindings, encoders, Studio, split connection and computer pairings are retained.
Neither image includes diagnostic or pairing-repair code.

| Half | Firmware | Screen |
|---|---|---|
| Left | `sofle_choc_pro_left_gradient.uf2` | Latest key from either half, battery, connection and layer |
| Right | `sofle_choc_pro_right_gradient_orbit.uf2` | Planet with an orbiting moon, battery and connection |

Both halves have a **static lilac-to-pink gradient**. The moon animation is only
on the right LCD: 32 frames, four frames/second, one revolution every eight seconds.
It pauses at idle/sleep and resumes on activity. It neither records nor transmits
key information. The static right artwork remains available in the solid-lilac
build as a fallback.

## Flash both halves on Linux

1. Extract `sofle-gradient-orbit-matched.zip`. Optionally check both files with
   `sha256sum -c SHA256SUMS` in the extracted directory.
2. Unplug both halves and switch both off.
3. Connect **right** by USB, double-press its recessed reset button underneath by
   USB, and copy **only** `sofle_choc_pro_right_gradient_orbit.uf2` onto **KEEBART**.
   Wait for automatic reboot, then unplug it and leave its battery switch off.
4. Connect **left**, double-press reset, and copy **only**
   `sofle_choc_pro_left_gradient.uf2` onto KEEBART. Wait for reboot.
5. Turn both on. Leave USB connected only to the left. Wait ten seconds.
6. Confirm Q and J both type and appear on the left screen. Check the static
   lighting gradient on both halves, moving moon and status icons on the right,
   both encoders, and the already paired Mac profile.
7. After five minutes without activity the right animation should stop and RGB
   should turn off; pressing a right-side key should wake both on that half.
   Right activity follows ZMK's peripheral activity state, not left-only typing.

If using a terminal, substitute the actual paths:

```sh
cp -- /path/to/correct-half.uf2 /media/"$USER"/KEEBART/
sync
```

No settings reset, Bluetooth clearing or forgetting the Mac pairing is needed.
For rollback, reflash both files from the saved `sofle-lilac-matched.zip`, or use
that package from the same run. It restores solid lilac and the original right
artwork while preserving pairings.

## Lighting and implementation

The extra `sofle_gradient` shield redirects ZMK's existing RGB output through a
small LED-strip filter and then the original WS2812 driver. No upstream ZMK
patch/fork, SPI pin change, extra LED timer, or concurrent strip writer is used.
The filter adds 0..40 degrees of hue across each 30-LED chain. The existing startup
colour H280/S35/B70 therefore ranges from lilac to pink. The existing 30% board
brightness cap is unchanged; output channel maxima never exceed incoming ones.
Black remains black, so RGB-off and idle power saving continue to work. Brightness,
hue and saturation controls still work; selecting another effect explicitly can
animate the lights, but startup selects a stationary gradient.

The gradient follows each half's **LED wiring order**. The published board files
provide no physical LED-coordinate map, so this does not claim an exact geometric
left-to-right sweep across key rows or the gap between halves. The palette is the
same on both halves. Physical colours depend on LEDs, switches and keycaps.

`CONFIG_SOFLE_RIGHT_ANIMATION` is independently configurable and right-only.
The animation draws one 68x68 canvas on the existing LVGL display queue, bounded
to 4 Hz. Battery/connection occupy a separate canvas. Activity events only enqueue
work; there is no blocking in the input path. The working left display code and
all existing keymaps and radio settings are unchanged.

## Validation and limits

CI builds both correct Sofle targets from the same source commit and checks UF2
application bounds. Pure-C gradient tests cover colour progression and brightness/
black preservation across 122,880 colour-position samples. A test using the pinned
LVGL implementation renders all 32 frames, verifies the loop and status isolation,
and exercises idle/sleep/wake with a simulated work queue. Existing key-display,
lighting-startup and settings-preservation regression tests also run.

Automated tests and compilation do not establish physical orientation, perceived
LED colour, radio reliability, battery drain or real-device latency. No hardware
is automatically flashed. The previous solid-lilac pair was user-confirmed working;
this new visual variant still requires the manual checks above.
