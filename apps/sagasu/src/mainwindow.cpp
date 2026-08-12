// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KLocalizedString>
#include <KZip>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSplitter>
#include <QTemporaryDir>
#include <magic_enum.hpp>

#include "cmppart.h"
#include "dicpart.h"
#include "excelresolver.h"
#include "exdpart.h"
#include "exlpart.h"
#include "filetypes.h"
#include "hexpart.h"
#include "luabpart.h"
#include "mdlimport.h"
#include "mdlpart.h"
#include "mtrlpart.h"
#include "openinwidget.h"
#include "pathedit.h"
#include "scdpart.h"
#include "scenepart.h"
#include "scenestate.h"
#include "settings.h"
#include "shcdpart.h"
#include "shpkpart.h"
#include "sklbpart.h"
#include "texpart.h"
#include "texteditor.h"
#include "tmbpart.h"

MainWindow::MainWindow(const QString &gamePath, const physis_SqPackResource data)
    : m_cache(data)
{
    m_mgr = new QNetworkAccessManager(this);

    m_excelResolver = new AbstractExcelResolver();

    const auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    m_tree = new FileTreeWindow(m_database, gamePath, m_cache);
    connect(m_tree, &FileTreeWindow::extractFile, this, [this](const QString &path, const QString &indexPath, const Hash hash) {
        const QFileInfo info(path);

        const QString savePath = getSaveFileName(this,
                                                 QStringLiteral("DataExplorerExtractFile"),
                                                 i18nc("@title:window", "Extract File"),
                                                 info.fileName(),
                                                 QStringLiteral("*.%1").arg(info.completeSuffix()));
        if (!savePath.isEmpty()) {
            auto fileData = physis_sqpack_read_from_hash(&m_cache.resource(), indexPath.toStdString().c_str(), hash);
            // HACK: Read from path as a fallback, which somehow makes more PS3 files load. I don't know why.
            if (fileData.size == 0) {
                fileData = m_cache.read(path);
            }
            if (fileData.size == 0) {
                return;
            }

            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reinterpret_cast<const char *>(fileData.data), fileData.size);
            } else {
                qFatal() << "Failed to write to" << savePath;
            }
        }
    });
    connect(m_tree, &FileTreeWindow::pathSelected, this, [this](const QString &indexPath, const Hash hash, const QString &path) {
        m_currentPath = path;
        refreshParts(indexPath, hash, path);
    });
    dummyWidget->addWidget(m_tree);

    const auto partLayout = new QVBoxLayout();
    partLayout->setContentsMargins(0, 0, 0, 0);
    partLayout->setSpacing(0);

    const auto partLayoutHolder = new QWidget();
    partLayoutHolder->setLayout(partLayout);

    m_urlEdit = new QLineEdit();
    m_urlEdit->setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
    m_urlEdit->setReadOnly(true); // TODO: make this editable in the future
    partLayout->addWidget(m_urlEdit);

    m_partHolder = new QTabWidget();
    m_partHolder->setDocumentMode(true); // hide borders
    partLayout->addWidget(m_partHolder);

    dummyWidget->addWidget(partLayoutHolder);

    // Part holder should stretch, not the list widget
    dummyWidget->setStretchFactor(0, 0);
    dummyWidget->setStretchFactor(1, 1);

    setupActions();
    setupGUI(QSize(1280, 720), Keys | Save | Create | ToolBar, QStringLiteral("dataexplorer.rc"));
    updateNavigationActions();

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
    // We don't use this well enough
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::WhatsThis)));

    // Open paths in our own instance (see PathEdit)
    PathEdit::handler()->setEmitSignal(true);
    connect(PathEdit::handler(), &OpenPathHandler::pathOpened, this, [this](const QString &path) {
        Q_UNUSED(m_tree->selectPath(path));
    });

    const auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);
}

bool MainWindow::selectPath(const QString &path) const
{
    return m_tree->selectPath(path);
}

QString MainWindow::getArguments() const
{
    return m_currentPath;
}

void MainWindow::back()
{
    if (m_navigationPointer <= 0) {
        return;
    }

    m_navigatingStack = true;
    m_navigationPointer--;
    Q_UNUSED(selectPath(m_navigationStack[m_navigationPointer]));
    m_navigatingStack = false;
    updateNavigationActions();
}

void MainWindow::forward()
{
    if (m_navigationPointer + 1 >= m_navigationStack.size()) {
        return;
    }

    m_navigatingStack = true;
    m_navigationPointer++;
    Q_UNUSED(selectPath(m_navigationStack[m_navigationPointer]));
    m_navigatingStack = false;
    updateNavigationActions();
}

