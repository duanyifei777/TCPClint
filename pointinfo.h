#ifndef POINTINFO_H
#define POINTINFO_H

#include <QWidget>

namespace Ui {
class pointInfo;
}

class pointInfo : public QWidget
{
    Q_OBJECT

public:
    explicit pointInfo(QWidget *parent = nullptr);
    ~pointInfo();

private slots:
    void on_getPointButton_clicked();  // 获取共享点位表中点位信息

    void on_writePointButton_clicked();  // 往共享点位表中写入单个点位信息

    void on_recordCartButton_clicked();  // 记录当前机器人笛卡尔坐标到共享点位表中

    void on_clearTableButton_clicked();  // 清除table数据

public slots:
    void handelPointInfo(QStringList &vallist);
    void handelTechPoint(QString &val);

    void updateTableRow(int pointNumber, const QStringList &vallist);

private:
    Ui::pointInfo *ui;

    int currentrow = 0;  // 表示当前处理的行号
    int basePointNumber = 0;  // 起始点位号

signals:
    void sendCommandToServer(const QString &cmd);
};

#endif // POINTINFO_H
