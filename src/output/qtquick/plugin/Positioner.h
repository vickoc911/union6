/*
 * SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>
#include <QProperty>
#include <QQuickItem>
#include <qqmlregistration.h>

#include <PropertiesTypes.h>
#include <properties/StyleProperty.h>

#include "properties/AlignmentPropertyGroup.h"

class QuickElement;
class QuickStyle;

namespace PositionerSource
{
Q_NAMESPACE
QML_ELEMENT

/**
 * Indicates what property to use for alignment information.
 */
enum class Source {
    Layout, ///< Read from the Layout property.
    Text, ///< Read from the Text property.
    Icon ///< Read from the Icon property.
};
Q_ENUM_NS(Source)

}

class PositionerAttached : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
public:
    using QObject::QObject;

    Q_PROPERTY(PositionerSource::Source source READ source WRITE setSource NOTIFY sourceChanged)
    PositionerSource::Source source() const;
    void setSource(PositionerSource::Source newSource);
    Q_SIGNAL void sourceChanged();

    Q_PROPERTY(Union::Properties::Alignment horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment RESET resetHorizontalAlignment //
                   NOTIFY horizontalAlignmentChanged)
    Union::Properties::Alignment horizontalAlignment() const;
    void setHorizontalAlignment(Union::Properties::Alignment newAlignment);
    void resetHorizontalAlignment();
    Q_SIGNAL void horizontalAlignmentChanged();

    Q_PROPERTY(Union::Properties::Alignment verticalAlignment READ verticalAlignment WRITE setVerticalAlignment RESET resetVerticalAlignment //
                   NOTIFY verticalAlignmentChanged)
    Union::Properties::Alignment verticalAlignment() const;
    void setVerticalAlignment(Union::Properties::Alignment newAlignment);
    void resetVerticalAlignment();
    Q_SIGNAL void verticalAlignmentChanged();

private:
    PositionerSource::Source m_source = PositionerSource::Source::Layout;
    Union::Properties::Alignment m_horizontalAlignment = Union::Properties::Alignment::Unspecified;
    Union::Properties::Alignment m_verticalAlignment = Union::Properties::Alignment::Unspecified;
};

class PositionerContainer : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Container)
    QML_ATTACHED(PositionerContainer)

public:
    PositionerContainer(QObject *parent = nullptr);

    void addItem(QQuickItem *item);
    void removeItem(QQuickItem *item);

    void layout();

    static PositionerContainer *qmlAttachedProperties(QObject *parent);

private:
    // struct PositionerItemBinding {
    //     QQuickItem *item = nullptr;
    //     QPropertyNotifier widthObserver;
    //     QPropertyNotifier heightObserver;
    //     QMetaObject::Connection visibleObserver;
    // };

    QQuickItem *m_parentItem = nullptr;
    // std::vector<PositionerItemBinding> m_items;
    std::vector<QQuickItem *> m_items;
    bool m_layoutDirty = true;
};

class Positioner : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(PositionerAttached)

public:
    Positioner(QQuickItem *parentItem = nullptr);

    Q_PROPERTY(QQuickItem *container READ container WRITE setContainer BINDABLE bindableContainer NOTIFY containerChanged)
    QQuickItem *container() const;
    void setContainer(QQuickItem *newContainer);
    QBindable<QQuickItem *> bindableContainer();
    Q_SIGNAL void containerChanged();

    static PositionerAttached *qmlAttachedProperties(QObject *parent);

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void updatePolish() override;

private:
    Q_OBJECT_BINDABLE_PROPERTY(Positioner, QQuickItem *, m_container, &Positioner::containerChanged)
    PositionerContainer *m_containerAttached = nullptr;
    QPropertyNotifier m_containerNotifier;
};

class PositionedItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    PositionedItem(QQuickItem *parentItem = nullptr);

    Q_PROPERTY(QQuickItem *container READ container WRITE setContainer NOTIFY containerChanged)
    QQuickItem *container() const;
    void setContainer(QQuickItem *newContainer);
    Q_SIGNAL void containerChanged();

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void updatePolish() override;

private:
    void updateImplicitSize();

    QQuickItem *m_container = nullptr;
    PositionerContainer *m_containerAttached = nullptr;
};
