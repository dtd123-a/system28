/*
    * bootloader_data.cpp
    * Bootloader data component
    * Created 01/09/23 DanielH
*/

#include <limine.h>
#include <early/bootloader_data.hpp>

static volatile struct limine_framebuffer_request framebuffer_request =
{
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = 0
};

BootloaderData GetBootloaderData()
{
    BootloaderData ret = {
        .fbData = *framebuffer_request.response
    };

    return ret;
}