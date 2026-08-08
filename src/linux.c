// too much time has been spent on this
#include "linux.h"
#include "efidef.h"
#include "efilib.h"

#include <efibind.h>
#include <efiprot.h>
#include <efierr.h>

#define BOOT_PARAMS_SIZE 0x1000
#define SETUP_HEADER_OFFSET 0x1f1

#define HDR_SETUP_SECTS        0x1f1
#define HDR_KERNEL_VERSION     0x20e
#define HDR_KERNEL_ALIGNMENT   0x230
#define HDR_RELOCATABLE_KERNEL 0x234
#define HDR_XLOADFLAGS         0x236
#define HDR_CODE32_START       0x214
#define HDR_CMD_LINE_PTR       0x228
#define HDR_RAMDISK_IMAGE      0x218
#define HDR_RAMDISK_SIZE       0x21c
#define HDR_PREF_ADDRESS       0x258
#define HDR_INIT_SIZE          0x260
#define HDR_HANDOVER_OFFSET    0x264

#define XLF_KERNEL_64              (1 << 0)
#define XLF_CAN_BE_LOADED_ABOVE_4G (1 << 1)
#define XLF_EFI_HANDOVER_32        (1 << 2)
#define XLF_EFI_HANDOVER_64        (1 << 3)

/*
 * identify as loader and mark kernel as loaded high
 * this is done because bzImage protected-mode code is placed above 1mb;
*/
#define HDR_TYPE_OF_LOADER 0x210
#define HDR_LOADFLAGS      0x211
#define LOADED_HIGH        0x01

#define CODE32_START_OFFSET 0x200

typedef struct {
    UINT8 data[BOOT_PARAMS_SIZE];
} LinuxBootParams;

static UINT16 read16(UINT8* p) {
    return (UINT16)p[0] | ((UINT16)p[1] << 8);
}

static UINT32 read32(UINT8* p) {
    return (UINT32)p[0] | ((UINT32)p[1] << 8) |
           ((UINT32)p[2] << 16) | ((UINT32)p[3] << 24);
}

static UINT64 read64(UINT8* p) {
    return (UINT64)read32(p) | ((UINT64)read32(p + 4) << 32);
}

static void write32(UINT8* p, UINT32 value) {
    p[0] = value & 0xff;
    p[1] = (value >> 8) & 0xff;
    p[2] = (value >> 16) & 0xff;
    p[3] = (value >> 24) & 0xff;
}

static void write64(UINT8* p, UINT64 value) {
    write32(p, value & 0xffffffff);
    write32(p + 4, value >> 32);
}

static EFI_STATUS read_file(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* path, VOID** buffer, UINTN* size) {
    EFI_STATUS status;

    EFI_LOADED_IMAGE_PROTOCOL* loadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    EFI_FILE_PROTOCOL* root;
    EFI_FILE_PROTOCOL* file;
    EFI_FILE_INFO* info;

    status = uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (void** )&loadedImage);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(
        SystemTable->BootServices->HandleProtocol,
        3,
        loadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (void**)&fs
    );

    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(fs->OpenVolume, 2, fs, &root);
    if (EFI_ERROR(status)) return status;

    status = uefi_call_wrapper(root->Open, 5, root, &file, path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }

    UINTN infoSize = 0;
    status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid, &infoSize, NULL);
    if (status != EFI_BUFFER_TOO_SMALL) {
        uefi_call_wrapper(file->Close, 1, file);
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }

    status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool,
        3,
        EfiLoaderData,
        infoSize,
        (VOID**)&info
    );

    if (EFI_ERROR(status)) {
        uefi_call_wrapper(file->Close, 1, file);
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }

    status = uefi_call_wrapper(file->GetInfo, 4, file, &gEfiFileInfoGuid, &infoSize, info);

    if (EFI_ERROR(status)) {
        FreePool(info);
        uefi_call_wrapper(file->Close, 1, file);
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }

    *size = info->FileSize;

    FreePool(info);

    status = uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, *size, buffer);

    if (EFI_ERROR(status)) {
        uefi_call_wrapper(file->Close, 1, file);
        uefi_call_wrapper(root->Close, 1, root);
        return status;
    }

    UINTN readSize = *size;

    status = uefi_call_wrapper(file->Read, 3, file, &readSize, *buffer);

    uefi_call_wrapper(file->Close, 1, file);
    uefi_call_wrapper(root->Close, 1, root);

    if (EFI_ERROR(status) || readSize != *size) {
        FreePool(*buffer);
        *buffer = NULL;
        return EFI_LOAD_ERROR;
    }

    return EFI_SUCCESS;
}

