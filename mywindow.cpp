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
    // 主窗口处理IO窗口发出的消息
    connect(iowindow, &IO::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // 总定时器
    mainTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, &myWindow::myTimerUpdate);
    // 绑定修改速度框与回车键
    connect(ui->changeSpeedlineEdit, &QLineEdit::returnPressed, this, &myWindow::changeSpeed);

    // 初始化使能按钮
    ui->enabledButton->setCheckable(true);
    ui->enabledButton->setStyleSheet("background-color:gray;");
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
        mainTimer->start(3000);
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
        mainTimer->stop();
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
    qDebug() << "收到数据原始：" << text;

    QRegularExpression re("MB_\\w+:[^\\s]+");
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while(it.hasNext())
    {
        QString cmd = it.next().captured(0);
        QString key = cmd.section(":", 0, 0);
        QString value = cmd.section(":", 1, 1);

        if(key == "MB_IOIN")
        {
            int val = value.toInt();
            iowindow->updateInputState(val);
        }
        else if(key == "MB_IOOUT")
        {
            int val = value.toInt();
            iowindow->updateOutputState(val);
        }
        else if(key == "MB_RATE")
        {
            this->updateSpeed(value);
        }
        else if(key == "MB_ENABLE")
        {
            int val = value.toInt();
            this->updateEnabled(val);
        }
        else if(key == "MB_SCRAM")
        {
            QStringList vallist = value.split(',');  // 将返回值以逗号拆分 [glScram,scramNo]
            this->updateAlarm(vallist);
        }
        else if(key == "MB_CART")
        {
            QStringList vallist = value.split(',');  // (x,y,z,a,b,c,h,e,w,j1Flag,j2Flag,j3Flag,j4Flag,j5Flag,j6Flag)
            this->updateCart(vallist);
        }
        else if(key == "MB_JOINT")
        {
            QStringList vallist = value.split(",");
            this->updateJoint(vallist);
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
        qDebug() << "已发送查询命令：" << tosend.trimmed();
    }
    else{
        QMessageBox::warning(this, "提示", "未连接到服务器");
    }
}

void myWindow::changeSpeed()
{
    QString rateVal = ui->changeSpeedlineEdit->text().trimmed();
    bool ok;
    int rate = rateVal.toInt(&ok);
    if(!ok || rate<0 || rate>100)
    {
        QMessageBox::warning(this, "提示", "请输入有效速率");
        return;
    }
    QString cmd = QString("MB_RATE:%1").arg(rateVal);
    sendCommandToServer(cmd);
}

void myWindow::updateSpeed(QString val)
{
    if(val != '0')  // 速度不为0更新
    {
        ui->showSpeedlabel->setText(val);
    }
}

void myWindow::on_enabledButton_clicked()
{
    if(tcpsocket && tcpsocket->isOpen())
    {
        bool enabled = ui->enabledButton->isChecked();
        if(enabled)
        {
            sendCommandToServer("MB_ENABLE:1");
            ui->enabledButton->setStyleSheet("background-color:green; font-size:28px; border:none;"
                                             "font-family: 'Microsoft YaHei'");
        }
        else{
            sendCommandToServer("MB_ENABLE:0");
            ui->enabledButton->setStyleSheet("background-color:gray; font-size:28px; border:none; "
                                             "font-family: 'Microsoft YaHei'");
        }
    }
    else{
        QMessageBox::warning(this, "提示", "未连接，无法改变使能状态");
    }
}

void myWindow::updateEnabled(int val)
{
    if(val == 0)
    {
        ui->enabledButton->setChecked(val);
        ui->enabledButton->setStyleSheet("background-color:gray; font-size:28px; border:none; "
                                         "font-family: 'Microsoft YaHei'");
    }
    else{
        ui->enabledButton->setChecked(val);
        ui->enabledButton->setStyleSheet("background-color:green; font-size:28px; border:none; "
                                         "font-family: 'Microsoft YaHei'");
    }
}

void myWindow::updateAlarm(QStringList vallist)
{
    if(vallist.size() == 2)
    {
        QString glScram = vallist[0];  // 当前报警状态
        QString scramNo = vallist[1];  // 当前报警编号

        // qDebug() << "报警状态:" << glScram << " 报警编号:" << scramNo;

        if(glScram == '0')
        {
            ui->warningtextBrowser->setText("✅正常");

        }
        else{
            ui->warningtextBrowser->setText("❌报警,报警编号:"+scramNo);
        }
    }
}

void myWindow::updateCart(QStringList vallist)
{
    if(vallist.size() >= 6)
    {
        QString x = vallist[0];
        QString y = vallist[1];
        QString z = vallist[2];
        QString a = vallist[3];
        QString b = vallist[4];
        QString c = vallist[5];

        ui->xlineEdit->setText(x);
        ui->ylineEdit->setText(y);
        ui->zlineEdit->setText(z);
        ui->alineEdit->setText(a);
        ui->blineEdit->setText(b);
        ui->clineEdit->setText(c);
    }
}

void myWindow::updateJoint(QStringList vallist)
{
    if(vallist.size() >=6)
    {
        QString j1 = vallist[0];
        QString j2 = vallist[1];
        QString j3 = vallist[2];
        QString j4 = vallist[3];
        QString j5 = vallist[4];
        QString j6 = vallist[5];

        ui->J1lineEdit->setText(j1);
        ui->J2lineEdit->setText(j2);
        ui->J3lineEdit->setText(j3);
        ui->J4lineEdit->setText(j4);
        ui->J5lineEdit->setText(j5);
        ui->J6lineEdit->setText(j6);
    }
}

void myWindow::myTimerUpdate()
{
    if(tcpsocket && tcpsocket->isOpen())
    {
        QStringList commands = {"MB_RATE", "MB_ENABLE", "MB_SCRAM", "MB_CART", "MB_JOINT"};
        for (int i=0; i<commands.size(); i++)
        {
            QTimer::singleShot(50*i, this, [=](){
                sendCommandToServer(commands[i]);
            });
        }
    }
}
