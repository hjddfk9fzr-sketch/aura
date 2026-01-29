#!/usr/bin/env bash
set -euo pipefail

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=$ROOT/build
mkdir -p "$OUT"
IMAGE=$OUT/image.img
SIZE=64M

BOOT_EFI=$ROOT/build/BOOTAA64.EFI
KERNEL=$ROOT/kernel.elf

if [ ! -f "$BOOT_EFI" ]; then
    echo "Error: $BOOT_EFI not found. Build boot.efi first (scripts/build_boot.sh)" >&2
    exit 1
fi
if [ ! -f "$KERNEL" ]; then
    echo "Error: $KERNEL not found. Run 'make' to build kernel.elf" >&2
    exit 1
fi

# Create empty image file
rm -f "$IMAGE"
dd if=/dev/zero of="$IMAGE" bs=1 count=0 seek=$SIZE

# Format as vfat
if command -v mkfs.vfat >/dev/null 2>&1; then
    mkfs.vfat -n AURABOOT "$IMAGE"
else
    echo "mkfs.vfat not found. Install dosfstools (Linux) or use mtools on macOS." >&2
    exit 1
fi

# Use mtools if available to copy files without root
if command -v mcopy >/dev/null 2>&1; then
    echo "Using mtools to copy files into image..."
    # prepare EFI dir
    mmd -i "$IMAGE" ::/EFI || true
    mmd -i "$IMAGE" ::/EFI/BOOT || true
    mcopy -i "$IMAGE" "$BOOT_EFI" ::/EFI/BOOT/BOOTAA64.EFI
    mcopy -i "$IMAGE" "$KERNEL" ::/kernel.elf
    echo "Created image: $IMAGE"
    exit 0
fi

# Fallback: try mounting (Linux with loop)
if [ "$(uname)" = "Linux" ]; then
    echo "Mounting image (requires root)..."
    TMPMNT=$(mktemp -d)
    sudo mount -o loop "$IMAGE" "$TMPMNT"
    sudo mkdir -p "$TMPMNT/EFI/BOOT"
    sudo cp "$BOOT_EFI" "$TMPMNT/EFI/BOOT/BOOTAA64.EFI"
    sudo cp "$KERNEL" "$TMPMNT/"
    sudo umount "$TMPMNT"
    rmdir "$TMPMNT"
    echo "Created image: $IMAGE"
    exit 0
fi

echo "Could not copy files into image automatically. Install 'mtools' or run the mount-based copy method on Linux." >&2
exit 2
