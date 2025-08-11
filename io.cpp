#include "io.h"
#include "ui_io.h"
#include<QDebug>

IO::IO(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IO)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window);

    // 初始化16路Output
    for(int i=0; i<16; i++)
    {
        QString pbName = QString("O%1Button").arg(i);
        QPushButton *pb =findChild<QPushButton*>(pbName);
        if(pb)
        {
            pb->setCheckable(true);
            connect(pb, &QPushButton::clicked, this, &IO::handleOutputButton);
        }
    }
    // 初始化16路Input
    for(int i=0; i<16; i++)
    {
        QString pbName = QString("I%1Button").arg(i);
        QPushButton *pb = findChild<QPushButton*>(pbName);
        if(pb)
        {
            pb->setCheckable(false);
        }
    }
    // 初始化I/O 100-131
    for (int i=0; i<32; i++)
    {
        QString ipbName = QString("I%1Button").arg(100+i);
        QString opbName = QString("O%1Button").arg(100+i);
        QPushButton *ipb = findChild<QPushButton*>(ipbName);
        QPushButton *opb = findChild<QPushButton*>(opbName);
        if(ipb)
        {
            ipb->setCheckable(false);
            ipb->setStyleSheet("background-color:gray; border-radius:10px; border:1px solid black");
        }
        if(opb)
        {
            opb->setCheckable(false);
            opb->setStyleSheet("background-color:gray; border-radius:10px; border:1px solid black");
        }
    }
    // 初始化I/O 200-231
    for (int i=0; i<32; i++)
    {
        QString ipbName = QString("I%1Button").arg(200+i);
        QString opbName = QString("O%1Button").arg(200+i);
        QPushButton *ipb = findChild<QPushButton*>(ipbName);
        QPushButton *opb = findChild<QPushButton*>(opbName);
        if(ipb)
        {
            ipb->setCheckable(false);
            ipb->setStyleSheet("background-color:gray; border-radius:10px; border:1px solid black");
        }
        if(opb)
        {
            opb->setCheckable(false);
            opb->setStyleSheet("background-color:gray; border-radius:10px; border:1px solid black");
        }
    }
    // 初始化定时器
    iotimer = new QTimer(this);
    connect(iotimer, &QTimer::timeout, this, [=]() mutable {
        static bool mark;
        QString cmd = mark ? "MB_IOIN" : "MB_IOOUT";
        emit sendCommandToServer(cmd);
        // qDebug() << "已发送查询命令：" << cmd.trimmed();
        mark = !mark;
    });

}

IO::~IO()
{
    delete ui;
}

void IO::handleOutputButton()
{
    QPushButton *pb = qobject_cast<QPushButton*>(sender());
    if(!pb) return;

    QString name = pb->objectName();
    QString num = name.mid(1, name.indexOf("Button")-1);

    bool checked = pb->isChecked();

    QString color = checked ? "green" : "red";
    pb->setStyleSheet(QString("background-color:%1;border-radius:10px; border:1px solid black;").arg(color));// 指示灯变为绿色

    QString cmd = QString("MB_IOOUTBITSET:%1,%2").arg(num).arg(checked ? 1 : 0);
    emit sendCommandToServer(cmd);  // 给主窗口发信号
}

void IO::updateInputState(int instate)
{
    QString binarystate = QString::number(instate,2).rightJustified(16, '0');
    int num = binarystate.size();
    for(int i=0; i<num; i++)
    {
        QString pbName = QString("I%1Button").arg(num-1-i);
        QPushButton *pb = findChild<QPushButton*>(pbName);
        if(pb)
        {
            QString color = (binarystate[i] == '1') ? "green" : "red";
            pb->setStyleSheet(QString("background-color:%1; border-radius:10px; border:1px solid black;").arg(color));
        }
    }
}

void IO::updateOutputState(int outstate)
{
    // qDebug() << outstate;
    QString binarystate = QString::number(outstate, 2).rightJustified(16, '0');
    // qDebug() << binarystate;
    int num = binarystate.size();
    for(int i=0; i<num; i++)
    {
        QString pbName = QString("O%1Button").arg(num-1-i);
        QPushButton *pb = findChild<QPushButton*>(pbName);
        if(pb)
        {
            QString color = (binarystate[i] == '1') ? "green" : "red";
            pb->setChecked(binarystate[i] == '1');
            pb->setStyleSheet(QString("background-color:%1; border-radius:10px; border:1px solid black").arg(color));
        }
    }
}

void IO::setButtonEnabled(QString &name, bool enabled)
{
    QPushButton *pb = findChild<QPushButton*>(name);
    if(!pb) return;

    pb->setEnabled(enabled);
    if(enabled)
    {
        pb->setStyleSheet("background-color:red;border-radius:10px; border:1px solid black");
    }
    else{
        pb->setStyleSheet("background-color:gray;border-radius:10px; border:1px solid black");
    }
}

void IO::startTimer()
{
    if(!iotimer->isActive())
    {
        iotimer->start(500);
    }
}

void IO::stopTimer()
{
    if(iotimer->isActive())
    {
        iotimer->stop();
    }
}
