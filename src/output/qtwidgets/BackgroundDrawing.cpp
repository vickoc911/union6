// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "BackgroundDrawing.h"

#include "StyleUtils.h"
#include <Element.h>
#include <ElementQuery.h>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <StyleRegistry.h>
#include <StyleUtils.h>
#include <algorithm>

#include "LruCache.h"

using namespace Union::Properties;
static Union::LruImageCache imageCache;

BackgroundDrawing::BackgroundDrawing(QPainter *painter, const Union::StylePropertyGroupPtr &properties, const QStyleOption *option, const QWidget *widget)
    : QObject(nullptr)
    , m_painter(painter)
    , m_properties(properties)
    , m_option(option)
    , m_widget(widget)
    , m_drawParts(Parts::All)
    , m_drawRect(option->rect)
    , m_overrideColor(QColor())
{
}

BackgroundDrawing::~BackgroundDrawing()
{
}

void BackgroundDrawing::draw()
{
    if (!m_properties) {
        return;
    }

    m_painter->save();

    QRectF rect = m_drawRect;
    // Remove any insets we may have, we do not want to draw them
    if (m_properties->layout() && m_properties->layout()->inset()) {
        auto height = rect.height();
        auto width = rect.width();
        // Ensure we do not go out of bounds
        rect = rect.marginsRemoved(m_properties->layout()->inset()->toMargins());
        if (rect.height() <= 0 || rect.width() <= 0) {
            rect = centerRect(rect.toRect(), width, height);
        }
    }
    QRectF innerRect = rect;

    // Borders
    bool allBordersEqual = false;
    bool allBorderColorsEqual = false;
    Borders borders;

    if (m_drawParts == Parts::Outline) {
        borders.top.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::top, &LinePropertyGroup::size);
        borders.left.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::left, &LinePropertyGroup::size);
        borders.bottom.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::bottom, &LinePropertyGroup::size);
        borders.right.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::outline, &OutlinePropertyGroup::right, &LinePropertyGroup::size);
        borders.top.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::outline, &OutlinePropertyGroup::top, &LinePropertyGroup::color).toQColor();
        borders.left.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::outline, &OutlinePropertyGroup::left, &LinePropertyGroup::color).toQColor();
        borders.bottom.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::outline, &OutlinePropertyGroup::bottom, &LinePropertyGroup::color).toQColor();
        borders.right.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::outline, &OutlinePropertyGroup::right, &LinePropertyGroup::color).toQColor();
    } else {
        borders.top.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::border, &BorderPropertyGroup::top, &LinePropertyGroup::size);
        borders.left.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::border, &BorderPropertyGroup::left, &LinePropertyGroup::size);
        borders.bottom.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::border, &BorderPropertyGroup::bottom, &LinePropertyGroup::size);
        borders.right.size = safePropertyLookup(m_properties, 0.0, &StylePropertyGroup::border, &BorderPropertyGroup::right, &LinePropertyGroup::size);
        borders.top.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::border, &BorderPropertyGroup::top, &LinePropertyGroup::color).toQColor();
        borders.left.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::border, &BorderPropertyGroup::left, &LinePropertyGroup::color).toQColor();
        borders.bottom.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::border, &BorderPropertyGroup::bottom, &LinePropertyGroup::color).toQColor();
        borders.right.color =
            safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::border, &BorderPropertyGroup::right, &LinePropertyGroup::color).toQColor();
    }

    borders.cornerRadii = safePropertyLookup(m_properties, CornersPropertyGroup::CornerRadii{}, &StylePropertyGroup::corners, &CornersPropertyGroup::radii);

    QMarginsF borderSizes = {borders.left.size, borders.top.size, borders.right.size, borders.bottom.size};
    allBordersEqual = qFuzzyCompare(borderSizes.left(), borderSizes.right()) && qFuzzyCompare(borderSizes.top(), borderSizes.bottom())
        && qFuzzyCompare(borderSizes.left(), borderSizes.top());

    allBorderColorsEqual =
        (borders.left.color == borders.right.color) && (borders.top.color == borders.bottom.color) && (borders.left.color == borders.top.color);

    // Make space for outlines
    innerRect -= borderSizes;

    // Corners
    Union::Properties::CornersPropertyGroup::CornerRadii constrainedRadii = {0, 0, 0, 0};
    Union::Properties::CornersPropertyGroup::CornerRadii innerCornerRadii = {0, 0, 0, 0};
    bool allCornerRadiiEqual = false;
    constrainedRadii = constrainRadii(rect, borders.cornerRadii);
    allCornerRadiiEqual = qFuzzyCompare(constrainedRadii.topLeft, constrainedRadii.topRight)
        && qFuzzyCompare(constrainedRadii.bottomLeft, constrainedRadii.bottomRight) && qFuzzyCompare(constrainedRadii.topLeft, constrainedRadii.bottomRight);
    innerCornerRadii = constrainedRadii;

    // Make space for outlines
    // Shrink inner radii
    innerCornerRadii = {std::max(innerCornerRadii.topLeft - std::min(borderSizes.top(), borderSizes.left()), 0.0),
                        std::max(innerCornerRadii.topRight - std::min(borderSizes.top(), borderSizes.right()), 0.0),
                        std::max(innerCornerRadii.bottomLeft - std::min(borderSizes.bottom(), borderSizes.left()), 0.0),
                        std::max(innerCornerRadii.bottomRight - std::min(borderSizes.bottom(), borderSizes.right()), 0.0)};

    // Draw background (panel)
    if (m_drawParts != Parts::FrameOnly && m_drawParts != Parts::Outline) {
        if (const auto background = m_properties->background()) {
            QPainterPath path;
            // Draw less complex rectangles if complex ones are not needed
            if (allCornerRadiiEqual && !constrainedRadii.topLeft) {
                path.addRect(innerRect);
            } else if (allCornerRadiiEqual) {
                path.addRoundedRect(innerRect, innerCornerRadii.topLeft, innerCornerRadii.topLeft);
            } else {
                path = unevenRadiiRectPath(innerRect, innerCornerRadii);
            }

            if (m_overrideColor.isValid()) {
                m_painter->setPen(Qt::transparent);
                m_painter->setBrush(m_overrideColor);
                m_painter->drawPath(path);
            } else {
                m_painter->setPen(Qt::transparent);
                m_painter->setBrush(
                    safePropertyLookup(m_properties, Union::Color(), &StylePropertyGroup::background, &BackgroundPropertyGroup::color).toQColor());
                m_painter->drawPath(path);
            }
            if (const auto image = background->image(); image && image->source()) {
                m_painter->save();
                m_painter->setClipPath(path);

                QSizeF imageSize;
                imageSize.setHeight(safePropertyLookup(m_properties, 16.0, &StylePropertyGroup::layout, &LayoutPropertyGroup::height));
                imageSize.setWidth(safePropertyLookup(m_properties, 16.0, &StylePropertyGroup::layout, &LayoutPropertyGroup::width));
                auto loadedImage = imageCache.load(image->source().value(), imageSize);
                if (image->flags().value().testAnyFlag(Union::Properties::ImageFlag::Mask)) {
                    m_painter->setBrush(Qt::transparent);
                    m_painter->setPen(image->maskColor()->toQColor());
                    QBitmap mask = QBitmap::fromImage(loadedImage.createAlphaMask());
                    auto imageRect = innerRect;
                    imageRect.setWidth(mask.width());
                    imageRect.setHeight(mask.height());
                    imageRect.moveCenter(innerRect.center());
                    m_painter->drawPixmap(imageRect.topLeft(), mask);
                } else {
                    m_painter->drawImage(innerRect, loadedImage);
                }

                m_painter->restore();
            }
        }
    }
    // Draw borders and corners (frame)
    if (m_drawParts != Parts::PanelOnly) {
        m_painter->setRenderHint(QPainter::Antialiasing, true);
        // Make simpler border shapes if complex ones are not necessary
        if (allBordersEqual && allBorderColorsEqual && allCornerRadiiEqual) { // All radii and borders identical
            QPainterPath rectangularOutline;
            QPainterPath innerRectangularOutline;
            innerRectangularOutline.addRoundedRect(innerRect, innerCornerRadii.topLeft, innerCornerRadii.topLeft);
            rectangularOutline.addRoundedRect(rect, constrainedRadii.topLeft, constrainedRadii.topLeft);
            rectangularOutline = rectangularOutline.subtracted(innerRectangularOutline);

            m_painter->setPen(Qt::transparent);
            m_painter->setBrush(borders.top.color);
            m_painter->drawPath(rectangularOutline);
        } else if (allBordersEqual && allBorderColorsEqual) { // All borders identical
            QPainterPath rectangularOutline;
            QPainterPath innerRectangularOutline;
            innerRectangularOutline = unevenRadiiRectPath(innerRect, innerCornerRadii);
            rectangularOutline = unevenRadiiRectPath(rect, constrainedRadii);
            rectangularOutline = rectangularOutline.subtracted(innerRectangularOutline);

            m_painter->setPen(Qt::transparent);
            m_painter->setBrush(borders.top.color);
            m_painter->drawPath(rectangularOutline);
        } else { // All other complex scenarios
            // Draw borders
            if (borders.top.size > 0) {
                drawLineProperty(rect, SubNodeIndex::Top, borderSizes, borders);
            }
            if (borders.right.size > 0) {
                drawLineProperty(rect, SubNodeIndex::Right, borderSizes, borders);
            }
            if (borders.bottom.size > 0) {
                drawLineProperty(rect, SubNodeIndex::Bottom, borderSizes, borders);
            }
            if (borders.left.size > 0) {
                drawLineProperty(rect, SubNodeIndex::Left, borderSizes, borders);
            }
            // Draw corners if at least one border is present
            if (borderSizes.top() || borderSizes.left()) {
                drawCornerProperty(rect, SubNodeIndex::TopLeft, borderSizes, borders);
            }
            if (borderSizes.top() || borderSizes.right()) {
                drawCornerProperty(rect, SubNodeIndex::TopRight, borderSizes, borders);
            }
            if (borderSizes.bottom() || borderSizes.right()) {
                drawCornerProperty(rect, SubNodeIndex::BottomRight, borderSizes, borders);
            }
            if (borderSizes.bottom() || borderSizes.left()) {
                drawCornerProperty(rect, SubNodeIndex::BottomLeft, borderSizes, borders);
            }
        }
    }

    m_painter->restore();
}

