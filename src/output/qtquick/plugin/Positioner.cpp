/*
 * SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Positioner.h"

#include "QuickStyle.h"

#include "qtquick_logging.h"

using namespace Union;

struct LayoutItem {
    QRectF geometry;
    Union::Properties::Alignment verticalAlignment = Union::Properties::Alignment::Unspecified;
    int order = 0;
    QMarginsF margins;
    QQuickItem *item = nullptr;
};

struct LayoutBucket {
    QRectF geometry;
    qreal spacing = 0.0;
    QList<LayoutItem> items;
};

struct LayoutContainer {
    QRectF geometry;

    LayoutBucket start;
    LayoutBucket center;
    LayoutBucket end;
    LayoutBucket fill;
};

void layoutBucket(LayoutBucket &bucket);

void layoutContainer(LayoutContainer &container)
{
    for (auto bucket : {&container.start, &container.end, &container.center, &container.fill}) {
        bucket->geometry.setY(container.geometry.y());
        bucket->geometry.setHeight(container.geometry.height());
        layoutBucket(*bucket);
    }

    QRectF placementRect = container.geometry;
    container.start.geometry.moveLeft(container.geometry.x());
    placementRect.setLeft(container.start.geometry.right());
    placementRect.setWidth(placementRect.width() - container.end.geometry.width());
    container.end.geometry.moveLeft(placementRect.right());
    container.center.geometry.moveLeft(placementRect.center().x() - container.center.geometry.width() / 2);
    container.fill.geometry = placementRect;

    if (container.fill.items.count() > 0) {
        auto x = 0.0;
        auto width = container.fill.geometry.width() / container.fill.items.count();
        for (auto &item : container.fill.items) {
            x += item.margins.left();
            item.geometry.moveLeft(x);
            item.geometry.setWidth(width);
            x += width + item.margins.right() + container.fill.spacing;
        }
    }
}

void layoutBucket(LayoutBucket &bucket)
{
    qreal x = 0.0;
    for (auto &item : bucket.items) {
        x += item.margins.left();
        item.geometry.moveLeft(x);
        x += item.geometry.width() + item.margins.right() + bucket.spacing;

        switch (item.verticalAlignment) {
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::Start:
            item.geometry.moveTop(0);
            break;
        case Union::Properties::Alignment::Center:
            item.geometry.moveTop(bucket.geometry.height() / 2 - item.geometry.height() / 2);
            break;
        case Union::Properties::Alignment::End:
            item.geometry.moveTop(bucket.geometry.height() - item.geometry.height());
            break;
        case Union::Properties::Alignment::Stack:
        case Union::Properties::Alignment::Fill:
            item.geometry.moveTop(0);
            item.geometry.setHeight(bucket.geometry.height());
            break;
        }
    }

    bucket.geometry.setWidth(x);
}

PositionerSource::Source PositionerAttached::source() const
{
    return m_source;
}

void PositionerAttached::setSource(PositionerSource::Source newSource)
{
    m_source = newSource;
}

Union::Properties::Alignment PositionerAttached::horizontalAlignment() const
{
    return m_horizontalAlignment;
}

void PositionerAttached::setHorizontalAlignment(Union::Properties::Alignment newAlignment)
{
    if (newAlignment == m_horizontalAlignment) {
        return;
    }

    m_horizontalAlignment = newAlignment;
    Q_EMIT horizontalAlignmentChanged();
}

void PositionerAttached::resetHorizontalAlignment()
{
    setHorizontalAlignment(Union::Properties::Alignment::Unspecified);
}

Union::Properties::Alignment PositionerAttached::verticalAlignment() const
{
    return m_verticalAlignment;
}

void PositionerAttached::setVerticalAlignment(Union::Properties::Alignment newAlignment)
{
    if (newAlignment == m_verticalAlignment) {
        return;
    }

    m_verticalAlignment = newAlignment;
    Q_EMIT verticalAlignmentChanged();
}

void PositionerAttached::resetVerticalAlignment()
{
    setVerticalAlignment(Union::Properties::Alignment::Unspecified);
}

PositionerContainer::PositionerContainer(QObject *parent)
    : QObject(parent)
{
    m_parentItem = qobject_cast<QQuickItem *>(parent);
    if (!m_parentItem) {
        qCWarning(UNION_QTQUICK) << "PositionerContainer attached to something that's not an Item, this won't work properly!";
    }

    connect(m_parentItem, &QQuickItem::widthChanged, this, [this]() {
        m_layoutDirty = true;
        m_parentItem->polish();
    });
    connect(m_parentItem, &QQuickItem::heightChanged, this, [this]() {
        m_layoutDirty = true;
        m_parentItem->polish();
    });
}

void PositionerContainer::addItem(QQuickItem *item)
{
    auto changeHandler = [this, item]() {
        m_layoutDirty = true;
        item->parentItem()->polish();
    };

    connect(item, &QQuickItem::implicitWidthChanged, this, changeHandler);
    connect(item, &QQuickItem::implicitHeightChanged, this, changeHandler);
    connect(item, &QQuickItem::visibleChanged, this, changeHandler);

    m_items.push_back(item);

    m_layoutDirty = true;
}

void PositionerContainer::removeItem(QQuickItem *item)
{
    auto itr = std::find(m_items.begin(), m_items.end(), item);
    if (itr == m_items.end()) {
        return;
    }

    (*itr)->disconnect(this);
    m_items.erase(itr);
    m_layoutDirty = true;
}

void PositionerContainer::layout()
{
    if (!m_layoutDirty) {
        return;
    }

    LayoutContainer itemRelative;
    LayoutContainer contentRelative;
    LayoutContainer backgroundRelative;

    for (auto &item : m_items) {
        auto source = PositionerSource::Source::Layout;

        if (!item->isVisible()) {
            continue;
        }

        auto positionerAttached = qobject_cast<PositionerAttached *>(qmlAttachedPropertiesObject<Positioner>(item, false));
        if (positionerAttached) {
            source = positionerAttached->source();
        }

        auto styleAttached = qobject_cast<QuickStyle *>(qmlAttachedPropertiesObject<QuickStyle>(item, true));
        AlignmentPropertyGroup *alignment = nullptr;
        switch (source) {
        case PositionerSource::Source::Layout:
            alignment = styleAttached->properties()->layout()->alignment();
            break;
        case PositionerSource::Source::Icon:
            alignment = styleAttached->properties()->icon()->alignment();
            break;
        case PositionerSource::Source::Text:
            alignment = styleAttached->properties()->text()->alignment();
            break;
        }

        if (!alignment) {
            continue;
        }

        auto horizontalAlignment = alignment->horizontal();
        auto verticalAlignment = alignment->vertical();

        if (positionerAttached) {
            if (positionerAttached->horizontalAlignment() != Union::Properties::Alignment::Unspecified) {
                horizontalAlignment = positionerAttached->horizontalAlignment();
            }

            if (positionerAttached->verticalAlignment() != Union::Properties::Alignment::Unspecified) {
                verticalAlignment = positionerAttached->verticalAlignment();
            }
        }

        LayoutItem layoutItem{
            .geometry = QRectF{0, 0, item->implicitWidth(), item->implicitHeight()},
            .verticalAlignment = verticalAlignment,
            .order = alignment->order(),
            .margins = QMarginsF{},
            .item = item,
        };

        if (source == PositionerSource::Source::Layout) {
            auto margins = styleAttached->properties()->layout()->margins();
            layoutItem.margins = QMarginsF(margins->left(), margins->top(), margins->right(), margins->bottom());
        }

        LayoutContainer *container = &itemRelative;
        switch (alignment->container()) {
        case Union::Properties::AlignmentContainer::Content:
            container = &contentRelative;
            break;
        case Union::Properties::AlignmentContainer::Background:
            container = &backgroundRelative;
            break;
        case Union::Properties::AlignmentContainer::Item:
            break;
        }

        switch (horizontalAlignment) {
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::Start:
            container->start.items.append(layoutItem);
            break;
        case Union::Properties::Alignment::Center:
            container->center.items.append(layoutItem);
            break;
        case Union::Properties::Alignment::End:
            container->end.items.append(layoutItem);
            break;
        case Union::Properties::Alignment::Stack:
        case Union::Properties::Alignment::Fill:
            container->fill.items.append(layoutItem);
            break;
        }
    }

    auto styleAttached = qobject_cast<QuickStyle *>(qmlAttachedPropertiesObject<QuickStyle>(m_parentItem, true));
    qreal spacing = styleAttached->properties()->layout()->spacing();

    auto sort = [](auto &container) {
        std::stable_sort(container.begin(), container.end(), [](auto first, auto second) {
            return first.order < second.order;
        });
    };

    for (auto container : {&itemRelative, &contentRelative, &backgroundRelative}) {
        container->start.spacing = spacing;
        container->center.spacing = spacing;
        container->end.spacing = spacing;
        container->fill.spacing = spacing;

        sort(container->start.items);
        sort(container->center.items);
        sort(container->end.items);
        sort(container->fill.items);
    }

    itemRelative.geometry = m_parentItem->boundingRect();

    layoutContainer(itemRelative);
    QRectF remaining = itemRelative.geometry.adjusted(itemRelative.start.geometry.width(), 0.0, -itemRelative.end.geometry.width(), 0.0);

    auto inset = styleAttached->properties()->layout()->inset();
    backgroundRelative.geometry = remaining.adjusted(inset->left(), inset->top(), -inset->right(), -inset->bottom());
    layoutContainer(backgroundRelative);

    auto padding = styleAttached->properties()->layout()->padding();
    contentRelative.geometry = remaining.adjusted(padding->left(), padding->top(), -padding->right(), -padding->bottom());
    layoutContainer(contentRelative);

    for (auto container : {&itemRelative, &backgroundRelative, &contentRelative}) {
        for (auto bucket : {&container->start, &container->end, &container->center, &container->fill}) {
            auto geometry = bucket->geometry;
            for (auto item : bucket->items) {
                auto position = QPointF(geometry.left() + item.geometry.left(), geometry.top() + item.geometry.top());
                auto mapped = m_parentItem->mapToItem(item.item->parentItem(), position);
                item.item->setX(std::round(mapped.x()));
                item.item->setY(std::round(mapped.y()));
                item.item->setWidth(std::round(item.geometry.width()));
                item.item->setHeight(std::round(item.geometry.height()));
            }
        }
    }
}

PositionerContainer *PositionerContainer::qmlAttachedProperties(QObject *parent)
{
    if (!qobject_cast<QQuickItem *>(parent)) {
        qCWarning(UNION_QTQUICK) << "Cannot attach PositionerContainer to an object that is not a QQuickItem";
        return nullptr;
    }

    return new PositionerContainer(parent);
}

Positioner::Positioner(QQuickItem *parentItem)
    : QQuickItem(parentItem)
{
    m_containerNotifier = m_container.addNotifier([this]() {
        m_containerAttached = qobject_cast<PositionerContainer *>(qmlAttachedPropertiesObject<PositionerContainer>(m_container, true));
        const auto items = childItems();
        for (const auto &child : items) {
            if (m_containerAttached) {
                m_containerAttached->addItem(child);
            }
        }
        polish();
    });
}

QQuickItem *Positioner::container() const
{
    return m_container;
}

void Positioner::setContainer(QQuickItem *newContainer)
{
    m_container = newContainer;
}

QBindable<QQuickItem *> Positioner::bindableContainer()
{
    return QBindable<QQuickItem *>(&m_container);
}

PositionerAttached *Positioner::qmlAttachedProperties(QObject *parent)
{
    return new PositionerAttached(parent);
}

void Positioner::itemChange(ItemChange change, const ItemChangeData &data)
{
    if ((change != ItemChildAddedChange && change != ItemChildRemovedChange) || !m_containerAttached) {
        QQuickItem::itemChange(change, data);
        return;
    }

    if (change == ItemChildAddedChange) {
        m_containerAttached->addItem(data.item);
    } else {
        m_containerAttached->removeItem(data.item);
    }

    polish();
}

void Positioner::updatePolish()
{
    if (m_containerAttached) {
        m_containerAttached->layout();
    }
}

PositionedItem::PositionedItem(QQuickItem *parentItem)
    : QQuickItem(parentItem)
{
}

QQuickItem *PositionedItem::container() const
{
    return m_container;
}

void PositionedItem::setContainer(QQuickItem *newContainer)
{
    if (newContainer == m_container) {
        return;
    }

    auto newContainerAttached = qobject_cast<PositionerContainer *>(qmlAttachedPropertiesObject<PositionerContainer>(newContainer, true));
    if (newContainerAttached == m_containerAttached) {
        return;
    }

    if (m_containerAttached) {
        m_containerAttached->removeItem(this);
    }

    m_container = newContainer;
    m_containerAttached = newContainerAttached;
    if (m_containerAttached) {
        m_containerAttached->addItem(this);
    }
    polish();
}

void PositionedItem::itemChange(ItemChange change, const ItemChangeData &data)
{
    if (change == ItemChildAddedChange) {
        connect(data.item, &QQuickItem::implicitWidthChanged, this, &PositionedItem::updateImplicitSize);
        connect(data.item, &QQuickItem::implicitHeightChanged, this, &PositionedItem::updateImplicitSize);
        updateImplicitSize();
    }

    if (change == ItemChildRemovedChange) {
        disconnect(data.item, &QQuickItem::implicitWidthChanged, this, &PositionedItem::updateImplicitSize);
        disconnect(data.item, &QQuickItem::implicitHeightChanged, this, &PositionedItem::updateImplicitSize);
        updateImplicitSize();
    }

    QQuickItem::itemChange(change, data);
}

void PositionedItem::updatePolish()
{
    if (m_containerAttached) {
        m_containerAttached->layout();
    }
}

void PositionedItem::updateImplicitSize()
{
    qreal width = 0.0;
    qreal height = 0.0;

    const auto children = childItems();
    for (auto child : children) {
        width = std::max(width, child->implicitWidth());
        height = std::max(height, child->implicitHeight());
    }
    setImplicitSize(width, height);
}
