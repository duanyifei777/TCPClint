#include "mywindow.h"
#include "ui_mywindow.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QFile>
#include <QQueue>

myWindow::myWindow(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::myWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("客户端");
    this->resize(1800,1000);

    // 加载配置文件.qss
    QFile qss(":/qss/clientwindow.qss");
    // QFile qss(":/qss/black.qss");
    if(!qss.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "open fail";
    }
    QString styleSheet = QLatin1String(qss.readAll());
    this->setStyleSheet(styleSheet);

    tcpsocket = new QTcpSocket(this);
    iowindow = new IO(this);
    manualwindow = new manualControl(this);
    pointwindow = new pointInfo(this);
    ramwindow = new RAMData(this);
    coordwindow = new globalCoordinate(this);

    QWidget *centerWidget = new QWidget(this);
    QHBoxLayout *centerlayout = new QHBoxLayout(centerWidget);
    centerlayout->setContentsMargins(0, 0, 0, 0);
    centerlayout->setSpacing(0);

    centerlayout->addWidget(ui->mainWidget, 3);
    centerlayout->addWidget(ui->tabWidget, 3);
    centerlayout->addWidget(manualwindow, 2);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(0);
    mainlayout->addWidget(centerWidget, 1);
    this->setLayout(mainlayout);

    // IO界面
    ioindex = ui->tabWidget->indexOf(ui->tabIO);  // 获取IO界面的索引
    ui->tabWidget->removeTab(ioindex);
    ui->tabWidget->insertTab(ioindex, iowindow, tr("IO监控"));
    connect(iowindow, &IO::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // 定时器
    mainTimer = new QTimer(this);
    connect(mainTimer, &QTimer::timeout, this, &myWindow::myTimerUpdate);
    coordTimer = new QTimer(this);
    connect(coordTimer, &QTimer::timeout, this, &myWindow::coordTimerUpdate);
    coordScheduler = new QTimer(this);
    connect(coordScheduler, &QTimer::timeout, this, &myWindow::processCoordQueue);
    mainScheduler = new QTimer(this);
    connect(mainScheduler, &QTimer::timeout, this, &myWindow::processMainQueue);

    // 修改速度框绑定回车键
    connect(ui->changeSpeedlineEdit, &QLineEdit::returnPressed, this, &myWindow::changeSpeed);

    // 使能按钮初始化
    ui->enabledButton->setCheckable(true);
    ui->enabledButton->setStyleSheet("background-color:gray;");

    // 手动控制界面
    // QVBoxLayout *layout = new QVBoxLayout(ui->manualControlwidget);
    // layout->setContentsMargins(0, 0, 0, 0);
    // layout->addWidget(manualwindow);
    connect(manualwindow, &manualControl::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // 切换界面绑定到每个界面的定时器，这样就可以确保当前界面显示时定时器才开启
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &myWindow::changeTab);

    // 绑定端口号与回车键 用于连接
    connect(ui->portlineEdit, &QLineEdit::returnPressed, this, &myWindow::on_connectButton_clicked);

    // 点位信息界面
    pointindex = ui->tabWidget->indexOf(ui->tabPoint);
    ui->tabWidget->removeTab(pointindex);
    ui->tabWidget->insertTab(pointindex, pointwindow, tr("点位信息"));
    connect(pointwindow, &pointInfo::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // RAM数据界面
    ramindex = ui->tabWidget->indexOf(ui->tabRAM);
    ui->tabWidget->removeTab(ramindex);
    ui->tabWidget->insertTab(ramindex, ramwindow, tr("RAM数据"));
    connect(ramwindow, &RAMData::sendCommandToServer, this, &myWindow::sendCommandToServer);

    // 全局坐标系界面
    coordindex = ui->tabWidget->indexOf((ui->tabCoord));
    ui->tabWidget->removeTab(coordindex);
    ui->tabWidget->insertTab(coordindex, coordwindow, tr("全局坐标系"));
    connect(coordwindow, &globalCoordinate::sendCommendToServer, this, &myWindow::sendCommandToServer);

    // 初始化机器人运行状态按钮:开始/停止/暂停/复位
    ui->startARButton->setIcon(QIcon(":/icon/start.png"));
    ui->startARButton->setIconSize(QSize(30,30));
    ui->startARButton->setFixedSize(50,50);
    ui->stopARButton->setIcon(QIcon(":/icon/stop.png"));
    ui->stopARButton->setIconSize(QSize(25,25));
    ui->stopARButton->setFixedSize(50,50);
    ui->pauseARButton->setIcon(QIcon(":/icon/pause.png"));
    ui->pauseARButton->setIconSize(QSize(25,25));
    ui->pauseARButton->setFixedSize(50,50);
    ui->restartARButton->setIcon(QIcon(":/icon/restart.png"));
    ui->restartARButton->setIconSize(QSize(25,25));
    ui->restartARButton->setFixedSize(50,50);

    // 初始化机器人状态指示灯
    ui->ledLabel->setFixedSize(40,40);
    ui->ledLabel->setStyleSheet(R"(
    QLabel{
        background-color: qradialgradient(cx:0.3, cy:0.3, radius:1,fx:0.3, fy:0.3,stop:0 white,stop:1 gray);
        border: 4px solid #898989;
        border-radius: 20px;}
    )");
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
        sendCommandToServer("MB_MRELEASE");

        // 启动定时器
        mainTimer->start(600);  // 其他信息
        coordTimer->start(100);  // 坐标信息

        // 启动调度器
        coordScheduler->start(100);  // 坐标队列调度器
        mainScheduler->start(150);  // 其余信息队列调度器
    } else {
        QMessageBox::warning(this, "提示", "连接服务器失败");
    }
}

void myWindow::on_closeButton_clicked()
{
    if (tcpsocket->isOpen()) {
        sendCommandToServer("MB_MRELEASE");

        mainTimer->stop();
        coordTimer->stop();
        coordScheduler->stop();
        mainScheduler->stop();

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
    if(data.isEmpty()) return;
    QString text = QString::fromUtf8(data);
    // qDebug() << "收到数据原始：" << text;

    QStringList message = text.split("\r\n", Qt::SkipEmptyParts);

    for(const QString &msg : message)
    {
        QRegularExpression re("MB_\\w+:[^\\s]+");
        QRegularExpressionMatch match = re.match(msg);

        if(match.hasMatch())
        {
            QString cmd = match.captured(0);
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
            else if(key == "MB_GLOBALPOINT")
            {
                QStringList vallist = value.split(',');
                pointwindow->handelPointInfo(vallist);
            }
            else if(key == "MB_TEACH")
            {
                pointwindow->handelTechPoint(value);
            }
            else if(key == "MB_SRAM")
            {
                ramwindow->handleSRAMData(value);
                // qDebug() << "收到：" << text;
            }
            else if(key == "MB_DRAM")
            {
                ramwindow->handleDRAMData(value);
            }
            else if(key == "MB_ARWORK")
            {
                int val = value.toInt();
                this->handleARWork(val);
            }
            else if(key == "MB_AR_STATE")
            {
                int val = value.toInt();
                this->updateARState(val);
            }
            else if(key == "MB_MODE")
            {
                int val = value.toInt();
                this->updateMode(val);
            }
            else if(key == "MB_MANUALMODE")
            {
                int val = value.toInt();
                manualwindow->handleManualMode(val);
            }
            else if(key == "MB_MACCEPT")
            {
                int val = value.toInt();
                pointwindow->handleMotionAccept(val);
            }
            else if(key == "MB_MRELEASE")
            {
                int val = value.toInt();
                pointwindow->handleMotionRelease(val);
            }
            else if(key == "MB_TRACKMOTION")
            {
                int val = value.toInt();
                pointwindow->handleTrackPoint(val);
            }
            else if(key == "MB_COORD")
            {
                if(coordQueryForm == CoordQueryFrom::Main)
                {
                    int val = value.toInt();
                    this->handleCoord(val);
                }
                else if(coordQueryForm == CoordQueryFrom::TabCoord)
                {
                    int val = value.toInt();
                    coordwindow->handleCurrentIndex(val);
                }
            }
            else if(key == "MB_MOVP")
            {
                int val = value.toInt();
                pointwindow->handleMoveP(val);
            }
            else if(key == "MB_MARCHP")
            {
                int val = value.toInt();
                pointwindow->handleMArchP(val);
            }
            else if(key == "MB_MOVJ")
            {
                int val = value.toInt();
                pointwindow->handleMovJ(val);
            }
            else if(key == "MB_MOVL")
            {
                int val = value.toInt();
                pointwindow->handleMovL(val);
            }
            else if(key == "MB_MARC")
            {
                int val = value.toInt();
                pointwindow->handleMarc(val);
            }
            else if(key == "MB_WAITPOS")
            {
                int val = value.toInt();
                pointwindow->handleWaitPos(val);
            }
            else if(key == "MB_WAITREALPOS")
            {
                int val = value.toInt();
                pointwindow->handleWaitRealPos(val);
            }
            else if(key == "MB_COORD_USER")
            {
                QStringList vallist = value.split(",");
                coordwindow->handleCoordUser(vallist);
            }
            else if(key == "MB_COORD_TOOL")
            {
                QStringList vallist = value.split(",");
                coordwindow->handleCoordTool(vallist);
            }
            // else if(key == "MB_JOINTMOVE")
            // {
            //     qDebug() << "收到指令：" << text;
            // }
        }
    }
}

void myWindow::sendCommandToServer(const QString &cmd)
{
    if (tcpsocket && tcpsocket->isOpen()) {
        QByteArray tosend = cmd.toUtf8();

        tosend.append("\r\n");
        tcpsocket->write(tosend);
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
    if (!ok || rate < 0 || rate > 100)
    {
        QMessageBox::warning(this, "提示", "请输入有效速率");
        return;
    }
    QString cmd = QString("MB_RATE:%1").arg(rate);
    sendCommandToServer(cmd);
}

void myWindow::on_changeSpeedButton_clicked()
{
    changeSpeed();
}

void myWindow::updateSpeed(const QString &val)
{
    QString numval;
    for(QChar c : val)
    {
        if(c.isDigit())
        {
            numval.append(c);
        }
    }
    if (!numval.isEmpty() && numval != "0") {
        ui->showSpeedlabel->setText(numval + "%");
    }
}

void myWindow::on_enabledButton_clicked()
{
    if (tcpsocket && tcpsocket->isOpen()) {
        bool enabled = ui->enabledButton->isChecked();
        if (enabled) {
            sendCommandToServer("MB_ENABLE:1");
            ui->enabledButton->setStyleSheet("background-color:green; border:none; "
                                             "font-family:'Microsoft YaHei'");
            ui->enabledButton->setText("开启");
        } else {
            sendCommandToServer("MB_ENABLE:0");
            ui->enabledButton->setStyleSheet("background-color:gray; border:none; "
                                             "font-family:'Microsoft YaHei'");
            ui->enabledButton->setText("关闭");
        }
    } else {
        QMessageBox::warning(this, "提示", "未连接，无法改变使能状态");
    }
}

void myWindow::updateEnabled(int val)
{
    ui->enabledButton->setChecked(val);
    if (val == 0) {
        ui->enabledButton->setStyleSheet("background-color:gray; border:none; "
                                         "font-family:'Microsoft YaHei'");
        ui->enabledButton->setText("关闭");
    } else {
        ui->enabledButton->setStyleSheet("background-color:green; border:none; "
                                         "font-family:'Microsoft YaHei'");
        ui->enabledButton->setText("开启");
    }
}

void myWindow::updateAlarm(const QStringList &vallist)
{
    if (vallist.size() == 2) {
        QString glScram = vallist[0];
        QString scramNo = vallist[1];

        if (glScram == "0") {
            isAlarm = false;
            ui->warningtextBrowser->setText("✅ 正常");
        }
        else {
            isAlarm = true;
            ui->warningtextBrowser->setText("❌ 报警，编号：" + scramNo);
            ui->ledLabel->setStyleSheet(R"(QLabel{
            background-color: qradialgradient(cx:0.3, cy:0.3, radius:1,fx:0.3, fy:0.3,
            stop:0 #ffcccc,stop:1 #cc0000);
            border: 4px solid #898989;
            border-radius: 20px;})");
            ui->ledLabel->setToolTip("报警");
        }
    }
}

void myWindow::updateCart(const QStringList &vallist)
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

void myWindow::updateJoint(const QStringList &vallist)
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
        coordQueryForm = CoordQueryFrom::Main;  // 从<主界面>定时器发出的查询坐标系指令
        QStringList commands = {"MB_RATE", "MB_ENABLE", "MB_SCRAM",
                                "MB_AR_STATE", "MB_MODE", "MB_COORD"};
        for(const QString &cmd : commands)
        {
            mainQueue.enqueue(cmd);
        }
    }
}

void myWindow::changeTab(int index)
{
    // mainTimer->stop();
    iowindow->stopTimer();
    // pointwindow->stopTimer();

    // if(index == mainindex)  // 主界面
    // {
    //     if(tcpsocket && tcpsocket->isOpen())
    //     {
    //         mainTimer->start(500);
    //     }
    // }
    if(index == ioindex)  // IO界面
    {
        if(tcpsocket && tcpsocket->isOpen())
        {
            iowindow->startTimer();
        }
    }
    // else if(index == pointindex)  // 点位信息界面
    // {
    //     if(tcpsocket && tcpsocket->isOpen())
    //     {
    //         pointwindow->startTimer();
    //     }
    // }
}

void myWindow::handleARWork(int val)  // MB_ARWORK:flag 0：正常, 1：失败
{
    // qDebug() << val ;
    if(val == 0)  // 设置AR运行状态成功
    {
        if(ARWorkStatus == "start")  // 成功启动
        {
            ui->startARButton->setEnabled(false);  // 暂时禁用start
            ARPause = false;
        }
        else if(ARWorkStatus == "pause")
        {
            ui->startARButton->setEnabled(true);
            ARPause = true;
        }
        else if(ARWorkStatus == "stop")  // 停止成功
        {
            ui->startARButton->setEnabled(true);  // 启用start
            ARPause = false;
        }
    }
    else if(val == 1)  // 设置AR运行状态失败
    {
        QMessageBox::warning(this, "提示", "失败");
    }
}

void myWindow::on_startARButton_clicked()
{
    QString text;
    if(ARPause)  // 表示处于暂停状态
    {
        text = "继续运行AR程序";
    }
    else{
        text = "是否启动AR程序";
    }
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认操作", text,
                                  QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        ARWorkStatus = "start";
        QString cmd = QString("MB_ARWORK:1");
        sendCommandToServer(cmd);
    }
}

