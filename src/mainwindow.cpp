#include "mainwindow.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <fmt/core.h>

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("Novus");

    QWidget* dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    QVBoxLayout* layout = new QVBoxLayout();
    dummyWidget->setLayout(layout);

    QTableWidget* listWidget = new QTableWidget();

    data.extractFile("exd/map.exh", "map.exh");
    data.extractFile("exd/map_0.exd", "map_0.exd");

    auto exh = readEXH("map.exh");
    for(auto column : exh.columnDefinitions) {
        fmt::print("type = {}, offset = {}\n", column.type, column.offset);
    }

    listWidget->setColumnCount(exh.columnDefinitions.size());
    listWidget->setRowCount(exh.header.rowCount);

    for(auto page : exh.pages) {
        fmt::print("page, row count = {}, start id = {}\n", page.rowCount, page.startId);
        auto exd = readEXD(exh, page);
        for(int i = 0; i < exd.rows.size(); i++) {
            for(int j = 0; j < exd.rows[i].data.size(); j++) {
                auto newItem = new QTableWidgetItem(exd.rows[i].data[j].data.c_str());

                listWidget->setItem(i, j, newItem);
            }
        }
    }

    layout->addWidget(listWidget);
}