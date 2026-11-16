# Temporary Sofle right-side diagnosis

This is a diagnostic image, not a confirmed repair. The right screen's original
connection icon only reports a BLE connection; it does not prove encryption,
notification subscription, or delivery of key positions to the left.

Flash only `sofle_choc_pro_right_diagnostics.uf2` to the **right** half. Leave the
working left keypress firmware installed. This uses the same ZMK v0.3 and the
same board, keymap, BLE, encoder and persistent-settings configuration as the
matched pair. It adds read-only status sampling and event observers. It never
clears bonds, changes subscriptions, writes settings, or logs/transmits keys.

1. Connect the right half to Linux with a data USB cable.
2. Double-press its recessed reset button underneath beside the USB connector.
3. Copy this one UF2 to the KEEBART drive and wait for reboot.
4. Disconnect right USB, reconnect USB to the left, and switch both halves on.
5. Confirm **D1** appears on the right screen. This identifies the diagnostic
   image and confirms that it was actually installed on the correct half.
6. Press J repeatedly on the right and turn its encoder. Report the three lines
   while testing, or provide a photo. A single isolated tap lights KEY for one
   second; encoder activity similarly lights ENC. Neither stores the key identity.

| Indicator | Meaning |
| --- | --- |
| `D1 C0N0` | No connected BLE peer seen |
| `C1` | Connected, but not encrypted |
| `C2`, `C3`, `C4` | Encrypted BLE connection |
| `N1` | Peer subscribed to the right's key-position notifications |
| `N0` | No such subscription found |
| `KEY 1` | Right firmware saw a local key press within the last second |
| `KEY 0` | No recent local key press |
| `ENC1` | Right firmware saw an encoder event within the last second |
| `E0` | No reported BLE security error |
| `E1`..`E9` | Last BLE security error code; report the exact digit |

With working scanning and forwarding, expect `C2` or higher, `N1`, and `KEY 1`
when pressing. C2/N1 alone still does not prove the left consumed a notification.
Also report whether the press changes the left display. The diagnostic uses the
right status area, partially covering the upper artwork; the battery and link
icon remain visible. Normal firmware keeps the artwork unchanged.

Restore `sofle_choc_pro_bt_right.uf2` from the previous matched package to remove
diagnostics. Neither this diagnostic nor its removal requires settings reset.
Do not reset pairings solely because the connection icon is visible but typing
fails. No physical-device result has been established by compilation alone.
