// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>

#include "PositionerLayout.h"

#include <QWidget>

#include <properties/StylePropertyGroup.h>

#include "Layout.h"
#include "Positioner.h"

using namespace Union::Properties;
using namespace Union::Widgets;

using namespace Qt::StringLiterals;

class PositionerLayout::Private
{
public:
    QWidgetSet items;
    QSizeF implicitSize;
    QSizeF parentSize;

    Qt::LayoutDirection layoutDirection = Qt::LayoutDirectionAuto;

    bool layoutDirty : 1 = true;
    bool layouting : 1 = false;
    bool requeuePolish : 1 = false;
    bool ignorePositionChange : 1 = false;

    bool paddingValid : 1 = false;
    bool insetValid : 1 = false;

    bool mirroredProperty;
    qreal spacingProperty;
    qreal leftPaddingProperty;
    qreal rightPaddingProperty;
    qreal topPaddingProperty;
    qreal bottomPaddingProperty;
    qreal leftInsetProperty;
    qreal rightInsetProperty;
    qreal topInsetProperty;
    qreal bottomInsetProperty;

    inline static const Union::Properties::LayoutPropertyGroup EmptyLayoutGroup;
};

PositionerLayout::PositionerLayout(QWidget *parentWidget)
    : QWidget(parentWidget)
    , d(std::make_unique<Private>())
{
    if (parentWidget) {
        // connect(parentWidget, &QWidget::widthChanged, this, &PositionerLayout::onParentSizeChanged);
        // connect(parentWidget, &QWidget::heightChanged, this, &PositionerLayout::onParentSizeChanged);
        parentWidget->installEventFilter(this);

        d->mirroredProperty = parentWidget->property("mirrored").toBool();

        d->spacingProperty = parentWidget->property("spacing").toFloat();

        d->leftPaddingProperty = parentWidget->property("leftPadding").toFloat();

        d->rightPaddingProperty = parentWidget->property("rightPadding").toFloat();

        d->topPaddingProperty = parentWidget->property("topPadding").toFloat();

        d->bottomPaddingProperty = parentWidget->property("bottomPadding").toFloat();

        const auto paddingProperties = {&d->leftPaddingProperty, &d->rightPaddingProperty, &d->topPaddingProperty, &d->bottomPaddingProperty};
        //        d->paddingValid = std::ranges::any_of(paddingProperties, std::bind_front(std::equal_to{}, true), &QQmlProperty::isValid);

        d->leftInsetProperty = parentWidget->property("leftInset").toFloat();

        d->rightInsetProperty = parentWidget->property("rightInset").toFloat();

        d->topInsetProperty = parentWidget->property("topInset").toFloat();

        d->bottomInsetProperty = parentWidget->property("bottomInset").toFloat();

        const auto insetProperties = {&d->leftInsetProperty, &d->rightInsetProperty, &d->topInsetProperty, &d->bottomInsetProperty};
        //        d->insetValid = std::ranges::any_of(insetProperties, std::bind_front(std::equal_to{}, true), &QQmlProperty::isValid);
    }
    //    polish();
}

void PositionerLayout::markDirty()
{
    if (!d->layouting) {
        d->layoutDirty = true;
        //        polish();
    } else {
        d->requeuePolish = true;
    }
}

void PositionerLayout::addItem(QWidget *item)
{
    //    connect(item, &QWidget::implicitWidthChanged, this, &PositionerLayout::markDirty);
    //    connect(item, &QWidget::implicitHeightChanged, this, &PositionerLayout::markDirty);
    //    connect(item, &QWidget::visibleChanged, this, &PositionerLayout::markDirty);
    //    connect(item, &QWidget::xChanged, this, &PositionerLayout::onItemPositionChanged);
    //    connect(item, &QWidget::yChanged, this, &PositionerLayout::onItemPositionChanged);
    item->installEventFilter(this);

    d->items.insert(item);

    debug("Add item to layout", item);

    markDirty();
}

void PositionerLayout::removeItem(QWidget *item)
{
    auto itr = std::find(d->items.begin(), d->items.end(), item);
    if (itr == d->items.end()) {
        return;
    }

    (*itr)->disconnect(this);
    (*itr)->removeEventFilter(this);

    debug("Remove item from layout", *itr);
    d->items.erase(itr);

    markDirty();
}

bool PositionerLayout::isDebugEnabled() const
{
    return m_debugEnabled;
}

