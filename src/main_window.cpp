#include "main_window.h"

#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QAbstractItemModel>

#include "components.h"
#include "render.h"

namespace gui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(QIcon(":/icon.png"));
    _centralWidget = new QWidget(this);
    setCentralWidget(_centralWidget);
    _mainLayout = new QVBoxLayout(_centralWidget);

    initProcessSection();
    initConfigSection();
    initInjectSection();
    initConnection();

    onRefreshBtnClicked();
    onInjectMethodsComboBoxIndexChanged(0);
}

void MainWindow::initProcessSection() {
    _processComboBox = new QComboBox(_centralWidget);
    _processComboBox->setEditable(_centralWidget);
    _processComboBox->setMinimumWidth(256);
    _processComboBox->setMaxVisibleItems(20);

    _processPopupDelayTimer = new QTimer(this);
    _processPopupDelayTimer->setSingleShot(true);
    _processPopupDelayTimer->setInterval(500);

    _refreshBtn = new QPushButton("刷新", _centralWidget);
    _refreshBtn->setFixedWidth(60);

    _injectMethodsComboBox = new QComboBox(_centralWidget);
    _injectMethodsComboBox->addItems({
        "远程线程注入", "消息钩子注入", "输入法注入",
        "APC注入", "EIP注入", "注册表注入",
    });

    auto* hLayout = new QHBoxLayout;
    hLayout->addWidget(new QLabel("选择进程", _centralWidget));
    hLayout->addWidget(_processComboBox, 1);
    hLayout->addWidget(_refreshBtn);
    hLayout->addWidget(_injectMethodsComboBox);

    auto* processLayout = new QVBoxLayout;
    processLayout->addLayout(hLayout);

    _mainLayout->addLayout(processLayout);
    _mainLayout->addWidget(new SplitLine(_centralWidget));
}

void MainWindow::initConfigSection() {
    _defaultDLLPathLineEdit = new QLineEdit(_centralWidget);
    _32bitsDLLPathLineEdit = new QLineEdit(_centralWidget);
    _64bitsDLLPathLineEdit = new QLineEdit(_centralWidget);
    _defaultDLLSelectBtn = new QPushButton("选择DLL", _centralWidget);
    _defaultDLLSelectBtn->setFixedWidth(88);
    _32bitsDLLSelectBtn = new QPushButton("选择32位DLL", _centralWidget);
    _64bitsDLLSelectBtn = new QPushButton("选择64位DLL", _centralWidget);
    _hookMethodNameLabel = new QLabel("HOOK方法名", _centralWidget);
    _hookTypeLabel = new QLabel("HOOK类型", _centralWidget);
    _hookMethodNameLineEdit = new QLineEdit(_centralWidget);
    _hookTypeComboBox = new QComboBox(_centralWidget);
    _hookTypeComboBox->addItems({
        "WH_JOURNALRECORD", "WH_JOURNALPLAYBACK", "WH_KEYBOARD", "WH_GETMESSAGE",
        "WH_CALLWNDPROC", "WH_CBT", "WH_SYSMSGFILTER", "WH_MOUSE"
    });

    auto* hLayout1 = new QHBoxLayout;
    hLayout1->addWidget(_defaultDLLPathLineEdit);
    hLayout1->addWidget(_defaultDLLSelectBtn);
    auto* hLayout2 = new QHBoxLayout;
    hLayout2->addWidget(_32bitsDLLPathLineEdit);
    hLayout2->addWidget(_32bitsDLLSelectBtn);
    auto* hLayout3 = new QHBoxLayout;
    hLayout3->addWidget(_64bitsDLLPathLineEdit);
    hLayout3->addWidget(_64bitsDLLSelectBtn);
    auto* hLayout4 = new QHBoxLayout;
    hLayout4->addWidget(_hookMethodNameLabel);
    hLayout4->addWidget(_hookMethodNameLineEdit);
    hLayout4->addWidget(_hookTypeLabel);
    hLayout4->addWidget(_hookTypeComboBox);

    auto* dllLayout = new QVBoxLayout;
    dllLayout->addLayout(hLayout1);
    dllLayout->addLayout(hLayout2);
    dllLayout->addLayout(hLayout3);
    dllLayout->addLayout(hLayout4);
    _mainLayout->addLayout(dllLayout);
    _mainLayout->addWidget(new SplitLine(_centralWidget));
}

