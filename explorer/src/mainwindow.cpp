#include "mainwindow.h"

#include <QHBoxLayout>
#include <QTableWidget>
#include <QTreeWidget>
#include <QDebug>

#include "gamedata.h"
#include "exhparser.h"
#include "exdparser.h"

MainWindow::MainWindow(GameData& data) : data(data) {
    setWindowTitle("explorer");

    addPath("exd/root.exl");

    for(auto sheetName : data.getAllSheetNames()) {
        auto nameLowercase = QString(sheetName.c_str()).toLower().toStdString();

        addPath("exd/" + QString(nameLowercase.c_str()) + ".exh");

        auto exh = *data.readExcelSheet(sheetName);
        for(auto page : exh.pages) {
            for(auto language : exh.language) {
                std::string path;
                if (language == Language::None) {
                    path = getEXDFilename(exh, nameLowercase, "", page);
                } else {
                    path = getEXDFilename(exh, nameLowercase, getLanguageCode(language), page);
                }

                addPath(("exd/" + path).c_str());
            }
        }
    }

    addPath("common/font/AXIS_12.fdt");

    auto commonIndex = data.getIndexListing("common");
    for(auto entry : commonIndex.entries) {
        addUnknownPath("common", entry.hash);
    }

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QHBoxLayout();
    dummyWidget->setLayout(layout);

    auto treeWidget = new QTreeWidget();
    treeWidget->setHeaderLabel("Name");

    addPaths(treeWidget);

    layout->addWidget(treeWidget);
}

void MainWindow::addPath(QString path) {
    auto tokens = path.split('/');
    auto nextToken = tokens[0];
    tokens.pop_front();

    traversePart(tokens, rootParts[nextToken], nextToken);
}

void MainWindow::traversePart(QList<QString> tokens, PathPart& part, QString pathSoFar) {
    if(tokens.empty())
        return;

    auto nextToken = tokens[0];
    tokens.pop_front();

    pathSoFar = pathSoFar + "/" + nextToken;
    part.children[nextToken].crcHash = data.calculateHash(pathSoFar.toStdString());

    traversePart(tokens, part.children[nextToken], pathSoFar);
}

void MainWindow::addPaths(QTreeWidget *pWidget) {
    for(const auto& name : rootParts.keys()) {
        auto item = addPartAndChildren(name, rootParts.value(name));
        pWidget->addTopLevelItem(item);
    }
}

QTreeWidgetItem* MainWindow::addPartAndChildren(const QString& qString, const PathPart& part) {
    auto item = new QTreeWidgetItem();
    item->setText(0, qString);

    for(const auto& name : part.children.keys()) {
        auto childItem = addPartAndChildren(name, part.children.value(name));
        item->addChild(childItem);
    }

    return item;
}

void MainWindow::addUnknownPath(QString knownDirectory, uint32_t crcHash) {
    auto [found, path] = traverseUnknownPath(crcHash, rootParts[knownDirectory], knownDirectory);
    if(found)
        addPath(path);
    else
        addPath(knownDirectory + "/Unknown File Hash " + QString::number(crcHash));
}

std::tuple<bool, QString> MainWindow::traverseUnknownPath(uint32_t crcHash, PathPart &part, QString pathSoFar) {
    if(part.crcHash == crcHash)
        return {true, pathSoFar};

    bool found = false;
    QString childPath = pathSoFar;
    for(auto path : part.children.keys()) {
        if(path.contains("Unknown"))
            continue;

        auto [childFound, newPath] = traverseUnknownPath(crcHash, part.children[path], pathSoFar + "/" + path);
        found |= childFound;
        if(childFound)
            childPath = newPath;
    }

    return {found, childPath};
}
