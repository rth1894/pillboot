# PillBoot

A custom graphical UEFI bootloader inspired by the red pill / blue pill scene from *The Matrix*.

Instead of a traditional boot menu, two choices are given:

- Red Pill → Boot Arch Linux
- Blue Pill → Boot Windows

![PillBoot demo](./works.gif)

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
- Custom Linux boot (EFI handover protocol, no GRUB/stub required)
- Designed to chainload existing Windows and Linux bootloaders

---

## Project Status

The project currently boots as a native UEFI application, initializes GOP graphics, accesses the EFI filesystem, and successfully loads QOI assets from disk. The graphical menu and asset pipeline are fully functional.

Booting Arch Linux via the Red Pill option now works end-to-end: the kernel and initramfs are loaded from the ESP, `boot_params` is populated (including copying the kernel's real `setup_header` so fields like `initrd_addr_max` are valid), and control is handed off via the 64-bit EFI handover protocol.

Next major task is implementing the Blue Pill (Windows) boot action, which will chainload the existing Windows Boot Manager (`bootmgfw.efi`).

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

Red Pill boots Linux. Blue Pill (Windows) boot action is not implemented yet.

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

## Linux Boot

Project loads the Linux kernel and initramfs directly from the ESP and boots them using the 64-bit EFI handover protocol (no GRUB or separate EFI stub chainload required).

```text
    vmlinuz + initramfs (ESP)
       |
    read_file()
       |
    load_kernel() -> relocatable kernel placed in memory
       |
    boot_params populated
    (setup_header copied, ramdisk + cmdline pointers set)
       |
    EFI handover entry (kernel_addr + handover_offset + 0x200)
       |
    Linux kernel
```

Requires a 64-bit EFI handover-capable kernel (`xloadflags` with `XLF_EFI_HANDOVER_64` set, protocol >= 0x020b).
