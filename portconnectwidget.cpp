#include "portconnectwidget.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QComboBox>
#include <QDebug>
#include <QSerialPortInfo>

PortConnectWidget::PortConnectWidget(QWidget *parent)
    : QWidget(parent), mainWindowUi(nullptr), board(new BoardWithLeds())
{
    // Initialize the BoardWithLeds instance
}

void PortConnectWidget::setUi(Ui::MainWindow *ui)
{
    mainWindowUi = ui;

    if (mainWindowUi) {
        // Connect the button click signal to the slot
        connect(mainWindowUi->connectPushButton, &QPushButton::clicked, this, &PortConnectWidget::onConnectPushButtonClicked);
        // Connect other necessary signals
    }
}

void PortConnectWidget::onButtonClick()
{
    // Example usage of comboBox and pushButton
    if (mainWindowUi) {
        QString selectedPort = mainWindowUi->comboBox->currentText();
        qDebug() << "Selected Port:" << selectedPort;
    }
}

void PortConnectWidget::onConnectPushButtonClicked()
{
    if (!board->isConnected()) {
        QString selectedPort = mainWindowUi->serialPortComboBox->currentText();
        if (selectedPort.isEmpty()) {
            qDebug() << "No COM port selected.";
            return;
        }

        board->setPortName(selectedPort);
        if (board->connect()) {
            mainWindowUi->connectPushButton->setText("Disconnect");
            mainWindowUi->connectPushButton->setChecked(true);
        } else {
            mainWindowUi->connectPushButton->setChecked(false);
        }
    } else {
        board->disconnect();
        mainWindowUi->connectPushButton->setText("Connect");
        mainWindowUi->connectPushButton->setChecked(false);
    }

    updateControlLockState();
}

void PortConnectWidget::updateAvailablePorts()
{
    QSet<QString> currentPorts;
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
        currentPorts.insert(info.portName());
    }

    QString selectedPort = mainWindowUi->serialPortComboBox->currentText();
    mainWindowUi->serialPortComboBox->clear();

    if (currentPorts.isEmpty()) {
        mainWindowUi->connectPushButton->setEnabled(false);
        mainWindowUi->connectPushButton->setText("No COM Port Available");
    } else {
        mainWindowUi->connectPushButton->setEnabled(true);
        if (!mainWindowUi->connectPushButton->isChecked()) {
            mainWindowUi->connectPushButton->setText("Connect");
        }

        for (const QString &portName : currentPorts) {
            mainWindowUi->serialPortComboBox->addItem(portName);
        }

        int index = mainWindowUi->serialPortComboBox->findText(selectedPort);
        if (index != -1) {
            mainWindowUi->serialPortComboBox->setCurrentIndex(index);
        }
    }
}

void PortConnectWidget::updateControlLockState()
{
    bool isConnected = board->isConnected();

    // Lock/unlock the controls based on connection status
    mainWindowUi->selectColourPushButton->setEnabled(isConnected);
    mainWindowUi->powerPushButton->setEnabled(isConnected);
    mainWindowUi->brightnessHorizontalSlider->setEnabled(isConnected);

    // Lock the port selection when connected
    mainWindowUi->serialPortComboBox->setEnabled(!isConnected);
}