static EFI_STATUS allocate_low_memory(EFI_SYSTEM_TABLE* SystemTable, UINTN size, VOID** buffer, EFI_PHYSICAL_ADDRESS* address) {
    UINTN pages = EFI_SIZE_TO_PAGES(size);
    EFI_PHYSICAL_ADDRESS maxAddress = 0xFFFFFFFFULL;

    EFI_STATUS status = uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateMaxAddress, EfiLoaderData, pages, &maxAddress);
    if (EFI_ERROR(status)) return status;

    *buffer = (VOID*) (UINTN)maxAddress;
    *address = maxAddress;
    return EFI_SUCCESS;
}

static EFI_STATUS load_kernel(EFI_SYSTEM_TABLE* SystemTable, VOID* kernelFile, UINTN kernelFileSize, VOID** kernelLoadAddress) {
    UINT8* kernel = kernelFile;
    UINT8 setupSectors = kernel[HDR_SETUP_SECTS];

    if (setupSectors == 0) setupSectors = 4;
    UINTN kernelOffset = (setupSectors + 1) * 512;

    if (kernelFileSize <= kernelOffset) return EFI_LOAD_ERROR;

    UINT32 kernelAlignment = read32(kernel + HDR_KERNEL_ALIGNMENT);
    UINT32 initSize = read32(kernel + HDR_INIT_SIZE);
    UINT8 relocatable = kernel[HDR_RELOCATABLE_KERNEL];

    if (!relocatable) return EFI_UNSUPPORTED;
    if (kernelAlignment == 0) kernelAlignment = 0x1000;

    UINTN payloadSize = kernelFileSize - kernelOffset;
    if (initSize < payloadSize) initSize = payloadSize;

    UINTN pages = EFI_SIZE_TO_PAGES(initSize);
    UINTN extraPages = EFI_SIZE_TO_PAGES(kernelAlignment);

    EFI_PHYSICAL_ADDRESS maxAddress = 0xFFFFFFFFULL;
    EFI_PHYSICAL_ADDRESS allocation;

    UINTN totalPages = pages + extraPages;

    EFI_STATUS status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePages,
        4,
        AllocateMaxAddress,
        EfiLoaderCode,
        totalPages,
        &maxAddress
    );

    if (EFI_ERROR(status)) return status;
    allocation = maxAddress;

    EFI_PHYSICAL_ADDRESS aligned =
        (allocation + kernelAlignment - 1) & ~((EFI_PHYSICAL_ADDRESS)kernelAlignment - 1);

    EFI_PHYSICAL_ADDRESS end = aligned + EFI_SIZE_TO_PAGES(initSize) * 4096ULL;

    EFI_PHYSICAL_ADDRESS allocatedEnd = allocation + totalPages * 4096ULL;
    if (end > allocatedEnd) {
        uefi_call_wrapper(SystemTable->BootServices->FreePages, 2, allocation, totalPages);
        return EFI_OUT_OF_RESOURCES;
    }

    CopyMem((VOID*)(UINTN)aligned, kernel + kernelOffset, payloadSize);
    *kernelLoadAddress = (VOID*)(UINTN)aligned;
    return EFI_SUCCESS;
}

