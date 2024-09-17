#include "boardwithleds.h"
#include <QDebug>

// Constructor: Initializes the serial port and LedInfo for the specified LED
BoardWithLeds::BoardWithLeds(Led led)
{
    serialPort = new QSerialPort();
    ledInfo = LedFactory::createLed(led);
}

// Destructor: Closes the serial port if open and cleans up
BoardWithLeds::~BoardWithLeds()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
    delete serialPort;
}

// Set the name of the COM port
void BoardWithLeds::setPortName(const QString &portName)
{
    this->portName = portName;
    qDebug() << "BoardWithLeds port name set to" << portName;
}

// Connect to the serial port
bool BoardWithLeds::connect()
{
    if (!serialPort->isOpen()) {
        serialPort->setPortName(portName);
        qDebug() << "Attempting to connect to" << portName;

        if (serialPort->open(QIODevice::ReadWrite)) {
            qDebug() << "Connected to" << portName;
            return true;
        } else {
            qDebug() << "Failed to open COM port:" << serialPort->errorString();
            return false;
        }
    }
    return false;
}

// Disconnect from the serial port
void BoardWithLeds::disconnect()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        qDebug() << "Disconnected from COM port.";
    }
}

// Check if the serial port is connected
bool BoardWithLeds::isConnected() const
{
    return serialPort->isOpen();
}

// Set the LED color by sending PWM values for red, green, and blue
void BoardWithLeds::setLedColor(int redValue, int greenValue, int blueValue)
{
    sendCommand(ledInfo.pwmForRedAddr, redValue);
    sendCommand(ledInfo.pwmForGreenAddr, greenValue);
    sendCommand(ledInfo.pwmForBlueAddr, blueValue);
}

// Create a command string to send to the board
QString BoardWithLeds::createCommand(int address, int value)
{
    return QString("S 10 %1 %2 P").arg(address, 0, 16).arg(value, 0, 16);
}

// Send a command to the serial port
void BoardWithLeds::sendCommand(int address, int value)
{
    if (serialPort->isOpen()) {
        QString command = createCommand(address, value);
        serialPort->write(command.toUtf8());
        serialPort->waitForBytesWritten(1000);
    } else {
        qDebug() << "Serial port is not open.";
    }
}
