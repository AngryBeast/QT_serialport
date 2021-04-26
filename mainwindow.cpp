#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QTextCodec>


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

    Timer = new QTimer(this);
    Timer->start(1000);

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

    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::red);
    ui->label_state->setText("未连接");
    ui->label_state->setPalette(pa);

    connect(ui->pushButton_openSerial,&QPushButton::clicked,this,&MainWindow::Open_pushButton_clicked);
    connect(ui->pushButton_send,&QPushButton::clicked,this,&MainWindow::Send_pushButton_clicked);
    connect(ui->pushButton_clearSend,&QPushButton::clicked,this,&MainWindow::ClearSend_pushButton_clicked);
    connect(ui->pushButton_clearRecived,&QPushButton::clicked,this,&MainWindow::ClearRecived_pushButton_clicked);
    connect(ui->pushButton_loadFile,&QPushButton::clicked,this,&MainWindow::LoadFile_pushButton_clicked);
    connect(ui->pushButton_savaFile,&QPushButton::clicked,this,&MainWindow::SaveFile_pushButton_clicked);

    connect(&MySerial,&QSerialPort::readyRead,this,&MainWindow::readSerialDataSlot);

    connect(MyTimer,&QTimer::timeout,this,&MainWindow::Send_pushButton_clicked);

    connect(Timer,&QTimer::timeout,this,&MainWindow::updateTime);

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
            //qDebug()<<info.portName();
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

            QPalette pa;
            pa.setColor(QPalette::WindowText,Qt::green);
            ui->label_state->setText("已连接");
            ui->label_state->setPalette(pa);
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
        QPalette pa;
        pa.setColor(QPalette::WindowText,Qt::red);
        ui->label_state->setText("未连接");
        ui->label_state->setPalette(pa);
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

            count_send += SendData.size();
            ui->label_Count_send->setText("S:" + QString::number(count_send));

            if(ui->checkBox_hexsend->isChecked())//SendCheckBox被选中HEX发送
            {
                Send.append(SendData.toLocal8Bit()).toHex();//转HEX存储
                // qDebug()<<SendHex<<endl;
            }
            else//没有选中HEX发送
            {
                Send.append(SendData.toLocal8Bit());
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
            ui->checkBox_timing->setChecked(false);
            MyTimer->stop();
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
    //count_recive += readData.size();
    //qDebug() <<  count_recive << Qt::endl;
    QString str = QString::fromLocal8Bit(readData);     //接收中文
    count_recive += str.size();
    ui->label_Count_rec->setText("R:" + QString::number(count_recive));
    if (ui->checkBox_time->isChecked())
    {
        curDateTime=QDateTime::currentDateTime();
        ui->textEdit_recived->append("[" + curDateTime.time().toString() + "]");
    }

    if(!readData.isNull())//将读到的数据显示到数据接收区
    {
        if(ui->checkBox_hexdisplay->isChecked())//选中HEX显示
        {
            readData=readData.toHex();//转为HEX
            ui->textEdit_recived->append(str);

        }
        else//未选中HEX显示
        {
            ui->textEdit_recived->append(str);
        }
        readData.clear();//清除接收缓存
    }

}

void MainWindow::CheckBox_timing_stateChanged()
{
//    connect(MyTimer,&QTimer::timeout,this,&MainWindow::Send_pushButton_clicked);
    if (ui->checkBox_timing->isChecked())
    {
        if (ui->lineEdit->text().isEmpty() || ui->pushButton_openSerial->text() == "打开串口")
        {
            ui->checkBox_timing->setChecked(false);
            QMessageBox::about(NULL, "提示", "串口未打开或未输入定时时间");
            return;
        }
        if (MyTimer->isActive())
        {
            return ;
        }
        else
        {
            int ms = ui->lineEdit->text().toInt();
            MyTimer->start(ms);
        }
    }
    else
    {
        if (MyTimer->isActive())
        {
            MyTimer->stop();
            return ;
        }
        else
        {
            qDebug() << "down" << Qt::endl;
            return;
        }


    }
}

void MainWindow::updateTime()
{
    curDateTime=QDateTime::currentDateTime();
    ui->label_time->setText("当前时间:"+curDateTime.time().toString());
}



void MainWindow::ClearSend_pushButton_clicked()
{
    ui->textEdit_send->clear();
}

void MainWindow::ClearRecived_pushButton_clicked()
{
    ui->textEdit_recived->clear();
}

void MainWindow::SaveFile_pushButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    "", tr("Text Files(*.txt);;C++ Files(*.cpp *.h)"));

    if (fileName != "")
    {
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly)){

        }
        else{
            QTextStream stream(&file);
            stream << ui->textEdit_recived->toPlainText();
            stream.flush();
            file.close();
        }
    }
}

void MainWindow::LoadFile_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "",tr("Text Files ( *.txt);;c++ Files (*.cpp *.h)"));
    if (fileName != "")
    {
        QString str = "Notepad  -";
        str += fileName ;
        setWindowTitle(str);

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }

        QTextStream in (&file);
        ui->textEdit_send->setText(in.readAll());
        file.close();
    }
}

