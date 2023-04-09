#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>
#include <physis.hpp>

#include "exdpart.h"

MainWindow::MainWindow(GameData* data) : data(data) {
    setWindowTitle("exdviewer");

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto listWidget = new QListWidget();

    auto names = physis_gamedata_get_all_sheet_names(data);
    for (int i = 0; i < names.name_count; i++) {
        listWidget->addItem(names.names[i]);
    }

    listWidget->setMaximumWidth(200);
    listWidget->sortItems();
    layout->addWidget(listWidget);

    auto exdPart = new EXDPart(data);
    layout->addWidget(exdPart);

    connect(listWidget, &QListWidget::itemClicked, this, [exdPart](QListWidgetItem* item) {
        auto name = item->text().toStdString();
        auto nameLowercase = item->text().toLower().toStdString();

        exdPart->loadSheet(name.c_str());
    });
}