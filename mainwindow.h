#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

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
    QString createCommand(int address, int value);
    void sendCommandsToSerialPort(const QStringList &commands);
    void updateAvailablePorts();
    void on_brightnessHorizontalSlider_valueChanged(int value);
    void on_powerPushButton_toggled(bool checked);
    void on_selectColourPushButton_clicked();
    void on_connectPushButton_clicked();
    void updateControlLockState();

private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
};
#endif // MAINWINDOW_H
