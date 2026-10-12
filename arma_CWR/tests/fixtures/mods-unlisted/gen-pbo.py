"""Pack @udunlisted/source/config.cpp into @udunlisted/addons/udunlisted.pbo.

Uncompressed OFP pbo: one header entry per file (name, NUL, five uint32:
packing method, original size, reserved, timestamp, data size), an empty
terminator entry, then the file data. Rerun after editing the config.
"""

import struct
from pathlib import Path

root = Path(__file__).resolve().parent / "@udunlisted"
data = (root / "source" / "config.cpp").read_bytes()
header = b"config.cpp\0" + struct.pack("<5I", 0, 0, 0, 0, len(data))
terminator = b"\0" + struct.pack("<5I", 0, 0, 0, 0, 0)
(root / "addons" / "udunlisted.pbo").write_bytes(header + terminator + data)
