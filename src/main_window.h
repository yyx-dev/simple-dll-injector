#pragma once

#include <QLabel.h>
#include <QMainWindow>
#include <QWidget>
#include <QBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <qobject.h>

namespace gui {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void log(const QString& s);

private:
    void onProcessPopupDelayTimeout();
    void onProcessComboBoxTextChanged(const QString& s);
    void onProcessComboBoxIndexChanged(int);
    void onRefreshBtnClicked();
    void onInjectMethodsComboBoxIndexChanged(int);
    void onDefaultDLLSelectBtnClicked();
    void on32bitsDLLSelectBtnClicked();
    void on64bitsDLLSelectBtnClicked();
    void onInjectBtnClicked();

    void initProcessSection();
    void initConfigSection();
    void initInjectSection();
    void initConnection();

    QWidget* _centralWidget;
    QVBoxLayout* _mainLayout;

    QComboBox* _processComboBox;
    QPushButton* _refreshBtn;
    QComboBox* _injectMethodsComboBox;
    QLineEdit* _defaultDLLPathLineEdit;
    QPushButton* _defaultDLLSelectBtn;
    QLineEdit* _32bitsDLLPathLineEdit;
    QPushButton* _32bitsDLLSelectBtn;
    QLineEdit* _64bitsDLLPathLineEdit;
    QPushButton* _64bitsDLLSelectBtn;
    QLabel* _hookMethodNameLabel;
    QLineEdit* _hookMethodNameLineEdit;
    QLabel* _hookTypeLabel;
    QComboBox* _hookTypeComboBox;
    QPushButton* _injectBtn;
    QTextEdit* _logTextEdit;

    QTimer* _processPopupDelayTimer;
    QString _processTimerPendingText;
};

}