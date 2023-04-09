#include <QHBoxLayout>
#include <QTreeWidget>
#include <QLabel>
#include <QFormLayout>
#include <QDebug>

#include "filepropertieswindow.h"

FilePropertiesWindow::FilePropertiesWindow(GameData *data, QString path, QWidget *parent) : QWidget(parent), data(data) {
    setWindowTitle("Properties for " + path);

    auto layout = new QFormLayout();
    setLayout(layout);

    auto pathLabel = new QLabel(path);
    layout->addRow("Path", pathLabel);

    auto typeLabel = new QLabel("Unknown type");
    layout->addRow("Type", typeLabel);

    auto file = physis_gamedata_extract_file(data, path.toStdString().c_str());

    auto sizeLabel = new QLabel(QString::number(file.size));
    layout->addRow("Size (in bytes)", sizeLabel);
}

#include "moc_filepropertieswindow.cpp"