QPainterPath BackgroundDrawing::unevenRadiiRectPath(const QRectF &rect, const Union::Properties::CornersPropertyGroup::CornerRadii cornerRadii)
{
    QPainterPath path;

    // Path
    path.moveTo(rect.x(), rect.y() + rect.height() - cornerRadii.bottomLeft);

    // Top Left
    path.lineTo(rect.x(), rect.y() + cornerRadii.topLeft);
    if (cornerRadii.topLeft) {
        path.arcTo(QRectF(rect.x(), rect.y(), cornerRadii.topLeft * 2, cornerRadii.topLeft * 2), 180, -90);
    }

    // Top Right
    path.lineTo(rect.x() + rect.width() - cornerRadii.topRight, rect.top());
    if (cornerRadii.topRight) {
        path.arcTo(QRectF(rect.x() + rect.width() - cornerRadii.topRight * 2, rect.y(), cornerRadii.topRight * 2, cornerRadii.topRight * 2), 90, -90);
    }

    // Bottom Right
    path.lineTo(rect.x() + rect.width(), rect.y() + rect.height() - cornerRadii.bottomRight);
    if (cornerRadii.bottomRight) {
        path.arcTo(QRectF(rect.x() + rect.width() - cornerRadii.bottomRight * 2,
                          rect.y() + rect.height() - cornerRadii.bottomRight * 2,
                          cornerRadii.bottomRight * 2,
                          cornerRadii.bottomRight * 2),
                   0,
                   -90);
    }

    // Bottom Left
    path.lineTo(rect.x() + cornerRadii.bottomLeft, rect.y() + rect.height());
    if (cornerRadii.bottomLeft) {
        path.arcTo(QRectF(rect.x(), rect.y() + rect.height() - cornerRadii.bottomLeft * 2, cornerRadii.bottomLeft * 2, cornerRadii.bottomLeft * 2), -90, -90);
    }

    path.closeSubpath();

    return path;
}

