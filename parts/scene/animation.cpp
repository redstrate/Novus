// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "animation.h"

#include "scenestate.h"

#include <QDebug>

FCurve::FCurve(const QList<TmfcRow> &points)
    : m_points(points)
{
}

float FCurve::atTime(const float time) const
{
    const auto startPoint = findStartPoint(time);
    const auto endPoint = findEndPoint(time);

    const auto startTime = startPoint.time; // Example: 0
    const auto endTime = endPoint.time; // Example: 400
    const auto duration = endTime - startTime; // Example: 400

    // If duration is zero, that means we only have one point and can take a shortcut here:
    if (duration == 0.0) {
        return startPoint.value;
    }

    const auto adjustedMomentStart = time - startTime; // Example: 20 - 400
    const auto adjustedMoment = adjustedMomentStart / duration; // ???

    qInfo() << "Between points:" << startTime << adjustedMoment << endTime;

    return std::lerp(startPoint.value, endPoint.value, adjustedMoment);
}

TmfcRow FCurve::findStartPoint(const float time) const
{
    Q_ASSERT(!m_points.isEmpty());

    for (const auto &point : m_points) {
        if (time >= point.time) {
            return point;
        }
    }

    // fallback
    return m_points.first();
}

TmfcRow FCurve::findEndPoint(const float time) const
{
    Q_ASSERT(!m_points.isEmpty());

    for (const auto &point : m_points) {
        if (time <= point.time) {
            return point;
        }
    }

    // fallback
    return m_points.first();
}

Track::Track(const int32_t tmfcId, const QHash<physis_Attribute, FCurve> &curves)
    : m_fCurves(curves)
    , m_tmfcId(tmfcId)
{
}

Transformation Track::transformationAtTime(const float time) const
{
    return Transformation{
        .translation =
            {
                m_fCurves[physis_Attribute::PositionX].atTime(time),
                m_fCurves[physis_Attribute::PositionY].atTime(time),
                m_fCurves[physis_Attribute::PositionZ].atTime(time),
            },
        .rotation =
            {
                qDegreesToRadians(m_fCurves[physis_Attribute::RotationX].atTime(time)),
                qDegreesToRadians(m_fCurves[physis_Attribute::RotationY].atTime(time)),
                qDegreesToRadians(m_fCurves[physis_Attribute::RotationZ].atTime(time)),
            },
        .scale = {1, 1, 1}, // TODO: support scale
    };
}

int32_t Track::tmfcId() const
{
    return m_tmfcId;
}

Animation::Animation(const ObjectScene &scene)
{
    processScene(scene);

    // TODO: I suspect this won't work out because nested scenes may share IDs, but I'm unsure. Then we would need an animation nesting system similar to
    // scenes.
    for (const auto &nestedScene : scene.nestedScenes.values()) {
        processScene(nestedScene);
    }

    qInfo() << "Processed" << m_tracks.size() << "timelines with" << m_actorTracks.size() << "actors!";
}

void Animation::update(ObjectScene &scene, const float time)
{
    processUpdateScene(scene, time);
}

float Animation::duration() const
{
    return m_duration;
}

