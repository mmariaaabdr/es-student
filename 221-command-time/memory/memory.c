#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "command.h"
#include "device.h"
#include "led.h"

#define SRAM_SIZE (264 * 1024)
#define ROM_SIZE (16 * 1024)

#define VECTOR_TABLE 0x10000100
#define GPIO_IN 0xd0000004

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

void fw_info(void)
{
    data_variable = data_variable + 1;
    bss_variable = bss_variable + 1;

    uint16_t *main_code = (uint16_t *)((uintptr_t)main & ~1u);
    uint16_t *fw_info_code = (uint16_t *)((uintptr_t)fw_info & ~1u);

    uint32_t stack_variable = 1946;
    uint32_t *heap_variable = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    printf("object          address     value\n");

    printf("%-15s 0x%08x  0x%04x\n", "main", (unsigned)(uintptr_t)main, (unsigned)*main_code);
    printf("%-15s 0x%08x  0x%04x\n", "fw_info", (unsigned)(uintptr_t)fw_info, (unsigned)*fw_info_code);

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

void boot_info(void)
{
    const uint32_t *vectors = (const uint32_t *)VECTOR_TABLE;

    uint32_t stack_top = vectors[0];
    uint32_t reset_handler = vectors[1];

    volatile uint32_t *gpio_in = (volatile uint32_t *)GPIO_IN;
    uint32_t level = (*gpio_in >> led_pin()) & 1u;

    printf("%-14s 0x%08x\n", "vector table", (unsigned)(uintptr_t)vectors);
    printf("  %-12s 0x%08x\n", "stack top", (unsigned)stack_top);
    printf("  %-12s 0x%08x\n", "reset", (unsigned)reset_handler);
    printf("  %-12s 0x%08x\n", "reset (even)", (unsigned)(reset_handler & ~1u));
    printf("%-14s 0x%08x\n", "gpio in", (unsigned)(uintptr_t)gpio_in);
    printf("  %-12s %u\n", "led bit", (unsigned)level);
    printf("  %-12s %u\n", "gpio_get", (unsigned)gpio_get(led_pin()));
}
