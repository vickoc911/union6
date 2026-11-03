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
        auto outlineSize = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::top, &LinePropertyGroup::size);

        if (const auto border = m_properties->outline()) {
            // painter->setRenderHint(QPainter::Antialiasing, true);
            //  Make simpler border shapes if complex ones are not necessary
            const auto corners = m_properties->corners();
            const auto rect = m_outlineRect;
            const QMarginsF borderSizes = {outlineSize, outlineSize, outlineSize, outlineSize};
            // Draw borders
            if (const auto top = border->top()) {
                drawLineProperty(painter, rect, SubNodeIndex::Top, borderSizes, top, corners);
            }
            if (const auto right = border->right()) {
                drawLineProperty(painter, rect, SubNodeIndex::Right, borderSizes, right, corners);
            }
            if (const auto bottom = border->bottom()) {
                drawLineProperty(painter, rect, SubNodeIndex::Bottom, borderSizes, bottom, corners);
            }
            if (const auto left = border->left()) {
                drawLineProperty(painter, rect, SubNodeIndex::Left, borderSizes, left, corners);
            }

            // Draw corners if at least one border is present
            if (borderSizes.top() || borderSizes.left()) {
                const auto topLeft = corners ? corners->topLeft() : nullptr;
                drawOutlineCornerProperty(painter, rect, SubNodeIndex::TopLeft, border, topLeft);
            }
            if (borderSizes.top() || borderSizes.right()) {
                const auto topRight = corners ? corners->topRight() : nullptr;
                drawOutlineCornerProperty(painter, rect, SubNodeIndex::TopRight, border, topRight);
            }
            if (borderSizes.bottom() || borderSizes.right()) {
                const auto bottomRight = corners ? corners->bottomRight() : nullptr;
                drawOutlineCornerProperty(painter, rect, SubNodeIndex::BottomRight, border, bottomRight);
            }
            if (borderSizes.bottom() || borderSizes.left()) {
                const auto bottomLeft = corners ? corners->bottomLeft() : nullptr;
                drawOutlineCornerProperty(painter, rect, SubNodeIndex::BottomLeft, border, bottomLeft);
            }
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

void UnionGraphicsEffect::setOutlineRect(const QRectF &rect)
{
    m_outlineRect = rect;
}