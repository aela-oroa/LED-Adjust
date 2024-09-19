#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "boardwithleds.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrightnessHorizontalSliderValueChanged(int value);
    void onPowerPushButtonToggled(bool checked);
    void onSelectColourPushButtonClicked();
    void updateControlLockState(bool isConnected);
    void onLedComboBoxCurrentTextChanged(const QString &arg1);
    void handlePortsAvailable(bool available);

private:
    Ui::MainWindow *ui;
    BoardWithLeds board;
    Led selectedLed;

    void sendToOtherLeds(uint8_t value);
};

#endif // MAINWINDOW_H
