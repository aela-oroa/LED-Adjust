#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QTimer>
#include <QColorDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , board(new BoardWithLeds(Led::Led1))
    , selectedLed(Led::Led1)
{
    ui->setupUi(this);
    setWindowTitle("LED Colour Adjuster");

    ui->portControlWidget->setBoard(board);

    // Update control lock state based on connection status
    connect(ui->portControlWidget, &PortControlWidget::connectionStatusChanged, this, [this](bool isConnected) {
        updateControlLockState(isConnected);
        qDebug() << "PortControlWidget connection status changed:" << isConnected;
    });

    // Handle COM port availability
    connect(ui->portControlWidget, &PortControlWidget::portsAvailable, this, &MainWindow::handlePortsAvailable);

    updateControlLockState(false);  // Initially disconnected
}

MainWindow::~MainWindow()
{
    delete ui;
    delete board;
}

// Utility function to update control state based on connection status
void MainWindow::updateControlLockState(bool isConnected)
{
    ui->selectColourPushButton->setEnabled(isConnected);
    ui->powerPushButton->setEnabled(isConnected);
    ui->brightnessHorizontalSlider->setEnabled(isConnected);
    ui->ledComboBox->setEnabled(!isConnected);

}

// Helper function to send "0x00" to other LEDs
void MainWindow::sendToOtherLeds(uint8_t value)
{
    for (int i = static_cast<int>(Led::Led1); i <= static_cast<int>(Led::Led4); ++i) {
        if (i != static_cast<int>(selectedLed)) {
            LedInfo otherLedInfo = LedFactory::createLed(static_cast<Led>(i));

            board->sendCommand(otherLedInfo.connectionForRedAddr, value);
            board->sendCommand(otherLedInfo.connectionForGreenAddr, value);
            board->sendCommand(otherLedInfo.connectionForBlueAddr, value);
        }
    }
}

// Handle COM port availability
void MainWindow::handlePortsAvailable(bool available)
{
    if (!board->isConnected()) {
        ui->ledComboBox->setEnabled(available);
    }
}
// Slot: Brightness slider value changed
void MainWindow::on_brightnessHorizontalSlider_valueChanged(int value)
{
    int step = 5;
    int newValue = (value / step) * step;
    ui->brightnessHorizontalSlider->setValue(newValue);

    int green_value = 4 + (newValue / step);
    int blue_value = green_value;
    int red_value = green_value - 3;

    // Clamp values within the valid range
    green_value = qMax(green_value, 4);
    red_value = qMax(red_value, 1);

    qDebug() << "green:" << green_value << "red:" << red_value << "blue:" << blue_value;

    LedInfo ledInfo = LedFactory::createLed(selectedLed);

    // Send brightness values to the PWM addresses
    board->sendCommand(ledInfo.pwmForRedAddr, red_value);
    board->sendCommand(ledInfo.pwmForGreenAddr, green_value);
    board->sendCommand(ledInfo.pwmForBlueAddr, blue_value);

    // Turn off other LEDs
    sendToOtherLeds(0x00);
}

// Slot: Power button toggled
void MainWindow::on_powerPushButton_toggled(bool checked)
{
    LedInfo ledInfo = LedFactory::createLed(Led::Led1);

    // Set LED color based on power button state
    if (checked) {
        board->sendCommand(ledInfo.pwmForRedAddr, 0xFF);
        board->sendCommand(ledInfo.pwmForGreenAddr, 0xFB);
        board->sendCommand(ledInfo.pwmForBlueAddr, 0xFF);
    } else {
        board->sendCommand(ledInfo.pwmForRedAddr, 0x0);
        board->sendCommand(ledInfo.pwmForGreenAddr, 0x0);
        board->sendCommand(ledInfo.pwmForBlueAddr, 0x0);
    }

    ui->powerPushButton->setText(checked ? "OFF" : "ON");

    // Turn off other LEDs
    sendToOtherLeds(0x00);
}

// Slot: Color selection button clicked
void MainWindow::on_selectColourPushButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");
    if (!color.isValid()) return;

    // Update UI to reflect the selected color
    ui->colourLabel->setStyleSheet("background-color: " + color.name());
    ui->colourLabel->update();

    // Convert RGB to PWM values and send to the hardware
    int red_value = color.red() / 15;
    int green_value = color.green() / 15;
    int blue_value = color.blue() / 15;

    qDebug() << "Red Value:" << red_value << "Green Value:" << green_value << "Blue Value:" << blue_value;

    LedInfo ledInfo = LedFactory::createLed(selectedLed);
    board->sendCommand(ledInfo.pwmForRedAddr, red_value);
    board->sendCommand(ledInfo.pwmForGreenAddr, green_value);
    board->sendCommand(ledInfo.pwmForBlueAddr, blue_value);

    // Turn off other LEDs
    sendToOtherLeds(0x00);
}

// Slot: LED ComboBox text changed
void MainWindow::on_ledComboBox_currentTextChanged(const QString &arg1)
{
    if (board) {
        delete board;
        board = nullptr;
    }

    // Select the correct LED based on the ComboBox value
    if (arg1 == "Led1") {
        selectedLed = Led::Led1;
    } else if (arg1 == "Led2") {
        selectedLed = Led::Led2;
    } else if (arg1 == "Led3") {
        selectedLed = Led::Led3;
    } else if (arg1 == "Led4") {
        selectedLed = Led::Led4;
    }

    // Instantiate new board for selected LED and update PortControlWidget
    board = new BoardWithLeds(selectedLed);
    ui->portControlWidget->setBoard(board);

    qDebug() << "Selected LED changed to:" << arg1;
}
