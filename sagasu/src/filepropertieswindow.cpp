// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>
#include <QFormLayout>
#include <QLabel>
#include <QTreeWidget>

#include "filepropertieswindow.h"

FilePropertiesWindow::FilePropertiesWindow(const QString &path, physis_Buffer buffer, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(i18nc("@title:window", "Properties for ") + path);

    auto layout = new QFormLayout();
    setLayout(layout);

    auto pathLabel = new QLabel(path);
    layout->addRow(i18nc("@label", "Path"), pathLabel);

    auto typeLabel = new QLabel(i18n("Unknown type"));
    layout->addRow(i18nc("@label", "Type"), typeLabel);

    auto sizeLabel = new QLabel(QString::number(buffer.size));
    layout->addRow(i18nc("@label", "Size (in bytes)"), sizeLabel);
}

#include "moc_filepropertieswindow.cpp"