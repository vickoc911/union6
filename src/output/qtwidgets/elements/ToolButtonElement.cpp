// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ToolButtonElement.h"
#include "BackgroundDrawing.h"
#include "PropertiesTypes.h"
#include "SharedNames.h"
#include "StyleUtils.h"
#include "UnionStyle.h"
#include "elements/MenuBarElement.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>
#include <qnamespace.h>
#include <qstyle.h>

using namespace Qt::StringLiterals;

ToolButtonElement::ToolButtonElement(const QStyleOptionToolButton *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_toolButtonOption(option)
    , m_hasIndicator(false)
    , m_hasArrows(false)
    , m_hasIcon(false)
    , m_hasText(false)
{
    update();
}

ToolButtonElement::~ToolButtonElement()
{
}

void ToolButtonElement::update()
{
    m_hasIndicator =
        m_toolButtonOption->features.testFlag(QStyleOptionToolButton::HasMenu) || m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Menu);
    m_hasArrows = m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Arrow) && m_toolButtonOption->toolButtonStyle != Qt::ToolButtonTextOnly;
    m_hasIcon = !m_toolButtonOption->icon.isNull() && m_toolButtonOption->toolButtonStyle != Qt::ToolButtonTextOnly;
    m_hasText = !m_toolButtonOption->text.isEmpty() && m_toolButtonOption->toolButtonStyle != Qt::ToolButtonIconOnly;
    m_indicatorElementList = prepareElements(m_toolButtonOption, m_widget, {ElementString::Indicator});
    setIndicator(QIcon());
    if (!m_indicatorElementList.isEmpty()) {
        m_indicatorProperties = queryProperties(m_indicatorElementList);
        setIndicator(m_style->unionIcon(m_indicatorProperties, QString()));
    }
    setIcon(m_toolButtonOption->icon);
    setText(m_toolButtonOption->text);
    updateSubElementList();
    layout();
}

void ToolButtonElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_hasIcon || m_hasArrows) {
        m_subElementList.append(ElementString::Icon);
    }
    if (m_hasText) {
        m_subElementList.append(ElementString::Text);
    }
    if (m_hasIndicator) {
        m_subElementList.append(ElementString::Indicator);
    }
}

QSizeF ToolButtonElement::contentsSize(const QSizeF &contentsSizeFromStyle) const
{
    QSizeF size = applyPaddingToSize(contentsSizeFromStyle);
    size = size.expandedTo(menuButtonRect().size());
    return size;
}

QRectF ToolButtonElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qCWarning(UNION_QTWIDGETS) << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    QRectF backgroundRect = backgroundRectangle(m_toolButtonOption, m_backgroundProperties).toRect();
    if (subControl == QStyle::SC_ToolButton) {
        QRectF rect = m_toolButtonOption->rect;
        QRectF unifiedRect;
        for (const auto &m : m_layoutMap) {
            if (m.elementName == ElementString::Indicator) {
                unifiedRect = unifiedRect.united(menuButtonRect());
            } else {
                unifiedRect = unifiedRect.united(m.rect.toRect());
            }
        }
        rect = unifiedRect;
        return rect;
    }
    if (subControl == QStyle::SC_ToolButtonMenu) {
        return menuButtonRect();
    }
    return QRect();
}

void ToolButtonElement::draw(QPainter *painter, DrawEnums enums) const
{
    if (!m_isValid) {
        return;
    }

    switch (enums.ControlElement) {
    case QStyle::CE_ToolButtonLabel:
        drawText(painter);
        break;
    }

    switch (enums.ComplexControl) {
    case QStyle::CC_ToolButton:
        drawBackground(painter);
        drawIcon(painter);
        drawText(painter);
        drawIndicator(painter);
        break;
    }

    switch (enums.PrimitiveElement) {
    case QStyle::PE_FrameButtonTool:
        drawFrame(painter);
        break;
    case QStyle::PE_PanelButtonTool:
        drawBackground(painter);
        break;
    }
}

void ToolButtonElement::drawText(QPainter *painter) const
{
    if (m_toolButtonOption->toolButtonStyle == Qt::ToolButtonIconOnly) {
        return;
    }
    AbstractElement::drawText(painter);
}

