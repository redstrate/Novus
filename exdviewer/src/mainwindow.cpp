#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"
#include "mdlparser.h"

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("exdviewer");

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto listWidget = new QListWidget();
    for(auto name : data.getAllSheetNames()) {
        listWidget->addItem(name.c_str());
    }

    listWidget->setMaximumWidth(200);

    auto* pageTabWidget = new QTabWidget();

    connect(listWidget, &QListWidget::itemClicked, this, [&data, pageTabWidget](QListWidgetItem* item) {
        pageTabWidget->clear();

        auto name = item->text().toStdString();
        auto nameLowercase = item->text().toLower().toStdString();

        auto exh = *data.readExcelSheet(name);
        for (auto column: exh.columnDefinitions) {
            fmt::print("type = {}, offset = {}\n", column.type, column.offset);
        }

        for (auto page : exh.pages) {
            QTableWidget* tableWidget = new QTableWidget();

            tableWidget->setColumnCount(exh.columnDefinitions.size());
            tableWidget->setRowCount(exh.header.rowCount);
            fmt::print("page, row count = {}, start id = {}\n", page.rowCount, page.startId);

            std::string path;
            if (exh.language[0] == Language::None) {
                path = getEXDFilename(exh, nameLowercase, "", page);
            } else {
                path = getEXDFilename(exh, nameLowercase, getLanguageCode(Language::English), page);
            }

            data.extractFile("exd/" + path, path);
            auto exd = readEXD(exh, path, page);
            for (int i = 0; i < exd.rows.size(); i++) {
                for (int j = 0; j < exd.rows[i].data.size(); j++) {
                    auto newItem = new QTableWidgetItem(exd.rows[i].data[j].data.c_str());

                    tableWidget->setItem(i, j, newItem);

                    QTableWidgetItem* headerItem = new QTableWidgetItem();
                    headerItem->setText(exd.rows[i].data[j].type.c_str());
                    tableWidget->setHorizontalHeaderItem(j, headerItem);
                }
            }

            pageTabWidget->addTab(tableWidget, QString("Page %1").arg(page.startId));
        }
    });

    layout->addWidget(listWidget);
    layout->addWidget(pageTabWidget);
}