#include "paraminputdialog.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QMap>
#include <QLabel>
#include <QDialogButtonBox>

ParamInputDialog::ParamInputDialog(const QStringList &paramNames,
                                   const QMap<QString, QString> &paramHints,
                                   QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("请输入参数");
    // 窗口主垂直布局
    QVBoxLayout *mainlayout = new QVBoxLayout(this);

    // 表单布局，存放标签：输入框
    QFormLayout *formlayout = new QFormLayout();

    QFont labelFont("微软雅黑", 12);
    QFont inputFont("微软雅黑", 12);

    // 遍历参数名列表
    for(const QString &paramName : paramNames)
    {
        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setFont(inputFont);

        QLabel *label = new QLabel(paramName + "：", this);
        label->setFont(labelFont);

        if(paramHints.contains(paramName))
        {
            lineEdit->setPlaceholderText(paramHints[paramName]);
        }

        inputFields[paramName] = lineEdit;  // 保存输入框的指针
        formlayout->addRow(label, lineEdit);
    }

    mainlayout->addLayout(formlayout);

    // 添加确认/取消按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainlayout->addWidget(buttonBox);

    // 自动根据内容调整窗口大小
    this->adjustSize();
    this->setSizeGripEnabled(true);  // 允许手动拉伸窗口
}

ParamInputDialog::~ParamInputDialog() {}

QMap<QString, QString> ParamInputDialog::getParamValues() const
{
    QMap<QString, QString> values;

    for(auto it = inputFields.constBegin(); it != inputFields.constEnd(); it++)
    {
        values[it.key()] = it.value()->text();
    }
    return values;
}
