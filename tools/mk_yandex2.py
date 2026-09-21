"""Pack a Telink .bin into the Yandex bootloader .ybl format.

Format: .bin body as-is, BIN_SIZE@0x18 = len(body) WITHOUT trailing CRC,
then CRC-32 LE appended. Result size = .bin + 4.

Usage:
    python mk_yandex2.py <sampleGW_b91.bin> [<output.ybl>]

Stdlib only.
"""
import struct
import sys
import zlib
from pathlib import Path

if len(sys.argv) < 2:
    sys.exit("usage: mk_yandex2.py <input.bin> [<output.ybl>]")

src = Path(sys.argv[1])
dst = Path(sys.argv[2]) if len(sys.argv) > 2 else src.with_name(src.stem + "_yandex2.ybl")

d = bytearray(src.read_bytes())
# BIN_SIZE at 0x18 must equal the image size WITHOUT the trailing CRC
# (matches genuine gateway.ybl: file 218232 = 218228 + 4, sz@0x18 = 218228).
sz = struct.unpack("<I", d[0x18:0x1C])[0]
crc = zlib.crc32(bytes(d)) & 0xFFFFFFFF
d += struct.pack("<I", crc)
dst.write_bytes(d)

v = dst.read_bytes()
ok = (zlib.crc32(v[:-4]) & 0xFFFFFFFF) == struct.unpack("<I", v[-4:])[0]
print(f"wrote {dst} size={len(v)} BIN_SIZE={sz} crc_ok={ok}")
sys.exit(0 if (ok and sz == len(v) - 4) else 1)
