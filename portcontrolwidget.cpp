#include "portcontrolwidget.h"
#include "ui_portcontrolwidget.h"
#include <QSerialPortInfo>
#include <QTimer>
#include <QDebug>

PortControlWidget::PortControlWidget(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::PortControlWidget)
{
    ui->setupUi(this);

    this->setFrameStyle(QFrame::Box | QFrame::Plain);
    this->setLineWidth(1);

    // Timer to update available ports periodically
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PortControlWidget::updateAvailablePorts);
    timer->start(1000);

    connect(ui->connectWidget, &QPushButton::clicked, this, &PortControlWidget::onConnectWidgetClicked);
}

PortControlWidget::~PortControlWidget()
{
    delete ui;
}

void PortControlWidget::setBoard(BoardWithLeds *sharedBoard)
{
    this->board = sharedBoard;
}

void PortControlWidget::updateAvailablePorts()
{
    QSet<QString> currentPorts;
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
        currentPorts.insert(info.portName());
    }

    QString selectedPort = ui->serialPortWidget->currentText();
    ui->serialPortWidget->clear();

    if (currentPorts.isEmpty()) {
        ui->connectWidget->setEnabled(false);
        ui->connectWidget->setText("No COM Port Available");
        emit portsAvailable(false);  // No ports available
    } else {
        ui->connectWidget->setEnabled(true);
        if (!ui->connectWidget->isChecked()) {
            ui->connectWidget->setText("Connect");
        }

        for (const QString &portName : currentPorts) {
            ui->serialPortWidget->addItem(portName);
        }

        int index = ui->serialPortWidget->findText(selectedPort);
        if (index != -1) {
            ui->serialPortWidget->setCurrentIndex(index);
        }
        emit portsAvailable(true);  // Ports available
    }
}

void PortControlWidget::onConnectWidgetClicked()
{
    if (!board->isConnected()) {
        QString selectedPort = ui->serialPortWidget->currentText();
        if (selectedPort.isEmpty()) {
            qDebug() << "No COM port selected.";
            return;
        }

        board->setPortName(selectedPort);  // Set the port name on the shared board instance
        qDebug() << "Setting port name to" << selectedPort;

        if (board->connect()) {
            ui->connectWidget->setText("Disconnect");
            ui->connectWidget->setChecked(true);
            ui->serialPortWidget->setEnabled(false);
            emit connectionStatusChanged(true);  // Emit signal when connected
            qDebug() << "Connected to port";
        } else {
            ui->connectWidget->setChecked(false);
            qDebug() << "Failed to connect";
        }
    } else {
        resetLedStates();

        board->disconnect();
        ui->connectWidget->setText("Connect");
        ui->connectWidget->setChecked(false);
        ui->serialPortWidget->setEnabled(true);
        emit connectionStatusChanged(false);  // Emit signal when disconnected
        qDebug() << "Disconnected";
    }

    updateStatusLabel();
}

void PortControlWidget::resetLedStates()
{
    const std::vector<std::vector<int>> commands = {
        {0xFE, 0x61, -1},    // LED1
        {0x86, -1, 0xE5},    // LED2
        {0xE3, 0xFE, 0xE5},  // LED3
        {0xFE, 0xE1, 0xFE}   // LED4
    };

    for (int i = static_cast<int>(Led::Led1); i <= static_cast<int>(Led::Led4); ++i) {
        LedInfo ledInfo = LedFactory::createLed(static_cast<Led>(i));
        const auto &cmds = commands[i];

        board->sendCommand(ledInfo.connectionForRedAddr, cmds[0]);
        board->sendCommand(ledInfo.connectionForGreenAddr, cmds[1]);
        board->sendCommand(ledInfo.connectionForBlueAddr, cmds[2]);
    }
}

void PortControlWidget::updateStatusLabel()
{
    if (board->isConnected()) {
        ui->statusLabel->setText("Connected");
    } else {
        ui->statusLabel->setText("Disconnected");
    }
}

QString PortControlWidget::getPortName() const
{
    return ui->serialPortWidget->currentText();
}
