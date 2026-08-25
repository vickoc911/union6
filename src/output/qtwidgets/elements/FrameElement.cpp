// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "FrameElement.h"
#include "UnionStyle.h"
#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>

#include "SharedNames.h"

using namespace Qt::StringLiterals;

FrameElement::FrameElement(const QStyleOptionFrame *option, const UnionStyle *style, const QWidget *widget)
    : AbstractElement(option, style, widget)
    , m_frameOption(option)
{
    update();
}

FrameElement::~FrameElement()
{
}

void FrameElement::update()
{
    setIndicator(QIcon());
    setIcon(QIcon());
    setText(QString());
    updateSubElementList();
    layout();
}

void FrameElement::updateSubElementList()
{
    m_subElementList.clear();
    m_subElementList.append(ElementString::Frame);
}

QVariantMap FrameElement::elementAttributes() const
{
    return QVariantMap();
}

QStringList FrameElement::elementHints() const
{
    QStringList hints;
    if (m_frameOption->features.testFlag(QStyleOptionFrame::Flat)) {
        hints.append(u"flat"_s);
    }
    if (m_frameOption->features.testFlag(QStyleOptionFrame::Rounded)) {
        hints.append(u"rounded"_s);
    }
    switch (m_frameOption->frameShape) {
    case QFrame::NoFrame: {
        if (!hints.contains(u"flat"_s)) {
            hints.append(u"flat"_s);
        }
    }
    case QFrame::Box:
    case QFrame::Panel:
    case QFrame::WinPanel:
    case QFrame::HLine:
    case QFrame::VLine:
    case QFrame::StyledPanel:
        break;
    }
    return hints;
}
