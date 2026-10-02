// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#pragma once

#include "qtwidgets_logging.h"
#include <Element.h>
#include <properties/SizePropertyGroup.h>
#include <properties/StylePropertyGroup.h>

#include "SharedNames.h"
#include <QMargins>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

class QStyleOption;

enum class BucketType {
    Start,
    Center,
    End,
    Fill
};

struct LayoutItem {
    QString elementName;
    int order;
    Union::Properties::Alignment horizontalAlignment;
    Union::Properties::Alignment verticalAlignment;
    QRectF rect;
};

struct LayoutBucket {
    BucketType type;
    QRectF rect;
    QList<LayoutItem> items;
};

const char property_union_member_list[] = "_union_member_list";

Qt::Alignment toQtHorizontalAlignment(Union::Properties::Alignment alignment);
Qt::Alignment toQtVerticalAlignment(Union::Properties::Alignment alignment);
Qt::TextElideMode toQtElideMode(Union::Properties::TextElide elideMode);
Qt::TextFlag toQtWrapMode(Union::Properties::TextWrapMode wrapMode);

/*!
 * \brief Returns the background rectangle of an option, but removes its insets according to the properties first.
 */
QRectF backgroundRectangle(const QStyleOption *option, const Union::Properties::StylePropertyGroup *properties);

/*!
 * \brief Helper function to get text from any QStyleOption that has a field with QString (text/title)
 */
QString textFromOption(const QStyleOption *opt);

/*!
 * \brief Helper function to get the icon size from any QStyleOption that has it declared
 */
QSizeF iconSizeFromOption(const QStyleOption *opt);

/*!
 * \brief Returns flags for text drawing purposes. Applies the text alignment based on the layouting.
 */
int textFlagsFromProperties(Union::Properties::StylePropertyGroup *properties);

/*!
 * \brief Centers a rectangle depending on width and height. Copied from Breeze.
 */
QRectF centerRect(const QRectF &rect, int width, int height);

/*!
 * \brief Tries to match styleOption type to a potential element.
 * Used when widget is null.
 */
QString styleOptionToElementName(const QStyleOption *option);

// Calculates the bounding box rectangle from items inside layoutMap
QRectF unifiedRect(QMap<QString, LayoutItem> layoutMap);

// Shared hints for frameOptions
QStringList frameHints(const QStyleOptionFrame *frameOption);

// QStyle::visualAlignment, but constexpr
constexpr Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment, Qt::Alignment defaultHorizontalAlignment = Qt::AlignLeft)
{
    if (!alignment.testAnyFlags(Qt::AlignHorizontal_Mask)) {
        alignment |= defaultHorizontalAlignment & Qt::AlignHorizontal_Mask;
    }
    if (alignment.testFlag(Qt::AlignAbsolute)) {
        return alignment;
    }
    constexpr Qt::Alignment leftRightMask = Qt::AlignLeft | Qt::AlignRight;
    const auto leftRightFlags = alignment & leftRightMask;
    if (direction == Qt::RightToLeft && leftRightFlags != 0 && leftRightFlags != leftRightMask) {
        alignment ^= leftRightMask;
    }
    return alignment | Qt::AlignAbsolute;
}

// QStyle::visualRect, but constexpr and using QRectF
constexpr QRectF visualRect(Qt::LayoutDirection direction, const QRectF &boundingRect, const QRectF &logicalRect)
{
    if (direction == Qt::LeftToRight) {
        return logicalRect;
    }
    return {boundingRect.x() + boundingRect.right() - logicalRect.right(), logicalRect.y(), logicalRect.width(), logicalRect.height()};
}

// QStyle::visualPos, but constexpr and using QRectF/QPointF
constexpr QPointF visualPos(Qt::LayoutDirection direction, const QRectF &boundingRect,
                            const QPointF &logicalPos)
{
    if (direction == Qt::LeftToRight) {
        return logicalPos;
    }
    return {boundingRect.right() - logicalPos.x(), logicalPos.y()};
}

// QStyle::alignedRect, but constexpr, no direction parameter and using QSizeF/QRectF.
// Unlike QStyle::alignedRect, you need to do RTL alignment adjustments with
// visualAlignment outside of this function.
constexpr QRectF alignedRect(Qt::Alignment alignment, const QSizeF &contentSize, const QRectF &containerRect)
{
    QRectF contentRect{containerRect.topLeft(), contentSize};
    if (alignment.testFlag(Qt::AlignVCenter)) {
        contentRect.moveTop(containerRect.y() + (containerRect.height() - contentRect.height()) / 2);
    } else if (alignment.testFlag(Qt::AlignBottom)) {
        contentRect.moveBottom(containerRect.bottom());
    }
    if (alignment.testFlag(Qt::AlignRight)) {
        contentRect.moveRight(containerRect.right());
    } else if (alignment.testFlag(Qt::AlignHCenter)) {
        contentRect.moveLeft(containerRect.x() + (containerRect.width() - contentRect.width()) / 2);
    }
    return contentRect;
}

// QStyle::alignedRect, but constexpr and using QSizeF/QRectF
// A mostly drop-in replacement for QStyle::alignedRect.
constexpr QRectF alignedRect(Qt::LayoutDirection direction, Qt::Alignment alignment, const QSizeF &contentSize, const QRectF &containerRect)
{
    alignment = visualAlignment(direction, alignment);
    return alignedRect(alignment, contentSize, containerRect);
}

// QStyle::drawItemText, but using QRectF
inline void drawItemText(QPainter *painter, const QRectF &rect, int flags, const QPalette &palette, bool enabled [[maybe_unused]], const QString &text, QPalette::ColorRole textRole = QPalette::NoRole)
{
    if (text.isEmpty()) {
        return;
    }
    if (textRole == QPalette::NoRole) {
        painter->drawText(rect, flags, text);
        return;
    }
    auto oldPen = painter->pen();
    auto newPen = oldPen;
    newPen.setBrush(palette.brush(textRole));
    painter->setPen(newPen);
    painter->drawText(rect, flags, text);
    painter->setPen(oldPen);
}

// QStyle::drawItemPixmap, but using QRectF.
// Unlike QStyle::drawItemPixmap, you need to do RTL alignment adjustments with
// visualAlignment outside of this function.
inline void drawItemPixmap(QPainter *painter, const QRectF &rect, Qt::Alignment alignment, const QPixmap &pixmap)
{
    // Try to align to physical pixels.
    auto dpr = pixmap.devicePixelRatio();
    QRectF pixmapRect = QRectF{rect.topLeft() * dpr, rect.size() * dpr}.toRect();
    QRectF aligned = alignedRect(alignment, pixmap.size(), pixmapRect).toRect();
    pixmapRect = aligned.intersected(pixmapRect);
    auto sourceRect = pixmapRect.translated(-aligned.topLeft());
    painter->drawPixmap({pixmapRect.topLeft() / dpr, pixmapRect.size() / dpr}, pixmap, sourceRect);
}
