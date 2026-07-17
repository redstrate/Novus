// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "excelresolver.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KZip>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSplitter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <physis.hpp>

#include "exdpart.h"
#include "openinwidget.h"
#include "sheetlistwidget.h"

#include <QLineEdit>

MainWindow::MainWindow(const physis_SqPackResource data)
    : m_cache(data)
{
    setMinimumSize(1280, 720);

    m_mgr = new QNetworkAccessManager(this);

    const auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    m_sheetListWidget = new SheetListWidget(&data);
    m_sheetListWidget->setMaximumWidth(200);
    dummyWidget->addWidget(m_sheetListWidget);

    m_excelResolver = std::make_unique<CachingExcelResolver>(m_cache);

    m_exdPart = new EXDPart(m_cache, m_excelResolver.get());
    m_exdPart->setWhatsThis(i18nc("@info:whatsthis", "Contents of an Excel sheet. If it's made up of multiple pages, select the page from the tabs below."));
    connect(m_exdPart, &EXDPart::requestJump, this, &MainWindow::jumpToSheetAndRow);
    connect(m_exdPart, &EXDPart::modified, this, &MainWindow::updateDocumentActions);
    dummyWidget->addWidget(m_exdPart);

    connect(m_sheetListWidget, &SheetListWidget::sheetSelected, this, &MainWindow::jumpToSheet);

    setupActions();
    setupGUI(ToolBar | Keys | Save | Create, QStringLiteral("exceleditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));

    const auto openInWidget = new OpenInWidget(this);
    menuBar()->setCornerWidget(openInWidget);

    updateDocumentActions();
}

MainWindow::~MainWindow() = default;

QString MainWindow::getArguments() const
{
    if (m_exdPart) {
        if (const auto query = m_exdPart->selectedRow(); !query.isEmpty()) {
            return QStringLiteral("%1#%2").arg(m_exdPart->name()).arg(query);
        }
        return m_exdPart->name();
    }
    return {};
}

void MainWindow::jumpToSheet(const QString &name)
{
    if (name.isEmpty()) {
        m_exdPart->clear();
        setPlainCaption({});
        return;
    }

    const auto path = QStringLiteral("exd/%1.exh").arg(name.toLower());

    const auto file = m_cache.read(path);
    m_exdPart->loadSheet(name, file);
    m_sheetListWidget->goToSheet(name);

    setPlainCaption(name);
}

void MainWindow::jumpToSheetAndRow(const QString &name, const QString &rowQuery)
{
    jumpToSheet(name);
    m_exdPart->goToRow(rowQuery);
}

static bool copyDirectory(const QString &srcFilePath, const QString &tgtFilePath)
{
    const QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        const QDir targetDir(tgtFilePath);
        const QDir sourceDir(srcFilePath);

        const QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        for (const QString &fileName : fileNames) {
            const QString newSrcFilePath = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath = tgtFilePath + QLatin1Char('/') + fileName;

            if (!QFile::copy(newSrcFilePath, newTgtFilePath)) {
                return false;
            }
        }

        return true;
    }
    return false;
}

void MainWindow::setupActions()
{
    const auto openList = new QAction(i18nc("@action:inmenu", "Import Schema…"), this);
    openList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openList, &QAction::triggered, [this] {
        const auto fileName = QFileDialog::getExistingDirectory(nullptr, i18nc("@title:window", "Open Schema Directory"), QStringLiteral("~"));

        const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir definitionsDir = dataDir.absoluteFilePath(QStringLiteral("schema"));

        // delete old directory
        if (definitionsDir.exists()) {
            QDir().rmdir(definitionsDir.absolutePath());
        }

        QDir().mkpath(definitionsDir.absolutePath());

        copyDirectory(fileName, definitionsDir.absolutePath());

        QMessageBox::information(this, i18nc("@title:window", "Schema"), i18n("Successfully imported schema!"));
    });
    actionCollection()->addAction(QStringLiteral("import_list"), openList);

    const auto downloadList = new QAction(i18nc("@action:inmenu", "Download Schema…"), this);
    downloadList->setIcon(QIcon::fromTheme(QStringLiteral("download-symbolic")));
    connect(downloadList, &QAction::triggered, [this] {
        const int ret = QMessageBox::information(
            this,
            i18nc("@title:window", "Download Confirmation"),
            i18n("Novus will download the schema from the <a "
                 "href=\"https://github.com/xivdev/EXDSchema\">EXDSchema repository on GitHub</a>.<br><br>Would you still like to continue?"),
            QMessageBox::Ok | QMessageBox::Cancel,
            QMessageBox::Ok);

        if (ret != QMessageBox::Ok) {
            return;
        }

        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(QStringLiteral("github.com"));
        url.setPath(QStringLiteral("/xivdev/EXDSchema/releases/latest/download/latest.zip"));

        // TODO: Use Qcoro?
        auto reply = m_mgr->get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            qInfo() << "Finished downloading definitions!";

            const QTemporaryDir tempDir;

            QFile file(tempDir.filePath(QStringLiteral("latest.zip")));
            if (!file.open(QIODevice::WriteOnly)) {
                qFatal() << "Failed to write schema zip!";
                return;
            }
            file.write(reply->readAll());
            file.close();

            KZip archive(tempDir.filePath(QStringLiteral("latest.zip")));
            if (!archive.open(QIODevice::ReadOnly)) {
                // TODO: these should show as message boxes
                qFatal() << "Failed to open schema zip!";
                return;
            }

            const auto root = archive.directory();

            const QDir dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            const QDir definitionsDir = dataDir.absoluteFilePath(QStringLiteral("schema"));

            // delete old directory
            if (definitionsDir.exists()) {
                QDir().rmdir(definitionsDir.absolutePath());
            }

            QDir().mkpath(definitionsDir.absolutePath());

            root->copyTo(definitionsDir.absolutePath(), true);

            archive.close();

            QMessageBox::information(this, i18nc("@title:window", "Schema"), i18n("Successfully downloaded and imported schema!"));
        });
    });
    actionCollection()->addAction(QStringLiteral("download_list"), downloadList);

    const auto goToRow = new QAction(i18nc("@action:inmenu", "To Row…"), this);
    goToRow->setIcon(QIcon::fromTheme(QStringLiteral("go-jump-symbolic")));
    KActionCollection::setDefaultShortcut(goToRow, QKeySequence(Qt::Modifier::CTRL | Qt::Key::Key_G));
    connect(goToRow, &QAction::triggered, [this] {
        bool ok = false;
        const QString text = QInputDialog::getText(this, i18n("Go To…"), i18n("Row or Subrow ID:"), QLineEdit::Normal, QString{}, &ok);
        if (ok && !text.isEmpty()) {
            m_exdPart->goToRow(text);
        }
    });
    actionCollection()->addAction(QStringLiteral("goto_row"), goToRow);

    actionCollection()->addAction(QStringLiteral("select_language"), m_exdPart->selectLanguageAction());
    actionCollection()->addAction(QStringLiteral("save_csv"), m_exdPart->saveCsvAction());

    const auto focusSearch = new QAction(i18nc("@action:inmenu", "Search"), this);
    focusSearch->setIcon(QIcon::fromTheme(QStringLiteral("search-symbolic")));
    KActionCollection::setDefaultShortcut(focusSearch, QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(focusSearch, &QAction::triggered, m_sheetListWidget, &SheetListWidget::focusSearchField);
    actionCollection()->addAction(QStringLiteral("search"), focusSearch);

    const auto focusFilter = new QAction(i18nc("@action:inmenu", "Filter"), this);
    focusFilter->setIcon(QIcon::fromTheme(QStringLiteral("view-filter-symbolic")));
    KActionCollection::setDefaultShortcut(focusFilter, QKeySequence(Qt::CTRL | Qt::Key_I));
    connect(focusFilter, &QAction::triggered, m_exdPart, &EXDPart::focusFilterField);
    actionCollection()->addAction(QStringLiteral("filter"), focusFilter);

    m_saveAction = KStandardAction::save(
        qApp,
        [this] {
            m_exdPart->save();
        },
        actionCollection());

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
    KStandardAction::close(
        qApp,
        [this] {
            jumpToSheet({});
        },
        actionCollection());
}

void MainWindow::updateDocumentActions() const
{
    m_saveAction->setEnabled(m_exdPart->isModified());
}

#include "moc_mainwindow.cpp"
