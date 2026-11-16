"""Validate the delivered UF2s against the existing Sofle flash partition map.

UF2 block format: https://github.com/microsoft/uf2
Board map: boards/arm/sofle_choc_pro/sofle_choc_pro.dtsi
This checks image integrity and application-only writes, not device behavior.
"""
import hashlib
from pathlib import Path
import struct
import sys


def verify(path):
    data = path.read_bytes()
    assert data and len(data) % 512 == 0, f"{path}: invalid UF2 length"
    count = len(data) // 512
    end = 0x26000
    for i in range(count):
        block = data[i * 512 : (i + 1) * 512]
        magic0, magic1, flags, address, size, number, total, family = struct.unpack_from(
            "<8I", block
        )
        assert (magic0, magic1) == (0x0A324655, 0x9E5D5157), "Invalid UF2 magic"
        assert struct.unpack_from("<I", block, 508)[0] == 0x0AB16F30, "Invalid trailer"
        assert flags == 0x2000 and family == 0xADA52840, "Expected nRF52840 flash data"
        assert number == i and total == count, "Missing or reordered blocks"
        assert 0 < size <= 476 and size % 4 == 0, "Invalid payload size"
        assert address == end, "Expected contiguous application image"
        end = address + size
        assert 0x26000 <= address < end <= 0xEC000, "Write outside application partition"
        if i == 0:
            stack, reset = struct.unpack_from("<II", block, 32)
            assert 0x20000000 < stack <= 0x20040000, "Invalid initial stack"
            assert reset & 1 and 0x26000 <= reset < 0xEC000, "Invalid reset vector"
    print(f"PASS {path.name}: {count} nRF52840 blocks; application 0x26000..0x{end:x}; "
          "settings/bootloader untouched")
    print(f"SHA256 {hashlib.sha256(data).hexdigest()} {path.name}")


if __name__ == "__main__":
    assert len(sys.argv) > 1, "Pass one or more Sofle UF2 paths"
    for filename in sys.argv[1:]:
        verify(Path(filename))
