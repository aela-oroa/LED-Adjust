#include "ledinfo.h"

LedInfo LedFactory::createLed(Led led)
{
    switch (led)
    {
    case Led::Led1:
        return {0xB8, 0xAF, 0xC1, 0x38, 0x39, -1};

    case Led::Led2:
        return {0xB8, 0xAF, 0xC1, 0x35, -1, 0x36};

    case Led::Led3:
        return {0xB8, 0xAF, 0xC1, 0x3F, 0x41, 0x42};

    case Led::Led4:
        return {0xB8, 0xAF, 0xC1, 0x3B, 0x3C, 0x3E};

    default:
        return {0, 0, 0, 0, 0, 0};
    }
}
