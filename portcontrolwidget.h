#ifndef PORTCONTROLWIDGET_H
#define PORTCONTROLWIDGET_H

#include <QFrame>
#include <QSet>
#include <QSerialPortInfo>
#include "boardwithleds.h"

namespace Ui {
class PortControlWidget;
}

class PortControlWidget : public QFrame
{
    Q_OBJECT

public:
    explicit PortControlWidget(QWidget *parent = nullptr);
    ~PortControlWidget();

    void updateAvailablePorts();
    QString getPortName() const;
    void setBoard(BoardWithLeds *board);

signals:
    void connectionStatusChanged(bool isConnected);
    void portsAvailable(bool available);

private slots:
    void onConnectWidgetClicked();
    void updateStatusLabel();

private:
    Ui::PortControlWidget *ui;
    BoardWithLeds *board;

    void resetLedStates();
};

#endif // PORTCONTROLWIDGET_H
