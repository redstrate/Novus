// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QFormLayout>
#include <QLabel>
#include <QTreeWidget>

#include "filepropertieswindow.h"

FilePropertiesWindow::FilePropertiesWindow(const QString &path, physis_Buffer buffer, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Properties for ") + path);

    auto layout = new QFormLayout();
    setLayout(layout);

    auto pathLabel = new QLabel(path);
    layout->addRow(QStringLiteral("Path"), pathLabel);

    auto typeLabel = new QLabel(QStringLiteral("Unknown type"));
    layout->addRow(QStringLiteral("Type"), typeLabel);

    auto sizeLabel = new QLabel(QString::number(buffer.size));
    layout->addRow(QStringLiteral("Size (in bytes)"), sizeLabel);
}

#include "moc_filepropertieswindow.cpp"