void MainWindow::initInjectSection() {
    _injectBtn = new QPushButton("注入", _centralWidget);
    _logTextEdit = new QTextEdit(_centralWidget);
    _logTextEdit->setReadOnly(true);

    auto* injectLayout = new QVBoxLayout;
    injectLayout->addWidget(_injectBtn);
    injectLayout->addWidget(_logTextEdit);
    _mainLayout->addLayout(injectLayout);
}

void MainWindow::initConnection() {
    connect(_processPopupDelayTimer, &QTimer::timeout, this,
            &MainWindow::onProcessPopupDelayTimeout);
    connect(_processComboBox->lineEdit(), &QLineEdit::textEdited, this,
            &MainWindow::onProcessComboBoxTextChanged);
    connect(_processComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::onProcessComboBoxIndexChanged);
    connect(_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshBtnClicked);
    connect(_injectMethodsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::onInjectMethodsComboBoxIndexChanged);
    connect(_defaultDLLSelectBtn, &QPushButton::clicked, this,
            &MainWindow::onDefaultDLLSelectBtnClicked);
    connect(_32bitsDLLSelectBtn, &QPushButton::clicked, this,
            &MainWindow::on32bitsDLLSelectBtnClicked);
    connect(_64bitsDLLSelectBtn, &QPushButton::clicked, this,
            &MainWindow::on64bitsDLLSelectBtnClicked);
    connect(_injectBtn, &QPushButton::clicked, this, &MainWindow::onInjectBtnClicked);
}

void MainWindow::onProcessComboBoxTextChanged(const QString& text)
{
    qDebug() << "onProcessComboBoxTextChanged:" << text;

    if (text.isEmpty()) {
        _processPopupDelayTimer->stop();
        return;
    }

    _processPopupDelayTimer->start();
    _processTimerPendingText = text;
}

void MainWindow::onProcessPopupDelayTimeout()
{
    auto* model = _processComboBox->model();
    QModelIndex idx;
    for (int row = 0; row < model->rowCount(); ++row) {
        idx = model->index(row, 0);
        QString display = idx.data(Qt::DisplayRole).toString();
        int pos = display.indexOf(']');
        QString name = (pos >= 0) ? display.mid(pos + 1) : display;

        if (name.startsWith(_processTimerPendingText, Qt::CaseInsensitive)) {
            break;
        }
    }

    if (idx.isValid()) {
        qDebug() << _processTimerPendingText << idx.row();
        _processComboBox->showPopup();
        _processComboBox->view()->scrollTo(idx, QAbstractItemView::PositionAtTop);
    }
}

void MainWindow::onProcessComboBoxIndexChanged(int i)
{
    qDebug() << "onProcessComboBoxIndexChanged";

    QVariant pidVar = _processComboBox->currentData();
    if (_processComboBox->currentIndex() < 0 || !pidVar.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择进程"));
        return;
    }
    quint32 pid = pidVar.toUInt();

    const QString bits = render::IsProcess32Bits(pid) ? "32" : "64";

    log(QStringLiteral("目标进程为 %1 位，请确认DLL是否为相同位数。").arg(bits));
}

void MainWindow::onRefreshBtnClicked()
{
    qDebug() << "onRefreshBtnClicked";

    auto plist = render::GetAllProcesses();

    auto* itemModel = new QStandardItemModel(this);
    for (const auto& e : plist) {
        auto* item = new QStandardItem(QStringLiteral("[%1]%2").arg(e.pid).arg(e.name));
        item->setData(e.pid, Qt::UserRole);
        itemModel->appendRow(item);
    }

    _processComboBox->setModel(itemModel);
}

