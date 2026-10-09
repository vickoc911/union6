// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ShadowTileMap.h"
#include <QPainter>

using namespace Union;

ShadowTileMap::ShadowTileMap()
    : m_texture(QPixmap())
    , m_pixmaps(QVector<QPixmap>())
    , m_topLeft(QSizeF())
    , m_middle(QSizeF())
    , m_bottomRight(QSizeF())
{
    m_pixmaps.reserve(tileMapSize);
}

ShadowTileMap::ShadowTileMap(const QPixmap &texture, const QSizeF &topLeftSize, const QSizeF &middleSize)
    : m_texture(texture)
    , m_pixmaps(QVector<QPixmap>())
    , m_topLeft(topLeftSize)
    , m_middle(middleSize)
    , m_bottomRight(QSizeF())
{
    m_pixmaps.reserve(tileMapSize);
    if (texture.isNull()) {
        return;
    }

    m_bottomRight = QSizeF(texture.width() / texture.devicePixelRatio() - (m_topLeft.width() + m_middle.width()),
                           texture.height() / texture.devicePixelRatio() - (m_topLeft.height() + m_middle.height()));

    // Top left
    initializeTile(m_topLeft, QRectF(0, 0, m_topLeft.width(), m_topLeft.height()));

    // Top center
    initializeTile(QSizeF(m_middle.width(), m_topLeft.height()), QRectF(topLeftSize.width(), 0, m_middle.width(), m_topLeft.height()));

    // Top right
    initializeTile(QSizeF(m_bottomRight.width(), m_topLeft.height()),
                   QRectF(m_topLeft.width() + m_middle.width(), 0, m_bottomRight.width(), m_topLeft.height()));

    // Middle left
    initializeTile(QSizeF(m_topLeft.width(), m_middle.height()), QRectF(0, m_topLeft.height(), m_topLeft.width(), m_middle.height()));

    // Middle center
    initializeTile(m_middle, QRectF(m_topLeft.width(), m_topLeft.height(), m_middle.width(), m_middle.height()));

    // Middle right
    initializeTile(QSizeF(m_bottomRight.width(), m_middle.height()),
                   QRectF(m_topLeft.width() + m_middle.width(), m_topLeft.height(), m_bottomRight.width(), m_middle.height()));

    // Bottom left
    initializeTile(QSizeF(m_topLeft.width(), m_bottomRight.height()),
                   QRectF(0, m_topLeft.height() + m_middle.height(), m_topLeft.width(), m_bottomRight.height()));

    // Bottom middle
    initializeTile(QSizeF(m_middle.width(), m_bottomRight.height()),
                   QRectF(m_topLeft.width(), m_topLeft.height() + m_middle.height(), m_middle.width(), m_bottomRight.height()));

    // Bottom right
    initializeTile(m_bottomRight,
                   QRectF(m_topLeft.width() + m_middle.width(), m_topLeft.height() + m_middle.height(), m_bottomRight.width(), m_bottomRight.height()));
}

void ShadowTileMap::initializeTile(const QSizeF &size, const QRectF &targetRect)
{
    if (!(size.isValid() && targetRect.isValid())) {
        m_pixmaps.append(QPixmap());
        return;
    }
    const qreal dpi = m_texture.devicePixelRatio();
    const QRectF scaledRect = QRectF(targetRect.topLeft() * dpi, targetRect.size() * dpi);
    QPixmap scaledPixmap;
    if (size != targetRect.size()) {
        const QSizeF scaledSize = size * dpi;
        const QPixmap tile(m_texture.copy(scaledRect.toRect()));
        scaledPixmap = QPixmap(scaledSize.toSize());
        scaledPixmap.fill(Qt::transparent);
        QPainter painter(&scaledPixmap);
        painter.drawTiledPixmap(0, 0, scaledSize.width(), scaledSize.height(), tile);
    } else {
        scaledPixmap = QPixmap(m_texture.copy(scaledRect.toRect()));
    }
    scaledPixmap.setDevicePixelRatio(dpi);
    m_pixmaps.append(scaledPixmap);
}

bool ShadowTileMap::isValid() const
{
    return m_pixmaps.size() == tileMapSize;
}

QVector<QPixmap> ShadowTileMap::pixmaps() const
{
    return m_pixmaps;
}

