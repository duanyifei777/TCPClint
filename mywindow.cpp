#include "mywindow.h"
#include "ui_mywindow.h"
#include "io.h"

#include <QMessageBox>
#include <QDebug>
#include <QRegularExpression>

myWindow::myWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::myWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("客户端");
    this->resize(800,800);

    tcpsocket = new QTcpSocket;  // 创建socket对象
    connect(tcpsocket, &QTcpSocket::readyRead, this, &myWindow::readyRead_slot);

    // 创建IO界面，并添加到tabWidget中
    int index = ui->tabWidget->indexOf(ui->tabIO);
    iowindow = new IO(this);
    ui->tabWidget->removeTab(index);
    ui->tabWidget->addTab(iowindow, tr("IO监控"));

    connect(iowindow, &IO::sendCommandToServer, this, &myWindow::sendCommandToServer);
}

myWindow::~myWindow()
{
    delete ui;
}


void myWindow::on_connectButton_clicked()
{
    QString ip = ui->IPlineEdit->text();  // 获取输入框中的IP
    int port = ui->portlineEdit->text().toInt();  // 获取输入框中的端口号

    if(ip.isEmpty() || port==0)
    {
        QMessageBox::warning(this, "提示", "ip与端口号不能为空");
        return;
    }

    tcpsocket->connectToHost(ip, port);  // 进行tcp连接
    if(tcpsocket->waitForConnected(5000))
    {
        iowindow->startTimer();  // 开启定时器
        QMessageBox::information(this, "提示", "连接服务器成功");
    }
    else{
        QMessageBox::warning(this, "提示", "连接服务器失败");
    }
}

void myWindow::on_closeButton_clicked()
{
    if(tcpsocket->isOpen())
    {
        tcpsocket->disconnectFromHost();  // 断开与服务器的连接
        iowindow->stopTimer();  // 关闭定时器
        tcpsocket->close();
        QMessageBox::information(this, "提示", "已断开服务器连接");
    }
    else{
        QMessageBox::warning(this, "提示", "未连接到服务器");
    }
}

void myWindow::readyRead_slot()
{
    QByteArray data = tcpsocket->readAll();
    QString text = QString::fromUtf8(data);
    // qDebug() << "收到数据原始：" << text;

    QRegularExpression re("MB_IOIN:(\\d+)|MB_IOOUT:(\\d+)");
    QRegularExpressionMatchIterator it = re.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        if (match.captured(1) != "") {
            int val = match.captured(1).toInt();
            iowindow->updateInputState(val);
        } else if (match.captured(2) != "") {
            int val = match.captured(2).toInt();
            iowindow->updateOutputState(val);
        }
    }
}

void myWindow::sendCommandToServer(const QString &cmd)
{
    if(tcpsocket && tcpsocket->isOpen())
    {
        QString tosend = cmd.trimmed();
        tcpsocket->write(tosend.toUtf8());
        tcpsocket->flush();
        // qDebug() << "已发送查询命令：" << tosend.trimmed();
    }
    else{
        QMessageBox::warning(this, "提示", "无法发送IO指令");
    }
}