void myWindow::on_pauseARButton_clicked()
{
    ARWorkStatus = "pause";
    QString cmd = QString("MB_ARWORK:2");
    sendCommandToServer(cmd);
}

void myWindow::on_stopARButton_clicked()
{
    ARWorkStatus = "stop";
    QString cmd = QString("MB_ARWORK:3");
    sendCommandToServer(cmd);
}

void myWindow::on_restartARButton_clicked()
{
    QString cmd = QString("MB_ARWORK:4");
    sendCommandToServer(cmd);
}

void myWindow::updateARState(int val)
{
    if(isAlarm) return;

    if(val == 0)  // 停止状态(空闲)
    {
        ui->ledLabel->setStyleSheet(R"(QLabel{
        background-color: qradialgradient(cx:0.3, cy:0.3, radius:1,fx:0.3, fy:0.3,stop:0 white,stop:1 gray);
        border: 4px solid #898989;
        border-radius: 20px;})");
        ui->ledLabel->setToolTip("空闲");
    }
    else if(val == 1)  // 暂停状态
    {
        ui->ledLabel->setStyleSheet(R"(QLabel{
        background-color: qradialgradient(cx:0.3, cy:0.3, radius:1, fx:0.3, fy:0.3,
        stop:0 #ffffcc, stop:1 #ffcc00);
        border: 4px solid #898989;
        border-radius: 20px;})");
        ui->ledLabel->setToolTip("暂停");
    }
    else if(val == 2)  // 运行状态
    {
        ui->ledLabel->setStyleSheet(R"(QLabel{
        background-color: qradialgradient(cx:0.3, cy:0.3, radius:1, fx:0.3, fy:0.3,
        stop:0 #ccffcc, stop:1 #00cc00);
        border: 4px solid #898989;
        border-radius: 20px;})");
        ui->ledLabel->setToolTip("运行中");
    }
    else  // 手动控制状态
    {
        ui->ledLabel->setStyleSheet(R"(QLabel{
        background-color: qradialgradient(cx:0.3, cy:0.3, radius:1, fx:0.3, fy:0.3,
        stop:0 #cce6ff, stop:1 #3399ff);
        border: 4px solid #898989;
        border-radius: 20px;})");
        ui->ledLabel->setToolTip("手动控制中");
    }
}

