// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "SizeGripElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>
#include <qstyleoption.h>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

SizeGripElement::SizeGripElement(const QStyleOptionSizeGrip *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_sizeGripOption(option)
{
    update();
}

SizeGripElement::~SizeGripElement()
{
}

void SizeGripElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    layout();
}

void SizeGripElement::layout()
{
    m_backgroundElementList = prepareElements(m_styleOption, m_widget, {ElementString::SizeGrip});
    if (!m_backgroundElementList.isEmpty()) {
        m_backgroundProperties = queryProperties(m_backgroundElementList);
        m_isValid = true;
    }
}
