#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include "io.h"

namespace Ui {
class myWindow;
}

class myWindow : public QWidget
{
    Q_OBJECT

public:
    explicit myWindow(QWidget *parent = nullptr);
    ~myWindow();

private slots:
    void on_connectButton_clicked();
    void on_closeButton_clicked();

    void readyRead_slot();

    void sendCommandToServer(const QString &cmd);  // 处理IO面板发出的信号

private:
    Ui::myWindow *ui;
    QTcpSocket *tcpsocket;
    IO *iowindow = nullptr;
};

#endif // MYWINDOW_H
