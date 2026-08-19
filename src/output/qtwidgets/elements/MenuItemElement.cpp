// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "MenuItemElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

MenuItemElement::MenuItemElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_menuItemOption(qstyleoption_cast<const QStyleOptionMenuItem *>(option))
    , m_isSeparator(false)
    , m_hasSubMenu(false)
    , m_hasCheckBox(false)
    , m_hasRadioButton(false)
    , m_shortcutText(QString())
{
    if (m_menuItemOption) {
        m_isSeparator = (m_menuItemOption->menuItemType == QStyleOptionMenuItem::Separator);
        m_hasSubMenu = (m_menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu);
        m_hasCheckBox = (m_menuItemOption->checkType == QStyleOptionMenuItem::NonExclusive);
        m_hasRadioButton = (m_menuItemOption->checkType == QStyleOptionMenuItem::Exclusive);

        if (m_hasSubMenu) {
            m_indicatorElementList = prepareElements(m_menuItemOption, widget, {ElementString::MenuItem});
            m_indicatorProperties = queryProperties(m_indicatorElementList);
            if (m_indicatorProperties->layout() && m_indicatorProperties->icon()) {
                setIndicator(QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(u"arrow-right-symbolic"_s)));
            }
        }

        if (!m_menuItemOption->icon.isNull()) {
            setIcon(m_menuItemOption->icon);
        }
        if (!m_menuItemOption->text.isEmpty()) {
            setText(m_menuItemOption->text);
        }
    }
    updateSubElementList();
    layout();
}

MenuItemElement::~MenuItemElement()
{
}

void MenuItemElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }
    drawBg(painter);
    drawIcon(painter);
    drawText(painter);
    drawIndicator(painter);
    if (m_hasCheckBox || m_hasRadioButton) {
        QStyleOptionButton button;
        button.initFrom(m_widget);
        button.rect = m_layoutMap[ElementString::Indicator].rect.toRect();
        button.state = m_menuItemOption->state;
        button.state.setFlag(QStyle::State_On, m_menuItemOption->checked);
        drawElementBackground(painter, &button, m_widget, {m_hasCheckBox ? ElementString::CheckBox : ElementString::RadioButton, ElementString::Indicator});
    }
}

void MenuItemElement::updateSubElementList()
{
    m_subElementList.clear();
    if (m_menuItemOption) {
        if (!m_menuItemOption->text.isEmpty()) {
            m_subElementList.append(ElementString::Text);
        }
        if (!m_menuItemOption->icon.isNull()) {
            m_subElementList.append(ElementString::Icon);
        }
        if (m_menuItemOption->menuHasCheckableItems) {
            m_subElementList.append(ElementString::CheckBox);
        }
        if (m_menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu) {
            m_subElementList.append(ElementString::Arrow);
        }
        if (m_menuItemOption->menuItemType == QStyleOptionMenuItem::Separator) {
            m_subElementList.append(ElementString::MenuSeparator);
        }
        if (m_menuItemOption->checkType != QStyleOptionMenuItem::NotCheckable) {
            m_subElementList.append(ElementString::Indicator);
        }
    }
}

void MenuItemElement::layout()
{
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::Menu});
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
    }

    if (m_isSeparator) {
        m_contentElementList = prepareElements(m_styleOption, m_widget, {ElementString::MenuSeparator});

    } else {
        m_contentElementList = prepareElements(m_styleOption, m_widget, {ElementString::MenuItem});
    }
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
    }

    QStringList subElements;
    QString itemText = text();
    if (m_isSeparator) {
        if (hasText()) {
            subElements.append(ElementString::Text);
        }
    } else {
        if (m_hasCheckBox || m_hasRadioButton) {
            subElements.append(ElementString::Indicator);
        }
        if (hasIcon()) {
            subElements.append(ElementString::Icon);
        }
        if (hasText()) {
            subElements.append(ElementString::Text);
            const int tabPosition(itemText.indexOf(QLatin1Char('\t')));
            if (tabPosition >= 0) {
                subElements.append(ElementString::ShortcutText);
                m_shortcutText = itemText.mid(tabPosition + 1);
                m_text = itemText.left(tabPosition);
            }
        }
        if (hasIndicator()) {
            subElements.append(ElementString::Arrow);
        }

        if (subElements.empty()) {
            m_isValid = false;
            return;
        }
    }

    m_layoutMap = layoutMap(m_contentElementList, m_styleOption, subElements);
    m_isValid = true;
}

