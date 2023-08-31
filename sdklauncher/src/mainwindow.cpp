#include "mainwindow.h"

#include <QListWidget>
#include <QProcess>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QDebug>

static QMap<QString, QString> applications = {
    {"Armoury", "armoury"},
    {"EXD Viewer", "exdviewer"},
    {"Explorer", "explorer"},
    {"Model Viewer", "mdlviewer"}
};

MainWindow::MainWindow() {
    setWindowTitle("Novus SDK");

    auto appList = new QListWidget();

    auto applicationHeader = new QListWidgetItem();
    applicationHeader->setText("Applications");
    applicationHeader->setFlags(Qt::NoItemFlags);

    appList->addItem(applicationHeader);

    for(const auto& name : applications.keys()) {
        appList->addItem(name);
    }

    connect(appList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        QString exec = "./" + applications[item->text()];

        qDebug() << "Launching" << exec;

        QProcess::startDetached(exec, QStringList());
    });

    auto appListLayout = new QVBoxLayout();
    appListLayout->addWidget(appList);

    auto centralFrame = new QFrame();
    centralFrame->setLayout(appListLayout);

    auto formLayout = new QFormLayout();

    auto gameCombo = new QComboBox();
    formLayout->addRow("Current Game", gameCombo);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    gameCombo->addItem("Placeholder");

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(centralFrame);
    mainLayout->addLayout(formLayout);
    auto centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);

    setCentralWidget(centralWidget);
}
