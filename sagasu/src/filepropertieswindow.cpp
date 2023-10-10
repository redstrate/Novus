// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTreeWidget>

#include "filepropertieswindow.h"

FilePropertiesWindow::FilePropertiesWindow(GameData *data, QString path, QWidget *parent)
    : QWidget(parent)
    , data(data)
{
    setWindowTitle(QStringLiteral("Properties for ") + path);

    auto layout = new QFormLayout();
    setLayout(layout);

    auto pathLabel = new QLabel(path);
    layout->addRow(QStringLiteral("Path"), pathLabel);

    auto typeLabel = new QLabel(QStringLiteral("Unknown type"));
    layout->addRow(QStringLiteral("Type"), typeLabel);

    auto file = physis_gamedata_extract_file(data, path.toStdString().c_str());

    auto sizeLabel = new QLabel(QString::number(file.size));
    layout->addRow(QStringLiteral("Size (in bytes)"), sizeLabel);
}

#include "moc_filepropertieswindow.cpp"