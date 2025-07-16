#include "widget.h"
#include "ui_widget.h"
#include "io.h"

#include<QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("客户端");
    //this->resize(600,600);

    tcpsocket = new QTcpSocket;  //创建socket对象
    // 连接信号与槽（谁发出信号，发出什么信号，谁连接信号，怎么处理）
    connect(tcpsocket, &QTcpSocket::readyRead, this, &Widget::readready_slot);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_connectButton_clicked()
{
    QString ip = ui->IPlineEdit->text();
    quint16 port = ui->portlineEdit->text().toUShort();

    if(ip.isEmpty() || port==NULL)
    {
        QMessageBox::warning(this, "提示", "ip与端口号不能为空");
        return;
    }
    tcpsocket->connectToHost(ip, port); //连接到服务器
    if (tcpsocket->waitForConnected((10000))) //判断是否连接成功，返回ture则成功
    {
        QMessageBox::information(this, "提示", "连接服务器成功");
    }
    else{
        QMessageBox::warning(this, "提示", "连接服务器失败");
    }
}


void Widget::on_closeButton_clicked()
{
    if(tcpsocket->isOpen())
    {
        tcpsocket->disconnectFromHost(); //断开与服务器的连接
        tcpsocket->close();
        QMessageBox::information(this, "提示", "已断开服务器连接");
    }
}


void Widget::on_sendButton_clicked()
{
    QString data = ui->sendTextEdit->toPlainText();
    if(!data.isEmpty() && tcpsocket->isOpen())
    {
        tcpsocket->write(data.toUtf8());
    }
    else{
        QMessageBox::warning(this, "提示", "发送失败");
    }
}

void Widget::readready_slot()
{
    QByteArray data = tcpsocket->readAll();
    ui->receiveTextEdit->appendPlainText(QString::fromUtf8(data));
}

void Widget::on_IOButton_clicked()
{
    if(!iowindow)
    {
        iowindow = new IO();
        iowindow->setWindowTitle("IO监控窗口");

        // 连接控制信号
        connect(iowindow, &IO::sendCommandToServer, this, &Widget::sendCommandToServer);
    }

    iowindow->show();  // 显示IO窗口

    iowindow->raise();
    iowindow->activateWindow();  // 展示在最前面
}

void Widget::sendCommandToServer(const QString &cmd)
{
    if(tcpsocket && tcpsocket->isOpen())
    {
        tcpsocket->write(cmd.toUtf8());
        tcpsocket->flush();
    }
    else{
        QMessageBox::warning(this, "提示", "未连接服务器，无法发送IO指令");
    }
}
