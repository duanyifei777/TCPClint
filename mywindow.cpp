#include "mywindow.h"
#include "ui_mywindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>

myWindow::myWindow(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::myWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("客户端");
    this->resize(1100,800);

    tcpsocket = new QTcpSocket(this);
    iowindow = new IO(this);
    manualwindow = new manualControl(this);

    mainindex = ui->tabWidget->indexOf(ui->Main);  // 获取main界面的索引
    // IO界面
    ioindex = ui->tabWidget->indexOf(ui->tabIO);  // 获取IO界面的索引
    ui->tabWidget->removeTab(ioindex);
    ui->tabWidget->addTab(iowindow, tr("IO监控"));
    connect(iowindow, &IO::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // 定时器
    mainTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, &myWindow::myTimerUpdate);

    // 修改速度框绑定回车键
    connect(ui->changeSpeedlineEdit, &QLineEdit::returnPressed, this, &myWindow::changeSpeed);

    // 使能按钮初始化
    ui->enabledButton->setCheckable(true);
    ui->enabledButton->setStyleSheet("background-color:gray;");

    // 手动控制界面
    QVBoxLayout *layout = new QVBoxLayout(ui->manualControlwidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(manualwindow);
    connect(manualwindow, &manualControl::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // 切换界面绑定到每个界面的定时器，这样就可以确保当前界面显示时定时器才开启
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &myWindow::changeTab);

    connect(ui->portlineEdit, &QLineEdit::returnPressed, this, &myWindow::on_connectButton_clicked);
}

myWindow::~myWindow()
{
    delete ui;
}

void myWindow::on_connectButton_clicked()
{
    QString ip = ui->iplineEdit->text().trimmed();  // 获取输入框中的IP
    int port = ui->portlineEdit->text().toInt();  // 获取输入框中的端口号

    if(ip.isEmpty() || port==0)
    {
        QMessageBox::warning(this, "提示", "ip与端口号不能为空");
        return;
    }

    tcpsocket->connectToHost(ip, port);  // 进行tcp连接

    if (tcpsocket->waitForConnected(5000)) {
        connect(tcpsocket, &QTcpSocket::readyRead, this, &myWindow::readyRead_slot);
        QMessageBox::information(this, "提示", "连接服务器成功");
    } else {
        QMessageBox::warning(this, "提示", "连接服务器失败");
    }
}

void myWindow::on_closeButton_clicked()
{
    if (tcpsocket->isOpen()) {
        tcpsocket->disconnectFromHost();
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

    QRegularExpression re("MB_\\w+:[^\\s]+");
    QRegularExpressionMatchIterator it = re.globalMatch(text);

    while (it.hasNext()) {
        QString cmd = it.next().captured(0);
        QString key = cmd.section(":", 0, 0);
        QString value = cmd.section(":", 1, 1);

        if (key == "MB_IOIN")
        {
            int val = value.toInt();
            iowindow->updateInputState(val);
        }
        else if (key == "MB_IOOUT")
        {
            int val = value.toInt();
            iowindow->updateOutputState(val);
        }
        else if (key == "MB_RATE")
        {
            this->updateSpeed(value);
        }
        else if (key == "MB_ENABLE")
        {
            int val = value.toInt();
            this->updateEnabled(val);
        }
        else if (key == "MB_SCRAM")
        {
            QStringList vallist = value.split(',');
            this->updateAlarm(vallist);
        }
        else if (key == "MB_CART")
        {
            QStringList vallist = value.split(',');
            this->updateCart(vallist);
        }
        else if (key == "MB_JOINT")
        {
            QStringList vallist = value.split(',');
            this->updateJoint(vallist);
        }
    }
}

void myWindow::sendCommandToServer(const QString &cmd)
{
    if (tcpsocket && tcpsocket->isOpen()) {
        QString tosend = cmd.trimmed();
        tcpsocket->write(tosend.toUtf8());
        tcpsocket->flush();
        // qDebug() << "已发送查询命令：" << tosend;
    } else {
        QMessageBox::warning(this, "提示", "未连接到服务器");
    }
}

void myWindow::changeSpeed()
{
    QString rateVal = ui->changeSpeedlineEdit->text().trimmed();
    bool ok;
    int rate = rateVal.toInt(&ok);
    if (!ok || rate < 0 || rate > 100) {
        QMessageBox::warning(this, "提示", "请输入有效速率");
        return;
    }
    QString cmd = QString("MB_RATE:%1").arg(rateVal);
    sendCommandToServer(cmd);
}

void myWindow::updateSpeed(QString val)
{
    if (val != "0") {
        ui->showSpeedlabel->setText(val);
    }
}

void myWindow::on_enabledButton_clicked()
{
    if (tcpsocket && tcpsocket->isOpen()) {
        bool enabled = ui->enabledButton->isChecked();
        if (enabled) {
            sendCommandToServer("MB_ENABLE:1");
            ui->enabledButton->setStyleSheet("background-color:green; font-size:28px; border:none; "
                                             "font-family:'Microsoft YaHei'");
        } else {
            sendCommandToServer("MB_ENABLE:0");
            ui->enabledButton->setStyleSheet("background-color:gray; font-size:28px; border:none; "
                                             "font-family:'Microsoft YaHei'");
        }
    } else {
        QMessageBox::warning(this, "提示", "未连接，无法改变使能状态");
    }
}

void myWindow::updateEnabled(int val)
{
    ui->enabledButton->setChecked(val);
    if (val == 0) {
        ui->enabledButton->setStyleSheet("background-color:gray; font-size:28px; border:none; "
                                         "font-family:'Microsoft YaHei'");
    } else {
        ui->enabledButton->setStyleSheet("background-color:green; font-size:28px; border:none; "
                                         "font-family:'Microsoft YaHei'");
    }
}

void myWindow::updateAlarm(QStringList vallist)
{
    if (vallist.size() == 2) {
        QString glScram = vallist[0];
        QString scramNo = vallist[1];

        if (glScram == "0") {
            ui->warningtextBrowser->setText("✅ 正常");
        } else {
            ui->warningtextBrowser->setText("❌ 报警，编号：" + scramNo);
        }
    }
}

void myWindow::updateCart(QStringList vallist)
{
    if (vallist.size() >= 6) {
        ui->xlineEdit->setText(vallist[0]);
        ui->ylineEdit->setText(vallist[1]);
        ui->zlineEdit->setText(vallist[2]);
        ui->alineEdit->setText(vallist[3]);
        ui->blineEdit->setText(vallist[4]);
        ui->clineEdit->setText(vallist[5]);
    }
}

void myWindow::updateJoint(QStringList vallist)
{
    if (vallist.size() >= 6) {
        ui->J1lineEdit->setText(vallist[0]);
        ui->J2lineEdit->setText(vallist[1]);
        ui->J3lineEdit->setText(vallist[2]);
        ui->J4lineEdit->setText(vallist[3]);
        ui->J5lineEdit->setText(vallist[4]);
        ui->J6lineEdit->setText(vallist[5]);
    }
}

void myWindow::myTimerUpdate()
{
    if (tcpsocket && tcpsocket->isOpen()) {
        QStringList commands = {"MB_RATE", "MB_ENABLE", "MB_SCRAM", "MB_CART", "MB_JOINT"};
        for (int i = 0; i < commands.size(); i++) {
            QTimer::singleShot(50 * i, this, [=]() {
                sendCommandToServer(commands[i]);
            });
        }
    }
}

void myWindow::changeTab(int index)
{
    mainTimer->stop();
    iowindow->stopTimer();

    if(index == mainindex)  // 主界面
    {
        if(tcpsocket && tcpsocket->isOpen())
        {
            mainTimer->start(500);
        }
    }
    else if(index == ioindex)  // IO界面
    {
        if(tcpsocket && tcpsocket->isOpen())
        {
            iowindow->startTimer();
        }
    }
}
