#!/usr/bin/env python3
"""Static checks for the Sofle's two-page, app-independent LaTeX layer."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
KEYMAP = (ROOT / "config/sofle_choc_pro.keymap").read_text()
DISPLAY = (ROOT / "boards/shields/sharp_mip/widgets/keypress.c").read_text()

def node(name: str) -> str:
    match = re.search(r"\b" + re.escape(name) + r"\s*\{", KEYMAP)
    assert match, f"missing devicetree node {name}"
    start = match.end()
    level = 1
    for pos in range(start, len(KEYMAP)):
        if KEYMAP[pos] == "{":
            level += 1
        elif KEYMAP[pos] == "}":
            level -= 1
            if level == 0:
                return KEYMAP[start:pos]
    raise AssertionError(f"unclosed devicetree node {name}")

def key_rows(name: str) -> list[list[str]]:
    match = re.search(r"\bbindings\s*=\s*<(.+?)>;", node(name), re.S)
    assert match, f"{name} has no bindings"
    rows = []
    for line in match.group(1).strip().splitlines():
        rows.append(re.findall(r"&[a-zA-Z_]\w*(?:\s+(?:LS\([A-Z]\)|[A-Z][A-Z0-9_]*))?", line))
    assert list(map(len, rows)) == [12, 12, 12, 14, 10], (name, rows)
    return rows

primary = key_rows("latex_layer")
operators = key_rows("latex_operator_layer")
assert primary[3][12] == "&kp BSLH", "LaTeX slash position must type a literal backslash"
assert "if-layers = <LATEX RAISE>;" in node("latex_ops_conditional")
assert "then-layer = <LATEX_OPS>;" in node("latex_ops_conditional")
assert '#define LATEX 4' in KEYMAP
assert '#define LATEX_OPS 5' in KEYMAP

commands = {
    "sigma", "Delta", "phi", "pi", "neg", "equiv", "subseteq", "leq", "geq",
    "land", "lor", "cup", "cap", "notin", "Rightarrow", "rightarrow", "bot",
    "top", "iff", "hat", "overline", "langle", "rangle",
}
# Verify every new macro emits exactly the intended US HID command characters.
for command in commands:
    body = node("lx_" + command)
    m = re.search(r"\bbindings\s*=\s*<(.+?)>;", body, re.S)
    assert m, command
    actual = re.findall(r"&kp\s+([A-Z][A-Z0-9_]*(?:\([A-Z]\))?)", m.group(1))
    expected = ["BSLH"] + [c.upper() if c.islower() else f"LS({c})" for c in command]
    if command in ("hat", "overline"):
        expected += ["LBRC", "RBRC", "LEFT"]
    assert actual == expected, (command, actual, expected)
    assert "wait-ms = <35>;" in body and "tap-ms = <35>;" in body

# Position-based operator page, physical QWERTY rows.
expected = {
    (1, 1): "rightarrow", (1, 2): "overline", (1, 3): "equiv",
    (1, 4): "Rightarrow", (1, 5): "top", (1, 6): "iff",
    (1, 7): "subseteq", (1, 8): "notin", (1, 9): "omega", (1, 10): "pi",
    (2, 1): "approx", (2, 2): "sigma", (2, 3): "Delta",
    (2, 4): "phi", (2, 5): "geq", (2, 7): "land",
    (2, 8): "lor", (2, 9): "leq",
    (3, 3): "cup", (3, 4): "cap", (3, 5): "bot",
    (3, 8): "neg", (3, 10): "langle", (3, 11): "rangle",
}
for (row, col), command in expected.items():
    assert operators[row][col] == "&lx_" + command, (row, col, command)

# The layer title must be rendered in the same canvas *above* the key glyph.
assert 'text(canvas, 1, &lv_font_unscii_8, layer);' in DISPLAY
assert 'text(canvas, key_top + (key_height - font->line_height) / 2, font, label);' in DISPLAY
assert 'draw_key(shown_label);' in DISPLAY.split("static void status_update(", 1)[1]
assert 'if (!strcmp(name, "latex")) { return "LATEX"; }' in DISPLAY
print("LaTeX keymap and layer-over-key display checks passed.")
