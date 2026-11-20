# System-wide LaTeX layer (Sofle Choc Pro BT)

This is a ZMK firmware layer. It emits ordinary keyboard HID keystrokes, so it works in ChatGPT, terminals, editors, browsers, Linux and macOS. Applications decide whether to render the inserted LaTeX as typeset mathematics.

## Enter and exit

**Hold LOWER (left thumb) and tap SPACE (right thumb)** to toggle LaTeX mode. Repeat the same chord to exit. No existing BASE key was changed. Keep RAISE, LOWER, and ADJUST functionality when the layer is off.

While LaTeX mode is active, number keys, Space, Enter, Backspace, Escape, punctuation, modifier keys and encoders inherit their previous behavior. The A-Z keys produce LaTeX macros. **RAISE navigation shortcuts on letter positions (e.g. RAISE+I/J/K/L) do not take precedence over the active LaTeX layer**: toggle LaTeX off first when you need them.

## Key reference

| Key | Inserts | Key | Inserts |
| --- | --- | --- | --- |
| A | `\\alpha` | N | `\\nabla` |
| B | `\\beta` | O | `\\Omega` |
| C | `\\cdot` | P | `\\partial` |
| D | `\\delta` | Q | `\\quad` |
| E | `\\epsilon` | R | `\\sqrt{}` |
| F | `\\frac{}{}` | S | `\\sum_{}^{}` |
| G | `\\gamma` | T | `\\theta` |
| H | `\\hbar` | U | `\\infty` |
| I | `\\int_{}^{}` | V | `\\vec{}` |
| J | `\\approx` | W | `\\omega` |
| K | `\\kappa` | X | `\\times` |
| L | `\\lambda` | Y | `\\psi` |
| M | `\\mu` | Z | `\\zeta` |

The structural macros position the cursor automatically:
- **F:** `\\frac{|}{}` (cursor in numerator)
- **R:** `\\sqrt{|}`
- **S:** `\\sum_{|}^{}`
- **I:** `\\int_{|}^{}`
- **V:** `\\vec{|}`

Here `|` is the cursor, **not** a character that the macro inserts. ZMK types the expression and sends left-arrow keystrokes to reposition the insertion point.

## Firmware build and flashing

1. On the PR branch, manually run **Actions → Build ZMK firmware → Run workflow**, selecting `feat/latex-layer-display-20261120`. Once the PR is validated and merged, select `feature/sofle-live-keypress-display` for subsequent builds. GitHub App commits may not start Actions automatically.
2. Download `sofle-blue-lilac-starfield-matched` to preserve the live key display, physical gradient lighting, and right starfield. The paired UF2 images are named `sofle_choc_pro_left_blue_lilac.uf2` and `sofle_choc_pro_right_blue_lilac_starfield.uf2`. A plain default firmware pair is also available in the generic `firmware` artifact.
3. Back up the existing working UF2 files.
4. Connect the left half by USB. Double-press its recessed reset switch to mount the `KEEBART` bootloader drive; copy **only the left UF2** to that drive.
5. Repeat with the right half, using **only the right UF2**. Never swap halves.
6. Reconnect and test `LOWER + SPACE` then `A`, `F`, `R`, and the exit chord in a plain text editor.

## Notes

- The host must use a layout compatible with the US punctuation keycodes used by the existing ZMK configuration. Different keyboard layouts may change how backslash, underscore, braces and caret are typed.
- The layer inserts LaTeX **source**, not universal mathematical glyphs. Rendered math is an application feature.
- Macros need to finish typing before you press the next key. Their timing is intentionally conservative for Bluetooth.
- ZMK Studio may remap existing behaviors but defining new text macros requires a firmware build.
