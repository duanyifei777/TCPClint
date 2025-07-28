#include "pointinfo.h"
#include "ui_pointinfo.h"

#include <QMessageBox>

pointInfo::pointInfo(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::pointInfo)
{
    ui->setupUi(this);
}

pointInfo::~pointInfo()
{
    delete ui;
}

void pointInfo::on_getPointButton_clicked()
{
    QString point = ui->pointlineEdit->text().trimmed();
    bool ok;
    int pointNumber = point.toInt(&ok);  // 共享点位序号
    if(ok && pointNumber>=0 && pointNumber<=999)
    {
        QString cmd = QString("MB_GLOBALPOINT:%1").arg(pointNumber);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请输入正确点位号");
    }
}

void pointInfo::handelPointInfo(QStringList &vallist)  // 处理获取/写入共享点位信息的返回指令，判断是查询还是写入结果
{
    if(vallist.size() == 1)
    {
        if(vallist[0] == "0")
        {
            return;  // 写入正常
        }
        else if(vallist[0] == "1")
        {
            QMessageBox::warning(this, "提示", "失败");  // 获取失败或写入失败
            return;
        }
    }
    else
    {
        for(int i=0; i<vallist.size(); i++)
        {
            QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);  // 创建表格单元项，并写入数据
            ui->pointInfoTable->setItem(0, i, item);
        }
    }
}

void pointInfo::on_writePointButton_clicked()
{
    QStringList datalist;
    int col = ui->pointInfoTable->columnCount();  // 获取表格列数
    for(int i=0; i<col; i++)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(0, i);
        if(item)
        {
            datalist << item->text();
        }
        else{
            datalist << "";
        }
    }
    QString point = ui->pointlineEdit->text().trimmed();
    bool ok;
    int pointNumber = point.toInt(&ok);
    if(ok && pointNumber >= 0 && pointNumber <= 999)
    {
        QString cmd = QString("MB_GLOBALPOINT:%1,%2").arg(pointNumber).arg(datalist.join(","));
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请输入正确点位号");
    }
}


void pointInfo::on_recordCartButton_clicked()
{
    QString point = ui->pointlineEdit->text().trimmed();
    bool ok;
    int pointNumber = point.toInt(&ok);
    if(ok && pointNumber >= 0 && pointNumber <=999)
    {
        QString cmd = QString("MB_TEACH:%1").arg(pointNumber);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "请输入正确点位号");
    }
}

void pointInfo::handelTechPoint(QString &val)
{
    if(val == "0")
    {
        return; // 机器人坐标写入正常
    }
    else{
        QMessageBox::warning(this, "提示", "发送数据失败");
    }
}
