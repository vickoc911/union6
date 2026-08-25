// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "TreeViewElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

TreeViewElement::TreeViewElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_treeViewOption(option)
{
}

TreeViewElement::~TreeViewElement()
{
}

void TreeViewElement::drawIndicatorBranch(QPainter *painter) const
{
    // For some reason treeview related items can have null styleoption :(
    if (!m_treeViewOption) {
        return;
    }
    if (!m_treeViewOption->state.testFlag(QStyle::State_Children)) {
        return;
    }
    auto defaultIconName = QString();
    if (m_treeViewOption->state.testFlag(QStyle::State_Item)) {
        defaultIconName = u"arrow-right-symbolic"_s;
    }
    if (m_treeViewOption->state.testFlag(QStyle::State_Open)) {
        defaultIconName = u"arrow-down-symbolic"_s;
    }

    QSizeF size(1, 1);
    auto elements = prepareElements(m_treeViewOption, m_widget, {ElementString::TreeViewDelegate, ElementString::Indicator});
    auto properties = queryProperties(elements);
    auto name = defaultIconName;
    if (properties) {
        if (properties->icon()) {
            name = properties->icon()->name().value_or(name);
        }
        if (properties->layout()) {
            size = QSizeF(properties->layout()->width().value_or(0), properties->layout()->height().value_or(0));
        }
    }

    auto icon = QIcon::fromTheme(name);
    const bool enabled = m_treeViewOption->state.testFlag(QStyle::State_Enabled);

    const QPalette activePalette = m_treeViewOption->palette;
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
    auto iconSize = m_treeViewOption->rect.size();
    const QPixmap pixmap = icon.pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

    QColor penColor = m_treeViewOption->palette.text().color(); // Use text color as fallback
    if (properties && properties->icon() && properties->icon()->color()) {
        penColor = properties->icon()->color()->toQColor();
    }
    painter->save();
    painter->setPen(penColor);
    auto rect = centerRect(m_treeViewOption->rect, size.width(), size.height());
    m_style->drawItemPixmap(painter, rect.toRect(), Qt::AlignCenter, pixmap);
    painter->restore();
}

qreal TreeViewElement::indentation() const
{
    auto elements = prepareElements(m_styleOption, m_widget, {ElementString::TreeViewDelegate, ElementString::Indentation});
    if (elements.isEmpty()) {
        return 0;
    }
    auto properties = queryProperties(elements);
    if (properties && properties->layout()) {
        return properties->layout()->width().value_or(0);
    }
    return 0;
}