void BackgroundDrawing::setParts(Parts parts)
{
    m_drawParts = parts;
}

void BackgroundDrawing::setOverrideBackgroundColor(const QColor &color)
{
    m_overrideColor = color;
}
void BackgroundDrawing::setRect(const QRectF &rect)
{
    m_drawRect = rect;
}

Union::Properties::CornersPropertyGroup::CornerRadii BackgroundDrawing::constrainRadii(const QRectF &rect,
                                                                                       const Union::Properties::CornersPropertyGroup::CornerRadii cornerRadii)
{
    auto topLeft = std::min({rect.width() / 2.0, rect.height() / 2.0, cornerRadii.topLeft});
    auto topRight = std::min({rect.width() / 2.0, rect.height() / 2.0, cornerRadii.topRight});
    auto bottomLeft = std::min({rect.width() / 2.0, rect.height() / 2.0, cornerRadii.bottomLeft});
    auto bottomRight = std::min({rect.width() / 2.0, rect.height() / 2.0, cornerRadii.bottomRight});

    Union::Properties::CornersPropertyGroup::CornerRadii constrainedRadii = {std::max(topLeft, 0.0),
                                                                             std::max(topRight, 0.0),
                                                                             std::max(bottomLeft, 0.0),
                                                                             std::max(bottomRight, 0.0)};

    return constrainedRadii;
}

