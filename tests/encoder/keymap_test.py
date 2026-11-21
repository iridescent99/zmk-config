#!/usr/bin/env python3
"""Exercise both compiled encoder selections without changing typing bindings.

The real firmware build checks the full devicetree and ZMK behavior types. This
small host check tests the C-preprocessor branch used by both Sofle keymaps;
unrelated includes are empty because keycode values are immaterial here.
"""

from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
KEYMAPS = ("config/sofle_choc_pro.keymap", "boards/arm/sofle_choc_pro/sofle_choc_pro.keymap")
SENSORS = re.compile(r"sensor-bindings\s*=\s*<([^>]+)>;")


def main():
    with tempfile.TemporaryDirectory(prefix="sofle-encoder-test-") as directory:
        headers = Path(directory)
        for relative in KEYMAPS:
            source = ROOT / relative
            overlay = ROOT / "boards/shields/sofle_scroll/sofle_scroll.overlay"
            for name in re.findall(r"^#include <([^>]+)>", source.read_text() + overlay.read_text(), re.M):
                header = headers / name
                header.parent.mkdir(parents=True, exist_ok=True)
                header.touch()

            def expand(scroll):
                # Zephyr uses assembler-with-cpp mode so devicetree properties
                # such as #binding-cells pass through instead of being directives.
                command = ["cpp", "-x", "assembler-with-cpp", "-P", "-nostdinc", "-I", directory]
                command += ["-I", str(overlay.parent), "-"]
                # Mirror Zephyr's actual shield-before-keymap include ordering.
                # CONFIG_ symbols are not available during devicetree processing.
                prefix = '#include "sofle_scroll.overlay"\n' if scroll else ""
                result = subprocess.run(command, input=prefix + "SOFLE_KEYMAP_START\n" + source.read_text(),
                                        capture_output=True, text=True, timeout=10)
                if result.returncode:
                    raise RuntimeError(result.stderr[:1500])
                return result.stdout.split("SOFLE_KEYMAP_START", 1)[1]

            stock, scroll = expand(False), expand(True)
            before, after = SENSORS.findall(stock), SENSORS.findall(scroll)
            assert len(before) == len(after) and len(before) >= 3, "Missing encoder layer coverage"
            for old, new in zip(before, after):
                assert old.split() == ["&inc_dec_kp", "C_VOL_DN", "C_VOL_UP", "&inc_dec_kp", "C_PREV", "C_NEXT"], relative
                assert new.split() == ["&inc_dec_kp", "C_VOL_DN", "C_VOL_UP", "&sofle_encoder_scroll"], relative
            assert SENSORS.sub("", stock) == SENSORS.sub("", scroll), "Scroll variant changed typing/layer bindings"
            print(f"PASS {relative}: {len(after)} encoder layers; typing and macros unchanged")


if __name__ == "__main__":
    main()
