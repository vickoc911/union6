// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#pragma once

#include <QPainterPath>
#include <QStyle>
#include <properties/StylePropertyGroup.h>

class QPainter;

class BackgroundDrawing : public QObject
{
    Q_OBJECT
public:
    enum class Parts {
        All,
        PanelOnly,
        FrameOnly
    };
    Q_ENUM(Parts)

    BackgroundDrawing(QPainter *painter, const Union::StylePropertyGroupPtr &properties, const QStyleOption *option, const QWidget *widget = nullptr);
    ~BackgroundDrawing() override;

    /*!
     * \brief Draw a Union StylePropertyGroup, such as the center, border and corners.
     */
    void draw();

    /*!
     * \brief Generates a QPainterPath to use with outlines and rounded corners.
     */
    static QPainterPath unevenRadiiRectPath(const QRectF &rect, const Union::Properties::CornersPropertyGroup::CornerRadii cornerRadii);

    void setParts(Parts parts);
    void setOverrideBackgroundColor(const QColor &color);
    void setRect(const QRectF &rect);

private:
    enum class SubNodeIndex {
        Left,
        Right,
        Top,
        Bottom,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
    };
    /*!
     * \brief Constrains the corner radii to not go over the rectangle's size or under zero.
     */
    Union::Properties::CornersPropertyGroup::CornerRadii constrainRadii(const QRectF &rect,
                                                                        const Union::Properties::CornersPropertyGroup::CornerRadii cornerRadii);

    /*!
     * \brief Draw a Union LineProperty.
     */
    void drawLineProperty(QPainter *painter,
                          const QRectF &rect,
                          SubNodeIndex subNodeIndex,
                          const QMarginsF &borderSizes,
                          const Union::Properties::LinePropertyGroup *line,
                          const Union::Properties::CornersPropertyGroup *corners);

    /*!
     * \brief Draw a Union CornerProperty.
     */
    void drawCornerProperty(QPainter *painter,
                            const QRectF &rect,
                            SubNodeIndex subNodeIndex,
                            const Union::Properties::BorderPropertyGroup *border,
                            const Union::Properties::CornerPropertyGroup *corner);

    QPainter *m_painter;
    Union::StylePropertyGroupPtr m_properties;
    const QStyleOption *m_option;
    const QWidget *m_widget;

    Parts m_drawParts;
    QRectF m_drawRect;
    QColor m_overrideColor;
};
