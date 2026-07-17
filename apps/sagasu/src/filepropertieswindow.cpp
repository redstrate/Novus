// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>

#include "filepropertieswindow.h"
#include "filetypes.h"

FilePropertiesWindow::FilePropertiesWindow(const QString &path, const physis_Buffer buffer, QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(i18nc("@title:window", "Properties for ") + path);

    const auto layout = new QFormLayout();
    setLayout(layout);

    if (!path.isEmpty()) {
        const auto pathLabel = new QLabel(path);
        pathLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        layout->addRow(i18nc("@label", "Path:"), pathLabel);

        const QFileInfo info(path);
        const FileType type = FileTypes::getFileType(info.completeSuffix());

        const auto typeLabel = new QLabel(FileTypes::getFiletypeName(type));
        typeLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        layout->addRow(i18nc("@label", "Type:"), typeLabel);
    }

    const auto sizeLabel = new QLabel(QString::number(buffer.size));
    sizeLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    layout->addRow(i18nc("@label", "Size (in bytes):"), sizeLabel);
}

#include "moc_filepropertieswindow.cpp"
