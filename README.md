# PillBoot

A custom graphical UEFI bootloader inspired by the red pill / blue pill scene from *The Matrix*.

Instead of a traditional boot menu, two choices are given:

- Red Pill → Boot Arch Linux
- Blue Pill → Boot Windows

---

## Features

- Native UEFI application
- GNU-EFI based build system
- UEFI Graphics Output Protocol (GOP)
- Software framebuffer renderer
- Double-buffered rendering
- QOI image loading
- Custom image rendering
- Keyboard navigation
- Red Pill / Blue Pill menu scene
- Animated menu selection
- EFI filesystem access
- Designed to chainload existing Windows and Linux bootloaders

---

## Project Status

The project currently boots as a native UEFI, initializes GOP Graphics, accesses EFI filesystem, and successfully loads BMP assets from disk.

The graohical menu and asset pipeline are now functional.

Next major task is to implement the actual red pill / blue pill boot actions, which include chainloading the appropriate existing bootloader.

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

## Controls

| Key   | Action           |
| ----- | ---------------- |
| <-    | Select red Pill  |
| ->    | Select blue Pill |
| Enter | Confirm selection|
| Esc   | Escape           |

Actual boot actions are not implemented yet.

---

## Rendering

Project uses the UEFI Graphics output Protocol to obtain the framebuffer.
Rendering is performed into a software backbuffer before being copied to the framebuffer.

```text

    Assets
       |
    QOI Decoder
       |
    Image
       |
    Scene
       |
    Software renderer
       |
    Backbuffer
       |
    Framebuffer
       |
    Display

```

QOI was chosen because it is simple to decode and lightweight to use in a small freestanding bootloader env.

In previous bmp impl, color key transparency was to be used, did not work well for me.

## Screenshots

Coming soon.
