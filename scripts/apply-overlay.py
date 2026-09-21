"""Overlay our custom files onto a fresh telink_zigbee_sdk checkout.

Usage:
    python apply-overlay.py <overlay/tl_zigbee_sdk> <sdk/tl_zigbee_sdk>

Copies every file from the overlay tree onto the matching path in the SDK.
New files (zb_tunnel.c, zb_ieee.*) are added, existing files are overwritten
in full. Exits non-zero if an expected existing target is missing (SDK
version drift) — update SDK_REF in that case. Stdlib only.
"""
import shutil
import sys
from pathlib import Path

# Files that must already exist in the stock SDK (i.e. modifications).
# Anything else in the overlay tree is treated as a new file.
EXPECTED_EXISTING = {
    "CMakeLists.txt",
    "apps/common/comm_cfg.h",
    "apps/zigbee/sampleGW/sampleGateway.c",
    "apps/zigbee/sampleGW/app_ui.c",
    "apps/zigbee/sampleGW/board_b91_dongle.h",
    "apps/zigbee/sampleGW/zcl_sampleGatewayCb.c",
    "proj/drivers/drv_hw.c",
    "platform/chip_b91/clock.c",
    "stack/zigbee/zb_hci/zbhci.h",
    "stack/zigbee/zb_hci/zbhciCmdProcess.c",
}

if len(sys.argv) != 3:
    sys.exit("usage: apply-overlay.py <overlay/tl_zigbee_sdk> <sdk/tl_zigbee_sdk>")

overlay = Path(sys.argv[1])
sdk = Path(sys.argv[2])
assert (overlay / "apps" / "zigbee" / "sampleGW" / "zb_tunnel.c").is_file(), "bad overlay path?"
assert (sdk / "apps" / "zigbee" / "sampleGW").is_dir(), "bad sdk path?"

added, overwritten = [], []
for src in sorted(overlay.rglob("*")):
    if not src.is_file():
        continue
    rel = src.relative_to(overlay).as_posix()
    dst = sdk / rel
    if rel in EXPECTED_EXISTING and not dst.is_file():
        sys.exit(f"ERROR: expected stock file missing in SDK: {rel} "
                 f"(SDK version drift? check sdk-ref.txt)")
    dst.parent.mkdir(parents=True, exist_ok=True)
    is_new = not dst.is_file()
    shutil.copyfile(src, dst)
    (added if is_new else overwritten).append(rel)

print(f"overlay applied: {len(overwritten)} overwritten, {len(added)} new")
for r in overwritten:
    print(f"  M {r}")
for r in added:
    print(f"  A {r}")
