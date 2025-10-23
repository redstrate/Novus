// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
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
#include <QStatusBar>
#include <QTemporaryDir>

#include "cmppart.h"
#include "dicpart.h"
#include "exdpart.h"
#include "exlpart.h"
#include "filepropertieswindow.h"
#include "filetypes.h"
#include "hexpart.h"
#include "luabpart.h"
#include "mdlpart.h"
#include "mtrlpart.h"
#include "shpkpart.h"
#include "sklbpart.h"
#include "texpart.h"

#include <QInputDialog>

MainWindow::MainWindow(const QString &gamePath, SqPackResource *data)
    : KXmlGuiWindow()
    , data(data)
    , fileCache(*data)
{
    // setupMenubar();
    setMinimumSize(1280, 720);

    m_mgr = new QNetworkAccessManager(this);

    m_offsetLabel = new QLabel(i18n("Offset: Unknown"));
    statusBar()->addWidget(m_offsetLabel);
    auto separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::VLine);
    statusBar()->addWidget(separatorLine);
    m_hashLabel = new QLabel(i18n("Hash: Unknown"));
    statusBar()->addWidget(m_hashLabel);
    auto separatorLine2 = new QFrame();
    separatorLine2->setFrameShape(QFrame::VLine);
    statusBar()->addWidget(separatorLine2);
    m_fileTypeLabel = new QLabel(i18n("File Type: Unknown"));
    statusBar()->addWidget(m_fileTypeLabel);

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

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("dataexplorer.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
}

void MainWindow::refreshParts(const QString &path)
{
    partHolder->clear();

    std::string pathStd = path.toStdString();
    if (path.isEmpty() || !physis_gamedata_exists(data, pathStd.c_str())) {
        return;
    }

    QFileInfo info(path);
    std::string filenameStd = info.fileName().toStdString();

    // FIXME: add back once hashes are f igured out
    // auto crcHash = physis_calculate_hash(filenameStd.c_str());
    // m_hashLabel->setText(i18n("Hash: 0x%1", QString::number(crcHash, 16).toUpper().rightJustified(8, QLatin1Char('0'))));

    // FIXME: this is terrible, we should not be recalculating this. it isn't a huge deal with the file + index caching, but still
    auto datOffset = physis_gamedata_find_offset(data, pathStd.c_str());
    m_offsetLabel->setText(i18n("Offset: 0x%1", QString::number(datOffset, 16).toUpper().rightJustified(8, QLatin1Char('0'))));

    auto file = physis_gamedata_extract_file(data, path.toStdString().c_str());

    const FileType type = FileTypes::getFileType(info.completeSuffix());

    m_fileTypeLabel->setText(i18n("File Type: %1", FileTypes::getFiletypeName(type)));

    switch (type) {
    case FileType::ExcelList: {
        auto exlWidget = new EXLPart(data);
        exlWidget->load(file);
        partHolder->addTab(exlWidget, i18nc("@title:tab", "Excel List"));
    } break;
    case FileType::ExcelHeader: {
        auto exdWidget = new EXDPart(data);
        exdWidget->loadSheet(info.baseName(), file);
        partHolder->addTab(exdWidget, i18nc("@title:tab", "Excel Sheet"));
    } break;
    case FileType::ExcelData: {
        auto exdWidget = new QLabel(i18n("Note: Excel data files cannot be previewed standalone, select the EXH file instead."));
        partHolder->addTab(exdWidget, i18nc("@title:tab", "Note"));
    } break;
    case FileType::Model: {
        auto mdlWidget = new MDLPart(data, fileCache);
        mdlWidget->addModel(physis_mdl_parse(file), false, glm::vec3(), QStringLiteral("mdl"), {}, 0);
        partHolder->addTab(mdlWidget, i18nc("@title:tab", "Model"));
    } break;
    case FileType::Texture: {
        auto texWidget = new TexPart(data);
        texWidget->load(file);
        partHolder->addTab(texWidget, i18nc("@title:tab", "Texture"));
    } break;
    case FileType::ShaderPackage: {
        auto shpkWidget = new SHPKPart();
        shpkWidget->load(file);
        partHolder->addTab(shpkWidget, i18nc("@title:tab", "Shader Package"));
    } break;
    case FileType::CharaMakeParams: {
        auto cmpWidget = new CmpPart(data);
        cmpWidget->load(file);
        partHolder->addTab(cmpWidget, i18nc("@title:tab", "Chara Make Params"));
    } break;
    case FileType::Skeleton: {
        auto sklbWidget = new SklbPart();
        sklbWidget->load(physis_parse_skeleton(file));
        partHolder->addTab(sklbWidget, i18nc("@title:tab", "Skeleton"));
    } break;
    case FileType::Dictionary: {
        auto dicWidget = new DicPart();
        dicWidget->load(file);
        partHolder->addTab(dicWidget, i18nc("@title:tab", "Dictionary"));
    } break;
    case FileType::Material: {
        auto mtrlWidget = new MtrlPart(data);
        mtrlWidget->load(physis_material_parse(file));
        partHolder->addTab(mtrlWidget, i18nc("@title:tab", "Material"));
    } break;
    case FileType::LuaBytecode: {
        auto luabWidget = new LuabPart();
        luabWidget->load(file);
        partHolder->addTab(luabWidget, i18nc("@title:tab", "Lua"));
    } break;
    default:
        break;
    }

    auto hexWidget = new HexPart();
    hexWidget->loadFile(file);
    partHolder->addTab(hexWidget, i18nc("@title:tab", "Raw Hex"));

    auto propertiesWidget = new FilePropertiesWindow(path, file);
    partHolder->addTab(propertiesWidget, i18nc("@title:tab", "Properties"));

    partHolder->tabBar()->setExpanding(true);
}