void BackgroundDrawing::drawLineProperty(const QRectF &rect, SubNodeIndex subNodeIndex, const QMarginsF &borderSizes, const Borders &borders)
{
    auto cornerRadii = safePropertyLookup(m_properties, CornersPropertyGroup::CornerRadii{}, &StylePropertyGroup::corners, &CornersPropertyGroup::radii);

    QRectF lineRect;
    QColor color;
    switch (subNodeIndex) {
    case SubNodeIndex::Left: {
        const qreal topMargin = cornerRadii.topLeft;
        const qreal bottomMargin = cornerRadii.bottomLeft;

        lineRect = QRectF{static_cast<double>(rect.x()),
                          rect.y() + borderSizes.top() + topMargin,
                          borders.left.size,
                          rect.height() - borderSizes.top() - borderSizes.bottom() - bottomMargin - topMargin};
        color = borders.left.color;
    } break;
    case SubNodeIndex::Right: {
        const qreal topMargin = cornerRadii.topRight;
        const qreal bottomMargin = cornerRadii.bottomRight;

        lineRect = QRectF{rect.x() + rect.width() - borders.right.size,
                          rect.y() + borderSizes.top() + topMargin,
                          borders.right.size,
                          rect.height() - borderSizes.top() - borderSizes.bottom() - bottomMargin - topMargin};
        color = borders.right.color;
    } break;
    case SubNodeIndex::Top: {
        const qreal leftMargin = cornerRadii.topLeft;
        const qreal rightMargin = cornerRadii.topRight;

        lineRect = QRectF{rect.x() + borderSizes.left() + leftMargin,
                          static_cast<double>(rect.y()),
                          rect.width() - borderSizes.left() - borderSizes.right() - rightMargin - leftMargin,
                          borders.top.size};
        color = borders.top.color;
    } break;
    case SubNodeIndex::Bottom: {
        const qreal leftMargin = cornerRadii.bottomLeft;
        const qreal rightMargin = cornerRadii.bottomRight;

        lineRect = QRectF{rect.x() + borderSizes.left() + leftMargin,
                          rect.y() + rect.height() - borders.bottom.size,
                          rect.width() - borderSizes.left() - borderSizes.right() - rightMargin - leftMargin,
                          borders.bottom.size};
        color = borders.bottom.color;
    } break;
    default:
        Q_UNREACHABLE();
    }

    if (color.isValid()) {
        m_painter->save();
        m_painter->setPen(Qt::transparent);
        m_painter->setBrush(color);
        m_painter->drawRect(lineRect);
        m_painter->restore();
    }
}

