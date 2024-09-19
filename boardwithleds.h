#ifndef BOARDWITHLEDS_H
#define BOARDWITHLEDS_H

#include <QSerialPort>
#include <QString>
#include "ledinfo.h"

class BoardWithLeds
{
public:
    explicit BoardWithLeds(Led led);
    ~BoardWithLeds();

    void setLed(Led led);
    void setPortName(const QString &portName);
    bool connect();
    void disconnect();
    bool isConnected() const;
    void setLedColor(int redValue, int greenValue, int blueValue);
    void sendCommand(int address, int value);

private:
    QString createCommand(int address, int value);

    QSerialPort *serialPort;
    QString portName;
    LedInfo ledInfo;
};

#endif // BOARDWITHLEDS_H
