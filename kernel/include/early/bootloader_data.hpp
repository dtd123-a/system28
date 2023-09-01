/*
    * bootloader_data.hpp
    * Declarations for bootloader data component
    * Created 01/09/23 DanielH
*/

#pragma once
#include <limine.h>

struct BootloaderData
{
    limine_framebuffer_response fbData;
};

BootloaderData GetBootloaderData();
