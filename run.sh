#!/bin/bash

set -e

make

mkdir -p esp/EFI/BOOT
cp build/PillBoot.efi esp/EFI/BOOT/BOOTX64.EFI

cp /usr/share/edk2/x64/OVMF_VARS.4m.fd OVMF_VARS.fd 2>/dev/null || true

qemu-system-x86_64 \
    -machine q35 \
    -m 512M \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd \
    -drive if=pflash,format=raw,file=OVMF_VARS.fd \
    -drive format=raw,file=fat:rw:esp