void MainWindow::onInjectMethodsComboBoxIndexChanged(int i)
{
    qDebug() << "onInjectMethodsComboBoxIndexChanged";

    switch (i) {
    case 0: // 远程线程注入
    case 3: // APC注入
    case 4: // EIP注入
    case 5: // 注册表注入
        _defaultDLLPathLineEdit->setVisible(true);
        _defaultDLLSelectBtn->setVisible(true);
        _32bitsDLLPathLineEdit->setVisible(false);
        _32bitsDLLSelectBtn->setVisible(false);
        _64bitsDLLPathLineEdit->setVisible(false);
        _64bitsDLLSelectBtn->setVisible(false);
        _hookMethodNameLabel->setVisible(false);
        _hookMethodNameLineEdit->setVisible(false);
        _hookTypeLabel->setVisible(false);
        _hookTypeComboBox->setVisible(false);
        break;
    case 1: // 消息钩子注入
        _defaultDLLPathLineEdit->setVisible(true);
        _defaultDLLSelectBtn->setVisible(true);
        _32bitsDLLPathLineEdit->setVisible(false);
        _32bitsDLLSelectBtn->setVisible(false);
        _64bitsDLLPathLineEdit->setVisible(false);
        _64bitsDLLSelectBtn->setVisible(false);
        _hookMethodNameLabel->setVisible(true);
        _hookMethodNameLineEdit->setVisible(true);
        _hookTypeLabel->setVisible(true);
        _hookTypeComboBox->setVisible(true);
        break;
    case 2: // 输入法注入
        _defaultDLLPathLineEdit->setVisible(false);
        _defaultDLLSelectBtn->setVisible(false);
        _32bitsDLLPathLineEdit->setVisible(true);
        _32bitsDLLSelectBtn->setVisible(true);
        _64bitsDLLPathLineEdit->setVisible(true);
        _64bitsDLLSelectBtn->setVisible(true);
        _hookMethodNameLabel->setVisible(false);
        _hookMethodNameLineEdit->setVisible(false);
        _hookTypeLabel->setVisible(false);
        _hookTypeComboBox->setVisible(false);
        break;
    }
}

void MainWindow::onDefaultDLLSelectBtnClicked()
{
    qDebug() << "onDefaultDLLSelectBtnClicked";

    QString dllPath = QFileDialog::getOpenFileName(
        this, tr("选择 DLL"), QString(), tr("动态库 (*.dll *.so *.dylib);;所有文件 (*)"));

    if (dllPath.isEmpty())
        return;
    _defaultDLLPathLineEdit->setText(dllPath);
}
void MainWindow::on32bitsDLLSelectBtnClicked()
{
    qDebug() << "on32bitsDLLSelectBtnClicked";

    QString dllPath = QFileDialog::getOpenFileName(
        this, tr("选择 DLL"), QString(), tr("动态库 (*.dll *.so *.dylib);;所有文件 (*)"));

    if (dllPath.isEmpty())
        return;
    _32bitsDLLPathLineEdit->setText(dllPath);
}

void MainWindow::on64bitsDLLSelectBtnClicked()
{
    qDebug() << "on64bitsDLLSelectBtnClicked";

    QString dllPath = QFileDialog::getOpenFileName(
        this, tr("选择 DLL"), QString(), tr("动态库 (*.dll *.so *.dylib);;所有文件 (*)"));

    if (dllPath.isEmpty())
        return;
    _64bitsDLLPathLineEdit->setText(dllPath);
}

void MainWindow::onInjectBtnClicked()
{
    qDebug() << "onInjectBtnClicked";

    QVariant pidVar = _processComboBox->currentData();
    if (_processComboBox->currentIndex() < 0 || !pidVar.isValid()) {
        QMessageBox::warning(this, tr("提示"), tr("请先选择进程"));
        return;
    }
    quint32 pid = pidVar.toUInt();

    int idx = _injectMethodsComboBox->currentIndex();
    switch (idx) {
    case 0: // 远程线程注入
        render::RemoteThreadInject(pid, _defaultDLLPathLineEdit->text());
    break;
    case 1: // 消息钩子注入
    break;
    case 2: // 输入法注入
    break;
    case 3: // APC注入
    break;
    case 4: // EIP注入
    break;
    case 5: // 注册表注入
    break;
    }

}

void MainWindow::log(const QString& s)
{
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    _logTextEdit->append(QStringLiteral("%1 %2").arg(ts, s));
}

}