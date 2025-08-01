#ifndef RAMDATA_H
#define RAMDATA_H

#include <QWidget>

namespace Ui {
class RAMData;
}

class RAMData : public QWidget
{
    Q_OBJECT

public:
    explicit RAMData(QWidget *parent = nullptr);
    ~RAMData();

private:
    Ui::RAMData *ui;

    int baseindex = 0;  // 输入的起始地址
    int bias = 0;  // 地址的偏移量(增加量)

    bool isReadingSRAM = false;  // 判断是否是读取SRAM指令的标志位
    int readCommandNumber = 0;  // 判断点一次获取按钮一共要发送几次读取指令
    bool isReadingDRAM = false;  // 判断是否是读取DRAM指令的标志位

signals:
    void sendCommandToServer(const QString &cmd);

private slots:
    void on_getSRAMButton_clicked();  // 获取SRAM数据
    void on_writeSRAMButton_clicked();  // 写入SRAM数据

    void on_getDRAMButton_clicked();

    void on_writeDRAMButton_clicked();

public slots:
    void handleSRAMData(const QString &value);
    void handleDRAMData(const QString &value);
};

#endif // RAMDATA_H
