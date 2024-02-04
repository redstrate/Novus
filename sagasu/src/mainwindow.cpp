// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KLocalizedString>
#include <KZip>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSplitter>
#include <QTemporaryDir>

#include "cmppart.h"
#include "exdpart.h"
#include "exlpart.h"
#include "filepropertieswindow.h"
#include "hexpart.h"
#include "mdlpart.h"
#include "shpkpart.h"
#include "sklbpart.h"
#include "texpart.h"

MainWindow::MainWindow(const QString &gamePath, GameData *data)
    : NovusMainWindow()
    , data(data)
    , fileCache(*data)
{
    setupMenubar();
    setMinimumSize(1280, 720);

    m_mgr = new QNetworkAccessManager(this);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    m_tree = new FileTreeWindow(m_database, gamePath, data);
    connect(m_tree, &FileTreeWindow::extractFile, this, [this, data](const QString &path) {
        const QFileInfo info(path);

        const QString savePath =
            QFileDialog::getSaveFileName(this, i18nc("@title:window", "Save File"), info.fileName(), QStringLiteral("*.%1").arg(info.completeSuffix()));
        if (!savePath.isEmpty()) {
            qInfo() << "Saving to" << savePath;

            std::string savePathStd = path.toStdString();

            auto fileData = physis_gamedata_extract_file(data, savePathStd.c_str());
            QFile file(savePath);
            file.open(QIODevice::WriteOnly);
            file.write(reinterpret_cast<const char *>(fileData.data), fileData.size);
        }
    });
    connect(m_tree, &FileTreeWindow::pathSelected, this, [this](const QString &path) {
        refreshParts(path);
    });
    m_tree->setMaximumWidth(300);
    dummyWidget->addWidget(m_tree);

    partHolder = new QTabWidget();
    partHolder->setDocumentMode(true); // hide borders
    partHolder->setMinimumHeight(720);
    dummyWidget->addWidget(partHolder);

    refreshParts({});
}

void MainWindow::refreshParts(const QString &path)
{
    partHolder->clear();

    std::string pathStd = path.toStdString();
    if (path.isEmpty() || !physis_gamedata_exists(data, pathStd.c_str())) {
        return;
    }

    auto file = physis_gamedata_extract_file(data, path.toStdString().c_str());

    QFileInfo info(path);
    if (info.completeSuffix() == QStringLiteral("exl")) {
        auto exlWidget = new EXLPart(data);
        exlWidget->load(file);
        partHolder->addTab(exlWidget, i18nc("@title:tab", "Excel List"));
    } else if (info.completeSuffix() == QStringLiteral("exh")) {
        auto exdWidget = new EXDPart(data);
        exdWidget->loadSheet(info.baseName(), file);
        partHolder->addTab(exdWidget, i18nc("@title:tab", "Excel Sheet"));
    } else if (info.completeSuffix() == QStringLiteral("exd")) {
        auto exdWidget = new QLabel(i18n("Note: Excel data files cannot be previewed standalone, select the EXH file instead."));
        partHolder->addTab(exdWidget, i18nc("@title:tab", "Note"));
    } else if (info.completeSuffix() == QStringLiteral("mdl")) {
        auto mdlWidget = new MDLPart(data, fileCache);
        mdlWidget->addModel(physis_mdl_parse(file), false, glm::vec3(), QStringLiteral("mdl"), {}, 0);
        partHolder->addTab(mdlWidget, i18nc("@title:tab", "Model"));
    } else if (info.completeSuffix() == QStringLiteral("tex") || info.completeSuffix() == QStringLiteral("atex")) {
        auto texWidget = new TexPart(data);
        texWidget->load(file);
        partHolder->addTab(texWidget, i18nc("@title:tab", "Texture"));
    } else if (info.completeSuffix() == QStringLiteral("shpk")) {
        auto shpkWidget = new SHPKPart(data);
        shpkWidget->load(file);
        partHolder->addTab(shpkWidget, i18nc("@title:tab", "Shader Package"));
    } else if (info.completeSuffix() == QStringLiteral("cmp")) {
        auto cmpWidget = new CmpPart(data);
        cmpWidget->load(file);
        partHolder->addTab(cmpWidget, i18nc("@title:tab", "Chara Make Params"));
    } else if (info.completeSuffix() == QStringLiteral("sklb")) {
        auto sklbWidget = new SklbPart();
        sklbWidget->load(physis_parse_skeleton(file));
        partHolder->addTab(sklbWidget, i18nc("@title:tab", "Skeleton"));
    }

    auto hexWidget = new HexPart();
    hexWidget->loadFile(file);
    partHolder->addTab(hexWidget, i18nc("@title:tab", "Raw Hex"));

    auto propertiesWidget = new FilePropertiesWindow(path, file);
    partHolder->addTab(propertiesWidget, i18nc("@title:tab", "Properties"));

    partHolder->tabBar()->setExpanding(true);
}

void MainWindow::setupFileMenu(QMenu *menu)
{
    auto openList = menu->addAction(i18nc("@action:inmenu", "Import Path List…"));
    openList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openList, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Open Path List"), QStringLiteral("~"));

        QMessageBox::warning(this,
                             i18nc("@title:window", "Import Warning"),
                             i18n("Depending on the size of the import, this process usually takes a few minutes. The program may freeze. Please "
                                  "keep it open until the operation is finished."),
                             QMessageBox::Ok,
                             QMessageBox::Ok);

        QFile file(fileName);
        file.open(QIODevice::ReadOnly);

        m_database.importFileList(file.readAll());
        m_tree->refreshModel();

        QMessageBox::information(this, i18nc("@title:window", "Import Complete"), i18n("Successfully imported path list!"), QMessageBox::Ok, QMessageBox::Ok);
    });

    auto downloadList = menu->addAction(i18nc("@action:inmenu", "Download Path List…"));
    downloadList->setIcon(QIcon::fromTheme(QStringLiteral("download-symbolic")));
    connect(downloadList, &QAction::triggered, [this] {
        const int ret = QMessageBox::information(this,
                                                 i18nc("@title:window", "Download Confirmation"),
                                                 i18n("This will download the path list from <a "
                                                      "href=\"https://rl2.perchbird.dev/\">ResLogger</a>.this process usually takes a few minutes. The program "
                                                      "may freeze. Please keep it open until the operation is finished.<br><br>Continue?"),
                                                 QMessageBox::Ok | QMessageBox::Cancel,
                                                 QMessageBox::Ok);

        if (ret != QMessageBox::Ok) {
            return;
        }

        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("rl2.perchbird.dev"));
        url.setPath(QStringLiteral("/download/export/CurrentPathListWithHashes.zip"));

        // TODO: Use Qcoro?
        auto reply = m_mgr->get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            qInfo() << "Finished downloading path list!";

            QTemporaryDir tempDir;

            QFile file(tempDir.filePath(QStringLiteral("CurrentPathListWithHashes.zip")));
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.close();

            KZip archive(file.fileName());
            if (!archive.open(QIODevice::ReadOnly)) {
                // TODO: these should show as message boxes
                qFatal() << "Failed to open path list zip!" << archive.errorString();
                return;
            }

            const KArchiveFile *root = dynamic_cast<const KArchiveFile *>(archive.directory()->entry(QStringLiteral("CurrentPathListWithHashes.csv")));
            m_database.importFileList(root->data());
            m_tree->refreshModel();

            archive.close();

            QMessageBox::information(this,
                                     i18nc("@title:window", "Import Complete"),
                                     i18n("Successfully downloaded and imported path list!"),
                                     QMessageBox::Ok,
                                     QMessageBox::Ok);
        });
    });
}