void ShadowTileMap::draw(const QRectF &drawRect, QPainter *painter) const
{
    if (m_pixmaps.size() < tileMapSize) {
        return;
    }
    painter->save();

    auto pixmapDpi = [this](const int &index) -> qreal {
        return m_pixmaps.at(index).devicePixelRatio();
    };

    QRectF rect = drawRect;

    qreal x = rect.x();
    qreal y = rect.y();
    qreal width = rect.width();
    qreal height = rect.height();
    qreal leftWidth = 0;
    qreal rightWidth = 0;
    qreal topHeight = 0;
    qreal bottomHeight = 0;

    // Prepare widths
    if (m_topLeft.width() + m_bottomRight.width() > 0) {
        qreal widthRatio = m_topLeft.width() / (m_topLeft.width() + m_bottomRight.width());
        leftWidth = std::min(m_topLeft.width(), width * widthRatio);
        rightWidth = std::min(m_bottomRight.width(), width * (1.0 - widthRatio));
    }

    // Prepare heights
    if (m_topLeft.height() + m_bottomRight.height() > 0) {
        qreal heightRatio = m_topLeft.height() / (m_topLeft.height() + m_bottomRight.height());
        topHeight = std::min(m_topLeft.height(), height * heightRatio);
        bottomHeight = std::min(m_bottomRight.height(), height * (1.0 - heightRatio));
    }

    // Corners
    width -= leftWidth + rightWidth;
    height -= topHeight + bottomHeight;
    const QPointF cornerTopLeft(x + leftWidth, y + topHeight);
    const QPointF cornerBottomRight(cornerTopLeft.x() + width, cornerTopLeft.y() + height);
    const QSizeF cornerSize(m_pixmaps.at(7).width() / pixmapDpi(7), m_pixmaps.at(5).height() / pixmapDpi(5));

    // Draw corner tiles
    // Top left
    auto dpi = pixmapDpi(0);
    painter->drawPixmap(x, y, m_pixmaps.at(0), 0, 0, leftWidth * dpi, topHeight * dpi);

    // Top right
    dpi = pixmapDpi(2);
    painter->drawPixmap(cornerBottomRight.x(), y, m_pixmaps.at(2), (m_bottomRight.width() - rightWidth) * dpi, 0, rightWidth * dpi, topHeight * dpi);

    // Bottom left
    dpi = pixmapDpi(6);
    painter->drawPixmap(x, cornerBottomRight.y(), m_pixmaps.at(6), 0, (m_bottomRight.height() - bottomHeight) * dpi, leftWidth * dpi, bottomHeight * dpi);

    // Bottom right
    dpi = pixmapDpi(8);
    painter->drawPixmap(cornerBottomRight.x(),
                        cornerBottomRight.y(),
                        m_pixmaps.at(8),
                        (m_bottomRight.width() - rightWidth) * dpi,
                        (m_bottomRight.height() - bottomHeight) * dpi,
                        rightWidth * dpi,
                        bottomHeight * dpi);

    if (width > 0) {
        // Top center
        dpi = pixmapDpi(1);
        painter->drawPixmap(cornerTopLeft.x(), y, width, topHeight, m_pixmaps.at(1), 0, 0, cornerSize.width() * dpi, topHeight * dpi);

        // Bottom
        dpi = pixmapDpi(7);
        painter->drawPixmap(cornerTopLeft.x(),
                            cornerBottomRight.y(),
                            width,
                            bottomHeight,
                            m_pixmaps.at(7),
                            0,
                            (m_bottomRight.height() - bottomHeight) * dpi,
                            cornerSize.width() * dpi,
                            bottomHeight * dpi);
    }

    if (height > 0) {
        // Left
        dpi = pixmapDpi(3);
        painter->drawPixmap(x, cornerTopLeft.y(), leftWidth, height, m_pixmaps.at(3), 0, 0, leftWidth * dpi, cornerSize.height() * dpi);

        // Right

        dpi = pixmapDpi(5);
        painter->drawPixmap(cornerBottomRight.x(),
                            cornerTopLeft.y(),
                            rightWidth,
                            height,
                            m_pixmaps.at(5),
                            (m_bottomRight.width() - rightWidth) * dpi,
                            0,
                            rightWidth * dpi,
                            cornerSize.height() * dpi);
    }

    painter->restore();
}
