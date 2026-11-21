# System-wide LaTeX layer | Sofle Choc Pro BT

This is a **ZMK firmware keymap**, not an app plugin. Key combinations type literal LaTeX source in any focused application on Linux or macOS; LaTeX rendering still depends on the host app. The **left keyboard display** shows the live highest active layer **immediately above its large most-recent-key readout**, including while no HID key is emitted by a layer toggle.

## Usage

- **Toggle LaTeX on/off:** hold the left thumb **LOWER**, tap the right thumb **SPACE**, release.
- **Operator page:** while LaTeX is on, **hold RAISE** (right thumb); release it to return to the primary LaTeX mappings. The layer label changes from `LATEX` to `LATEX OP`.
- **Backslash:** in LaTeX mode press the ordinary physical **/** key for a literal ``\`` (the key at the far right of the bottom letter row). Outside LaTeX, the original **LOWER + /** also produces backslash.
- The digit row, normal whitespace, modifiers and encoder controls still work. In LaTeX mode, use the RAISE operator page instead of RAISE's ordinary navigation bindings. Toggle LaTeX off for normal navigation.

## Primary LaTeX page

| Key | LaTeX | Key | LaTeX |
| --- | --- | --- | --- |
| A | `\alpha` | N | `\nabla` |
| B | `\beta` | O | `\Omega` |
| C | `\cdot` | P | `\partial` |
| D | `\delta` | Q | `\quad` |
| E | `\epsilon` | R | `\sqrt{}` |
| F | `\frac{}{}` | S | `\sum_{}^{}` |
| G | `\gamma` | T | `\theta` |
| H | `\hbar` | U | `\infty` |
| I | `\int_{}^{}` | V | `\vec{}` |
| J | `\approx` | W | `\omega` |
| K | `\kappa` | X | `\times` |
| L | `\lambda` | Y | `\psi` |
| M | `\mu` | Z | `\zeta` |

## RAISE + LaTeX operator page

| Key | LaTeX | Key | LaTeX |
| --- | --- | --- | --- |
| A | `\approx` | N | `\neg` |
| B | `\bot` | O | `\omega` |
| C | `\cup` | P | `\pi` |
| D | `\Delta` | Q | `\rightarrow` |
| E | `\equiv` | R | `\Rightarrow` |
| F | `\phi` | S | `\sigma` |
| G | `\geq` | T | `\top` |
| H | `\hat{}` | U | `\subseteq` |
| I | `\notin` | V | `\cap` |
| J | `\land` | W | `\overline{}` |
| K | `\lor` | Y | `\iff` |
| L | `\leq` | , | `\langle` |
| . | `\rangle` | / | `\` |

Unlisted operator-page positions pass through to the primary LaTeX layer. The keys selected for the operator page are mnemonic where practical: **S**igma, **P**i, **F**hi, **E**quiv, **N**eg, **U** (subset), **L**ess, **G**reater, **B**ot and **T**op.

Templates place the caret inside the argument: `\hat{|}`, `\overline{|}`, `\frac{|}{}`, `\sqrt{|}`, `\int_{|}^{}`, `\sum_{|}^{}` and `\vec{|}`. The vertical bar in these examples represents the insertion point, not a typed character.

**Mathematical syntax note:** LaTeX control words such as `\sigma` must be separated from following letters by a space or braces. Neither the keyboard nor the display interprets formulas or inserts automatic delimiters. `\Rightarrow` (double arrow), `\rightarrow` (single arrow), `\equiv` and `\iff` are distinct.

## Display behavior

The Sofle left Sharp MIP display has three upright segments. The middle segment now contains the **active ZMK layer name at the top** and the **latest resolved key underneath**. Layer changes trigger a redraw even if they don't send a keycode. Other existing features remain: battery and connectivity, modifier indicators, live keypress display, physical blue/lilac gradient and right-half starfield. The display shows HID keystrokes, so a multi-key LaTeX macro may briefly show individual characters rather than its whole command.

## Build and flash

1. Open GitHub **Actions → Build ZMK firmware → Run workflow** and select `feat/latex-layer-display-20261120`. GitHub App commits may not start Actions automatically.
2. Check that the build succeeds for the final commit.
3. Download `sofle-blue-lilac-starfield-matched`. This is the *customised* matched pair:
   - `sofle_choc_pro_left_blue_lilac.uf2`
   - `sofle_choc_pro_right_blue_lilac_starfield.uf2`
4. Keep a copy of your existing working firmware. Connect **only the left half** by a data USB cable, double-click its recessed reset switch, verify the bootloader drive, and copy **the left UF2** onto it.
5. Repeat on the **right half** with **the right UF2**. Never swap halves or flash any `settings_reset` image.
6. Test normal typing and both halves' connectivity, then LOWER+SPACE, backslash, RAISE operator symbols, and the layer label over the key readout. Test both Linux and macOS HID keyboard layouts for punctuation.

The existing settings/flash storage and Bluetooth pairings are not intentionally modified by ordinary application UF2 flashing. The source build cannot prove which firmware is currently installed or that a physical flash succeeds. See `docs/sofle-full-gradient-starfield.md` for the existing display/lighting release and `docs/sofle-split-recovery.md` for split recovery.

## Constraints

- These are US HID keycodes. A host keyboard layout that translates punctuation differently can change literal `\`, `{`, `}`, `^`, and `_` output.
- Macros use 35 ms wait/tap timings to reduce out-of-order Bluetooth output.
- No changes to the physical key-switch positions, pairing, rotary encoders or right-display art.
