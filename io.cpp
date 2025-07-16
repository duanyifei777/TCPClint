#include "io.h"
#include "ui_io.h"

IO::IO(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::IO)
{
    ui->setupUi(this);

    // for(int i=0; i<16; i++)
    // {
    //     QString pbName = QString("I%1Button").arg(i);
    //     QPushButton *pb =findChild<QPushButton*>(pbName);
    //     if(pb)
    //     {
    //         pb->setCheckable(true);
    //         connect(pb, &QPushButton::clicked, this, &IO::handleIutputButton());
    //     }
    // }

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
    pb->setStyleSheet(QString("background-color:%1; border-radius:10px;").arg(color));

    QString cmd = QString("SET O%1 %2").arg(num).arg(checked ? 1 : 0);
    emit sendCommandToServer(cmd);
}

