#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QString>
#include <QComboBox>
#include <QTimer>
#include <QColorDialog>
#include <QIcon>
#include <QPixmap>
#include <QPainter>

// Constructor
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("LED Colour Adjuster");

    serialPort = new QSerialPort(this);

    // Timer to update available COM ports every second
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateAvailablePorts);
    timer->start(1000);

    updateControlLockState();
}

// Destructor
MainWindow::~MainWindow()
{
    delete ui;
}

// Create command for serial communication
QString MainWindow::createCommand(int address, int value)
{
    return QString("S 10 %1 %2 P").arg(address, 0, 16).arg(value, 0, 16);  // Address is now an int
}

// Send list of commands to serial port
void MainWindow::sendCommandsToSerialPort(const QStringList &commands)
{
    if (serialPort->isOpen()) {
        for (const QString &command : commands) {
            serialPort->write(command.toUtf8());
            serialPort->waitForBytesWritten(1000);  // Wait for data to be written
        }
    } else {
        qDebug() << "Serial port is not open.";
    }
}

// Update available COM ports
void MainWindow::updateAvailablePorts()
{
    QSet<QString> currentPorts;
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
        currentPorts.insert(info.portName());
    }

    QString selectedPort = ui->serialPortComboBox->currentText();
    ui->serialPortComboBox->clear();

    if (currentPorts.isEmpty()) {
        ui->connectPushButton->setEnabled(false);
        ui->connectPushButton->setText("No COM Port Available");
    } else {
        ui->connectPushButton->setEnabled(true);
        if (!ui->connectPushButton->isChecked()) {
            ui->connectPushButton->setText("Connect");
        }

        for (const QString &portName : currentPorts) {
            ui->serialPortComboBox->addItem(portName);
        }

        int index = ui->serialPortComboBox->findText(selectedPort);
        if (index != -1) {
            ui->serialPortComboBox->setCurrentIndex(index);
        }
    }
}

// Brightness slider value changed
void MainWindow::on_brightnessHorizontalSlider_valueChanged(int value)
{
    int step = 50;
    int newValue = (value / step) * step;
    ui->brightnessHorizontalSlider->setValue(newValue);

    QStringList commands;
    switch (newValue) {
    case 0:
        commands = {createCommand(0xAF, 0xFF), createCommand(0xAA, 0xFF), createCommand(0xB3, 0xFF)};
        break;
    case 50:
        commands = {createCommand(0xAF, 0x8), createCommand(0xAA, 0x8), createCommand(0xB3, 0x8)};
        break;
    case 100:
        commands = {createCommand(0xAF, 0), createCommand(0xAA, 0), createCommand(0xB3, 0)};
        break;
    default:
        qDebug() << "Unsupported value.";
        return;
    }

    sendCommandsToSerialPort(commands);
}

// Power button toggled
void MainWindow::on_powerPushButton_toggled(bool checked)
{
    QStringList commands = checked
                               ? QStringList{
                                   createCommand(0x37, 0x10), createCommand(0x38, 0xFE), createCommand(0x39, 0x9D),
                                   createCommand(0x3A, 0x08), createCommand(0xAF, 0x0), createCommand(0xAA, 0x0), createCommand(0xB3, 0x0)
                               }
                               : QStringList{
                                   createCommand(0x37, 0x0), createCommand(0x38, 0x0), createCommand(0x39, 0x0), createCommand(0x3A, 0x0)
                               };

    ui->powerPushButton->setText(checked ? "OFF" : "ON");
    sendCommandsToSerialPort(commands);
}

// Colour selection button clicked
void MainWindow::on_selectColourPushButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");
    if (!color.isValid()) return;

    ui->colourLabel->setStyleSheet("background-color: " + color.name());
    ui->colourLabel->update();

    int red_value = (255 - color.red()) / 15;
    int green_value = (255 - color.green()) / 15;
    int blue_value = (255 - color.blue()) / 15;

    qDebug() << "Red Value:" << red_value << "Green Value:" << green_value << "Blue Value:" << blue_value;

    QStringList commands = {
        createCommand(0xAF, red_value),
        createCommand(0xAA, green_value),
        createCommand(0xB3, blue_value)
    };

    sendCommandsToSerialPort(commands);
}

// Connect/Disconnect button clicked
void MainWindow::on_connectPushButton_clicked()
{
    if (!serialPort->isOpen()) {
        QString selectedPort = ui->serialPortComboBox->currentText();
        if (selectedPort.isEmpty()) {
            qDebug() << "No COM port selected.";
            return;
        }

        serialPort->setPortName(selectedPort);
        if (serialPort->open(QIODevice::ReadWrite)) {
            qDebug() << "Connected to" << selectedPort;
            ui->connectPushButton->setText("Disconnect");
            ui->connectPushButton->setChecked(true);
        } else {
            qDebug() << "Failed to open COM port.";
            ui->connectPushButton->setChecked(false);
        }
    } else {
        serialPort->close();
        qDebug() << "Disconnected from COM port.";
        ui->connectPushButton->setText("Connect");
        ui->connectPushButton->setChecked(false);
    }

    updateControlLockState();
}

// Update the state of controls based on connection status
void MainWindow::updateControlLockState()
{
    bool isConnected = serialPort->isOpen();

    // Lock/unlock the controls based on connection status
    ui->selectColourPushButton->setEnabled(isConnected);
    ui->powerPushButton->setEnabled(isConnected);
    ui->brightnessHorizontalSlider->setEnabled(isConnected);

    // Lock the port selection when connected
    ui->serialPortComboBox->setEnabled(!isConnected);
}
