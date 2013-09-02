// Utilities for accessing the GPIO on a Raspberry PI
// Used by StripBuffer

#include "utilsPrecomp.h"

#ifndef OS_WINDOWS
//#ifdef __arm__
#include "utilsGPIO.h"
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>

namespace GPIO {

// Raspberry Pi GPIO access
// Register offsets come from the data sheet
// This is probably the fastest way to access the GPIO but it requires root access.
const uint32    kBaseAddr   = 0x20200000; // for BCM2708 on Raspberry Pi

// This is the memory mapped reference to the GPIO pins
volatile uint32* gGPIO = NULL;

// Offsets to key registers (note that these are in terms of 4 byte words, so multiply by 4 to get the offset in bytes)
const uint32 kOffsetFcnSelect   =  0; // 3 bits are used to select the function, 10 GPIOs per register word
const uint32 kOffsetOutSet      =  7; // Bit field
const uint32 kOffsetOutClear    = 10; // Bit field


//------------------------------------------------------------------------
// Setup functions
//------------------------------------------------------------------------
// Returns true on success, false on error

bool InitializeGPIO(string* errmsgptr) {
    static bool firstTime = true;
    static bool status = false;
    static string errmsg;

    if (! firstTime) {
        if (errmsgptr) *errmsgptr = errmsg;
        return status;
    }

    // Memory map the address
    int     mem_fd;
    void*   gpio_map;

    // Map the GPIO Controller address
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        errmsg = "While initializing GPIO, failed to open /dev/mem. Rerun via 'sudo'");
        if (errmsgptr) *errmsgptr = errmsg;
        first_time = false;
        return false;
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,             //Any adddress in our space will do
        4 * 1024,         //Map length (more than enough)
        PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
        MAP_SHARED,       //Shared with other processes
        mem_fd,           //File to map
        kBaseAddr         //Offset to GPIO peripheral
    );

    close(mem_fd); //No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED) {
        errmsg = "While initializing GPIO, mmap error " + IntToStr((int)errno);
        if (errmsgptr) *errmsgptr = errmsg;
        first_time = false;
        return false;
    }

    // Always use volatile pointer!
    gGPIO = (volatile uint32 *)gpio_map;
    status = true;
    first_time = false;
    return true;
}

// Returns true of the command should be executed
bool ValidateGPIO(int gpio) {
    if (gpio < 0 || gpio > kMaxGPIO) return false;
    if (gGPIO) return true;
    InitializeGPIO();
    return gGPIO;
}

//------------------------------------------------------------------------
// Access functions
//------------------------------------------------------------------------

void SetModeInput(int gpio) {
    if (! ValidateGPIO(gpio)) return;
    // Clear the GPIO fcn select (0 = Input)
    *(gGPIO+kOffsetFcnSelect+(gpio/10)) &= ~(7<<(((gpio)%10)*3))
}

void SetModeOutput(int gpio) {
    if (! ValidateGPIO(gpio)) return;
    // Clear the GPIO controls
    *(gGPIO+kOffsetFcnSelect+(gpio/10)) &= ~(7<<((gpio%10)*3))
    // Now set output mode
    *(gGPIO+kOffsetFcnSelect+(gpio/10)) |=  (1<<((gpio%10)*3));
}

void Write(int gpio, bool value) {
    if (! ValidateGPIO(gpio)) return;
    if (value)
        *(gGPIO + kOffsetOutSet   + gpio/32) = 1UL << (gpio%32);
    else
        *(gGPIO + kOffsetOutClear + gpio/32) = 1UL << (gpio%32);
    }

}; // namespace GPIO

#endif // __arm__


