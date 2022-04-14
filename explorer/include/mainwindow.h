#pragma once

#include <QMainWindow>
#include <QMap>
#include <QTreeWidget>

struct PathPart {
    uint32_t crcHash;
    QMap<QString, PathPart> children;
};

class GameData;

class MainWindow : public QMainWindow {
public:
    MainWindow(GameData& data);

private:
    void addPath(QString path);
    void addUnknownPath(QString knownDirectory, uint32_t crcHash);
    void traversePart(QList<QString> tokens, PathPart& part, QString pathSoFar);
    std::tuple<bool, QString> traverseUnknownPath(uint32_t crcHash, PathPart& part, QString pathSoFar);

    QMap<QString, PathPart> rootParts;

    GameData& data;

    void addPaths(QTreeWidget *pWidget);

    QTreeWidgetItem* addPartAndChildren(const QString& qString, const PathPart& part);
};