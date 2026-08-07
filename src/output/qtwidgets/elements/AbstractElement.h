// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include "BackgroundDrawing.h"
#include "StyleUtils.h"
#include <QIcon>
#include <QObject>
#include <QStyleOption>

class UnionStyle;

class AbstractElement : public QObject, public std::enable_shared_from_this<AbstractElement>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<AbstractElement>;
    AbstractElement(const QStyleOption *option, const UnionStyle *style, const QWidget *widget = nullptr);
    ~AbstractElement() override;
    static AbstractElement::Ptr create(const QStyleOption *option, const UnionStyle *style, const QWidget *widget);

    QIcon icon() const;
    void setIcon(const QIcon &icon);
    bool hasIcon() const;

    QString text() const;
    void setText(const QString &text);
    bool hasText() const;

    bool isValid() const;

    virtual void draw(QPainter *painter) const;
    virtual void drawText(QPainter *painter) const;
    virtual void drawIcon(QPainter *painter) const;
    virtual void drawBg(QPainter *painter) const;
    virtual void layout();
    virtual QSize contentsSize(const QSize &contentsSizeFromStyle) const;
    virtual QRect subElementRect(QStyle::SubElement element) const;
    virtual QRect subControlRect(QStyle::ComplexControl complexControl, QStyle::SubControl subControl) const;

protected:
    const QStyleOption *m_styleOption;
    const UnionStyle *m_style;
    const QWidget *m_widget;
    QIcon m_icon;
    QString m_text;
    Union::ElementList m_backgroundElementList;
    Union::ElementList m_contentElementList;
    Union::Properties::StylePropertyGroup *m_backgroundProperties;
    Union::Properties::StylePropertyGroup *m_contentProperties;
    QMap<QString, LayoutItem> m_layoutMap;
    QStringList m_subElementList;

    virtual void updateSubElementList();

    QSize applyPaddingToSize(QSize oldSize) const;

    bool m_isValid;
};