// TODO: Fix tiny gaps caused by antialiasing between corners and borders
void BackgroundDrawing::drawCornerProperty(const QRectF &rect, SubNodeIndex subNodeIndex, const QMarginsF &borderSizes, const Borders &borders)
{
    QPainterPath path;
    QLinearGradient cornerTransition;

    // Start, end, top and left assume a top-left corner being drawn clockwise which gets rotated
    qreal cornerRadius = 0.0;
    double topMargin = 0.0;
    double leftMargin = 0.0;
    QColor startColor = Qt::transparent;
    QColor endColor = Qt::transparent;

    m_painter->save();
    m_painter->setPen(Qt::transparent);

    switch (subNodeIndex) {
    case SubNodeIndex::TopLeft: {
        leftMargin = borderSizes.left();
        topMargin = borderSizes.top();

        startColor = borders.left.color.isValid() ? borders.left.color.isValid() : Qt::transparent;
        endColor = borders.top.color.isValid() ? borders.top.color : Qt::transparent;

        cornerRadius = borders.cornerRadii.topLeft;

        m_painter->translate(rect.x(), rect.y());
    } break;

    case SubNodeIndex::TopRight: {
        leftMargin = borderSizes.top();
        topMargin = borderSizes.right();

        startColor = borders.top.color.isValid() ? borders.top.color : Qt::transparent;
        endColor = borders.right.color.isValid() ? borders.right.color : Qt::transparent;

        m_painter->translate(rect.x() + rect.width(), rect.y());
        m_painter->rotate(90);
    } break;

    case SubNodeIndex::BottomRight: {
        leftMargin = borderSizes.right();
        topMargin = borderSizes.bottom();

        startColor = borders.right.color.isValid() ? borders.right.color : Qt::transparent;
        endColor = borders.bottom.color.isValid() ? borders.bottom.color : Qt::transparent;

        m_painter->translate(rect.x() + rect.width(), rect.y() + rect.height());
        m_painter->rotate(180);
    } break;

    case SubNodeIndex::BottomLeft: {
        leftMargin = borderSizes.bottom();
        topMargin = borderSizes.left();

        startColor = borders.bottom.color.isValid() ? borders.bottom.color : Qt::transparent;
        endColor = borders.left.color.isValid() ? borders.left.color : Qt::transparent;

        m_painter->translate(rect.x(), rect.y() + rect.height());
        m_painter->rotate(270);
    } break;

    default: {
        break;
    }
    }

    const auto radius = std::min<double>({static_cast<double>(rect.width()) / 2, static_cast<double>(rect.height()) / 2, cornerRadius});
    double innerRadius = 0.0;

    innerRadius = std::max(radius - std::min(topMargin, leftMargin), 0.0);

    // Draw path only when necessary, otherwise draw a simple rectangle
    if (radius) {
        path.moveTo(radius + leftMargin, 0);

        path.lineTo(radius, 0);
        path.arcTo(QRectF(0, 0, radius * 2, radius * 2), 90, 90);
        path.lineTo(0, radius + topMargin);
        path.lineTo(leftMargin, radius + topMargin);
        path.lineTo(leftMargin, innerRadius + topMargin);
        if (innerRadius) {
            path.arcTo(QRectF(leftMargin, topMargin, innerRadius * 2, innerRadius * 2), 180, -90);
        }
        path.lineTo(leftMargin + radius, topMargin);
        path.lineTo(leftMargin + radius, 0);
        path.lineTo(radius, 0);

        path.closeSubpath();
    } else {
        path.addRect(QRectF(0, 0, leftMargin, topMargin));
    }

    if (startColor == endColor) {
        m_painter->setBrush(startColor);
    } else {
        cornerTransition = QLinearGradient(QPointF(0, radius + topMargin), QPointF(radius + leftMargin, 0));
        cornerTransition.setColorAt(0, startColor);
        cornerTransition.setColorAt(1, endColor);
        m_painter->setBrush(QBrush(cornerTransition));
    }

    m_painter->drawPath(path);
    m_painter->restore();
}
