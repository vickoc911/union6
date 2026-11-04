// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "UnionGraphicsEffect.h"
#include "BackgroundDrawing.h"
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
    if (m_isValid && !m_outlineRect.isNull()) {
        auto outlineColor =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::outline, &OutlinePropertyGroup::top, &LinePropertyGroup::color);

        BackgroundDrawing p(painter, m_properties);
        // need to resize the painter area for these
        p.setRect(m_outlineRect.adjusted(-10, -10, 10, 10));
        p.setParts(BackgroundDrawing::Parts::Outline);
        p.draw();
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

void UnionGraphicsEffect::setOutlineRect(const QRectF &rect)
{
    m_outlineRect = rect;
}