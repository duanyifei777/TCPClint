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

void IO::updateInputState(const int &instate)
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

void IO::updateOutputState(const int &outstate)
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

void IO::on_updateInputButton_clicked()
{
    emit sendCommandToServer("MB_IOIN");
}


void IO::on_updateOutputButton_clicked()
{
    emit sendCommandToServer("MB_IOOUT");
}

