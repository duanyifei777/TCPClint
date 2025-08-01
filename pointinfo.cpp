#include "pointinfo.h"
#include "ui_pointinfo.h"

#include <QMessageBox>
#include <QTimer>
#include <QDebug>

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
    ui->pointInfoTable->clearContents();  // 清空表格内容
    ui->pointInfoTable->setRowCount(0);  // 初始化表格的行数，重置当前行
    currentrow = 0;

    QString input = ui->pointlineEdit->text().trimmed();  // 获取框中输入的编号
    if(input.contains("-"))  // 说明是区间（1-10）
    {
        QStringList parts = input.split("-");
        if(parts.size() == 2)
        {
            bool ok1, ok2;
            int startpoint = parts[0].trimmed().toInt(&ok1);  // 起始点位号
            int endpoint = parts[1].trimmed().toInt(&ok2);  // 结束点位号
            if(ok1 && ok2 && startpoint >=0 && endpoint <=999 && startpoint <= endpoint)
            {
                basePointNumber = startpoint;  // 保存当前起始点位号

                for(int i=startpoint; i<=endpoint; i++)
                {
                    QString cmd = QString("MB_GLOBALPOINT:%1").arg(i);  // 循环获取单个点位信息的指令，直到获取所有点位信息
                    QTimer::singleShot(50 * (i-startpoint), this, [=](){
                       emit sendCommandToServer(cmd);
                    });
                }
            }
            else{
                QMessageBox::warning(this, "提示", "请输入有效的数字区间：0~999");
            }
        }
        else{
            QMessageBox::warning(this, "提示", "请输入有效的区间格式(如1-10)");
        }
    }
    else  // 说明是单个点位（1,2...）
    {
        bool ok;
        int pointNumber = input.toInt(&ok);  // 共享点位序号

        basePointNumber = pointNumber;

        if(ok && pointNumber>=0 && pointNumber<=999)
        {
            QString cmd = QString("MB_GLOBALPOINT:%1").arg(pointNumber);
            emit sendCommandToServer(cmd);
        }
        else{
            QMessageBox::warning(this, "提示", "请输入正确点位号");
        }
    }
}

void pointInfo::handelPointInfo(const QStringList &vallist)  // 处理获取/写入共享点位信息的返回指令，判断是查询还是写入结果
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
        int poinNumber = basePointNumber + currentrow;
        // qDebug() << "basepointnumber=" << basePointNumber << " " << "currentrow=" << currentrow;
        updateTableRow(poinNumber, vallist);

        currentrow++;  // 下一次写入表格的下一行
    }
}

void pointInfo::on_writePointButton_clicked()
{
    int selectrow = ui->pointInfoTable->currentRow();  // 获取选中的行
    QTableWidgetItem *pointItem = ui->pointInfoTable->item(selectrow, 0);  // 获取点位号（第0列）
    if(!pointItem)
    {
        QMessageBox::warning(this, "提示", "点位号为空");
        return;
    }

    QString pointText = pointItem->text().trimmed();
    bool ok;
    int pointNumber = pointText.toInt(&ok);
    if(!ok || pointNumber < 0 || pointNumber >999)
    {
        QMessageBox::warning(this, "提示", "点位号无效");
        return;
    }

    QStringList datalist;
    int col = ui->pointInfoTable->columnCount();  // 获取表格列数
    for(int i=1 ; i<col; i++)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(selectrow, i);
        if(item)
        {
            datalist << item->text();
        }
        else{
            datalist << "";
        }
    }

    QString cmd = QString("MB_GLOBALPOINT:%1,%2").arg(pointNumber).arg(datalist.join(","));
    emit sendCommandToServer(cmd);
}


void pointInfo::on_recordCartButton_clicked()
{
    currentrow = 0;
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

void pointInfo::handelTechPoint(const QString &val)
{
    currentrow = 0;
    if(val == "0")  // 机器人坐标写入正常
    {
        QString point = ui->pointlineEdit->text().trimmed();
        int pointNumber = point.toInt();

        basePointNumber = pointNumber;  // 记录当前需要写入的点位号

        QString cmd = QString("MB_GLOBALPOINT:%1").arg(pointNumber);
        emit sendCommandToServer(cmd);
    }
    else{
        QMessageBox::warning(this, "提示", "发送数据失败");
    }
}

void pointInfo::updateTableRow(int pointNumber, const QStringList &vallist)
{
    int row = -1;

    int rowcount = ui->pointInfoTable->rowCount();
    for(int i=0; i<rowcount; i++)
    {
        QTableWidgetItem *item = ui->pointInfoTable->item(i, 0);  // 遍历table每行的点位号
        if(item && item->text() == QString::number(pointNumber))  // 如果要写入的点位号已在表中，则标记该行
        {
            row = i;
            break;
        }
    }

    if(row == -1)  // 不在表中，就新增一行用来显示机器人当前坐标的点位信息
    {
        row = ui->pointInfoTable->rowCount();
        ui->pointInfoTable->insertRow(row);  // 增加一行

        QTableWidgetItem *iditem = new QTableWidgetItem(QString::number(pointNumber));
        iditem->setFlags(iditem->flags() & ~Qt::ItemIsEnabled);
        ui->pointInfoTable->setItem(row, 0, iditem);
    }

    for(int i=0; i<vallist.size(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(vallist[i]);
        ui->pointInfoTable->setItem(row, i+1, item);
    }
}

void pointInfo::on_clearTableButton_clicked()
{
    ui->pointInfoTable->clearContents();
    ui->pointInfoTable->setRowCount(0);
}

