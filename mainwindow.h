#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void Open_pushButton_clicked();
    void Send_pushButton_clicked();
    void ClearSend_pushButton_clicked();
    void ClearRecived_pushButton_clicked();
    void SaveFile_pushButton_clicked();
    void LoadFile_pushButton_clicked();
    void readSerialDataSlot();
    void CheckBox_timing_stateChanged();
    void updateTime();

private:
    Ui::MainWindow *ui;

    void InitPort();
    void SeachPort();
    QSerialPort MySerial;
    QTimer *MyTimer;
    QTimer *Timer;
    int count_recive = 0;
    int count_send = 0;
    QDateTime curDateTime;
};
#endif // MAINWINDOW_H
