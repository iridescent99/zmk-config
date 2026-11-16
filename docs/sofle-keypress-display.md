# Sofle Choc Pro BT: live key display

Hardware: **58-key Keebart Sofle Choc Pro BT**, central board
`sofle_choc_pro_left`, shield `sharp_mip`. This feature is left-only.
Do not flash another keyboard's firmware or any `settings_reset` image.

## Files to use

- New display: `sofle_choc_pro_left_keypress.uf2`.
- Original display / rollback: `sofle_choc_pro_bt_left.uf2` from the same build.
- Leave the right half on its existing firmware. Its artwork, connection indicator,
  encoder bindings and split protocol have not been modified.

The branch is based on Keebart commit
`98c5a1181ff7db1f480f684f714126922d55b8f0`, with Keebart's ZMK `v0.3` manifest
and reusable build workflow. The upstream keymap, shared Sofle configuration,
board definitions, encoder resolution, Studio, USB, Bluetooth and settings
partitions are unchanged. Saved Studio keymaps take precedence as usual.
The version currently installed on your keyboard cannot be established from
source code; left-only split compatibility must therefore be checked on-device.

## Display

The screen retains Keebart's portrait orientation (the driver framebuffer is
160 x 68; its upright UI is 68 x 160). It shows:

- Battery percentage/charging icon, `USB` or Bluetooth profile and state.
- Active layer, obtained from ZMK rather than inferred from physical positions.
- Latest resolved key, with 48-, 26- or 16-pixel font chosen to fit its label.
- Currently held CTRL, ALT, GUI and SHFT indicators. A modifier pressed by itself
  uses the full label `SHIFT`, `CTRL`, `ALT` or `GUI` in the large area.

The stock Sofle layers differ from the Piantor: its lower layer mixes numbers,
symbols and function keys, while raise contains navigation and Bluetooth controls.
Display-only aliases are `default` → `QWERTY`, `lower` → `NUM/SYM`,
`raise` → `NAV`, `adjust` → `ADJUST`. Custom Studio names remain available,
truncated to eight bytes to fit. The keymap and layer bindings are not renamed.

A press replaces the single latest usage. Releasing that usage starts a
1,000 ms timeout; releasing another key does not. The newest pressed key wins,
including modifiers and encoder-generated media keys. It does not return to an
older key when the newest key is released. Held keys remain visible. Fast input
is coalesced into frames no more often than every 25 ms; intermediate keys within
one frame may not appear. Identical labels are not redrawn or blinked. No
keypress-driven polling continues after the timeout.

Letters are uppercase key legends. Digits/punctuation use US HID legends and the
modifier state at the press, including implicit modifiers such as `&kp EXCL`.
This is **not host text prediction**: non-US layouts, Caps Lock, Compose/dead
keys, AltGr, IMEs, remapping, application shortcuts and host-generated repeats
can produce different text. GUI is Command on macOS and Super/Windows on Linux.
Unknown usages display `?`; they still pass through to ZMK unchanged.
Layer/control behaviors that emit no HID keycode only update their relevant
status; no physical-position-to-character guess is made.

## Implementation and privacy

`CONFIG_SOFLE_KEYPRESS_DISPLAY=y` selects `widgets/keypress.c` and the portable
`keypress_model.c` instead of the original central `widgets/status.c`.
The option depends on the Sofle left board, custom Sharp widget and dedicated
display work queue. It defaults off. The extra build matrix entry enables it
only for the new artifact. The existing matrix entries still build the original
firmware, including the unchanged right side.

`zmk_keycode_state_changed` contains resolved HID usages from both halves;
`zmk_layer_state_changed` supplies layer feedback. Physical
`zmk_position_state_changed` precedes behavior/layer/hold-tap resolution and is
intentionally not used as a key legend source. No split protocol changes are
required. Encoder-generated consumer usages also pass through the same observer.

The key listener performs bounded state updates under a short spinlock, schedules
work, and always returns `ZMK_EV_EVENT_BUBBLE`. It makes no LVGL calls, blocking
waits, allocations, logging, HID changes or transport calls. LVGL rendering and
timeout processing run on `zmk_display_work_q()`, together with LVGL's tick.
Separate frame and expiry work items prevent continuous typing from postponing
rendering indefinitely. Expiry rechecks the current state so an old timeout cannot
erase a newer key. Modifier references distinguish left/right and overlapping
explicit modifier wrappers. Shifted legends remain stable after modifier release.

Only one latest usage and the current modifier references are maintained; there
is no typed-text history, flash/settings write or extra transmission. The current
label cache is replaced on each rendered update and cleared after expiry.
Normal keyboard input still reaches the selected host through unchanged ZMK HID.
The screen visibly shows keys even in password fields: firmware cannot detect
host input-field types. Disable the feature at build time when that is unwanted.

## Build and tests