void MainWindow::refreshParts(const QString &indexPath, Hash hash, const QString &path)
{
    const std::string pathStd = path.toStdString();
    if (indexPath.isEmpty()) {
        return;
    }

    // TODO: should this also apply to loose files?
    if (!m_navigatingStack) {
        // Erase what's ahead on the stack if we went back some.
        if (m_navigationPointer + 1 != m_navigationStack.size()) {
            m_navigationStack.resize(m_navigationPointer + 1);
        }

        // Add a new entry to the stack (if applicable)
        if (m_navigationStack.constLast() != path) {
            m_navigationStack.push_back(path);
            m_navigationPointer = m_navigationStack.size() - 1;
        }
    }

    updateNavigationActions();

    const QFileInfo info(path);

    auto file = physis_sqpack_read_from_hash(&m_cache.resource(), indexPath.toStdString().c_str(), hash);
    // HACK: Read from path as a fallback, which somehow makes more PS3 files load. I don't know why.
    if (file.size == 0) {
        file = m_cache.read(path);
    }

    loadPart(file, info);
}

void MainWindow::loadPart(const physis_Buffer file, const QFileInfo &info)
{
    for (auto i = 0; i < m_partHolder->count(); i++) {
        delete m_partHolder->widget(i);
    }
    m_partHolder->clear();

    m_urlEdit->setText(info.filePath());
    setPlainCaption(info.fileName());

    FileType type = FileTypes::getFileType(info.completeSuffix());
    // Try to guess from magic as a fallback
    if (type == FileType::Unknown) {
        type = FileTypes::guessFileType(file);
    }

    m_fileActionsMenu->clear();
    m_fileActions->setVisible(false);

    const auto addTab = [this, &type](QWidget *widget) {
        const QString typeName = FileTypes::getFiletypeName(type);
        const QString iconName = FileTypes::getFiletypeIcon(type);

        m_partHolder->addTab(widget, QIcon::fromTheme(iconName), typeName);
        m_fileActions->setText(typeName);
        m_fileActions->setVisible(true);
    };

    // Texture files are weird as they don't have an explicit magic, so we basically resort to brute-forcing them for now.
    if (type == FileType::Unknown || type == FileType::Texture) {
        const auto texWidget = new TexPart();
        if (texWidget->loadTex(m_cache.platform(), file)) {
            type = FileType::Texture;
            addTab(texWidget);

            m_fileActionsMenu->addAction(texWidget->saveImageAction());
        }
    }

    // Ditto for material files
    if (type == FileType::Unknown || type == FileType::Material) {
        auto mtrl = physis_material_parse(m_cache.platform(), file);
        if (mtrl.shpk_name) {
            const auto mtrlWidget = new MtrlPart(m_cache);
            mtrlWidget->load(mtrl);

            type = FileType::Material;
            addTab(mtrlWidget);
        }
    }

    // ... and model files!
    if (type == FileType::Unknown || type == FileType::Model) {
        auto mdl = physis_mdl_parse(m_cache.platform(), file);
        if (mdl.p_ptr) {
            Transformation transformation{};
            transformation.scale[0] = 1;
            transformation.scale[1] = 1;
            transformation.scale[2] = 1;

            auto mdlWidget = new MDLPart(m_cache, true, this);
            std::vector<std::pair<std::string, physis_Material>> materials(mdl.num_material_names);
            for (uint32_t i = 0; i < mdl.num_material_names; i++) {
                materials[i] = {mdl.material_names[i], physis_material_parse(m_cache.platform(), m_cache.read(QString::fromUtf8(mdl.material_names[i])))};
            }
            mdlWidget->addThreePointLighting();
            mdlWidget->addModel(mdl, false, transformation, QStringLiteral("mdl"), materials);

            type = FileType::Model;
            addTab(mdlWidget);

            const auto importAction = m_fileActionsMenu->addAction(QStringLiteral("Import glTF"));
            connect(importAction, &QAction::triggered, this, [this, mdlWidget](bool) {
                const QString importFileName = getOpenFileName(this,
                                                               QStringLiteral("MDLImportGLBFile"),
                                                               i18nc("@title:window", "Import Model"),
                                                               QDir::homePath(),
                                                               i18n("glTF Binary File (*.glb)"));
                const QString exportFileName = getSaveFileName(this,
                                                               QStringLiteral("MDLSaveGLBMDLFile"),
                                                               i18nc("@title:window", "Import Model"),
                                                               QDir::homePath(),
                                                               i18n("Model file (*.mdl)"));
                if (!importFileName.isEmpty() && !exportFileName.isEmpty()) {
                    auto mdl = mdlWidget->getModel(0).model;
                    importModel(mdl, importFileName);
                    const auto buffer = physis_mdl_write(m_cache.platform(), &mdl);

                    QFile file(exportFileName);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(reinterpret_cast<char *>(buffer.data), buffer.size);
                    } else {
                        qFatal() << "Failed to write to" << exportFileName;
                    }
                }
            });

            const auto exportAction = m_fileActionsMenu->addAction(QStringLiteral("Export glTF"));
            connect(exportAction, &QAction::triggered, this, [this, mdlWidget](bool) {
                const QString fileName = getSaveFileName(this,
                                                         QStringLiteral("MDLSaveAsGLBFile"),
                                                         i18nc("@title:window", "Export Model"),
                                                         QDir::homePath(),
                                                         i18n("glTF Binary File (*.glb)"));
                if (!fileName.isEmpty()) {
                    mdlWidget->exportModel(fileName);
                }
            });
        }
    }

    switch (type) {
    case FileType::ExcelList: {
        const auto exlWidget = new EXLPart();
        exlWidget->load(m_cache.platform(), file);

        addTab(exlWidget);
    } break;
    case FileType::ExcelHeader: {
        const auto exdWidget = new EXDPart(m_cache, m_excelResolver);
        exdWidget->setReadOnly(true); // Editing here isn't supported yet
        exdWidget->loadSheet(info.filePath().remove(QStringLiteral(".exh")).remove(QStringLiteral("exd/")), file);
        addTab(exdWidget);

        m_fileActionsMenu->addAction(exdWidget->selectLanguageAction());
        m_fileActionsMenu->addAction(exdWidget->saveCsvAction());
    } break;
    case FileType::ExcelData: {
        const auto exdLayout = new QVBoxLayout();

        const auto helpLabel = new QLabel(i18n("Excel data files cannot be viewed standalone, select the EXH file instead."));
        helpLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        exdLayout->addWidget(helpLabel, 0, Qt::AlignHCenter | Qt::AlignBottom);

        const auto goToButton = new QPushButton(i18n("Go to EXH"));
        goToButton->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-symbolic")));
        connect(goToButton, &QPushButton::clicked, this, [this, info] {
            // TODO: this doesn't work for some names!
            const auto baseName = info.filePath().split(QStringLiteral("_")).constFirst();
            const auto newName = QStringLiteral("%1.exh").arg(baseName);
            Q_UNUSED(m_tree->selectPath(newName));
        });
        exdLayout->addWidget(goToButton, 0, Qt::AlignHCenter | Qt::AlignTop);

        const auto exdWidget = new QWidget();
        exdWidget->setLayout(exdLayout);

        addTab(exdWidget);
    } break;
    case FileType::ShaderPackage: {
        const auto shpkWidget = new SHPKPart();
        shpkWidget->load(m_cache.platform(), file);
        addTab(shpkWidget);
    } break;
    case FileType::CharaMakeParams: {
        const auto cmpWidget = new CmpPart();
        cmpWidget->load(m_cache.platform(), file);
        addTab(cmpWidget);
    } break;
    case FileType::Skeleton: {
        const auto sklbWidget = new SklbPart();
        sklbWidget->load(physis_skeleton_parse(m_cache.platform(), file));
        addTab(sklbWidget);
    } break;
    case FileType::Dictionary: {
        const auto dicWidget = new DicPart();
        dicWidget->load(m_cache.platform(), file);
        addTab(dicWidget);
    } break;
    case FileType::LuaBytecode: {
        const auto luabWidget = new LuabPart();
        luabWidget->load(file);
        addTab(luabWidget);
    } break;
    case FileType::HardwareCursor: {
        const auto texWidget = new TexPart();
        texWidget->loadHwc(m_cache.platform(), file);
        addTab(texWidget);

        m_fileActionsMenu->addAction(texWidget->saveImageAction());
    } break;
    case FileType::SharedGroup: {
        const auto scenePart = new ScenePart(m_cache);
        scenePart->loadSgb(file);
        Q_EMIT scenePart->sceneState()->mapLoaded();
        addTab(scenePart);
    } break;
    case FileType::TimelineMotion: {
        const auto tmbPart = new TmbPart();
        tmbPart->load(m_cache.platform(), file);
        addTab(tmbPart);
    } break;
    case FileType::Shader: {
        const auto shcdPart = new SHCDPart();
        shcdPart->load(m_cache.platform(), file);
        addTab(shcdPart);
    } break;
    case FileType::LayerVariableBinary: {
        const auto scenePart = new ScenePart(m_cache);
        scenePart->loadLvb(file);
        Q_EMIT scenePart->sceneState()->mapLoaded();
        addTab(scenePart);
    } break;
    case FileType::Png: {
        const auto texWidget = new TexPart();
        texWidget->loadPng(file);
        addTab(texWidget);

        m_fileActionsMenu->addAction(texWidget->saveImageAction());
    } break;
    case FileType::AnimatedVisualEffect: {
        const auto avfx = physis_avfx_parse(m_cache.platform(), file);

        Transformation transformation{};
        transformation.scale[0] = 1;
        transformation.scale[1] = 1;
        transformation.scale[2] = 1;

        const auto mdlWidget = new MDLPart(m_cache, false, this);
        mdlWidget->addThreePointLighting();
        mdlWidget->addVfx(avfx, transformation, QStringLiteral("vfx"));
        addTab(mdlWidget);
    } break;
    case FileType::SoundCompressedData: {
        const auto scdWidget = new ScdPart();
        scdWidget->load(m_cache.platform(), file);
        addTab(scdWidget);
    } break;
    default:
        break;
    }

    if (FileTypes::isDebugInformationApplicable(type)) {
        auto debugInformation = FileTypes::printDebugInformation(type, m_cache.platform(), file);
        constexpr int maxDebugInformationLength = 1000000; // NOTE: This is in place so files like bg.shpk (which contains HLSL bytecode) doesn't OOM
        if (debugInformation.length() > maxDebugInformationLength) {
            debugInformation.resize(maxDebugInformationLength);
            debugInformation.append(i18n("<truncated>"));
        }

        const auto debugInformationText = new TextEditor();
        debugInformationText->setHighlightingMode(QStringLiteral("rust"));
        debugInformationText->setText(debugInformation);

        m_partHolder->addTab(debugInformationText, QIcon::fromTheme(QStringLiteral("format-text-code-symbolic")), i18nc("@title:tab", "Parsed"));
    }

    // Disable file actions if there are no actions to take
    m_fileActions->setEnabled(!m_fileActionsMenu->isEmpty());

    if (file.size > 0) {
        const auto hexWidget = new HexPart();
        hexWidget->loadFile(file);
        m_partHolder->addTab(hexWidget, QIcon::fromTheme(QStringLiteral("text-x-hex")), i18nc("@title:tab", "Bytes"));
    }

    m_partHolder->tabBar()->setExpanding(true);
}

