// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "UnionGraphicsEffect.h"
#include "UnionPainter.h"
#include <QPainter>

using namespace Union::Properties;

UnionGraphicsEffect::UnionGraphicsEffect(QObject *parent)
    : QGraphicsDropShadowEffect(parent)
    , m_isValid(false)
{
    setEnabled(false);
}

UnionGraphicsEffect::~UnionGraphicsEffect()
{
}

void UnionGraphicsEffect::draw(QPainter *painter)
{
    // Draw shadow first with QGraphicsDropShadowEffect
    QGraphicsDropShadowEffect::draw(painter);

    // Now draw outline
    if (m_isValid) {
        auto outlineColor =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::outline, &OutlinePropertyGroup::top, &LinePropertyGroup::color);

        if (outlineColor.isValid()) {
            UnionPainter p(painter, m_properties);
            p.setRect(boundingRect());
            p.setParts(UnionPainter::Parts::Outline);
            p.draw();
        }
    }
}

void UnionGraphicsEffect::setProperties(Union::StylePropertyGroupPtr properties)
{
    if (properties) {
        m_properties = properties;
        m_isValid = true;
        setEnabled(true);
        auto shadowColor = safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::shadow, &ShadowPropertyGroup::color);
        auto shadowOffset =
            safePropertyLookup(m_properties, QVector2D(), &StylePropertyGroup::shadow, &ShadowPropertyGroup::offset, &OffsetPropertyGroup::toVector2D);
        auto shadowBlur = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::shadow, &ShadowPropertyGroup::blur);
        setColor(shadowColor.toQColor());
        setBlurRadius(shadowBlur);
        setOffset(shadowOffset.x(), shadowOffset.y());
    }
}

QRectF UnionGraphicsEffect::boundingRectFor(const QRectF &rect) const
{
    auto newRect = QGraphicsDropShadowEffect::boundingRectFor(rect);
    if (m_properties) {
        QMarginsF outlineSize;
        outlineSize.setTop(safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::top, &LinePropertyGroup::size));
        outlineSize.setLeft(safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::left, &LinePropertyGroup::size));
        outlineSize.setBottom(safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::bottom, &LinePropertyGroup::size));
        outlineSize.setRight(safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::right, &LinePropertyGroup::size));
        newRect = newRect.marginsAdded(outlineSize);
    }
    return newRect;
}
