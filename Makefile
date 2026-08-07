ARCH := x86_64

CC := gcc
LD := ld

EFILIB := /usr/lib

CFLAGS := \
    -Iinclude \
    -I/usr/include/efi \
    -I/usr/include/efi/$(ARCH) \
    -DCONFIG_$(ARCH) \
    -DGNU_EFI_USE_MS_ABI \
    -std=c11 \
    -fPIC \
    -fPIE \
	-fshort-wchar \
    -maccumulate-outgoing-args \
    -mno-red-zone \
    -mno-avx \
    -ffreestanding \
    -fno-stack-protector \
    -fno-strict-aliasing \
    -fno-merge-all-constants \
    -Wall \
    -Wextra \
    -Wstrict-prototypes

LDFLAGS := \
    -nostdlib \
    --warn-common \
    --no-undefined \
    --fatal-warnings \
    --build-id=sha1 \
    -z norelro \
    -z nocombreloc \
    -pie \
    -Bsymbolic \
    --no-dynamic-linker \
    -T /usr/lib/elf_$(ARCH)_efi.lds \
    /usr/lib/crt0-efi-$(ARCH).o

LIBS := \
    -L/usr/lib \
    -lefi \
    -lgnuefi \
    $(shell $(CC) -print-libgcc-file-name)

BUILD := build
TARGET := PillBoot

all: $(BUILD)/$(TARGET).efi

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/main.o: src/main.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/graphics.o: src/graphics.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).so: $(BUILD)/main.o $(BUILD)/graphics.o
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD)/$(TARGET).efi: $(BUILD)/$(TARGET).so
	objcopy \
		-j .text \
		-j .sdata \
		-j .data \
		-j .dynamic \
		-j .rodata \
		-j .rel \
		-j .rela \
		-j .rel.* \
		-j .rela.* \
		-j .rel* \
		-j .rela* \
		-j .areloc \
		-j .reloc \
		-O efi-app-$(ARCH) \
		$< $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