void PositionerLayout::setDebugEnabled(bool newDebug)
{
    m_debugEnabled = newDebug;
}

Qt::LayoutDirection PositionerLayout::layoutDirection() const
{
    return d->layoutDirection;
}

void PositionerLayout::setLayoutDirection(Qt::LayoutDirection direction)
{
    if (direction == d->layoutDirection) {
        return;
    }

    d->layoutDirection = direction;
    markDirty();
    Q_EMIT layoutDirectionChanged();
}

QSizeF PositionerLayout::implicitSize() const
{
    return d->implicitSize;
}

bool PositionerLayout::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == PositionedItemChangedEvent::s_type) {
        markDirty();
        return false;
    }

    return QWidget::eventFilter(target, event);
}

void PositionerLayout::updatePolish()
{
    if (!d->layoutDirty) {
        return;
    }

    if (!parentWidget()->isVisible()) {
        return;
    }

    d->layouting = true;
    d->layoutDirty = false;

    Layout layout;

    debug("Performing layout for positioner of", parentWidget());

    for (const auto &item : std::as_const(d->items)) {
        auto source = PositionerSource::Source::Layout;

        if (!item->isVisible()) {
            debug("  Ignoring invisible item", item);
            continue;
        }

        //        auto positionedItemAttached = qobject_cast<PositionedItem *>(qmlAttachedPropertiesObject<PositionedItem>(item, false));
        //        if (positionedItemAttached) {
        //            source = positionedItemAttached->source();
        //        }

        //        auto query = qobject_cast<QStyle *>(qmlAttachedPropertiesObject<QStyle>(item, true))->query();
        //        if (!query) {
        //            // Apparently the item has not completed yet, abort layouting and
        //            // try again the next frame.
        ////            polish();
        //            break;
        //        }

        /* auto properties = query->properties();
        auto layoutProperties = properties->layout();

        Union::Properties::AlignmentPropertyGroup *alignment;
        switch (source) {
        case PositionerSource::Source::Layout:
            alignment = layoutProperties ? layoutProperties->alignment() : nullptr;
            break;
        case PositionerSource::Source::Icon:
            alignment = properties->icon() ? properties->icon()->alignment() : nullptr;
            break;
        case PositionerSource::Source::Text:
            alignment = properties->text() ? properties->text()->alignment() : nullptr;
            break;
        }

        if (!alignment) {
            debug("  No alignment found for item", item);
            continue;
        }

        auto horizontalAlignment = alignment->horizontal().value_or(Union::Properties::Alignment::Unspecified);
        auto verticalAlignment = alignment->vertical().value_or(Union::Properties::Alignment::Unspecified);

        if (positionedItemAttached) {
            if (positionedItemAttached->horizontalAlignment() != Union::Properties::Alignment::Unspecified) {
                horizontalAlignment = positionedItemAttached->horizontalAlignment();
            }

            if (positionedItemAttached->verticalAlignment() != Union::Properties::Alignment::Unspecified) {
                verticalAlignment = positionedItemAttached->verticalAlignment();
            }
        }

        LayoutItem layoutItem{
            .implicitSize = QSizeF{static_cast<qreal>(item->rect().width()), static_cast<qreal>(item->rect().height())},
            .verticalAlignment = verticalAlignment,
            .order = alignment->order().value_or(0),
            .margins = QMarginsF{},
            .item = item,
        };

        if (positionedItemAttached) {
            layoutItem.minimumSize = QSizeF{positionedItemAttached->minimumWidth() > 0.0 ? positionedItemAttached->minimumWidth() : 0.0,
                                            positionedItemAttached->minimumHeight() > 0.0 ? positionedItemAttached->minimumHeight() : 0.0};
        }

        if (source == PositionerSource::Source::Layout && layoutProperties && layoutProperties->margins()) {
            auto margins = layoutProperties->margins()->toMargins();
            layoutItem.margins = QMarginsF(margins.left(), margins.top(), margins.right(), margins.bottom());
        }

        LayoutContainer *container = &(layout.itemContainer);
        switch (alignment->container().value_or(AlignmentContainer::Item)) {
        case AlignmentContainer::Content:
            container = &(layout.contentContainer);
            break;
        case AlignmentContainer::Background:
            container = &(layout.backgroundContainer);
            break;
        case AlignmentContainer::Item:
            break;
        }

        bool stackCenter = verticalAlignment == Union::Properties::Alignment::StackCenter;
        bool stackFill = verticalAlignment == Union::Properties::Alignment::StackFill;

        switch (horizontalAlignment) {
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::StackCenter:
        case Union::Properties::Alignment::StackFill:
            qCWarning(UNION_QTQUICK) << "Alignment" << horizontalAlignment << "not supported for horizontal alignment, item" << item
                                     << "will use Start alignment";
            [[fallthrough]];
        case Union::Properties::Alignment::Start:
            container->start.items.append(layoutItem);
            container->start.stackCenter = container->start.stackCenter || stackCenter;
            container->start.stackFill = container->start.stackFill || stackFill;
            break;
        case Union::Properties::Alignment::Center:
            container->center.items.append(layoutItem);
            container->center.stackCenter = container->center.stackCenter || stackCenter;
            container->center.stackFill = container->center.stackFill || stackFill;
            break;
        case Union::Properties::Alignment::End:
            container->end.items.append(layoutItem);
            container->end.stackCenter = container->end.stackCenter || stackCenter;
            container->end.stackFill = container->end.stackFill || stackFill;
            break;
        case Union::Properties::Alignment::Fill:
            container->fill.items.append(layoutItem);
            container->fill.stackCenter = container->fill.stackCenter || stackCenter;
            container->fill.stackFill = container->fill.stackFill || stackFill;
            break;
        }

        debug("  Layout item", item);
        debug("    Implicit Size:", layoutItem.implicitSize);
        debug("    Container:", alignment->container());
        debug("    Alignment: (h)", horizontalAlignment, "(v)", verticalAlignment);
        debug("    Order:", layoutItem.order);
    }

    const auto containerItem = parentWidget();
    const auto query = qobject_cast<QStyle *>(qmlAttachedPropertiesObject<QStyle>(containerItem, true))->query();
    if (!query || !query->properties()) {
//        polish();
        return;
    }

    const auto properties = query->properties();
    const auto &layoutGroup = properties->layout() ? *(properties->layout()) : Private::EmptyLayoutGroup;

    layout.size = containerItem->size(); */

        //  layout.spacing = d->spacingProperty.isValid() ? d->spacingProperty.read().toReal() : layoutGroup.spacing().value_or(0.0);

        //   if (d->paddingValid) {
        //       layout.padding = QMarginsF{d->leftPaddingProperty.read().toReal(),
        //                                  d->topPaddingProperty.read().toReal(),
        //                                  d->rightPaddingProperty.read().toReal(),
        //                                  d->bottomPaddingProperty.read().toReal()};
        //   } else {
        //       layout.padding = layoutGroup.padding() ? layoutGroup.padding()->toMargins() : QMarginsF{};
        //   }

        //   if (d->insetValid) {
        //       layout.inset = QMarginsF{d->leftInsetProperty.read().toReal(),
        //                                d->topInsetProperty.read().toReal(),
        //                                d->rightInsetProperty.read().toReal(),
        //                                d->bottomInsetProperty.read().toReal()};
        //   } else {
        //       layout.inset = layoutGroup.inset() ? layoutGroup.inset()->toMargins() : QMarginsF{};
    }

    layout.layout();

    if (layout.implicitSize != d->implicitSize) {
        debug("  New implcit size is", layout.implicitSize);
        d->implicitSize = layout.implicitSize;
        Q_EMIT implicitSizeChanged();
    }

    d->ignorePositionChange = true;

    auto direction = d->layoutDirection;
    if (direction == Qt::LayoutDirectionAuto) {
        //        if (d->mirroredProperty.isValid() && d->mirroredProperty.read().toBool()) {
        //            direction = Qt::LayoutDirection::RightToLeft;
        //        } else {
        //            direction = qApp->layoutDirection();
        //        }
    }

    layout.positionItems(parentWidget(), direction);
    d->ignorePositionChange = false;

    d->layouting = false;

    Q_EMIT layoutFinished();

    if (d->requeuePolish) {
        markDirty();
        d->requeuePolish = false;
    }
}

void PositionerLayout::onParentSizeChanged()
{
    auto newSize = parentWidget()->rect().size();
    if (newSize == d->parentSize) {
        return;
    }

    d->parentSize = newSize;
    markDirty();
}

void PositionerLayout::onItemPositionChanged()
{
    if (d->ignorePositionChange) {
        return;
    }

    markDirty();
}

#include "moc_PositionerLayout.cpp"