void myWindow::updateMode(int val)
{
    ui->showModelabel->setText(val == 0 ? "手动" : "自动");
}

void myWindow::handleCoord(int val)
{
    if(ChangeCoord)
    {
        ChangeCoord = false;
        if(val == 0) return;
        else if(val == 1)
        {
            QMessageBox::warning(this, "提示", "运动中设置失败");
        }
        else if(val == 2)
        {
            QMessageBox::warning(this, "提示", "同步位置失败");
        }
        else if(val == -1)
        {
            QMessageBox::warning(this, "提示", "目标坐标系未创建");
        }
    }
    else{
        QString binarystate = QString::number(val, 2).rightJustified(16, '0');  // 转换为16位2进制字符串
        QString highstate = binarystate.left(8);  // 高8位
        QString lowstate = binarystate.right(8);  // 低8位

        int user = highstate.toInt(nullptr, 2);  // 用户坐标系编号
        int tool = lowstate.toInt(nullptr, 2);  // 工具坐标系编号

        ui->showUCoordlabel->setText(QString::number(user));
        ui->showTCoordlabel->setText(QString::number(tool));
    }
}

void myWindow::on_changeCoordButton_clicked()
{
    int user = ui->changeUCoordBox->currentText().toInt();
    int tool = ui->changeTCoordBox->currentText().toInt();

    int combine = (user<<8) | tool;  // 将user左移八位作为高八位，和tool做或运算得到16位编码的十进制数

    QString cmd = QString("MB_COORD:%1").arg(combine);
    sendCommandToServer(cmd);
    ChangeCoord = true;  // 表示是修改坐标系
}
void myWindow::on_changeCoordButton_2_clicked()
{
    int user = ui->changeUCoordBox->currentText().toInt();
    int tool = ui->changeTCoordBox->currentText().toInt();

    int combine = (user<<8) | tool;  // 将user左移八位作为高八位，和tool做或运算得到16位编码的十进制数

    QString cmd = QString("MB_COORD:%1").arg(combine);
    sendCommandToServer(cmd);
    ChangeCoord = true;  // 表示是修改坐标系
}

void myWindow::coordTimerUpdate()
{
    if(tcpsocket && tcpsocket->isOpen())
    {
        coordQueue.enqueue("MB_CART");
        coordQueue.enqueue("MB_JOINT");
    }
}


void myWindow::processCoordQueue()  // 调度器函数
{
    if(tcpsocket && tcpsocket->isOpen() && !coordQueue.isEmpty())
    {
        QString cmd = coordQueue.dequeue();
        sendCommandToServer(cmd);
    }
}

void myWindow::processMainQueue()
{
    if(tcpsocket && tcpsocket->isOpen() && !mainQueue.isEmpty())
    {
        QString cmd = mainQueue.dequeue();
        sendCommandToServer(cmd);
    }
}
