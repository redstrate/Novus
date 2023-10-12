// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenuBar>

#include "filepropertieswindow.h"
#include "filetreewindow.h"

MainWindow::MainWindow(GameData *data)
    : NovusMainWindow()
    , data(data)
{
    setupMenubar();

    mdiArea = new QMdiArea();
    setCentralWidget(mdiArea);

    auto tree = new FileTreeWindow(data);
    connect(tree, &FileTreeWindow::openFileProperties, this, [=](QString path) {
        auto window = mdiArea->addSubWindow(new FilePropertiesWindow(data, path));
        window->show();
    });
    connect(tree, &FileTreeWindow::extractFile, this, [this, data](QString path) {
        const QFileInfo info(path);

        const QString savePath = QFileDialog::getSaveFileName(this, tr("Save File"), info.fileName(), QStringLiteral("*.%1").arg(info.completeSuffix()));
        if (!savePath.isEmpty()) {
            qInfo() << "Saving to" << savePath;

            std::string savePathStd = path.toStdString();

            auto fileData = physis_gamedata_extract_file(data, savePathStd.c_str());
            QFile file(savePath);
            file.open(QIODevice::WriteOnly);
            file.write(reinterpret_cast<const char *>(fileData.data), fileData.size);
        }
    });

    mdiArea->addSubWindow(tree);
}