void Animation::processScene(const ObjectScene &scene)
{
    const auto findNode =
        [](const physis_ScnTimeline &timeline, const physis_TimelineNodeData::Tag tag, const uint16_t id) -> std::optional<physis_TimelineNodeData> {
        for (uint32_t j = 0; j < timeline.tmb.node_count; j++) {
            const auto &node = timeline.tmb.nodes[j].data;
            if (node.tag == tag) {
                bool matching = false;
                switch (node.tag) {
                case physis_TimelineNodeData::Tag::Tmac:
                    matching = node.tmac._0.time == id; // TODO: bad naming lol
                    break;
                case physis_TimelineNodeData::Tag::Tmtr:
                    matching = node.tmtr._0.id == id;
                    break;
                case physis_TimelineNodeData::Tag::C013:
                    matching = node.c013._0.id == id;
                    break;
                default:
                    break;
                }

                if (matching) {
                    return node;
                }
            }
        }

        return std::nullopt;
    };

    const auto findTrack = [this](const int32_t tmfcId) -> std::optional<Track *> {
        for (const auto &track : m_tracks) {
            if (track->tmfcId() == tmfcId) {
                return track;
            }
        }

        return std::nullopt;
    };

    for (const auto &timeline : scene.embeddedTimelines) {
        // Load timeline data
        processTimeline(timeline.tmb);

        // Build actor associations
        for (uint32_t i = 0; i < timeline.instance_count; i++) {
            const auto &instance = timeline.instances[i];

            const auto tmacNode = findNode(timeline, physis_TimelineNodeData::Tag::Tmac, instance.tmac_time);
            if (tmacNode) {
                const auto tmac = tmacNode->tmac._0;
                for (uint32_t j = 0; j < tmac.tmtr_id_count; j++) {
                    const auto tmtrNode = findNode(timeline, physis_TimelineNodeData::Tag::Tmtr, tmac.tmtr_ids[j]);
                    if (tmtrNode) {
                        const auto tmtr = tmtrNode->tmtr._0;
                        for (uint32_t z = 0; z < tmtr.animation_id_count; z++) {
                            // TODO: support other animation types eventually
                            const auto c013Node = findNode(timeline, physis_TimelineNodeData::Tag::C013, tmtr.animation_ids[z]);
                            if (c013Node) {
                                const auto c013 = c013Node->c013._0;

                                if (auto track = findTrack(c013.tmfc_id)) {
                                    m_actorTracks[instance.instance_id] = track.value();
                                } else {
                                    qWarning() << "Failed to find track associated with TMFC" << c013.tmfc_id;
                                }
                            } else {
                                qWarning() << "Couldn't find C013 for" << tmtr.animation_ids[z];
                            }
                        }
                    } else {
                        qWarning() << "Couldn't find TMTR for" << tmac.tmtr_ids[j];
                    }
                }
            } else {
                qWarning() << "Couldn't find TMAC for" << instance.tmac_time << "time and instance" << instance.instance_id;
            }
        }
    }
}

void Animation::processTimeline(const physis_Tmb &timeline)
{
    for (uint32_t i = 0; i < timeline.node_count; i++) {
        const auto &node = timeline.nodes[i];
        switch (node.data.tag) {
        case physis_TimelineNodeData::Tag::Tmdh:
            m_duration = std::max(m_duration, node.data.tmdh._0.duration);
            break;
        case physis_TimelineNodeData::Tag::Tmfc: {
            const auto &tmfc = node.data.tmfc._0;

            QHash<physis_Attribute, FCurve> curves;
            for (uint32_t j = 0; j < tmfc.data_count; j++) {
                const auto &data = tmfc.data[j];
                auto rows = std::span{data.rows, data.row_count};
                curves[data.attribute] = FCurve(QList(rows.begin(), rows.end()));
            }

            m_tracks.push_back(new Track(tmfc.id, curves));
        }
        default:
            break;
        }
    }
}

void printTransformation(const Transformation &transformation)
{
    qInfo() << "- pos[x] =" << transformation.translation[0];
    qInfo() << "- pos[y] =" << transformation.translation[1];
    qInfo() << "- pos[z] =" << transformation.translation[2];

    qInfo() << "- rot[x] =" << transformation.rotation[0];
    qInfo() << "- rot[y] =" << transformation.rotation[1];
    qInfo() << "- rot[z] =" << transformation.rotation[2];

    qInfo() << "- scl[x] =" << transformation.scale[0];
    qInfo() << "- scl[y] =" << transformation.scale[1];
    qInfo() << "- scl[z] =" << transformation.scale[2];
}

void Animation::processUpdateScene(ObjectScene &scene, const float time)
{
    for (const auto &[id, track] : m_actorTracks.asKeyValueRange()) {
        if (scene.nestedScenes.contains(id)) {
            qInfo() << "Updating nested scene" << id;
            scene.nestedScenes[id].transformation = track->transformationAtTime(time);
            printTransformation(scene.nestedScenes[id].transformation);
            qInfo() << "";
        }
    }
}
