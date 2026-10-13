// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texpart.h"

#include <KLocalizedString>
#include <QActionGroup>
#include <QFileDialog>
#include <QMenu>
#include <QVBoxLayout>
#include <physis.hpp>

#include "settings.h"

TexPart::TexPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins({0, 0, 0, 0});
    setLayout(layout);

    m_label = new ImageLabel();
    layout->addWidget(m_label);

    m_saveImageAction = new QAction(i18n("Save PNG…"), this);
    m_saveImageAction->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    connect(m_saveImageAction, &QAction::triggered, this, [this] {
        const QString savePath = getSaveFileName(this, QStringLiteral("TexPartPNGFile"), i18nc("@title:window", "Save PNG"), {}, QStringLiteral("*.png"));
        if (!savePath.isEmpty()) {
            m_label->pixmap().save(savePath, "PNG");
        }
    });

    m_layerAction = new QAction(i18n("Layer"), this);

    m_layerMenu = new QMenu(this);
    m_layerAction->setMenu(m_layerMenu);

    m_layerGroup = new QActionGroup(this);
    m_layerGroup->setExclusive(true);

    m_levelAction = new QAction(i18n("Level"), this);

    m_levelMenu = new QMenu(this);
    m_levelAction->setMenu(m_levelMenu);

    m_levelGroup = new QActionGroup(this);
    m_levelGroup->setExclusive(true);
}

bool TexPart::loadTex(const Platform platform, const physis_Buffer file)
{
    if (file.size == 0) {
        return false;
    }

    const auto tex = physis_texture_parse(platform, file);
    if (tex.width == 0 && tex.height == 0) {
        return false;
    }

    m_layerMenu->clear();
    m_levelMenu->clear();
    m_levels.clear();
    m_levels.resize(tex.mip_levels);

    for (int j = 0; j < tex.mip_levels; j++) {
        m_levels[j].resize(tex.layers);

        const auto levelAction = new QAction(i18n("Level %1", j), this);
        levelAction->setActionGroup(m_levelGroup);
        levelAction->setCheckable(true);
        levelAction->setChecked(m_level == j);

        connect(levelAction, &QAction::triggered, this, [this, j](const bool checked) {
            if (checked) {
                m_level = j;
                m_label->setQPixmap(QPixmap::fromImage(m_levels[m_level][m_layer]));
            }
        });

        m_levelMenu->addAction(levelAction);

        for (int i = 0; i < tex.layers; i++) {
            const auto rgba = physis_texture_decode(tex, j, i);

            const QImage image(rgba.rgba, rgba.width, rgba.height, QImage::Format_RGBA8888);
            m_levels[j][i] = image;
        }
    }

    for (int i = 0; i < tex.layers; i++) {
        const auto layerAction = new QAction(i18n("Layer %1", i), this);
        layerAction->setActionGroup(m_layerGroup);
        layerAction->setCheckable(true);
        layerAction->setChecked(m_layer == i);

        connect(layerAction, &QAction::triggered, this, [this, i](const bool checked) {
            if (checked) {
                m_layer = i;
                m_label->setQPixmap(QPixmap::fromImage(m_levels[m_level][m_layer]));
            }
        });

        m_layerMenu->addAction(layerAction);
    }

    m_levelAction->setEnabled(tex.mip_levels > 1);
    m_layerAction->setEnabled(tex.layers > 1);

    m_label->setQPixmap(QPixmap::fromImage(m_levels[m_level][m_layer]));

    physis_tex_free(&tex);

    return true;
}

void TexPart::loadHwc(const Platform platform, const physis_Buffer file) const
{
    const auto tex = physis_hwc_parse(platform, file);

    const QImage image(tex.rgba, Hwc_WIDTH, Hwc_HEIGHT, QImage::Format_RGBA8888);
    m_label->setQPixmap(QPixmap::fromImage(image));
}

void TexPart::loadPng(const physis_Buffer file) const
{
    const QImage image = QImage::fromData(file.data, file.size, "PNG");
    m_label->setQPixmap(QPixmap::fromImage(image));
}

QAction *TexPart::saveImageAction() const
{
    return m_saveImageAction;
}

QAction *TexPart::layerAction() const
{
    return m_layerAction;
}

QAction *TexPart::levelAction() const
{
    return m_levelAction;
}

#include "moc_texpart.cpp"
