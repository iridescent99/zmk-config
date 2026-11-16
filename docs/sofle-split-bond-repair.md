# One-time split-bond repair: Keebart Sofle Choc Pro BT

Use this package only for the diagnosed split security failure. Observed on this
keyboard: right KEY changes to 1, but `D1 C1N0` / `E4` persists. This establishes
working local key detection, an unencrypted BLE link, no key notification
subscription, and BT_SECURITY_ERR_AUTH_REQUIREMENT. It does not establish which
internal security check failed. A stale/inconsistent split bond is a likely
explanation after firmware updates; rebuilding alone leaves that stored bond intact.

**These two images intentionally remove only the saved bond between the halves,
once per controller. They preserve host pairings, local Bluetooth identity,
Studio mappings, keymap, encoders, and other keyboard preferences.** They keep
Secure Connections and encrypted notifications enabled. The left retains the
keypress visualiser; the right retains the diagnostic readout for verification.

## Flash in this order

Keep the other half completely powered off while flashing each repair image,
so the old half cannot immediately re-pair before its own bond has been repaired.

1. Extract `sofle-split-bond-repair.zip`. Optionally run
   `sha256sum -c SHA256SUMS` inside the extracted folder; both files must say OK.
2. Unplug USB from both halves and switch **both halves OFF**.
3. Connect only the **right** half to Linux with a data USB-C cable. Double-press
   its recessed reset button underneath beside the USB port. Copy only
   **`sofle_choc_pro_right_split_repair.uf2`** to the **KEEBART** drive.
   Wait for reboot and **FIX DONE** or **FIX NONE** on its display. Then unplug
   right USB and leave its battery switch OFF. The left must remain off throughout.
4. Connect only the **left** half to Linux. Double-press its reset button and copy
   only **`sofle_choc_pro_left_split_repair.uf2`** to KEEBART. Wait for reboot and
   **FIX DONE** on its screen. The right must still be unplugged and switched off.
5. Turn both battery switches ON; leave USB connected to the **left**. Keep the
   halves close together and allow about ten seconds for a fresh encrypted link.
6. Test a right-side letter in a Linux text editor and check the left display.
   Expected right diagnostic: `C2` or higher, `N1`, `E0`, and `KEY 1` on a press.
   Check both encoders, then the existing Mac Bluetooth profile without forgetting
   or re-pairing the keyboard on the Mac.

If a screen says **FIX STOP** or **FIX ERR**, stop and report which half and the
message. Do not use a general settings-reset image. FIX STOP means the saved
records were ambiguous, unexpected, or overlapped a host profile; no records
were intentionally changed. FIX ERR means a settings operation failed; a repair
may be incomplete. **FIX USED** on later boots is expected: the one-time attempt
guard prevents another deletion. If FIX USED appears on the very first test and
the split still fails, report it; this image will not retry automatically.

## Scope and implementation

`CONFIG_SOFLE_SPLIT_REPAIR` defaults off. Only the two explicitly named repair
builds enable it. A SYS_INIT hook at APPLICATION priority 60 runs after NVS/flash
initialization and before ZMK v0.3's main() calls settings_load(), whose commit
handlers enable split scanning/advertising. Direct settings reads do not invoke
those commit handlers. No key input path is modified or blocked.

On the left, the target is read from `ble/peripheral_addresses/0`. It is never
inferred from the list of host bonds. On the right, exactly one stored bond is
required (zero means nothing to erase). Both sides compare the target against
all readable saved host-profile addresses and refuse any overlap. Malformed
records, unsupported identity suffixes, multiple right bonds and callback read
errors prevent writes. No bond secret is read or logged.

The only deletion paths are `bt/keys/<peer>`, `bt/ccc/<peer>`, `bt/cf/<peer>` and
`bt/sc/<peer>` for that single peer on BT_ID_DEFAULT. They match the pinned
Zephyr 3.5 settings encoding. `bt/id`, `bt/irk`, every host peer's bond/CCC,
`ble/profiles/*`, `ble/active_profile`, the split address mapping, Studio/keymap
and other settings are untouched. There is no whole-store erase or unpair-all.

Before deleting any peer records the code writes a one-byte guard at
`sofle_split_repair/attempt_v1`. This prevents future boots or reflashes from
clearing a newly established bond. A power interruption may leave an incomplete
attempt; it deliberately does not trigger an automatic retry.

Host tests exercise the actual deletion planner and startup adapter with a fake
settings backend, including host preservation, malformed records, ambiguous
peers, an ignored callback error (Zephyr 3.5 behavior), marker failure, deletion
failure and repeated boots. CI compiles both targets and verifies UF2 application
address ranges. UF2 bounds alone cannot certify runtime settings behavior: the
repair code explicitly writes the targeted records described above.

## After recovery

The repair images are functional keyboard firmware and the guard makes the
cleanup one-shot. After verifying recovery, normal images from the same run's
`sofle-matched-firmware` package remove repair code and restore the original right
artwork; the left image retains the visualiser. Flash the corresponding half
normally, without resetting any settings. The harmless one-byte attempt guard
remains stored. Keep the diagnostic right image until the connection is verified.

This is a targeted recovery attempt, not hardware-verified proof of repair.
