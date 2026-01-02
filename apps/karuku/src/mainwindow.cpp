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
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSplitter>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <physis.hpp>

#include "exdpart.h"
#include "sheetlistwidget.h"

#include <QLineEdit>

MainWindow::MainWindow(physis_SqPackResource data)
    : m_data(data)
{
    setMinimumSize(1280, 720);

    mgr = new QNetworkAccessManager(this);

    auto dummyWidget = new QSplitter();
    dummyWidget->setChildrenCollapsible(false);
    setCentralWidget(dummyWidget);

    m_sheetListWidget = new SheetListWidget(&m_data);
    m_sheetListWidget->setMaximumWidth(200);
    dummyWidget->addWidget(m_sheetListWidget);

    m_excelResolver = new CachingExcelResolver(&m_data);

    m_exdPart = new EXDPart(&m_data, m_excelResolver);
    m_exdPart->setWhatsThis(i18nc("@info:whatsthis", "Contents of an Excel sheet. If it's made up of multiple pages, select the page from the tabs below."));
    dummyWidget->addWidget(m_exdPart);

    connect(m_sheetListWidget, &SheetListWidget::sheetSelected, this, [this](const QString &name) {
        auto path = QStringLiteral("exd/%1.exh").arg(name.toLower());
        auto pathStd = path.toStdString();

        auto file = physis_sqpack_read(&m_data, pathStd.c_str());

        m_exdPart->loadSheet(name, file);

        setWindowTitle(name);

        // update language selection
        const auto availableLanguages = m_exdPart->availableLanguages();
        if (availableLanguages.isEmpty()) {
            m_selectLanguage->setEnabled(false);
        } else {
            m_selectLanguage->setEnabled(true);

            m_languageMenu->clear();
            for (const auto &[name, language] : availableLanguages) {
                auto languageAction = new QAction(name);
                languageAction->setActionGroup(m_languageGroup);
                languageAction->setData(static_cast<int>(language));
                languageAction->setCheckable(true);
                languageAction->setChecked(language == m_exdPart->preferredLanguage());

                connect(languageAction, &QAction::triggered, this, [this, languageAction](const bool checked) {
                    if (checked) {
                        m_exdPart->setPreferredLanguage(static_cast<Language>(languageAction->data().toInt()));
                    }
                });

                m_languageMenu->addAction(languageAction);
            }
        }
    });

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("exceleditor.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
}

static bool copyDirectory(const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
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
    auto openList = new QAction(i18nc("@action:inmenu", "Import Schema…"));
    openList->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openList, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getExistingDirectory(nullptr, i18nc("@title:window", "Open Schema Directory"), QStringLiteral("~"));

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

    auto downloadList = new QAction(i18nc("@action:inmenu", "Download Schema…"));
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
        auto reply = mgr->get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [this, reply] {
            qInfo() << "Finished downloading definitions!";

            QTemporaryDir tempDir;

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

            const KArchiveDirectory *root = dynamic_cast<const KArchiveDirectory *>(archive.directory());

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

    auto goToRow = new QAction(i18nc("@action:inmenu", "To Row…"));
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

    m_selectLanguage = new QAction(i18nc("@action:inmenu", "Language"));
    m_selectLanguage->setEnabled(false);
    m_selectLanguage->setIcon(QIcon::fromTheme(QStringLiteral("languages-symbolic")));

    m_languageMenu = new QMenu();
    m_selectLanguage->setMenu(m_languageMenu);

    m_languageGroup = new QActionGroup(this);
    m_languageGroup->setExclusive(true);

    actionCollection()->addAction(QStringLiteral("select_language"), m_selectLanguage);

    auto focusSearch = new QAction(i18nc("@action:inmenu", "Search"));
    focusSearch->setIcon(QIcon::fromTheme(QStringLiteral("search-symbolic")));
    KActionCollection::setDefaultShortcut(focusSearch, QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(focusSearch, &QAction::triggered, m_sheetListWidget, &SheetListWidget::focusSearchField);
    actionCollection()->addAction(QStringLiteral("search"), focusSearch);

    auto resetSorting = new QAction(i18nc("@action:inmenu", "Reset Sorting"));
    resetSorting->setIcon(QIcon::fromTheme(QStringLiteral("edit-reset-symbolic")));
    connect(resetSorting, &QAction::triggered, m_exdPart, &EXDPart::resetSorting);
    actionCollection()->addAction(QStringLiteral("reset_sorting"), resetSorting);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

#include "moc_mainwindow.cpp"