QSize MenuItemElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    QSize minimumSize = contentsSizeFromStyle;
    // Handle separator separately (pun not intended)
    if (m_menuItemOption) {
        if (m_menuItemOption->menuItemType == QStyleOptionMenuItem::Separator) {
            if (m_contentProperties->layout()) {
                int width = m_contentProperties->layout()->width().value_or(1);
                int height = m_contentProperties->layout()->height().value_or(1);
                if (hasText()) {
                    if (minimumSize.width() > width) {
                        width = minimumSize.width();
                    }
                    if (minimumSize.height() > height) {
                        height = minimumSize.height();
                    }
                }
                QSize separatorSize(width, height);
                return applyPaddingToSize(separatorSize);
            }
        } else {
            if (m_contentProperties->layout()) {
                int width = m_contentProperties->layout()->width().value_or(1);
                int height = m_contentProperties->layout()->height().value_or(1);
                int spacing = m_backgroundProperties->layout()->spacing().value_or(0);
                if (minimumSize.width() > width) {
                    width = minimumSize.width();
                }
                if (minimumSize.height() > height) {
                    height = minimumSize.height();
                }
                QSize itemSize(width, height);
                itemSize.rwidth() += m_menuItemOption->maxIconWidth + spacing;
                if (m_menuItemOption->menuHasCheckableItems) {
                    const bool exclusive = (m_menuItemOption->checkType == QStyleOptionMenuItem::Exclusive);
                    itemSize.rwidth() +=
                        m_style->pixelMetric(exclusive ? QStyle::PM_ExclusiveIndicatorWidth : QStyle::PM_IndicatorWidth, m_menuItemOption, m_widget) + spacing;
                }
                return applyPaddingToSize(itemSize);
            }
        }
    }
    return minimumSize;
}

void MenuItemElement::drawBg(QPainter *painter) const
{
    if (m_isSeparator) {
        drawElementBackground(painter, m_menuItemOption, m_widget, {ElementString::MenuSeparator});
    } else {
        drawElementBackground(painter, m_menuItemOption, m_widget, {ElementString::MenuItem});
    }
}

void MenuItemElement::drawText(QPainter *painter) const
{
    int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
    if (hasText()) {
        AbstractElement::drawText(painter);
    }
    // ShortcutText is just like a regular text element but handled with different name
    // and has different coloration, so override the default colors
    if (!m_shortcutText.isEmpty()) {
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
        auto shortcutElements = prepareElements(m_styleOption, m_widget, {ElementString::MenuItem, ElementString::ShortcutText});
        const auto properties = queryProperties(shortcutElements);
        auto map = layoutMap(m_contentElementList, m_styleOption, {ElementString::ShortcutText});
        QRect textRect = map[ElementString::ShortcutText].rect.toRect();
        QColor shortcutColor = m_styleOption->palette.text().color();
        if (properties->text() && properties->text()->color().has_value()) {
            shortcutColor = properties->text()->color()->toQColor();
        }
        textFlags = textFlagsFromProperties(properties, true);
        painter->save();
        painter->setPen(shortcutColor);
        m_style->drawItemText(painter, textRect, textFlags, m_styleOption->palette, enabled, m_shortcutText);
        painter->restore();
    }
}

void MenuItemElement::drawIndicator(QPainter *painter) const
{
    if (hasIndicator()) {
        QRect indicatorRect = m_layoutMap[ElementString::Arrow].rect.toRect();
        const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);

        const QPalette activePalette = m_styleOption->palette;
        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
        auto iconSize = indicatorRect.size();
        const QPixmap pixmap = indicator().pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

        QColor penColor = m_styleOption->palette.text().color(); // Use text color as fallback
        if (m_indicatorProperties->icon() && m_indicatorProperties->icon()->color().has_value()) {
            auto iconColor = m_indicatorProperties->icon()->color();
            penColor = iconColor->toQColor();
        }

        painter->save();
        painter->setPen(penColor);
        m_style->drawItemPixmap(painter, indicatorRect, Qt::AlignCenter, pixmap);
        painter->restore();
    }
}
