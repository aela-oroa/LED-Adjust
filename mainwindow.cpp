#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QTimer>
#include <QColorDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , board(Led::Led1)
    , selectedLed(Led::Led1)
{
    ui->setupUi(this);
    setWindowTitle("LED Colour Adjuster");

    ui->portControlWidget->setBoard(&board);

    // Update control lock state based on connection status
    connect(ui->portControlWidget, &PortControlWidget::connectionStatusChanged, this, [this](bool isConnected) {
        updateControlLockState(isConnected);
        qDebug() << "PortControlWidget connection status changed:" << isConnected;
    });

    // Handle COM port availability
    connect(ui->portControlWidget, &PortControlWidget::portsAvailable, this, &MainWindow::handlePortsAvailable);

    connect(ui->brightnessHorizontalSlider, &QSlider::valueChanged, this, &MainWindow::onBrightnessHorizontalSliderValueChanged);
    connect(ui->powerPushButton, &QPushButton::toggled, this, &MainWindow::onPowerPushButtonToggled);
    connect(ui->selectColourPushButton, &QPushButton::clicked, this, &MainWindow::onSelectColourPushButtonClicked);
    connect(ui->ledComboBox, &QComboBox::currentTextChanged, this, &MainWindow::onLedComboBoxCurrentTextChanged);

    updateControlLockState(false);  // Initially disconnected
}

MainWindow::~MainWindow()
{
    delete ui;
    // delete board;
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

            board.sendCommand(otherLedInfo.connectionForRedAddr, value);
            board.sendCommand(otherLedInfo.connectionForGreenAddr, value);
            board.sendCommand(otherLedInfo.connectionForBlueAddr, value);
        }
    }
}

// Handle COM port availability
void MainWindow::handlePortsAvailable(bool available)
{
    if (!board.isConnected()) {
        ui->ledComboBox->setEnabled(available);
    }
}
// Slot: Brightness slider value changed
void MainWindow::onBrightnessHorizontalSliderValueChanged(int value)
{
    int step = 5;
    int newValue = (value / step) * step;
    ui->brightnessHorizontalSlider->setValue(newValue);

    int greenValue = 4 + (newValue / step);
    int blueValue = greenValue;
    int redValue = greenValue - 3;

    // Clamp values within the valid range
    greenValue = qMax(greenValue, 4);
    redValue = qMax(redValue, 1);

    qDebug() << "green:" << greenValue << "red:" << redValue << "blue:" << blueValue;

    LedInfo ledInfo = LedFactory::createLed(selectedLed);

    // Send brightness values to the PWM addresses
    board.sendCommand(ledInfo.pwmForRedAddr, redValue);
    board.sendCommand(ledInfo.pwmForGreenAddr, greenValue);
    board.sendCommand(ledInfo.pwmForBlueAddr, blueValue);

    // Turn off other LEDs
    sendToOtherLeds(0x00);
}

// Slot: Power button toggled
void MainWindow::onPowerPushButtonToggled(bool checked)
{
    LedInfo ledInfo = LedFactory::createLed(Led::Led1);

    // Set LED color based on power button state
    if (checked) {
        board.sendCommand(ledInfo.pwmForRedAddr, 0xFF);
        board.sendCommand(ledInfo.pwmForGreenAddr, 0xFB);
        board.sendCommand(ledInfo.pwmForBlueAddr, 0xFF);
    } else {
        board.sendCommand(ledInfo.pwmForRedAddr, 0x0);
        board.sendCommand(ledInfo.pwmForGreenAddr, 0x0);
        board.sendCommand(ledInfo.pwmForBlueAddr, 0x0);
    }

    ui->powerPushButton->setText(checked ? "OFF" : "ON");

    // Turn off other LEDs
    sendToOtherLeds(0x00);
}

// Slot: Color selection button clicked
void MainWindow::onSelectColourPushButtonClicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");
    if (!color.isValid()) return;

    // Update UI to reflect the selected color
    ui->colourLabel->setStyleSheet("background-color: " + color.name());
    ui->colourLabel->update();

    // Convert RGB to PWM values and send to the hardware
    int redValue = color.red() / 15;
    int greenValue = color.green() / 15;
    int blueValue = color.blue() / 15;

    qDebug() << "Red Value:" << redValue << "Green Value:" << greenValue << "Blue Value:" << blueValue;

    LedInfo ledInfo = LedFactory::createLed(selectedLed);
    board.sendCommand(ledInfo.pwmForRedAddr, redValue);
    board.sendCommand(ledInfo.pwmForGreenAddr, greenValue);
    board.sendCommand(ledInfo.pwmForBlueAddr, blueValue);

    // Turn off other LEDs
    sendToOtherLeds(0x00);
}

// Slot: LED ComboBox text changed
void MainWindow::onLedComboBoxCurrentTextChanged(const QString &arg1)
{
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

    // Update the existing board object to use the new LED
    board.setLed(selectedLed);
    qDebug() << "Selected LED changed to:" << arg1;
}
