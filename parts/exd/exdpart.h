#pragma once

#include <QWidget>
#include <QTabWidget>

struct GameData;

class EXDPart : public QWidget {
public:
    explicit EXDPart(GameData* data);

    void loadSheet(const QString& name);

private:
    GameData* data = nullptr;

    QTabWidget* pageTabWidget = nullptr;
};