static EFI_STATUS load_cmdline(EFI_SYSTEM_TABLE* SystemTable, CHAR8** cmdline, UINTN* cmdlineSize, EFI_PHYSICAL_ADDRESS* address) {
    CHAR8 commandLine[] = "root=UUID=f06b1f1a-fa76-4617-9315-2b3a0ecbe923 rw";
    // CHAR8 commandLine[] = "root=UUID=f06b1f1a-fa76-4617-9315-2b3a0ecbe923 rw rdinit=/bin/sh";


    UINTN size = sizeof(commandLine);
    EFI_PHYSICAL_ADDRESS maxAddress = 0xFFFFFFFFULL;

    EFI_STATUS status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePages,
        4,
        AllocateMaxAddress,
        EfiLoaderData,
        EFI_SIZE_TO_PAGES(size),
        &maxAddress
    );
    if (EFI_ERROR(status)) return status;

    CopyMem((VOID*)(UINTN)maxAddress, commandLine, size);

    *cmdline = (CHAR8*)(UINTN)maxAddress;
    *cmdlineSize = size;
    *address = maxAddress;

    return EFI_SUCCESS;
}

EFI_STATUS linux_boot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    EFI_STATUS status;

    VOID* kernelFile = NULL;
    VOID* initrd = NULL;
    VOID* kernelLoadAddress = NULL;

    UINTN kernelFileSize = 0;
    UINTN initrdSize = 0;

    Print(L"Loading Linux kernel...\r\n");

    status = read_file(ImageHandle, SystemTable, L"\\vmlinuz-linux-zen", &kernelFile, &kernelFileSize);


    if (EFI_ERROR(status)) {
        Print(L"Kernel load failed: %r\r\n", status);
        return status;
    }

    Print(L"Kernel file: %lu bytes\r\n", kernelFileSize);
    Print(L"Loading initramfs...\r\n");

    status = read_file(ImageHandle, SystemTable, L"\\initramfs-linux-zen.img", &initrd, &initrdSize);

    if (EFI_ERROR(status)) {
        Print(L"Initramfs load failed: %r\r\n", status);
        FreePool(kernelFile);
        return status;
    }

    Print(L"Initramfs: %lu bytes\r\n", initrdSize);
    UINT8* kernel = kernelFile;

    void* initrdLow = NULL;
    EFI_PHYSICAL_ADDRESS initrdLowAddress = 0;

    status = allocate_low_memory(SystemTable, initrdSize, &initrdLow, &initrdLowAddress);
    if (EFI_ERROR(status)) {
        Print(L"Failed to alloc low mem for initramfs: %r\r\n", status);
        FreePool(initrd);
        FreePool(kernelFile);
        return status;
    }

    CopyMem(initrdLow, initrd, initrdSize);
    FreePool(initrd);
    initrd = initrdLow;

    if (kernel[0x1fe] != 0x55 || kernel[0x1ff] != 0xaa) {
        Print(L"Invalid Linux boot signature\r\n");
        return EFI_LOAD_ERROR;
    }

    UINT16 bootFlag = read16(kernel + 0x1fe);
    UINT16 protocol = read16(kernel + 0x206);
    UINT16 xloadflags = read16(kernel + HDR_XLOADFLAGS);
    UINT32 handoverOffset = read32(kernel + HDR_HANDOVER_OFFSET);

    Print(L"Linux protocol: %x\r\n", protocol);
    Print(L"XLoadFlags: %x\r\n", xloadflags);
    Print(L"Handover: %x\r\n", handoverOffset);

    if (bootFlag != 0xaa55) {
        Print(L"Invalid boot flag\r\n");
        return EFI_LOAD_ERROR;
    }

    if (protocol < 0x020b) {
        Print(L"Linux protocol too old\r\n");
        return EFI_UNSUPPORTED;
    }

    if (!(xloadflags & XLF_EFI_HANDOVER_64)) {
        Print(L"64-bit EFI handover unsupported\r\n");
        return EFI_UNSUPPORTED;
    }

    if (handoverOffset == 0) {
        Print(L"No EFI handover entry\r\n");
        return EFI_UNSUPPORTED;
    }

    status = load_kernel(SystemTable, kernelFile, kernelFileSize, &kernelLoadAddress);

    if (EFI_ERROR(status)) {
        Print(L"Kernel placement failed: %r\r\n", status);
        return status;
    }

    LinuxBootParams* bp = NULL;
    EFI_PHYSICAL_ADDRESS bpAddress = 0;

    status = allocate_low_memory(SystemTable, sizeof(LinuxBootParams), (VOID**)&bp, &bpAddress);
    if (EFI_ERROR(status)) {
        Print(L"Boot params allocation failed: %r\r\n", status);
        return status;
    }

    SetMem(bp, sizeof(LinuxBootParams), 0);

    /*
     * copy real setup_header (and code) from kernel file to boot_params so fields like
     * initrd_addr_max, kernel_alignment, xloadflags, version are populated correctly
    */
    CopyMem(bp->data + SETUP_HEADER_OFFSET, kernel + SETUP_HEADER_OFFSET,
        kernelFileSize - SETUP_HEADER_OFFSET < BOOT_PARAMS_SIZE - SETUP_HEADER_OFFSET
            ? kernelFileSize - SETUP_HEADER_OFFSET : BOOT_PARAMS_SIZE - SETUP_HEADER_OFFSET);

    UINT64 kernelAddress = (UINT64)(UINTN)kernelLoadAddress;
    UINT64 initrdAddress = (UINT64)(UINTN)initrd;
    CHAR8* cmdline;
    UINTN cmdlineSize;
    EFI_PHYSICAL_ADDRESS cmdlineAddress;

    status = load_cmdline(SystemTable, &cmdline, &cmdlineSize, &cmdlineAddress);
    if (EFI_ERROR(status)) return status;

    /*
     * Linux boot protocol fields.
     */

    write32(bp->data + HDR_CODE32_START, (UINT32)kernelAddress);
    write32(bp->data + HDR_RAMDISK_IMAGE, (UINT32)initrdAddress);
    write32(bp->data + HDR_RAMDISK_SIZE, (UINT32)initrdSize);
    write32(bp->data + HDR_CMD_LINE_PTR, (UINT32)cmdlineAddress);

    bp->data[HDR_TYPE_OF_LOADER] = 0xFF;  // "unknown" bootloadet id
    bp->data[HDR_LOADFLAGS] = LOADED_HIGH;

    Print(L"bp ramdisk_image: %lx\r\n", (UINT64)read32(bp->data + HDR_RAMDISK_IMAGE));
    Print(L"bp ramdisk_size: %lx\r\n", (UINT64)read32(bp->data + HDR_RAMDISK_SIZE));
    Print(L"bp cmdline_ptr: %lx\r\n", (UINT64)read32(bp->data + HDR_CMD_LINE_PTR));
    Print(L"cmdlind: %a\r\n", cmdline);

    /*
     * The EFI handover entry is relative to the beginning of the kernel image.
     *
     * For the 64-bit handover protocol the entry point is handover_offset + 0x200.
     */

    UINT64 entry = kernelAddress + handoverOffset + CODE32_START_OFFSET;

    Print(L"Kernel address: %lx\r\n", kernelAddress);
    Print(L"Initrd address: %lx\r\n", initrdAddress);
    Print(L"Initrd size: %lx\r\n", (UINT64)initrdSize);
    Print(L"Cmdline address: %lx\r\n", cmdlineAddress);
    Print(L"Bootparams: %lx\r\n", (UINT64)(UINTN)bp);
    Print(L"Command line loaded\r\n");

    Print(L"initrd first bytes: %02x, %02x %02x %02x\r\n",
            ((UINT8*) initrd)[0], ((UINT8*) initrd)[1], ((UINT8*) initrd)[2], ((UINT8*) initrd)[3]);

    Print(L"cmdline: [%a]\r\n", cmdline);
    Print(L"initrd_addr_max: %lx\r\n", (UINT64)read32(bp->data + 0x22c));
    Print(L"Starting Linux...\r\n");

    typedef void (*LinuxHandover)(EFI_HANDLE, EFI_SYSTEM_TABLE*, LinuxBootParams*);

    LinuxHandover handover = (LinuxHandover)(UINTN)entry;
    handover(ImageHandle, SystemTable, bp);
    return EFI_LOAD_ERROR;
}
