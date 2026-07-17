// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texpart.h"

#include <KLocalizedString>
#include <QFileDialog>
#include <QVBoxLayout>
#include <physis.hpp>

#include "settings.h"

TexPart::TexPart(QWidget *parent)
    : QWidget(parent)
{
    const auto layout = new QVBoxLayout();
    setLayout(layout);

    m_label = new ImageLabel();
    layout->addWidget(m_label);

    m_saveImage = new QAction(i18n("Save PNG…"), this);
    m_saveImage->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    connect(m_saveImage, &QAction::triggered, this, [this] {
        const QString savePath = getSaveFileName(this, QStringLiteral("TexPartPNGFile"), i18nc("@title:window", "Save PNG"), {}, QStringLiteral("*.png"));
        if (!savePath.isEmpty()) {
            m_label->pixmap().save(savePath, "PNG");
        }
    });
}

bool TexPart::loadTex(const Platform platform, const physis_Buffer file) const
{
    if (file.size == 0) {
        return false;
    }

    const auto tex = physis_texture_parse(platform, file);
    if (tex.width == 0 && tex.height == 0) {
        return false;
    }

    const auto rgba = physis_texture_to_rgba(tex);

    const QImage image(rgba.rgba, tex.width, tex.height, QImage::Format_RGBA8888);
    m_label->setQPixmap(QPixmap::fromImage(image));

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
    return m_saveImage;
}

#include "moc_texpart.cpp"
