#pragma once

#include <QMdiSubWindow>
#include <physis.hpp>

class FilePropertiesWindow : public QWidget {
    Q_OBJECT
public:
    explicit FilePropertiesWindow(GameData* data, QString path, QWidget *parent = nullptr);

private:
    GameData* data = nullptr;
};