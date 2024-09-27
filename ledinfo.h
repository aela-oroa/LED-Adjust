#ifndef LEDINFO_H
#define LEDINFO_H


struct LedInfo
{
    int pwmForRedAddr;
    int pwmForGreenAddr;
    int pwmForBlueAddr;

    int connectionForRedAddr;
    int connectionForGreenAddr;
    int connectionForBlueAddr;
};


enum class Led
{
    Led1 = 0,
    Led2,
    Led3,
    Led4
};


namespace LedFactory
{
    LedInfo createLed(Led led);
}

#endif // LEDINFO_H
