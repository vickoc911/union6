// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "MenuItemElement.h"
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
    , m_shortcutText(QString())
{
    if (m_menuItemOption) {
        m_isSeparator = (m_menuItemOption->menuItemType == QStyleOptionMenuItem::Separator);
        m_hasSubMenu = (m_menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu);

        if (m_hasSubMenu) {
            m_indicatorElementList = prepareElements(m_menuItemOption, widget, {u"MenuItem"_s});
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
}

void MenuItemElement::layout()
{
    // Background and content is separate
    if (m_backgroundElementList.isEmpty()) {
        m_backgroundElementList = prepareElements(m_styleOption, m_widget);
    }
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
    }

    m_contentElementList = prepareElements(m_styleOption, m_widget, {u"MenuItem"_s});
    if (!m_contentElementList.isEmpty()) {
        m_contentProperties = queryProperties(m_contentElementList);
    }

    QStringList subElements;
    QString itemText = text();
    if (hasIcon()) {
        subElements.append(u"Icon"_s);
    }
    if (hasText()) {
        subElements.append(u"Text"_s);
        const int tabPosition(itemText.indexOf(QLatin1Char('\t')));
        if (tabPosition >= 0) {
            subElements.append(u"ShortcutText"_s);
            m_shortcutText = itemText.mid(tabPosition + 1);
            m_text = itemText.left(tabPosition);
        }
    }
    if (hasIndicator()) {
        subElements.append(u"Arrow"_s);
    }
    if (m_isSeparator) {
        subElements.append(u"Separator"_s);
    }
    if (subElements.empty()) {
        m_isValid = false;
        return;
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
            auto separatorProps = queryProperties(prepareElements(m_menuItemOption, m_widget, {u"MenuSeparator"_s}));
            if (separatorProps->layout()) {
                int width = separatorProps->layout()->width().value_or(1);
                int height = separatorProps->layout()->height().value_or(1);
                QSize separatorSize(width, height);
                return applyPaddingToSize(separatorSize);
            }
        } else {
            if (m_contentProperties->layout()) {
                int width = m_contentProperties->layout()->width().value_or(1);
                int height = m_contentProperties->layout()->height().value_or(1);
                if (minimumSize.width() > width) {
                    width = minimumSize.width();
                }
                if (minimumSize.height() > height) {
                    height = minimumSize.height();
                }
                QSize itemSize(width, height);
                return applyPaddingToSize(itemSize);
            }
        }
    }
    return minimumSize;
}

MenuItemElement::Ptr MenuItemElement::create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
{
    return std::make_shared<MenuItemElement>(option, style, widget);
}

void MenuItemElement::drawBg(QPainter *painter) const
{
    if (m_isSeparator) {
        drawElementBackground(painter, m_menuItemOption, m_widget, {u"MenuSeparator"_s});
    } else {
        drawElementBackground(painter, m_menuItemOption, m_widget, {u"MenuItem"_s});
    }
}

void MenuItemElement::drawText(QPainter *painter) const
{
    int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
    const bool enabled = m_styleOption->state.testFlag(QStyle::State_Enabled);
    if (hasText()) {
        QRect textRect = m_layoutMap[u"Text"_s].rect.toRect();
        QColor color = m_styleOption->palette.text().color();
        // TODO: hide mnemonics if requested
        if (m_contentProperties->text()) {
            auto textColor = m_contentProperties->text()->color();
            if (textColor) {
                color = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(m_contentProperties, true);
        }
        painter->save();
        painter->setPen(color);
        m_style->drawItemText(painter, textRect, textFlags, m_styleOption->palette, enabled, m_text);
        painter->restore();
    }
    // ShortcutText is just like a regular text element but handled with different name
    // and has different coloration, so override the default colors
    if (!m_shortcutText.isEmpty()) {
        auto shortcutElements = prepareElements(m_styleOption, m_widget, {u"MenuItem"_s, u"ShortcutText"_s});
        const auto properties = queryProperties(shortcutElements);
        auto map = layoutMap(m_contentElementList, m_styleOption, {u"ShortcutText"_s});
        QRect textRect = map[u"ShortcutText"_s].rect.toRect();
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
        QRect indicatorRect = m_layoutMap[u"Arrow"_s].rect.toRect();
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