void ToolButtonElement::drawIcon(QPainter *painter) const
{
    if (m_toolButtonOption->toolButtonStyle == Qt::ToolButtonTextOnly) {
        return;
    }

    QRectF iconRect = m_layoutMap[ElementString::Icon].rect;
    if (m_toolButtonOption->features.testFlag(QStyleOptionToolButton::Arrow)) {
        auto subopt = *m_toolButtonOption;
        subopt.rect = iconRect.toRect();
        switch (m_toolButtonOption->arrowType) {
        case Qt::LeftArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &subopt, painter);
            break;
        case Qt::RightArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &subopt, painter);
            break;
        case Qt::UpArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowUp, &subopt, painter);
            break;
        case Qt::DownArrow:
            m_style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &subopt, painter);
            break;
        default:
            break;
        }
        return;
    } else if (hasIcon()) {
        drawIconAtRect(painter, m_icon, iconRect);
    }
}

void ToolButtonElement::drawIndicator(QPainter *painter) const
{
    auto rect = subControlRect(QStyle::SC_ToolButtonMenu);
    auto indicatorRect = m_layoutMap[ElementString::Indicator].rect;
    indicatorRect.moveCenter(rect.center());
    drawBackgroundRectangle(painter, rect, m_indicatorProperties);
    drawIconAtRect(painter, m_indicator, indicatorRect);
}

QVariantMap ToolButtonElement::elementAttributes() const
{
    QVariantMap map;
    switch (m_toolButtonOption->toolButtonStyle) {
    case Qt::ToolButtonIconOnly:
        map[u"display"_s] = QVariant(u"icon-only"_s);
        break;
    case Qt::ToolButtonTextOnly:
        map[u"display"_s] = QVariant(u"text-only"_s);
        break;
    case Qt::ToolButtonTextBesideIcon:
        map[u"display"_s] = QVariant(u"text-beside-icon"_s);
        break;
    case Qt::ToolButtonTextUnderIcon:
        map[u"display"_s] = QVariant(u"text-under-icon"_s);
        break;
    default:
        return map;
    }
    return map;
}

QStringList ToolButtonElement::elementHints() const
{
    QStringList hints;
    if (m_toolButtonOption->features.testFlag(QStyleOptionToolButton::ToolButtonFeature::None)) {
        return hints;
    }
    if (m_toolButtonOption->features.testFlag(QStyleOptionToolButton::ToolButtonFeature::Menu)) {
        hints.append(u"with-menu"_s);
    }
    if (!m_toolButtonOption->state.testFlag(QStyle::State_AutoRaise)) {
        hints.append(u"raised"_s);
    }
    return hints;
}

QRectF ToolButtonElement::menuButtonRect() const
{
    if (!m_hasIndicator) {
        return QRectF();
    }

    QRectF menuRect = m_layoutMap[ElementString::Indicator].rect;
    QRectF buttonRect = m_toolButtonOption->rect;
    QRectF alignmentRect = m_layoutMap[ElementString::Icon].rect.united(m_layoutMap[ElementString::Text].rect);

    if (m_indicatorProperties && m_indicatorProperties->layout() && m_indicatorProperties->layout()->alignment()) {
        auto alignH = m_indicatorProperties->layout()->alignment()->horizontal().value_or(Union::Properties::Alignment::Unspecified);
        auto alignV = m_indicatorProperties->layout()->alignment()->vertical().value_or(Union::Properties::Alignment::Unspecified);

        switch (alignH) {
        case Union::Properties::Alignment::Start:
            menuRect.moveLeft(buttonRect.left());
            break;
        case Union::Properties::Alignment::Center:
            menuRect.moveCenter(QPoint(buttonRect.center().x(), menuRect.center().y()));
            break;
        case Union::Properties::Alignment::End:
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::StackCenter:
            menuRect.moveRight(buttonRect.right());
            break;
        case Union::Properties::Alignment::StackFill:
        case Union::Properties::Alignment::Fill:
            menuRect.setRight(buttonRect.right());
            menuRect.setLeft(buttonRect.left());
            break;
        }

        switch (alignV) {
        case Union::Properties::Alignment::Start:
            menuRect.moveTop(m_toolButtonOption->rect.top());
            menuRect.setBottom(alignmentRect.top());
            break;
        case Union::Properties::Alignment::Center:
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::StackCenter:
            menuRect.moveCenter(QPoint(menuRect.center().x(), m_toolButtonOption->rect.center().y()));
            break;
        case Union::Properties::Alignment::End:
            menuRect.moveBottom(m_toolButtonOption->rect.bottom());
            menuRect.setTop(alignmentRect.bottom());
            break;
        case Union::Properties::Alignment::Fill:
        case Union::Properties::Alignment::StackFill:
            menuRect.setTop(m_toolButtonOption->rect.top());
            menuRect.setBottom(m_toolButtonOption->rect.bottom());
            break;
        }
    }

    return menuRect;
}
