# PillBoot

A custom graphical UEFI bootloader inspired by the red pill / blue pill scene from *The Matrix*.

Instead of a traditional boot menu, two choices are given:

- Red Pill → Boot Arch Linux
- Blue Pill → Boot Windows

---

## Features

- Native UEFI application
- GNU-EFI based build system
- Direct framebuffer rendering using the UEFI Graphics Output Protocol (GOP)
- Software renderer
- BMP asset loading
- Custom software renderer
- Bitmap image rendering
- Keyboard navigation
- Animated menu
- Chainloads existing Windows and Linux bootloaders

---

## Project Status

The project currently boots as a native UEFI, initializes GOP Graphics, accesses EFI filesystem, and successfully loads BMP assets from disk.

The next task is to render decoded bitmap pixels to the framebuffer, after which the custom graphical boot menu and animations will be implemented.

---

## Building

Requirements:

- GNU-EFI
- GCC
- QEMU
- OVMF

Build:

```bash
make
```

Run:

```bash
./run.sh
```

---

## Screenshots

Coming soon.
