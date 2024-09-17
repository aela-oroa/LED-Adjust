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
    void on_brightnessHorizontalSlider_valueChanged(int value);
    void on_powerPushButton_toggled(bool checked);
    void on_selectColourPushButton_clicked();
    void updateControlLockState(bool isConnected);
    void on_ledComboBox_currentTextChanged(const QString &arg1);
    void handlePortsAvailable(bool available);

private:
    Ui::MainWindow *ui;
    BoardWithLeds *board;
    Led selectedLed;

    void sendToOtherLeds(uint8_t value);
};

#endif // MAINWINDOW_H