GitHub: push the feature branch, open Actions → **Build ZMK firmware**, and
download the `firmware` artifact from the successful run for that exact commit.
The **Keypress model tests** workflow runs the same C state machine with address
and undefined-behavior sanitizers. Model tests also run locally:

```sh
sh tests/keypress/run.sh
```

Local firmware build in a prepared ZMK/west workspace created using
`config/west.yml` (the same setup used by Keebart's reusable workflow):

```sh
config_repo=/absolute/path/to/zmk-config
west build -s zmk/app -d build/sofle-keypress -b sofle_choc_pro_left \
  -S studio-rpc-usb-uart -- \
  -DZMK_CONFIG="$config_repo/config" \
  -DZMK_EXTRA_MODULES="$config_repo" \
  -DSHIELD=sharp_mip \
  -DCONFIG_ZMK_STUDIO=y \
  -DCONFIG_SOFLE_KEYPRESS_DISPLAY=y
cp build/sofle-keypress/zephyr/zmk.uf2 sofle_choc_pro_left_keypress.uf2
```

For the original display, build into a separate directory with the same command,
changing the last option to `-DCONFIG_SOFLE_KEYPRESS_DISPLAY=n`.
Do not put the feature option in the shared `config/sofle_choc_pro.conf`, since
that file is also used for the right-half build.

## Flash on Linux (manual)

1. Extract the successful firmware ZIP. Keep both left images, including the
   original-display image, somewhere accessible. If your current firmware was
   customized elsewhere, retain its original left UF2 as well.
2. Keep the **left half** connected directly to Linux by a data-capable USB-C
   cable. Do not connect the right half by USB during this procedure.
3. Quickly double-press the physical reset button on the **left half**. A USB
   mass-storage bootloader volume should appear. This does not clear pairings.
4. Identify that volume in the file manager or with:

   ```sh
   lsblk -o NAME,LABEL,MODEL,SIZE,FSTYPE,MOUNTPOINTS
   ```

   Open its `INFO_UF2.TXT` to confirm it is the newly connected keyboard
   bootloader. If it is unmounted, mount its partition in the file manager or
   use `udisksctl mount -b /dev/<the-new-bootloader-partition>`.
5. Copy **only** `sofle_choc_pro_left_keypress.uf2` onto that mounted volume.
   Terminal equivalent, replacing both paths with the actual paths:

   ```sh
   firmware=/absolute/path/to/sofle_choc_pro_left_keypress.uf2
   boot_volume=/actual/mount/path/of/the/left/bootloader
   test -f "$firmware" && test -f "$boot_volume/INFO_UF2.TXT" &&
     cp -- "$firmware" "$boot_volume/"
   sync
   ```

6. The bootloader should eject/reboot automatically. Reconnect USB if needed,
   then complete the checks below. Do not press Bluetooth clear, erase settings,
   flash a settings-reset image or reset the right half's pairings.

## Restore the original display

Repeat the same left-only bootloader procedure with
`sofle_choc_pro_bt_left.uf2`, the unchanged original-display artifact from the
same build. To restore an exact previously installed custom firmware, use its
saved left UF2 instead. To obtain upstream firmware independently, download a
successful Keebart workflow artifact for the same hardware/version, or build the
upstream commit cited above with `sharp_mip` and Studio enabled.
Normal UF2 application flashing does not intentionally erase saved Bluetooth
pairings or Studio settings; neither the new feature nor the rollback requires
settings-reset firmware.

## Required device checks

Compilation and host-side model tests cannot establish real-device latency,
LCD orientation/readability, battery draw or radio compatibility. Check:

1. USB to Linux: press letters on **each** half; confirm both the displayed label
   and normal typed text. Check punctuation, Space, Enter, Backspace, Escape,
   all arrows and modifier-only presses.
2. Hold left and right Shift, release one, then type; check the held-modifier
   indicator and shifted legend. Try Ctrl/Alt/GUI shortcuts and rolled key chords.
3. Hold a letter for more than a second; release it and check clearing after
   roughly one second. Roll A→B, release A first, then B. Type rapidly and repeat
   the same key; confirm no missed input or flicker. Some coalesced labels may be
   skipped at very high speeds; input events must not be skipped.
4. Activate LOWER, RAISE and both together; verify their layer names and original
   behavior. Check any saved Studio mappings and Studio access over USB.
5. Turn/click **both rotary encoders**. Confirm the existing volume/track and
   mute/play actions, plus media labels on the left display.
6. Switch to the already paired Mac Bluetooth profile using the existing
   bindings. Check typing from both halves, shortcuts, battery/connection display,
   then switch back to Linux. Do not clear a profile.
7. Verify the right screen still shows its artwork and connection indicator.
   Check wake/reconnect and, separately, unplugged idle/power behavior.

No hardware has been flashed or pairings reset as part of this implementation.
