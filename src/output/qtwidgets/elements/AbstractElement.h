// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "BackgroundDrawing.h"
#include "StyleUtils.h"
#include "qtwidgets_logging.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

enum class PaddingDirection {
    Inward,
    Outward
};

class AbstractElement : public QObject
{
    Q_OBJECT

public:
    AbstractElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~AbstractElement() override;

    QIcon icon() const;
    void setIcon(const QIcon &icon);
    bool hasIcon() const;

    QString text() const;
    void setText(const QString &text);
    bool hasText() const;

    QIcon indicator() const;
    void setIndicator(const QIcon &indicator);
    bool hasIndicator() const;

    /*!
     * \brief Returns the validity status. If the element has no properties loaded,
     * it is not valid and can not be drawn.
     */
    bool isValid() const;

    /*!
     * \brief Draw the whole element, including text, icon, background and indicator.
     */
    virtual void draw(QPainter *painter) const;
    /*!
     * \brief Draw text of the element.
     */
    virtual void drawText(QPainter *painter) const;
    /*!
     * \brief Draw icon of the element.
     */
    virtual void drawIcon(QPainter *painter) const;
    /*!
     * \brief Draw whole background of the element.
     */
    virtual void drawBackground(QPainter *painter) const;
    /*!
     * \brief Draw only the background frame of the element.
     */
    virtual void drawFrame(QPainter *painter) const;
    /*!
     * \brief Draw only the background panel of the element.
     */
    virtual void drawPanel(QPainter *painter) const;
    /*!
     * \brief Draw the indicator of the element. This can vary from secondary icon, such as drop-down
     * arrow icon, to a checkbox, depending on the element.
     */
    virtual void drawIndicator(QPainter *painter) const;
    /*!
     * \brief Prepare the layoutMap of the element, and create the required properties.
     * By default this creates proeprties for background and content.
     */
    virtual void layout();
    /*!
     * \brief Return the contents size of the element with padding applied by default.
     */
    virtual QSizeF contentsSize(const QSizeF &contentsSizeFromStyle) const;
    /*!
     * \brief Return a subelement rectangle. If not found, empty QRect() is returned instead.
     */
    virtual QRectF subElementRect(QStyle::SubElement element) const;
    /*!
     * \brief Return a subcontrol rectangle. If not found, empty QRect() is returned instead.
     */
    virtual QRectF subControlRect(QStyle::SubControl subControl) const;

    virtual QMarginsF padding() const;

    virtual QMarginsF borderSize() const;

    virtual qreal height() const;

    virtual qreal width() const;

    virtual qreal spacing() const;

    virtual QSizeF indicatorSize() const;

    qreal averagePadding() const;

    qreal averageBorderSize() const;
    /*!
     * \brief Updates the properties of the element, such as text and layouting
     */
    virtual void update();

protected:
    const QStyleOption *m_styleOption;
    const UnionStyle *m_style;
    const QWidget *m_widget;
    QIcon m_icon = QIcon();
    QString m_text = QString();
    QIcon m_indicator = QIcon();
    Union::ElementList m_backgroundElementList;
    Union::ElementList m_contentElementList;
    Union::ElementList m_indicatorElementList;
    // Holds the properties for the background: This is the top-level properties of the item
    // by default.
    Union::Properties::StylePropertyGroup *m_backgroundProperties;
    // Holds the properties for any contents, such as text and icon.
    // This can vary a lot depending on the element.
    Union::Properties::StylePropertyGroup *m_contentProperties;
    // Holds the properties for any indicators, such as dropdown arrows.
    // This can vary a lot depending on the element.
    Union::Properties::StylePropertyGroup *m_indicatorProperties;
    QMap<QString, LayoutItem> m_layoutMap;
    QStringList m_subElementList;

    // Updates the m_subElementList with any values that are used when fetching a layout, so
    // that the element gets a proper hierarchy.
    virtual void updateSubElementList();

    // Utilizes the background property to apply a padding to the given size.
    QSizeF applyPaddingToSize(QSizeF oldSize, PaddingDirection direction = PaddingDirection::Outward) const;

    // Used to check if we have all elements properly prepared
    bool m_isValid = false;
};
