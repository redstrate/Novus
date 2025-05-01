// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <physis.hpp>

#include "mdlpart.h"

MainWindow::MainWindow(GameData *data)
    : KXmlGuiWindow()
    , data(data)
    , cache(FileCache{*data})
{
    setMinimumSize(640, 480);

    auto dummyWidget = new QWidget();
    setCentralWidget(dummyWidget);

    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    dummyWidget->setLayout(layout);

    part = new MDLPart(data, cache);
    part->minimumCameraDistance = 0.05f;

    const int raceCode = physis_get_race_code(Race::Hyur, Tribe::Midlander, Gender::Male);

    QString skelName = QStringLiteral("chara/human/c%1/skeleton/base/b0001/skl_c%1b0001.sklb").arg(raceCode, 4, 10, QLatin1Char{'0'});
    std::string skelNameStd = skelName.toStdString();
    part->setSkeleton(physis_parse_skeleton(physis_gamedata_extract_file(data, skelNameStd.c_str())));

    layout->addWidget(part);

    auto tabWidget = new QTabWidget();
    tabWidget->setMaximumHeight(150);

    auto renderWidget = new QWidget();
    auto renderLayout = new QVBoxLayout();
    renderWidget->setLayout(renderLayout);

    auto wireframeCheckbox = new QCheckBox(i18n("Wireframe"));
    connect(wireframeCheckbox, &QCheckBox::clicked, this, [this](bool checked) {
        part->setWireframe(checked);
    });
    renderLayout->addWidget(wireframeCheckbox);

    auto modelWidget = new QWidget();
    m_detailsLayout = new QFormLayout();
    modelWidget->setLayout(m_detailsLayout);

    tabWidget->addTab(renderWidget, i18nc("@title:tab", "Render"));
    tabWidget->addTab(modelWidget, i18nc("@title:tab", "Model"));

    tabWidget->setDocumentMode(true); // hide borders
    tabWidget->tabBar()->setExpanding(true);

    layout->addWidget(tabWidget);

    setupActions();
    setupGUI(Keys | Save | Create, QStringLiteral("mdlviewer.rc"));

    // We don't provide help (yet)
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::HelpContents)));
    // This isn't KDE software
    actionCollection()->removeAction(actionCollection()->action(KStandardAction::name(KStandardAction::AboutKDE)));
}

void MainWindow::setupActions()
{
    auto openMDLFile = new QAction(i18nc("@action:inmenu MDL is an abbreviation for a file type", "Open MDL…"));
    openMDLFile->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(openMDLFile, &QAction::triggered, [this] {
        auto fileName = QFileDialog::getOpenFileName(nullptr, i18nc("@title:window", "Open MDL File"), QStringLiteral("~"), i18n("FFXIV Model File (*.mdl)"));
        if (!fileName.isEmpty()) {
            auto buffer = physis_read_file(fileName.toStdString().c_str());
            if (buffer.data == nullptr) {
                return;
            }

            auto mdl = physis_mdl_parse(buffer);
            if (mdl.p_ptr == nullptr) {
                return;
            }

            part->clear();

            setWindowFilePath(fileName);

            part->addModel(mdl, false, glm::vec3(), QStringLiteral("mdl"), {}, 0);

            // Clear layout
            QLayoutItem *child = nullptr;
            while ((child = m_detailsLayout->takeAt(0)) != nullptr) {
                child->widget()->setParent(nullptr);
                child->widget()->deleteLater();
            }

            m_detailsLayout->addRow(i18n("LOD #:"), new QLabel(QString::number(mdl.num_lod)));

            uint32_t triangleCount = 0;
            for (uint32_t i = 0; i < mdl.lods[0].num_parts; i++) {
                triangleCount += mdl.lods[0].parts[i].num_indices / 3;
            }

            m_detailsLayout->addRow(i18n("Triangle #:"), new QLabel(QString::number(triangleCount)));
        }
    });
    actionCollection()->addAction(QStringLiteral("open_mdl"), openMDLFile);

    KStandardAction::quit(qApp, &QCoreApplication::quit, actionCollection());
}

#include "moc_mainwindow.cpp"
