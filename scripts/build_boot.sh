#!/usr/bin/env bash
set -euo pipefail

# Build aarch64 UEFI application (BOOTAA64.EFI) using gnu-efi if available.
# This script attempts common workflows and prints guidance if it can't proceed.

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=$ROOT/build
mkdir -p "$OUT"

# tool detection
CC=""
OBJCOPY=""
LD=""

if command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
    CC=aarch64-linux-gnu-gcc
    OBJCOPY=aarch64-linux-gnu-objcopy
    LD=aarch64-linux-gnu-ld
elif command -v aarch64-elf-gcc >/dev/null 2>&1; then
    CC=aarch64-elf-gcc
    OBJCOPY=aarch64-elf-objcopy
    LD=aarch64-elf-ld
elif command -v gcc >/dev/null 2>&1; then
    echo "Warning: native gcc found, but cross-compiler is preferred for aarch64 UEFI builds. Continuing with gcc may fail." >&2
    CC=gcc
    OBJCOPY=objcopy
    LD=ld
else
    echo "No suitable aarch64 compiler found (aarch64-linux-gnu-gcc or aarch64-elf-gcc)." >&2
    echo "Install cross toolchain or EDK2/gnu-efi. See README.md for instructions." >&2
    exit 1
fi

# Locate gnu-efi headers
EFIINC="/usr/include/efi"
EFIINC_AARCH64="/usr/include/efi/aarch64"
if [ ! -d "$EFIINC" ]; then
    echo "Warning: /usr/include/efi not found. gnu-efi headers may be missing." >&2
fi

echo "Using CC=$CC, OBJCOPY=$OBJCOPY"

# Compile
echo "Compiling boot/boot.c..."
$CC -fshort-wchar -fno-stack-protector -fPIC -I${EFIINC} -I${EFIINC_AARCH64} -c boot/boot.c -o $OUT/boot.o || true

# Try linking with gnu-efi style
if command -v ${LD} >/dev/null 2>&1; then
    echo "Attempting to link with ${LD} (gnu-efi style)."
    # Create intermediate shared object and convert to binary .efi
    ${LD} -Bsymbolic -Ttext 0 -o $OUT/boot.so $OUT/boot.o || true
    if command -v ${OBJCOPY} >/dev/null 2>&1; then
        echo "Converting to PE/COFF binary BOOTAA64.EFI..."
        ${OBJCOPY} -O binary $OUT/boot.so $OUT/BOOTAA64.EFI || true
        if [ -f $OUT/BOOTAA64.EFI ]; then
            echo "Created $OUT/BOOTAA64.EFI"
            exit 0
        fi
    fi
fi

cat <<'EOF' >&2
Could not automatically produce BOOTAA64.EFI. Build instructions:

Option A (recommended): install EDK II (AAVMF) and build using EDK2 toolchain.
Option B: install gnu-efi for aarch64 and the aarch64 cross-toolchain (aarch64-linux-gnu-gcc).
Then try:
  aarch64-linux-gnu-gcc -fshort-wchar -fno-stack-protector -fPIC -I /usr/include/efi -I /usr/include/efi/aarch64 -c boot/boot.c -o boot.o
  aarch64-linux-gnu-ld -Bsymbolic -Ttext 0 -o boot.so boot.o
  aarch64-linux-gnu-objcopy -O binary boot.so BOOTAA64.EFI

See README.md for more detailed steps.
EOF

exit 2
