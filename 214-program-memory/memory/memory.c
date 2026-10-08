#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "command.h"
#include "device.h"

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

int main(void);

uint32_t data_variable = 100;
uint32_t bss_variable;

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

static void code_row(const char *name, uintptr_t address)
{
    const uint16_t *code = (const uint16_t *)(address & ~1u);

    printf("%-15s 0x%08x  0x%04x\n", name, (unsigned)address, (unsigned)*code);
}

void fw_info(void)
{
    data_variable = data_variable + 1;
    bss_variable = bss_variable + 1;

    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    printf("object          address     value\n");

    code_row("main", (uintptr_t)main);
    code_row("fw_info", (uintptr_t)fw_info);

    printf("%-15s 0x%08x\n", "commands", (unsigned)(uintptr_t)commands);

    for (uint i = 0; i < command_count; i++)
    {
        printf("- %-13s 0x%08x\n", commands[i].name, (unsigned)(uintptr_t)commands[i].handler);
    }

    printf("%-15s 0x%08x  %s\n", "DEVICE_PROJECT", (unsigned)(uintptr_t)DEVICE_PROJECT, DEVICE_PROJECT);
    printf("%-15s 0x%08x  %s\n", "DEVICE_BOARD", (unsigned)(uintptr_t)DEVICE_BOARD, DEVICE_BOARD);

    printf("%-15s 0x%08x  %u\n", "data_variable", (unsigned)(uintptr_t)&data_variable, (unsigned)data_variable);
    printf("%-15s 0x%08x  %u\n", "bss_variable", (unsigned)(uintptr_t)&bss_variable, (unsigned)bss_variable);
    printf("%-15s 0x%08x  %u\n", "stack_variable", (unsigned)(uintptr_t)&stack_variable, (unsigned)stack_variable);

    if (heap_variable != NULL)
    {
        printf("%-15s 0x%08x  %u\n", "heap_variable", (unsigned)(uintptr_t)heap_variable, (unsigned)*heap_variable);
    }
    else
    {
        printf("%-15s no memory\n", "heap_variable");
    }

    free(heap_variable);
}

