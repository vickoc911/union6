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

/**
 * Attached property for Positioner related information.
 */
class PositionerAttached : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
public:
    using QObject::QObject;

    Q_PROPERTY(PositionerSource::Source source READ source WRITE setSource BINDABLE bindableSource NOTIFY sourceChanged)
    PositionerSource::Source source() const;
    void setSource(PositionerSource::Source newSource);
    QBindable<PositionerSource::Source> bindableSource();
    Q_SIGNAL void sourceChanged();

private:
    Q_OBJECT_BINDABLE_PROPERTY_WITH_ARGS(PositionerAttached,
                                         PositionerSource::Source,
                                         m_source,
                                         PositionerSource::Source::Layout,
                                         &PositionerAttached::sourceChanged)
};

/**
 * An internal attached object that is used to perform the actual positioning of items.
 */
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
    QQuickItem *m_parentItem = nullptr;
    std::vector<QQuickItem *> m_items;
    bool m_layoutDirty = true;
};

/**
 * An item that will place its children according to the style-provided alignment.
 *
 * This item will place its children based on the alignment values of the style
 * rules that match the current control.
 *
 * The items will be separated into three containers: Item, Background and
 * Content. These containers determine what the item will be placed relative to,
 * with Item meaning they will be placed relative to the root item, Background
 * meaning they will be relative to the background and Content meaning they will
 * be relative to the content. The background container will account for inset
 * while the Content container will account for padding.
 *
 * Within each container, the items will be further sorted into several buckets,
 * based on their horizontal alignment, one for Start, one for End, one for
 * Center and one for Fill. The items will first be laid out according to the
 * item's size hints, which determines the size of the bucket. These buckets
 * will then be positioned based on their alignment, with Start being placed at
 * the start of the control, End being placed at the end, Center being placed at
 * the center and Fill taking the available width and dividing it equally
 * between its items.
 *
 * Note that the start and end buckets reduce the space that is available for
 * the center and fill buckets. In addition, anything placed in the start and
 * end buckets of the Item container will reduce the space available for the
 * Background and Content containers.
 */
class Positioner : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(PositionerAttached)

public:
    Positioner(QQuickItem *parentItem = nullptr);

    /**
     * The container item to use as root for the layout.
     */
    Q_PROPERTY(QQuickItem *container READ container WRITE setContainer NOTIFY containerChanged)
    QQuickItem *container() const;
    void setContainer(QQuickItem *newContainer);
    Q_SIGNAL void containerChanged();

    static PositionerAttached *qmlAttachedProperties(QObject *parent);

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void updatePolish() override;

private:
    QQuickItem *m_container = nullptr;
    PositionerContainer *m_containerAttached = nullptr;
};