void MainWindow::setupActions()
{
    const auto openList = new QAction(i18nc("@action:inmenu", "Import Path List…"));
    openList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openList, &QAction::triggered, [this] {
        const auto fileName = getOpenFileName(this, QStringLiteral("DataExplorerPathListFile"), i18nc("@title:window", "Open Path List"));

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this,
                                 i18nc("@title:window", "Import Warning"),
                                 i18n("Depending on the size of the import, this process usually takes a few minutes. The program may freeze. Please "
                                      "keep it open until the operation is finished."),
                                 QMessageBox::Ok,
                                 QMessageBox::Ok);

            m_database.importFileList(file.readAll());
            m_tree->refreshModel();

            QMessageBox::information(this,
                                     i18nc("@title:window", "Import Complete"),
                                     i18n("Successfully imported path list!"),
                                     QMessageBox::Ok,
                                     QMessageBox::Ok);
        } else {
            qWarning() << "Failed to import list from" << fileName;
        }
    });
    actionCollection()->addAction(QStringLiteral("import_list"), openList);

    const auto downloadList = new QAction(i18nc("@action:inmenu", "Download Path List…"));
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
        url.setPath(QStringLiteral("/download/CurrentPathListWithHashes.zip"));

        // TODO: Use Qcoro?
        auto reply = m_mgr->get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            qInfo() << "Finished downloading path list!";

            const QTemporaryDir tempDir;

            QFile file(tempDir.filePath(QStringLiteral("CurrentPathListWithHashes.zip")));
            if (!file.open(QIODevice::WriteOnly)) {
                qFatal() << "Failed to open path list!";
                return;
            }
            file.write(reply->readAll());
            file.close();

            KZip archive(file.fileName());
            if (!archive.open(QIODevice::ReadOnly)) {
                // TODO: these should show as message boxes
                qFatal() << "Failed to open path list zip!" << archive.errorString();
                return;
            }

            const auto root = dynamic_cast<const KArchiveFile *>(archive.directory()->entry(QStringLiteral("CurrentPathListWithHashes.csv")));
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

    const auto manualAdd = new QAction(i18nc("@action:inmenu", "Manually Add Path…"));
    manualAdd->setIcon(QIcon::fromTheme(QStringLiteral("document-new-symbolic")));
    connect(manualAdd, &QAction::triggered, [this] {
        bool ok = false;
        const QString path = QInputDialog::getText(this, i18n("Manually Add Path…"), i18n("Path:"), QLineEdit::Normal, QString{}, &ok);
        if (ok && !path.isEmpty()) {
            // TODO: move into an addPath or something in HashDatabase
            QString filename;
            QString foldername;
            if (path.contains(QStringLiteral("/"))) {
                const int lastSlash = path.lastIndexOf(QStringLiteral("/"));
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

    const auto showUnknown = new QAction(i18nc("@action:inmenu", "Show Unknown Files"));
    KActionCollection::setDefaultShortcut(showUnknown, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_U));
    showUnknown->setCheckable(true);
    showUnknown->setIcon(QIcon::fromTheme(QStringLiteral("view-hidden-symbolic")));
    connect(showUnknown, &QAction::triggered, [this](const bool checked) {
        m_tree->setShowUnknown(checked);
    });
    actionCollection()->addAction(QStringLiteral("show_unknown"), showUnknown);

    const auto focusSearch = new QAction(i18nc("@action:inmenu", "Search"));
    focusSearch->setIcon(QIcon::fromTheme(QStringLiteral("search-symbolic")));
    KActionCollection::setDefaultShortcut(focusSearch,
                                          QKeySequence(Qt::ALT | Qt::Key_Q)); // NOTE: This isn't CTRL+F because that conflicts with KTextEditor right now
    connect(focusSearch, &QAction::triggered, m_tree, &FileTreeWindow::focusSearchField);
    actionCollection()->addAction(QStringLiteral("search"), focusSearch);

    const auto goToPath = new QAction(i18nc("@action:inmenu", "Go to Path…"));
    goToPath->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-symbolic")));
    KActionCollection::setDefaultShortcut(goToPath, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_G));
    connect(goToPath, &QAction::triggered, [this] {
        bool ok = false;
        const QString path = QInputDialog::getText(this, i18n("Go to Path…"), i18n("Path:"), QLineEdit::Normal, QString{}, &ok);
        if (ok && !path.isEmpty()) {
            Q_UNUSED(m_tree->selectPath(path));
        }
    });
    actionCollection()->addAction(QStringLiteral("go_to_path"), goToPath);

    m_fileActionsMenu = new QMenu();

    m_fileActions = new QAction();
    m_fileActions->setMenu(m_fileActionsMenu);
    actionCollection()->addAction(QStringLiteral("file_actions"), m_fileActions);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
    m_backAction = KStandardAction::back(this, &MainWindow::back, actionCollection());
    m_forwardAction = KStandardAction::forward(this, &MainWindow::forward, actionCollection());

    const auto openLooseAction = new QAction(QIcon::fromTheme(QStringLiteral("document-open-symbolic")), i18nc("@action:inmenu", "Open Loose File…"));
    connect(openLooseAction, &QAction::triggered, this, [this] {
        const auto fileName = getOpenFileName(this, QStringLiteral("DataExplorerLooseFile"), i18n("Open Loose File"), {}, i18n("All Files (*)"));
        if (!fileName.isEmpty()) {
            loadPart(physis_read_file(fileName.toStdString().c_str()), QFileInfo(fileName));
        }
    });
    actionCollection()->addAction(QStringLiteral("open_loose"), openLooseAction);

    // Window color scheme menu
    const auto manager = KColorSchemeManager::instance();
    const auto selectionMenu = KColorSchemeMenu::createMenu(manager, this);
    const auto windowColorSchemeMenu = new QAction(this);
    windowColorSchemeMenu->setMenu(selectionMenu->menu());
    windowColorSchemeMenu->menu()->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")));
    windowColorSchemeMenu->menu()->setTitle(i18n("&Window Color Scheme"));
    actionCollection()->addAction(QStringLiteral("window_color_scheme"), windowColorSchemeMenu);
}

void MainWindow::updateNavigationActions() const
{
    m_backAction->setEnabled(m_navigationPointer > 0);
    m_forwardAction->setEnabled(m_navigationPointer + 1 < m_navigationStack.size());
}

#include "moc_mainwindow.cpp"
