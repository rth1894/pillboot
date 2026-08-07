# PillBoot

A custom graphical UEFI bootloader inspired by the red pill / blue pill scene from *The Matrix*.

Instead of a traditional boot menu, two choices are given:

- Red Pill → Boot Arch Linux
- Blue Pill → Boot Windows

---

## Features

- Native UEFI application
- Direct framebuffer rendering using the UEFI Graphics Output Protocol (GOP)
- Custom software renderer
- Bitmap image rendering
- Keyboard navigation
- Animated menu
- Chainloads existing Windows and Linux bootloaders

---

## Project Status

### Completed

- [x] Standalone GNU-EFI build system
- [x] Graphics Output Protocol initialization
- [x] Framebuffer access
- [x] Screen clearing

### In Progress

- [ ] Software renderer
- [ ] Primitive drawing
- [ ] Bitmap loader
- [ ] Asset rendering
- [ ] Keyboard input
- [ ] Hand animations
- [ ] Boot menu
- [ ] Windows chainloader
- [ ] Linux chainloader

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
