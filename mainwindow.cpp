#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QDateTime>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("串口助手  BY:AngryBeast");
    MainWindow::InitPort();



}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::InitPort()
{
    SeachPort();

    MyTimer = new QTimer(this);
    QStringList baudList;//波特率
    QStringList parityList;//校验位
    QStringList dataBitsList;//数据位
    QStringList stopBitsList;//停止位

    baudList<<"1200"<<"2400"<<"4800"<<"9600"<<"38400"<<"115200";

    ui->comboBox_baud->addItems(baudList);//波特率
    ui->comboBox_baud->setCurrentIndex(5);//从0开始索引 9600

    parityList<<"无"<<"奇"<<"偶";

    ui->comboBox_parityCheck->addItems(parityList);//校验位
    ui->comboBox_parityCheck->setCurrentIndex(0);

    dataBitsList<<"5"<<"6"<<"7"<<"8";
    ui->comboBox_dataBits->addItems(dataBitsList);//数据位
    ui->comboBox_dataBits->setCurrentIndex(3);

    stopBitsList<<"1";
    stopBitsList<<"1.5";
    stopBitsList<<"2";

    ui->comboBox_stopBits->addItems(stopBitsList);//停止位
    ui->comboBox_stopBits->setCurrentIndex(0);

    //设置只能输入数的范围
    class QValidator *validator = new QIntValidator(0,99999,this);
    ui->lineEdit->setValidator(validator);

    connect(ui->pushButton_openSerial,&QPushButton::clicked,this,&MainWindow::Open_pushButton_clicked);
    connect(ui->pushButton_send,&QPushButton::clicked,this,&MainWindow::Send_pushButton_clicked);
    connect(ui->pushButton_clearSend,&QPushButton::clicked,this,&MainWindow::ClearSend_pushButton_clicked);
    connect(ui->pushButton_clearRecived,&QPushButton::clicked,this,&MainWindow::ClearRecived_pushButton_clicked);
    connect(&MySerial,&QSerialPort::readyRead,this,&MainWindow::readSerialDataSlot);

    //connect(MyTimer,&QTimer::timeout,this,&MainWindow::Send_pushButton_clicked);
    connect(ui->checkBox_timing,&QCheckBox::clicked,this,&MainWindow::CheckBox_timing_stateChanged);

}

void MainWindow::SeachPort()
{
    const auto infos = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : infos)
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_serial->addItem(info.portName());
            qDebug()<<info.portName();
            serial.close();
        }
    }
}




void MainWindow::Open_pushButton_clicked()
{
    //打开串口
    MySerial.setPortName(ui->comboBox_serial->currentText());

    if(ui->pushButton_openSerial->text() == "打开串口")
    {
        //设置波特率
        MySerial.setBaudRate(ui->comboBox_baud->currentText().toInt());

        //设校验位
        switch (ui->comboBox_parityCheck->currentIndex()) {
        case 0:     //无校验位
            MySerial.setParity(QSerialPort::NoParity);
            break;
        case 1:     //奇校验
            MySerial.setParity(QSerialPort::EvenParity);
            break;
        default:    //偶校验
            MySerial.setParity(QSerialPort::OddParity);
            break;
        }

        //设置数据位
        switch (ui->comboBox_dataBits->currentText().toInt()) {
        case 8:
            MySerial.setDataBits(QSerialPort::Data8);
            break;
        case 7:
            MySerial.setDataBits(QSerialPort::Data7);
            break;
        case 6:
            MySerial.setDataBits(QSerialPort::Data6);
            break;
        default:
            MySerial.setDataBits(QSerialPort::Data5);
        }


        //设置停止位
        switch (ui->comboBox_stopBits->currentIndex()) {
        case 0:
            MySerial.setStopBits(QSerialPort::OneStop);
            break;
        case 1:
            MySerial.setStopBits(QSerialPort::OneAndHalfStop);
            break;
        default:
            MySerial.setStopBits(QSerialPort::TwoStop);
            break;
        }
        if(MySerial.open(QIODevice::ReadWrite))
        {

            ui->pushButton_openSerial->setText("关闭串口");
            QMessageBox::about(NULL, "提示", "打开串口成功");
            //
        }
        else//串口打开失败
        {
//            QMessageBox::about(NULL, "提示", "打开串口失败");
            QMessageBox::critical(this, tr("Error"), MySerial.errorString());

            return ;
        }

    }
    else if(ui->pushButton_openSerial->text() == "关闭串口")
    {
        ui->pushButton_openSerial->setText("打开串口");
        SeachPort();
        MySerial.close();
    }
}

void MainWindow::Send_pushButton_clicked()
{
    //QString SendData = ui->textEdit_send->toPlainText();
    //qDebug() << str << Qt::endl;
    QString SendData;
    QByteArray Send;//HEX发送
    if(ui->pushButton_openSerial->text()=="关闭串口")//打开串口才可以发送数据
    {
        if(!ui->textEdit_send->toPlainText().isEmpty())//发送区不为空
        {
            SendData = ui->textEdit_send->toPlainText();//获取发送区的数据

            if(ui->checkBox_hexsend->isChecked())//SendCheckBox被选中HEX发送
            {
                Send.append(SendData).toHex();//转HEX存储
                // qDebug()<<SendHex<<endl;
            }
            else//没有选中HEX发送
            {
                Send.append(SendData);
                //qDebug()<<SendHex<<endl;
            }

            if(ui->checkBox_sendln->isChecked())    //发送新行
            {
                Send.append("\r\n");
            }

            MySerial.write(Send,Send.length());//写入缓冲区
        }
        else//发送区为空
        {
            QMessageBox::about(NULL, "提示", "没有数据哦");
        }
    }
    else//串口未打开
    {
        QMessageBox::about(NULL, "提示", "串口未打开!");
    }

}


void MainWindow::readSerialDataSlot()
{

    QByteArray readData = MySerial.readAll();//读取串口数据

    if (ui->checkBox_time->isChecked())
    {
        QDateTime curDateTime=QDateTime::currentDateTime();
        ui->textEdit_recived->append("[" + curDateTime.time().toString() + "]");
    }

    if(!readData.isNull())//将读到的数据显示到数据接收区
    {
        if(ui->checkBox_hexdisplay->isChecked())//选中HEX显示
        {
            readData=readData.toHex();//转为HEX
            ui->textEdit_recived->append(readData);

        }
        else//未选中HEX显示
        {
            ui->textEdit_recived->append(readData);
        }
        readData.clear();//清除接收缓存
    }

}

void MainWindow::CheckBox_timing_stateChanged()
{
    connect(MyTimer,&QTimer::timeout,this,&MainWindow::Send_pushButton_clicked);
    if (ui->checkBox_timing->isChecked())
    {
        MyTimer->start(ui->lineEdit->text().toInt());
//        qDebug() << ui->lineEdit->text().toInt();
    }
    else
    {
//        qDebug() << "down" << Qt::endl;
        MyTimer->stop();
    }
}

void MainWindow::ClearSend_pushButton_clicked()
{
    ui->textEdit_send->clear();
}

void MainWindow::ClearRecived_pushButton_clicked()
{
    ui->textEdit_recived->clear();
}

