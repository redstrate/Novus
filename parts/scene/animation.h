// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QHash>
#include <QList>

#include "physis.hpp"

struct physis_Tmb;
class ObjectScene;

/// An F-Curve for a single attribute (e.g. position.)
class FCurve
{
public:
    FCurve() = default;
    explicit FCurve(const QList<TmfcRow> &points);

    float atTime(float time) const;

private:
    TmfcRow findStartPoint(float time) const;
    TmfcRow findEndPoint(float time) const;

    QList<TmfcRow> m_points;
};

/// Collection of attributes that make up a complete animation for a transform.
class Track
{
public:
    explicit Track(int32_t tmfcId, const QHash<physis_Attribute, FCurve> &curves);

    void applyTransformation(float time, Transformation &existingTransformation) const;
    int32_t tmfcId() const;

private:
    QHash<physis_Attribute, FCurve> m_fCurves;
    int32_t m_tmfcId;
};

/// Represents a complete animation on an SGB, including all of its nested timelines.
class Animation
{
public:
    explicit Animation(const ObjectScene &scene);

    void update(ObjectScene &scene, float time);

    /// In seconds.
    float duration() const;

private:
    void processScene(const ObjectScene &scene);
    void processTimeline(const physis_Tmb &timeline);

    QList<Track *> m_tracks;
    QHash<uint32_t, Track *> m_actorTracks;
    uint16_t m_duration = 0.0f;
};
