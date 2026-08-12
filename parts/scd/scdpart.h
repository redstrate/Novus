// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <physis.hpp>

class QPushButton;
class QMediaPlayer;
class QAudioOutput;

class ScdPart : public QWidget
{
    Q_OBJECT

public:
    explicit ScdPart(QWidget *parent = nullptr);
    ~ScdPart() override;

    void load(Platform platform, physis_Buffer file);

private:
    QPushButton *m_playButton = nullptr;
    QMediaPlayer *m_mediaPlayer = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QByteArray m_audioData;
};
