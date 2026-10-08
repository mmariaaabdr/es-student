#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"

#define SRAM_SIZE (264 * 1024)
#define ROM_SIZE (16 * 1024)

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
           name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    uintptr_t flash_end = XIP_BASE + PICO_FLASH_SIZE_BYTES;
    uintptr_t data_size = (uintptr_t)(&__data_end__ - &__data_start__);

    printf("area       start      end        size\n");

    row("flash", XIP_BASE, flash_end);
    row("sram", SRAM_BASE, SRAM_BASE + SRAM_SIZE);
    row("rom", ROM_BASE, ROM_BASE + ROM_SIZE);

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free", (uintptr_t)&__flash_binary_end, flash_end);
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__etext + data_size);
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    unsigned image = (unsigned)(&__flash_binary_end - &__flash_binary_start);
    unsigned boot2 = (unsigned)(&__boot2_end__ - &__boot2_start__);
    unsigned text = (unsigned)(&__etext - &__boot2_end__);
    unsigned data = (unsigned)data_size;
    unsigned bss = (unsigned)(&__bss_end__ - &__bss_start__);
    unsigned heap = (unsigned)(&__HeapLimit - &__bss_end__);
    unsigned stack = (unsigned)(&__StackTop - &__StackBottom);

    printf("\ntotal\n");
    printf("  %-11s %8u = boot2 %u + text %u + data %u\n", "flash image", image, boot2, text, data);
    printf("  %-11s %8u of %u\n", "flash free", (unsigned)(flash_end - (uintptr_t)&__flash_binary_end),
           (unsigned)PICO_FLASH_SIZE_BYTES);
    printf("  %-11s %8u = data %u + bss %u\n", "ram used", data + bss, data, bss);
    printf("  %-11s %8u for heap and %u for stack\n", "ram free", heap, stack);
}

