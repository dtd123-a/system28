/*
    * bootloader_data.cpp
    * Bootloader data component
    * Created 01/09/23 DanielH
*/

#include <limine.h>
#include <early/bootloader_data.hpp>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
    .response = 0
};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
    .response = 0,
    .flags = 0 
};

static volatile struct limine_kernel_address_request kaddr_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0,
    .response = 0
};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
    .response = 0  
};

BootloaderData GetBootloaderData()
{
    BootloaderData ret = {
        .fbData = *framebuffer_request.response,
        .memmap = *memmap_request.response,
        .smp = *smp_request.response,
        .kernel_addr = *kaddr_request.response,
        .hhdm_response = *hhdm_request.response,
        .rsdp_response = *rsdp_request.response
    };

    return ret;
}