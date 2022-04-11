#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>
#include <QListWidget>

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("Novus");

    QWidget* dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    QHBoxLayout* layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    QListWidget* listWidget = new QListWidget();
    for(auto name : data.getAllSheetNames()) {
        listWidget->addItem(name.c_str());
    }

    QTableWidget* tableWidget = new QTableWidget();

    connect(listWidget, &QListWidget::itemClicked, this, [&data, tableWidget](QListWidgetItem* item) {
        auto name = item->text().toStdString();
        auto nameLowercase = item->text().toLower().toStdString();

        auto exh = *data.readExcelSheet(name);
        for(auto column : exh.columnDefinitions) {
            fmt::print("type = {}, offset = {}\n", column.type, column.offset);
        }

        tableWidget->setColumnCount(exh.columnDefinitions.size());
        tableWidget->setRowCount(exh.header.rowCount);

        for(auto page : exh.pages) {
            if(page.startId == 0) {
                fmt::print("page, row count = {}, start id = {}\n", page.rowCount, page.startId);

                std::string path;
                if(exh.language[0] == Language::None) {
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
                    }
                }
            }
        }
    });

    layout->addWidget(listWidget);
    layout->addWidget(tableWidget);
}