void MainWindow::setupActions()
{
    auto openList = new QAction(i18nc("@action:inmenu", "Import Path List…"));
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
    actionCollection()->addAction(QStringLiteral("import_list"), openList);

    auto downloadList = new QAction(i18nc("@action:inmenu", "Download Path List…"));
    downloadList->setIcon(QIcon::fromTheme(QStringLiteral("download-symbolic")));
    connect(downloadList, &QAction::triggered, [this] {
        const int ret =
            QMessageBox::information(this,
                                     i18nc("@title:window", "Download Confirmation"),
                                     i18n("Novus will download the path list from <a "
                                          "href=\"https://rl2.perchbird.dev/\">ResLogger</a>, this process usually takes a few minutes. The program "
                                          "may freeze. Please keep it open until the operation is finished.<br><br>Would you still like to continue?"),
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
    actionCollection()->addAction(QStringLiteral("download_list"), downloadList);

    auto manualAdd = new QAction(i18nc("@action:inmenu", "Manually Add Path…"));
    connect(manualAdd, &QAction::triggered, [this] {
        bool ok = false;
        const QString path = QInputDialog::getText(this, i18n("Manually Add Path…"), i18n("Path:"), QLineEdit::Normal, QString{}, &ok);
        if (ok && !path.isEmpty()) {
            // TODO: move into an addPath or something in HashDatabase
            QString filename;
            QString foldername;
            if (path.contains(QStringLiteral("/"))) {
                int lastSlash = path.lastIndexOf(QStringLiteral("/"));
                filename = path.sliced(lastSlash + 1, path.length() - lastSlash - 1);
                foldername = path.left(lastSlash);
            } else {
                filename = path;
            }

            if (!foldername.isEmpty()) {
                m_database.addFolder(foldername);
            }
            m_database.addFile(filename);
            m_tree->refreshModel();
        }
    });
    actionCollection()->addAction(QStringLiteral("manual_add"), manualAdd);

    auto showUnknown = new QAction(i18nc("@action:inmenu", "Show Unknown Files/Folders"));
    showUnknown->setCheckable(true);
    connect(showUnknown, &QAction::triggered, [this](const bool checked) {
        m_tree->setShowUnknown(checked);
    });
    actionCollection()->addAction(QStringLiteral("show_unknown"), showUnknown);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}
