// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "LineEditElement.h"
#include "StyleUtils.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>
#include <qstyle.h>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

LineEditElement::LineEditElement(const QStyleOptionFrame *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_frameOption(option)
{
    update();
}

LineEditElement::~LineEditElement()
{
}

void LineEditElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

QSizeF LineEditElement::iconSize() const
{
    return querySize({ElementString::LineEditIconSize});
}

void LineEditElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::TextField);
}

QMarginsF LineEditElement::iconPadding() const
{
    if (m_isValid && m_contentProperties->layout() && m_contentProperties->layout()->padding()) {
        return m_contentProperties->layout()->padding()->toMargins();
    }
    return QMarginsF();
}

QRectF LineEditElement::subElementRect(QStyle::SubElement element) const
{
    if (m_isValid && element == QStyle::SE_LineEditContents && m_backgroundProperties && m_backgroundProperties->layout()) {
        int frameWidth = m_style->pixelMetric(QStyle::PM_DefaultFrameWidth, m_frameOption, m_widget);
        return backgroundRectangle(m_frameOption, m_backgroundProperties).toRect().adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);
    }
    return QRectF();
}

QStringList LineEditElement::elementHints() const
{
    return frameHints(m_frameOption);
}
