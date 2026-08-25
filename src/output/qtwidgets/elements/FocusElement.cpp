// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "FocusElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

FocusElement::FocusElement(const QStyleOptionFocusRect *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_focusOption(option)
{
    update();
}

FocusElement::~FocusElement()
{
}

void FocusElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    layout();
}

void FocusElement::layout()
{
    // Background and content is separate
    m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::FocusFrame});
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
    }
}

QVariantMap FocusElement::elementAttributes() const
{
    return QVariantMap();
}

QStringList FocusElement::elementHints() const
{
    QStringList hints;
    if (m_focusOption->state.testFlag(QStyle::State_FocusAtBorder)) {
        hints.append(u"focus-at-border"_s);
    }
    return hints;
}
