/*
 * SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Positioner.h"

#include "QuickStyle.h"

#include "qtquick_logging.h"

using namespace Union;

PositionerSource::Source PositionerAttached::source() const
{
    return m_source;
}

void PositionerAttached::setSource(PositionerSource::Source newSource)
{
    m_source = newSource;
}

QBindable<PositionerSource::Source> PositionerAttached::bindableSource()
{
    return QBindable<PositionerSource::Source>(&m_source);
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
    auto &binding = m_items.emplace_back(item);
    binding.widthObserver = item->bindableWidth().addNotifier([this]() {
        m_layoutDirty = true;
        m_parentItem->polish();
    });
    binding.heightObserver = item->bindableHeight().addNotifier([this]() {
        m_layoutDirty = true;
        m_parentItem->polish();
    });

    m_layoutDirty = true;
}

void PositionerContainer::removeItem(QQuickItem *item)
{
    m_items.erase(std::remove_if(m_items.begin(),
                                 m_items.end(),
                                 [item](const auto &entry) {
                                     return entry.item == item;
                                 }),
                  m_items.end());
    m_layoutDirty = true;
}

void PositionerContainer::layout()
{
    if (!m_layoutDirty) {
        return;
    }

    LayoutList itemRelative;
    LayoutList contentRelative;
    LayoutList backgroundRelative;

    for (auto &item : m_items) {
        auto source = PositionerSource::Source::Layout;

        auto positionerAttached = qobject_cast<PositionerAttached *>(qmlAttachedPropertiesObject<Positioner>(item.item, false));
        if (positionerAttached) {
            source = positionerAttached->source();
        }

        qDebug() << source;

        auto styleAttached = qobject_cast<QuickStyle *>(qmlAttachedPropertiesObject<QuickStyle>(item.item, true));
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

        switch (alignment->container()) {
        case Union::Properties::AlignmentContainer::Item:
            itemRelative.append(std::make_pair(item.item, alignment));
            break;
        case Union::Properties::AlignmentContainer::Content:
            contentRelative.append(std::make_pair(item.item, alignment));
            break;
        case Union::Properties::AlignmentContainer::Background:
            backgroundRelative.append(std::make_pair(item.item, alignment));
            break;
        }
    }

    auto styleAttached = qobject_cast<QuickStyle *>(qmlAttachedPropertiesObject<QuickStyle>(m_parentItem, true));
    qreal spacing = styleAttached->properties()->layout()->spacing();

    QRectF result = layoutItems(m_parentItem->boundingRect(), spacing, itemRelative);

    QRectF backgroundBounds = result;
    auto inset = styleAttached->properties()->layout()->inset();
    backgroundBounds.adjust(-inset->left(), -inset->top(), -inset->right(), -inset->bottom());
    layoutItems(backgroundBounds, spacing, backgroundRelative);

    QRectF contentBounds = result;
    auto padding = styleAttached->properties()->layout()->padding();
    contentBounds.adjust(-padding->left(), -padding->top(), -padding->right(), -padding->bottom());
    layoutItems(contentBounds, spacing, contentRelative);

    m_layoutDirty = false;
}

PositionerContainer *PositionerContainer::qmlAttachedProperties(QObject *parent)
{
    if (!qobject_cast<QQuickItem *>(parent)) {
        qCWarning(UNION_QTQUICK) << "Cannot attach PositionerContainer to an object that is not a QQuickItem";
        return nullptr;
    }

    return new PositionerContainer(parent);
}

QRectF PositionerContainer::layoutItems(const QRectF &bounds, qreal spacing, const LayoutList &items)
{
    if (items.isEmpty()) {
        return bounds;
    }

    LayoutList start;
    LayoutList center;
    LayoutList end;
    LayoutList fill;
    for (auto entry : items) {
        switch (entry.second->horizontal()) {
        case Union::Properties::Alignment::Start:
            start.append(entry);
            break;
        case Union::Properties::Alignment::Center:
            center.append(entry);
            break;
        case Union::Properties::Alignment::End:
            end.append(entry);
            break;
        case Union::Properties::Alignment::Fill:
            fill.append(entry);
            break;
        }
    }

    auto compare = [](auto first, auto second) {
        return first.second->order() < second.second->order();
    };
    std::stable_sort(start.begin(), start.end(), compare);
    std::stable_sort(center.begin(), center.end(), compare);
    std::stable_sort(end.begin(), end.end(), compare);
    std::stable_sort(fill.begin(), fill.end(), compare);

    auto setPosition = [this](auto item, auto x, auto y) {
        auto mapped = m_parentItem->mapToItem(item, x, y);
        item->setX(mapped.x());
        item->setY(mapped.y());
    };

    auto setY = [this](auto bounds, auto x, auto item, auto alignment) {
        switch (alignment->vertical()) {
        case Union::Properties::Alignment::Start:
            item->setY(bounds.y());
            break;
        case Union::Properties::Alignment::Center: {
            auto y = bounds.y() + (bounds.height() - item->height()) / 2;
            auto mapped = m_parentItem->mapToItem(item, x, y);
            item->setX(mapped.x());
            item->setY(mapped.y());
            break;
        }
        case Union::Properties::Alignment::End:
            item->setY(bounds.y() + bounds.height() - item->height());
            break;
        case Union::Properties::Alignment::Fill:
            item->setY(bounds.y());
            item->setHeight(bounds.height());
            break;
        }
    };

    QRectF resultBounds = bounds;

    for (auto [item, alignment] : start) {
        auto x = resultBounds.x();
        resultBounds.moveLeft(item->width());
        setY(resultBounds, x, item, alignment);
    }

    for (auto [item, alignment] : end) {
        resultBounds.moveRight(-item->width());
        auto x = resultBounds.right();
        setY(resultBounds, x, item, alignment);
    }

    qreal width = 0.0;
    qreal height = 0.0;
    for (auto [item, alignment] : center) {
        width += item->width();
        height = std::max(height, item->height());
    }

    QRectF centerArea{resultBounds.x() + (resultBounds.width() - width) / 2.0, //
                      resultBounds.y() + (resultBounds.height() - height) / 2.0,
                      width,
                      height};

    auto x = centerArea.x();
    for (auto [item, alignment] : center) {
        setPosition(item, x, centerArea.y());
        x += item->width();
    }

    for (auto [item, alignment] : fill) {
        item->setWidth(resultBounds.width());
        setY(resultBounds, resultBounds.x(), item, alignment);
    }

    return resultBounds;
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
