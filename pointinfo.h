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
    void on_getPointButton_clicked();  // 获取共享点位表中单个点位信息

    void on_writePointButton_clicked();

    void on_recordCartButton_clicked();

public slots:
    void handelPointInfo(QStringList &vallist);
    void handelTechPoint(QString &val);

private:
    Ui::pointInfo *ui;

signals:
    void sendCommandToServer(QString &cmd);
};

#endif // POINTINFO_H
