// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "ComboBoxElement.h"
#include "SharedNames.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>

using namespace Qt::StringLiterals;

ComboBoxElement::ComboBoxElement(const QStyleOptionComboBox *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_comboBoxOption(option)
    , m_spacing(0)
    , m_editable(false)
{
    update();
}

void ComboBoxElement::update()
{
    m_editable = m_comboBoxOption->editable;

    m_indicatorElementList = prepareElements(m_comboBoxOption, m_widget, {ElementString::Indicator});
    if (!m_indicatorElementList.isEmpty()) {
        m_indicatorProperties = queryProperties(m_indicatorElementList);
        if (m_indicatorProperties->icon()) {
            setIndicator(QIcon::fromTheme(m_indicatorProperties->icon()->name().value_or(QString())));
        }
        if (m_indicatorProperties->layout()) {
            m_spacing = m_indicatorProperties->layout()->spacing().value_or(1);
        }
    }

    if (!m_comboBoxOption->currentIcon.isNull()) {
        setIcon(m_comboBoxOption->currentIcon);
    }
    if (!m_comboBoxOption->currentText.isEmpty()) {
        setText(m_comboBoxOption->currentText);
    }

    updateSubElementList();
    layout();
}

ComboBoxElement::~ComboBoxElement()
{
}

bool ComboBoxElement::isEditable() const
{
    return m_editable;
}

void ComboBoxElement::draw(QPainter *painter) const
{
    if (!m_isValid) {
        return;
    }

    drawBackground(painter);
    drawIcon(painter);
    if (!m_editable) {
        drawText(painter);
    }
    drawIndicator(painter);
}

void ComboBoxElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Indicator);
    if (hasText()) {
        m_subElementList.append(ElementString::Text);
    }
    if (hasIcon()) {
        m_subElementList.append(ElementString::Icon);
    }
}

QSize ComboBoxElement::contentsSize(const QSize &contentsSizeFromStyle) const
{
    QRect unifiedRect;
    for (const auto &m : m_layoutMap) {
        unifiedRect = unifiedRect.united(m.rect.toRect().normalized());
    }
    // Follow the contents width
    unifiedRect.setWidth(contentsSizeFromStyle.width());
    auto size = applyPaddingToSize(unifiedRect.size());
    if (m_indicatorProperties && m_indicatorProperties->layout()) {
        size.rwidth() += m_indicatorProperties->layout()->spacing().value_or(20);
    }
    return size;
}

QRect ComboBoxElement::subControlRect(QStyle::SubControl subControl) const
{
    if (!m_isValid) {
        qWarning() << "subControlRect for " << subControl << "is not valid";
        return QRect();
    }

    switch (subControl) {
    case QStyle::SC_ComboBoxFrame:
        return backgroundRectangle(m_comboBoxOption, m_backgroundProperties).toRect();
    case QStyle::SC_ComboBoxListBoxPopup:
        return m_comboBoxOption->rect;

    case QStyle::SC_ComboBoxArrow: {
        auto map = layoutMap(m_backgroundElementList, m_comboBoxOption, {ElementString::Indicator});
        auto rect = map[ElementString::Indicator].rect;
        rect = rect.adjusted(-m_spacing, 0, m_spacing, 0);
        return m_style->visualRect(m_comboBoxOption->direction, m_comboBoxOption->rect, rect.toRect());
    }

    case QStyle::SC_ComboBoxEditField: {
        QRect labelRect;
        auto rect = m_comboBoxOption->rect;
        auto indicatorRect = subControlRect(QStyle::SC_ComboBoxArrow);
        labelRect = QRect(rect.left(), rect.top(), rect.width() - indicatorRect.width(), rect.height());
        // Add some spacing between the icon and text in edit field
        if (m_backgroundProperties->layout() && !m_comboBoxOption->currentIcon.isNull()) {
            auto spacing = m_backgroundProperties->layout()->spacing().value_or(5);
            labelRect.adjust(spacing, 0, spacing, 0);
        }
        return m_style->visualRect(m_comboBoxOption->direction, m_comboBoxOption->rect, labelRect);
    }

    default:
        break;
    }
    return QRect();
}
