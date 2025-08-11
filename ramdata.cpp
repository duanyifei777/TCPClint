#include "ramdata.h"
#include "ui_ramdata.h"

#include <QMessageBox>
#include <QDebug>
#include <QTimer>

RAMData::RAMData(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RAMData)
{
    ui->setupUi(this);

    // 为表设置字体格式
    QFont font("微软雅黑", 9);
    ui->SRAMTable->setFont(font);
    ui->DRAMTable->setFont(font);
}

RAMData::~RAMData()
{
    delete ui;
}


void RAMData::on_getSRAMButton_clicked()
{
    isReadingSRAM = true;
    bias = 0;
    QString input = ui->SRAMlineEdit->text().trimmed();
    int type = (ui->SRAMtypeComboBox->currentText() == "整型") ? 0 : 1;  // 判断当前数据类型：0(整型)，1(浮点)
    if(input.contains("-"))  // 输入的是区间
    {
        QStringList parts = input.split("-");
        if(parts.size() == 2)  // 区间是否合法
        {
            bool ok1, ok2;
            int startaddress = parts[0].toInt(&ok1);  // 区间左值
            int endaddress = parts[1].toInt(&ok2);  // 区间右值
            if(ok1 && ok2 && startaddress >= 0 && endaddress <= 127 && startaddress <= endaddress)  // 地址是否合法
            {
                baseindex = startaddress;
                readCommandNumber = endaddress-startaddress+1;
                for(int i=startaddress; i<=endaddress; i++)
                {
                    QTimer::singleShot(50 * (i-startaddress), this, [=](){
                        QString cmd = QString("MB_SRAM:%1,%2").arg(type).arg(i);
                        emit sendCommandToServer(cmd);
                    });
                }
            }
            else{
                QMessageBox::warning(this, "提示", "请输入有效的地址区间：0~127");
            }
        }
        else{
            QMessageBox::warning(this, "提示", "请输入有效的区间格式(如1-10)");
        }
    }
    else  // 输入的是单个地址
    {
        bool ok;
        int address = input.toInt(&ok);
        if(ok && address >= 0 && address <= 127)
        {
            baseindex = address;
            readCommandNumber = 1;
            QString cmd = QString("MB_SRAM:%1,%2").arg(type).arg(address);
            emit sendCommandToServer(cmd);
        }
        else{
            QMessageBox::warning(this, "提示", "请输入正确地址");
        }
    }
}

void RAMData::handleSRAMData(const QString &val)  // 处理服务器返回的SRAM数据
{
    if(!isReadingSRAM) return;  // 如果读取标志位为假，就直接返回
    int currentindex = baseindex + bias;
    int row = currentindex / 8;
    int col = currentindex % 8;

    QTableWidgetItem *item = new QTableWidgetItem(val);
    ui->SRAMTable->setItem(row, col, item);

    if(currentindex == baseindex)
    {
        // 表格自动滚动到指定单元格
        ui->SRAMTable->scrollToItem(ui->SRAMTable->item(row, col), QAbstractItemView::PositionAtCenter);
        // 高亮选中首个单元格
        ui->SRAMTable->setCurrentItem(ui->SRAMTable->item(row, col));
    }

    bias++;
    readCommandNumber--;  // 收到一次读取指令返回，就减1
    if(readCommandNumber == 0)  // 减到0说明这一次的读取指令返回全部接收完了，读取结束了
    {
        isReadingSRAM = false;
    }
}

void RAMData::on_writeSRAMButton_clicked()
{
    int currentrow = ui->SRAMTable->currentRow();
    int currentcol = ui->SRAMTable->currentColumn();
    int type = (ui->SRAMtypeComboBox->currentText() == "整型") ? 0 : 1;
    QTableWidgetItem *item = ui->SRAMTable->item(currentrow, currentcol);

    int number = currentrow * 8 + currentcol;

    QString data = item->text().trimmed();
    QString cmd = QString("MB_SRAM:%1,%2,%3").arg(type).arg(number).arg(data);
    emit sendCommandToServer(cmd);
}


void RAMData::on_getDRAMButton_clicked()
{
    bias = 0;
    isReadingDRAM = true;
    QString input = ui->DRAMlineEdit->text().trimmed();
    int type = (ui->DRAMtypeComboBox->currentText() == "整型")? 0 : 1;
    if(input.contains("-"))
    {
        QStringList parts = input.split("-");
        if(parts.size() == 2)
        {
            bool ok1, ok2;
            int startaddress = parts[0].toInt(&ok1);
            int endaddress = parts[1].toInt(&ok2);
            if(ok1 && ok2 && startaddress >= 0 && endaddress <= 127 && startaddress <= endaddress)
            {
                baseindex = startaddress;
                readCommandNumber = endaddress - startaddress + 1;
                for(int i=startaddress; i<=endaddress; i++)
                {
                    QString cmd = QString("MB_DRAM:%1,%2").arg(type).arg(i);
                    QTimer::singleShot(50 * (i-startaddress), this, [=](){
                        emit sendCommandToServer(cmd);
                    });
                }
            }
            else{
                QMessageBox::warning(this, "提示", "请输入有效的地址区间：0~127");
            }
        }
        else{
            QMessageBox::warning(this, "提示", "请输入有效的区间格式(如1-10)");
        }
    }
    else
    {
        bool ok;
        int address = input.toInt(&ok);
        if(ok && address >= 0 && address <= 127)
        {
            baseindex = address;
            readCommandNumber = 1;
            QString cmd = QString("MB_DRAM:%1,%2").arg(type).arg(address);
            emit sendCommandToServer(cmd);
        }
        else{
            QMessageBox::warning(this, "提示", "请输入正确地址");
        }
    }
}

void RAMData::handleDRAMData(const QString &val)  // 处理服务器返回的DRAM数据
{
    if(!isReadingDRAM) return;  // 不是只读状态就直接返回
    int currentindex = baseindex + bias;
    int row = currentindex / 8;
    int col = currentindex % 8;

    QTableWidgetItem *item = new QTableWidgetItem(val);
    ui->DRAMTable->setItem(row, col, item);

    if(currentindex == baseindex)
    {
        ui->DRAMTable->scrollToItem(ui->DRAMTable->item(row, col), QAbstractItemView::PositionAtCenter);
        ui->DRAMTable->setCurrentItem(ui->DRAMTable->item(row, col));
    }

    readCommandNumber--;
    if(readCommandNumber == 0)
    {
        isReadingDRAM = false;
    }

    bias++;
}

void RAMData::on_writeDRAMButton_clicked()
{
    int currentrow = ui->DRAMTable->currentRow();
    int currentcol = ui->DRAMTable->currentColumn();
    QTableWidgetItem *item = ui->DRAMTable->item(currentrow, currentcol);
    int type = (ui->DRAMtypeComboBox->currentText() == "整型")? 0 : 1;

    int address = currentrow * 8 + currentcol;
    QString data = item->text().trimmed();

    QString cmd = QString("MB_DRAM:%1,%2,%3").arg(type).arg(address).arg(data);
    emit sendCommandToServer(cmd);
}

