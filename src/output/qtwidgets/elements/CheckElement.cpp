// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "CheckElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyle>
#include <qstyle.h>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

CheckElement::CheckElement(Type type, const QStyleOptionButton *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_buttonOption(option)
    , m_type(type)
{
    update();
}

CheckElement::~CheckElement()
{
}

void CheckElement::update()
{
    if (m_type == CheckElement::Type::CheckBox) {
        m_indicatorElementList = prepareElements(m_styleOption, m_widget, {ElementString::CheckBox, ElementString::Indicator});
    } else {
        m_indicatorElementList = prepareElements(m_styleOption, m_widget, {ElementString::RadioButton, ElementString::Indicator});
    }
    if (!m_indicatorElementList.isEmpty()) {
        m_indicatorProperties = queryProperties(m_indicatorElementList);
    }

    setIcon(m_buttonOption->icon);
    setText(m_buttonOption->text);
    updateSubElementList();
    layout();
}

void CheckElement::draw(QPainter *painter, DrawEnums enums) const
{
    if (!m_isValid) {
        return;
    }

    switch (enums.ControlElement) {
    case QStyle::CE_RadioButton:
    case QStyle::CE_CheckBox:
        drawBackground(painter);
        drawText(painter);
        drawIndicator(painter);
        break;
    case QStyle::CE_RadioButtonLabel:
    case QStyle::CE_CheckBoxLabel:
        drawText(painter);
        break;
    }

    switch (enums.PrimitiveElement) {
    case QStyle::PE_IndicatorRadioButton:
    case QStyle::PE_IndicatorCheckBox:
        drawIndicator(painter);
        break;
    }
}

QSizeF CheckElement::contentsSize(const QSizeF &contentsSizeFromStyle) const
{
    auto size = applyPaddingToSize(contentsSizeFromStyle);
    // Ensure indicator size is taken into account with the label
    size.rwidth() += spacing() + indicatorSize().width();
    return size;
}

QRectF CheckElement::subElementRect(QStyle::SubElement element) const
{
    if (!m_isValid) {
        qCWarning(UNION_QTWIDGETS) << "Subelementrect for " << element << "is not valid";
        return QRect();
    }

    if (element == QStyle::SE_CheckBoxIndicator || element == QStyle::SE_RadioButtonIndicator) {
        if (m_buttonOption->styleObject) {
            // The indicator is drawn as part of something
            return m_layoutMap[ElementString::Indicator].rect;
        } else {
            // The indicator is drawn standalone (PE_IndicatorCheckBox for example)
            return m_buttonOption->rect;
        }
    }

    return unifiedRect(m_layoutMap);
}

void CheckElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Indicator);
    if (!m_buttonOption->icon.isNull()) {
        m_subElementList.append(ElementString::Icon);
    }
    if (!m_buttonOption->text.isEmpty()) {
        m_subElementList.append(ElementString::Text);
    }
}

void CheckElement::drawIndicator(QPainter *painter) const
{
    if (m_type == CheckElement::Type::CheckBox) {
        drawBackgroundRectangle(painter, subElementRect(QStyle::SE_CheckBoxIndicator), m_indicatorProperties);
    } else {
        drawBackgroundRectangle(painter, subElementRect(QStyle::SE_RadioButtonIndicator), m_indicatorProperties);
    }
}

QStringList CheckElement::elementHints() const
{
    QStringList hints;
    if (!m_buttonOption->icon.isNull()) {
        hints.append(u"with-icon"_s);
    }
    // Used for "partial" checkbox
    if (m_buttonOption->state.testFlag(QStyle::State_NoChange)) {
        hints.append(u"no-change"_s);
    }
    return hints;
}
