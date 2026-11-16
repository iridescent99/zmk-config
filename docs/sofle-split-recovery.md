# Sofle Choc Pro BT: matched firmware and split recovery

For the **58-key Keebart Sofle Choc Pro BT** only. This package contains two
application images built in the same workflow run from the commit recorded in
`SOURCE_COMMIT.txt`, using Keebart's ZMK `v0.3` manifest and build infrastructure.

| Physical half | UF2 to copy | Board / shield | Role |
| --- | --- | --- | --- |
| Left | `sofle_choc_pro_left_keypress.uf2` | `sofle_choc_pro_left` / `sharp_mip` | Central; keypress visualiser, USB, Bluetooth and ZMK Studio |
| Right | `sofle_choc_pro_bt_right.uf2` | `sofle_choc_pro_right` / `sharp_mip` | BLE peripheral; original artwork and connection indicator |

Both retain the existing keymap and rotary encoder configuration. The right half
sends input to the left over BLE, including when the left uses USB to Linux.
Connecting the right half alone by USB is not a test of normal typing.

## Findings and limits

The right half stopped sending input after a left-only update. The user also
reported failure with the original-display left image built from the same source.
That comparison does not establish the cause, but makes the new display code a
less likely explanation. The firmware version originally installed on the right
is unknown. These two images eliminate differing firmware versions as a variable.

Inspection against Keebart baseline `98c5a1181ff7db1f480f684f714126922d55b8f0`
found no changes to the Sofle board definitions, shared configuration, keymap,
BLE settings, flash layout, or right display code. Both targets enable
`CONFIG_ZMK_SPLIT=y` and `CONFIG_ZMK_SPLIT_BLE=y`; only the left enables
`CONFIG_ZMK_SPLIT_ROLE_CENTRAL`. The visualiser observes resolved key events,
always returns `ZMK_EV_EVENT_BUBBLE`, and performs rendering on the display queue.
It neither changes BLE transport nor writes persistent settings.

The delivery job requires successful firmware builds and validates both UF2s'
format, nRF52840 family, vectors and address ranges. Payloads must stay in the
existing application partition `[0x26000, 0xEC000)`; settings and bootloader
partitions are excluded. The package contains no settings-reset image.
These checks establish build/image validity, **not physical radio connectivity**.

## Flash both halves on Linux

1. Download **sofle-matched-firmware** from the successful GitHub Actions run
   for the feature branch and extract the ZIP. Use both UF2s from this package.
   Keep any previously saved firmware backups. Optionally open a terminal in the
   extracted folder and run `sha256sum -c SHA256SUMS`; both files must report OK.
2. **Flash the right half first.** Connect only the right half to Linux using a
   data-capable USB-C cable. The recessed reset button is underneath the case,
   next to the USB port. Quickly double-press it using a SIM eject tool.
3. Open the new **KEEBART** drive in the file manager. Copy only
   **`sofle_choc_pro_bt_right.uf2`** onto it. Wait for the copy to finish and the
   drive to disappear as the board reboots. Then unplug the right USB cable.
4. **Flash the left half.** Connect only the left half by USB, double-press its
   reset button, and copy only **`sofle_choc_pro_left_keypress.uf2`** onto its
   KEEBART drive. Wait for the copy to finish and automatic reboot. This restores
   the working keypress visualiser if the comparison firmware was installed.
5. Unplug USB and switch **both halves off** using their inner-side switches
   (bottom position). Wait five seconds. Switch both on (top position), reconnect
   USB to the **left**, and keep the halves near each other. Allow about ten
   seconds to reconnect. Both halves need power; charge the right if necessary.
6. Open a text editor on Linux. Press `A` on the left, then `J` on the right.
   Each must type normally and update the left screen. Turn/click both encoders.
   Then check the existing Mac Bluetooth profile without clearing or re-pairing it.

The double-press enters the bootloader; it is not a persistent-settings reset.
These are normal application updates. Do not use `settings_reset`, `BT_CLR`,
Studio's settings reset, or the host's Forget Device action for this procedure.
Saved Bluetooth bonds and Studio mappings are intended to remain in place.

If the bootloader drive is not visible, run
`lsblk -o NAME,LABEL,MODEL,SIZE,FSTYPE,MOUNTPOINTS` and mount the newly appeared
KEEBART volume in the file manager. Its `INFO_UF2.TXT` identifies the bootloader.
Do not copy both images to the same half; each needs its own image.

## If the right still does not work

Record whether the right screen shows its normal artwork with an **X** or a
**connection symbol**, or remains blank. The existing right widget shows X when
its split BLE connection is absent. Report whether right-side keys update the
left display even if they do not type, and whether USB power to the right changes
the result. Keep USB to the left for the typing test.

Matching builds cannot repair every stored-bond or hardware fault. Investigate
power and split connection diagnostics next. Clearing persistent settings is a
separate last-resort decision: ZMK's documented reset procedure clears host
Bluetooth profiles too. It is **not included or required** by this package.

## Rollback

The same run's full `firmware` artifact includes `sofle_choc_pro_bt_left.uf2`,
which restores Keebart's original left display while retaining this matching
right firmware. It is a build of the upstream source, not a backup of the exact
factory-installed image. To restore an exact earlier firmware, use saved images
for both halves, or obtain a matching original pair from Keebart. Normal rollback
also does not require a settings-reset image.

References: [Keebart flashing guide](https://www.keebart.com/downloads/sofle-choc-pro-bt-guide.pdf),
[ZMK split connection troubleshooting](https://zmk.dev/docs/troubleshooting/connection-issues).
