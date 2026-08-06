// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "StyleUtils.h"
#include <ElementQuery.h>
#include <StyleRegistry.h>

#include <QCheckBox>
#include <QListView>
#include <QPushButton>
#include <QRadioButton>
#include <QStyleOption>
#include <QStyleOptionFrame>
#include <QTableView>
#include <QTextOption>
#include <QTreeView>

using namespace Qt::StringLiterals;

Union::Element::States statesFromOption(const QStyleOption *option)
{
    Union::Element::States states;
    if (!option) {
        return states;
    }
    if (option->state.testFlag(QStyle::State_None)) {
        return states;
    }
    if (option->state.testFlag(QStyle::State_MouseOver)) {
        states.setFlag(Union::Element::State::Hovered);
    }
    if (option->state.testFlag(QStyle::State_HasFocus)) {
        states.setFlag(Union::Element::State::ActiveFocus);
    }
    if (!option->state.testFlag(QStyle::State_Enabled)) {
        states.setFlag(Union::Element::State::Disabled);
    }
    if (option->state.testFlag(QStyle::State_On)) {
        states.setFlag(Union::Element::State::Checked);
    }
    if (option->state.testFlag(QStyle::State_Off)) {
        states.setFlag(Union::Element::State::Checked, false);
    }
    if (option->state.testFlag(QStyle::State_Sunken)) {
        states.setFlag(Union::Element::State::Pressed);
    }
    if (option->state.testFlag(QStyle::State_Raised)) {
        states.setFlag(Union::Element::State::Pressed, false);
    }
    if (option->state.testFlag(QStyle::State_Selected)) {
        states.setFlag(Union::Element::State::Highlighted);
    }

    return states;
}

QStringList hintsFromOption(const QStyleOption *option)
{
    QStringList hints;
    if (!option) {
        return hints;
    }
    switch ((QStyleOption::OptionType)option->type) {
    case QStyleOption::SO_FocusRect: {
        if (const auto optionFocusRect = static_cast<const QStyleOptionFocusRect *>(option)) {
            if (optionFocusRect->state.testFlag(QStyle::State_FocusAtBorder)) {
                hints.append(u"focus-at-border"_s);
            }
        }
    } break;
    case QStyleOption::SO_Button: {
        if (const auto optionButton = static_cast<const QStyleOptionButton *>(option)) {
            if (optionButton->features.testFlag(QStyleOptionButton::ButtonFeature::None)) {
                return hints;
            }
            if (optionButton->features.testFlag(QStyleOptionButton::ButtonFeature::Flat)) {
                hints.append(u"flat"_s);
            }
            if (optionButton->features.testFlag(QStyleOptionButton::ButtonFeature::HasMenu)) {
                hints.append(u"with-menu"_s);
            }
            if (optionButton->features.testFlag(QStyleOptionButton::ButtonFeature::DefaultButton)) {
                hints.append(u"default-button"_s);
            }
            if (optionButton->features.testFlag(QStyleOptionButton::ButtonFeature::AutoDefaultButton)) {
                hints.append(u"auto-default-button"_s);
            }
            if (optionButton->features.testFlag(QStyleOptionButton::ButtonFeature::CommandLinkButton)) {
                hints.append(u"command-link-button"_s);
            }
            if (!optionButton->state.testFlag(QStyle::State_AutoRaise)) {
                hints.append(u"raised"_s);
            }
        }
    } break;
    case QStyleOption::SO_ViewItem: {
        if (const auto optionViewItem = static_cast<const QStyleOptionViewItem *>(option)) {
            auto viewItemPosition = optionViewItem->viewItemPosition;
            const auto table = qobject_cast<const QTableView *>(optionViewItem->widget);
            const auto tree = qobject_cast<const QTreeView *>(optionViewItem->widget);
            const auto list = qobject_cast<const QListView *>(optionViewItem->widget);
            // For tables and such, we just want to select one item.
            if (table) {
                hints.append(u"inside-table"_s);
            }
            if (tree) {
                hints.append(u"inside-tree"_s);
            }
            if (list) {
                hints.append(u"inside-list"_s);
            }

            // These always have hover effect, i think
            hints.append(u"hover-enabled"_s);

            switch (viewItemPosition) {
            case QStyleOptionViewItem::Invalid:
                hints.append(u"position-invalid"_s);
                break;
            case QStyleOptionViewItem::Beginning:
                hints.append(u"position-beginning"_s);
                break;
            case QStyleOptionViewItem::Middle:
                hints.append(u"position-middle"_s);
                break;
            case QStyleOptionViewItem::End:
                hints.append(u"position-end"_s);
                break;
            case QStyleOptionViewItem::OnlyOne:
                hints.append(u"position-onlyone"_s);
                break;
            }
        }
    } break;
    case QStyleOption::SO_Frame: {
        if (const auto optionFrame = static_cast<const QStyleOptionFrame *>(option)) {
            if (optionFrame->features.testFlag(QStyleOptionFrame::Flat)) {
                hints.append(u"flat"_s);
            }
            if (optionFrame->features.testFlag(QStyleOptionFrame::Rounded)) {
                hints.append(u"rounded"_s);
            }
            switch (optionFrame->frameShape) {
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
        }
    } break;

    case QStyleOption::SO_ToolButton: {
        if (const auto optionButton = static_cast<const QStyleOptionToolButton *>(option)) {
            if (optionButton->features.testFlag(QStyleOptionToolButton::ToolButtonFeature::None)) {
                return hints;
            }
            if (optionButton->features.testFlag(QStyleOptionToolButton::ToolButtonFeature::Menu)) {
                hints.append(u"with-menu"_s);
            }
            if (!optionButton->state.testFlag(QStyle::State_AutoRaise)) {
                hints.append(u"raised"_s);
            }
        }
    } break;
    case QStyleOption::SO_Header: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            switch (opt->sortIndicator) {
            case QStyleOptionHeader::None:
                return hints;
            case QStyleOptionHeader::SortUp:
                hints.append(u"sort-ascending"_s);
                break;
            case QStyleOptionHeader::SortDown:
                hints.append(u"sort-descending"_s);
                break;
            }
        }
    } break;
    case QStyleOption::SO_MenuItem: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionMenuItemV2 *>(option)) {
            if (opt->checked) {
                hints.append(u"with-submenu"_s);
            }
        }
    } break;
    case QStyleOption::SO_GroupBox: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            if (opt->features.testFlag(QStyleOptionFrame::Flat)) {
                hints.append(u"flat"_s);
            }
            if (opt->features.testFlag(QStyleOptionFrame::Rounded)) {
                hints.append(u"rounded"_s);
            }
        }
    } break;
    case QStyleOption::SO_ComboBox: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            if (!opt->frame) {
                hints.append(u"flat"_s);
            }
            if (opt->editable) {
                hints.append(u"editable"_s);
            }
        }
    } break;
    case QStyleOption::SO_SpinBox: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            // TODO: Use constrained look for now, revisit this when we have better layouting
            hints.append(u"constrained"_s);
        }
    } break;
    case QStyleOption::SO_Complex:
    case QStyleOption::SO_Slider: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            if (opt->orientation == Qt::Horizontal) {
                hints.append(u"horizontal"_s);
            } else {
                hints.append(u"vertical"_s);
            }
        }
    } break;
    case QStyleOption::SO_TitleBar: {
        if (const auto opt = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            bool minimized = opt->titleBarState & Qt::WindowMinimized;
            bool maximized = opt->titleBarState & Qt::WindowMaximized;
            if (maximized) {
                hints.append(u"maximized"_s);
            }
            if (minimized) {
                hints.append(u"minimized"_s);
            }
        }
    } break;
    case QStyleOption::SO_Tab:
    case QStyleOption::SO_TabWidgetFrame:
    case QStyleOption::SO_TabBarBase:
    case QStyleOption::SO_ProgressBar:
    case QStyleOption::SO_ToolBox:
    case QStyleOption::SO_DockWidget:
    case QStyleOption::SO_RubberBand:
    case QStyleOption::SO_ToolBar:
    case QStyleOption::SO_GraphicsItem:
    case QStyleOption::SO_SizeGrip:
    default:
        return QStringList();
    }

    return hints;
}

Union::Element::ColorSet colorsetFromOption(const QStyleOption *option)
{
    /*
     *     enum class ColorSet {
     *  None,
     *  View,
     *  Window,
     *  Button,
     *  Selection,
     *  Tooltip,
     *  Complementary,
     *  Header,
    };*/
    if (!option) {
        return Union::Element::ColorSet::None;
    }
    switch ((QStyleOption::OptionType)option->type) {
    case QStyleOption::SO_Default:
    case QStyleOption::SO_FocusRect:
    case QStyleOption::SO_Tab:
    case QStyleOption::SO_MenuItem:
    case QStyleOption::SO_Frame:
    case QStyleOption::SO_ProgressBar:
    case QStyleOption::SO_ToolBox:
    case QStyleOption::SO_Header:
    case QStyleOption::SO_DockWidget:
    case QStyleOption::SO_ViewItem:
    case QStyleOption::SO_TabWidgetFrame:
    case QStyleOption::SO_TabBarBase:
    case QStyleOption::SO_GraphicsItem:
    case QStyleOption::SO_ToolBar:
    case QStyleOption::SO_Complex:
    case QStyleOption::SO_Slider:
    case QStyleOption::SO_SpinBox:
    case QStyleOption::SO_ComboBox:
    case QStyleOption::SO_TitleBar:
    case QStyleOption::SO_SizeGrip:
        return Union::Element::ColorSet::None;
    case QStyleOption::SO_Button:
    case QStyleOption::SO_ToolButton:
        return Union::Element::ColorSet::Button;
    case QStyleOption::SO_RubberBand:
        return Union::Element::ColorSet::Selection;
    case QStyleOption::SO_GroupBox:
        return Union::Element::ColorSet::Complementary;
    default:
        return Union::Element::ColorSet::None;
    }
}

QVariantMap attributesFromOption(const QStyleOption *option)
{
    if (!option) {
        return QVariantMap();
    }
    switch ((QStyleOption::OptionType)option->type) {
    case QStyleOption::SO_ToolButton:
        if (const auto optionButton = static_cast<const QStyleOptionToolButton *>(option)) {
            QVariantMap map;
            switch (optionButton->toolButtonStyle) {
            case Qt::ToolButtonIconOnly:
                map[u"display"_s] = QVariant(u"icon-only"_s);
                break;
            case Qt::ToolButtonTextOnly:
                map[u"display"_s] = QVariant(u"text-only"_s);
                break;
            case Qt::ToolButtonTextBesideIcon:
                map[u"display"_s] = QVariant(u"text-beside-icon"_s);
                break;
            case Qt::ToolButtonTextUnderIcon:
                map[u"display"_s] = QVariant(u"text-under-icon"_s);
                break;
            default:
                return map;
            }
            return map;
        }
        break;
    case QStyleOption::SO_Tab:
        if (const auto tabOption = static_cast<const QStyleOptionTab *>(option)) {
            QVariantMap map;
            const bool top = tabOption->shape == QTabBar::RoundedNorth || tabOption->shape == QTabBar::TriangularNorth;
            const bool bottom = tabOption->shape == QTabBar::RoundedSouth || tabOption->shape == QTabBar::TriangularSouth;
            const bool left = tabOption->shape == QTabBar::RoundedWest || tabOption->shape == QTabBar::TriangularWest;
            const bool right = tabOption->shape == QTabBar::RoundedEast || tabOption->shape == QTabBar::TriangularEast;

            if (top) {
                map[u"direction"_s] = QVariant(u"top"_s);
            }
            if (bottom) {
                map[u"direction"_s] = QVariant(u"bottom"_s);
            }
            if (left) {
                map[u"direction"_s] = QVariant(u"left"_s);
            }
            if (right) {
                map[u"direction"_s] = QVariant(u"right"_s);
            }
            return map;
        }
        break;
    case QStyleOption::SO_TabBarBase:
    case QStyleOption::SO_ViewItem:
    case QStyleOption::SO_Default:
    case QStyleOption::SO_FocusRect:
    case QStyleOption::SO_Button:
    case QStyleOption::SO_MenuItem:
    case QStyleOption::SO_Frame:
    case QStyleOption::SO_ProgressBar:
    case QStyleOption::SO_ToolBox:
    case QStyleOption::SO_Header:
    case QStyleOption::SO_DockWidget:
    case QStyleOption::SO_TabWidgetFrame:
    case QStyleOption::SO_RubberBand:
    case QStyleOption::SO_ToolBar:
    case QStyleOption::SO_GraphicsItem:
    case QStyleOption::SO_Complex:
    case QStyleOption::SO_Slider:
    case QStyleOption::SO_SpinBox:
    case QStyleOption::SO_ComboBox:
    case QStyleOption::SO_TitleBar:
    case QStyleOption::SO_GroupBox:
    case QStyleOption::SO_SizeGrip:
    case QStyleOption::SO_CustomBase:
    case QStyleOption::SO_ComplexCustomBase:
        break;
    }
    return QVariantMap();
}

Qt::Alignment toQtAlignment(Union::Properties::AlignmentPropertyGroup *alignmentGroup)
{
    Qt::Alignment verticalAlignment = Qt::AlignVCenter;
    Qt::Alignment horizontalAlignment = Qt::AlignLeft;

    if (!alignmentGroup) {
        return verticalAlignment | horizontalAlignment;
    }

    auto unionVertical = alignmentGroup->vertical().value_or(Union::Properties::Alignment::Unspecified);
    auto unionHorizontal = alignmentGroup->horizontal().value_or(Union::Properties::Alignment::Unspecified);

    switch (unionVertical) {
    case Union::Properties::Alignment::Unspecified:
    case Union::Properties::Alignment::Fill:
    case Union::Properties::Alignment::StackCenter:
    case Union::Properties::Alignment::StackFill:
    case Union::Properties::Alignment::Center:
        verticalAlignment = Qt::AlignVCenter;
        break;
    case Union::Properties::Alignment::Start:
        verticalAlignment = Qt::AlignTop;
        break;
    case Union::Properties::Alignment::End:
        verticalAlignment = Qt::AlignBottom;
        break;
    }

    switch (unionHorizontal) {
    case Union::Properties::Alignment::Unspecified:
    case Union::Properties::Alignment::Start:
        horizontalAlignment = Qt::AlignLeft;
        break;
    case Union::Properties::Alignment::Fill:
    case Union::Properties::Alignment::Center:
    case Union::Properties::Alignment::StackFill:
    case Union::Properties::Alignment::StackCenter:
        horizontalAlignment = Qt::AlignHCenter;
        break;
    case Union::Properties::Alignment::End:
        horizontalAlignment = Qt::AlignRight;
        break;
    }

    return verticalAlignment | horizontalAlignment;
}

Qt::TextElideMode toQtElideMode(Union::Properties::TextElide elideMode)
{
    Qt::TextElideMode elide;
    switch (elideMode) {
    case Union::Properties::TextElide::None:
        elide = Qt::TextElideMode::ElideNone;
        break;
    case Union::Properties::TextElide::Left:
        elide = Qt::TextElideMode::ElideLeft;
        break;
    case Union::Properties::TextElide::Middle:
        elide = Qt::TextElideMode::ElideMiddle;
        break;
    case Union::Properties::TextElide::Right:
        elide = Qt::TextElideMode::ElideRight;
        break;
    }
    return elide;
}
Qt::TextFlag toQtWrapMode(Union::Properties::TextWrapMode wrapMode)
{
    Qt::TextFlag wrap = Qt::TextFlag::TextDontClip;
    switch (wrapMode) {
    case Union::Properties::TextWrapMode::NoWrap:
    case Union::Properties::TextWrapMode::ManualWrap:
        break;
    case Union::Properties::TextWrapMode::WordWrap:
    case Union::Properties::TextWrapMode::WrapAtWordBoundaryOrAnywhere:
        wrap = Qt::TextFlag::TextWordWrap;
        break;
    case Union::Properties::TextWrapMode::WrapAnywhere:
        wrap = Qt::TextFlag::TextWrapAnywhere;
        break;
    }
    return wrap;
}

QRectF backgroundRectangle(const QStyleOption *option, const Union::Properties::StylePropertyGroup *properties)
{
    // Shrink the widget rect by the insets
    if (!option) {
        return QRectF();
    }
    QRectF rect = option->rect;
    if (const auto layout = properties->layout()) {
        if (layout->inset()) {
            rect = rect.marginsRemoved(layout->inset()->toMargins());
        }
    }
    return rect;
}

Union::ElementList prepareElements(const QStyleOption *opt, const QWidget *widget, QStringList targetHierarchy)
{
    Union::ElementList elements;
    QStringList elementTypes = {};

    if (widget) {
        elementTypes = widget->property(property_union_member_list).toStringList();
        if (elementTypes.isEmpty()) {
            elementTypes = setupMemberList(widget);
        }
    }

    elementTypes.append(targetHierarchy);

    for (const auto &elementType : elementTypes) {
        auto unionElement = Union::Element::create();
        unionElement->setType(elementType);
        unionElement->setStates(statesFromOption(opt));
        unionElement->setHints(hintsFromOption(opt));
        unionElement->setColorSet(colorsetFromOption(opt));
        unionElement->setAttributes(attributesFromOption(opt));
        elements.append(unionElement);
    }
    return elements;
}

Union::Properties::StylePropertyGroup *queryProperties(const Union::ElementList &elements)
{
    Q_ASSERT(!elements.isEmpty());
    const auto style = Union::StyleRegistry::instance()->defaultStyle();
    const auto query = std::make_unique<Union::ElementQuery>(style);
    query->setElements(elements);
    query->execute();
    return query->properties();
}

QStringList setupMemberList(const QWidget *widget)
{
    if (!widget) {
        return QStringList();
    }
    QStringList members;
    // We will have to check what items the widget inherits from,
    // as far as I know there is no better way to do this.
    const QMap<const char *, QString> parentClasses = {{"QCheckBox", u"CheckBox"_s},
                                                       {"QRadioButton", u"RadioButton"_s},
                                                       {"QPushButton", u"Button"_s},
                                                       {"QToolButton", u"ToolButton"_s},
                                                       {"QDial", u"Dial"_s},
                                                       {"QScrollBar", u"ScrollBar"_s},
                                                       {"QSlider", u"Slider"_s},
                                                       {"QAbstractSpinBox", u"SpinBox"_s},
                                                       {"QComboBox", u"ComboBox"_s},
                                                       {"QDialog", u"Dialog"_s},
                                                       {"QDialogButtonBox", u"DialogButtonBox"_s},
                                                       {"QDockWidget", u"Dock"_s},
                                                       {"QFocusFrame", u"FocusFrame"_s},
                                                       {"QFrame", u"Frame"_s},
                                                       {"QGroupBox", u"GroupBox"_s},
                                                       {"QKeySequenceEit", u"KeySequenceEdit"_s},
                                                       {"QLineEdit", u"TextField"_s},
                                                       {"QMainWindow", u"ApplicationWindow"_s},
                                                       {"QMdiSubWinow", u"MdiSubWindow"_s},
                                                       {"QMenu", u"Menu"_s},
                                                       {"QMenuBar", u"MenuBar"_s},
                                                       {"QProgressBar", u"ProgressBar"_s},
                                                       {"QRubberBand", u"RubberBand"_s},
                                                       {"QSizeGrip", u"SizeGrip"_s},
                                                       {"QSplitterHandle", u"SplitterHandle"_s},
                                                       {"QStatusBar", u"StatusBar"_s},
                                                       {"QTabBar", u"TabBar"_s},
                                                       {"QTabWidget", u"TabWidget"_s},
                                                       {"QToolBar", u"ToolBar"_s},
                                                       {"QAbstractScrollArea", u"ScrollArea"_s},
                                                       {"QListView", u"ListView"_s},
                                                       {"QTreeView", u"QTreeViewDelegate"_s},
                                                       {"QSplitter", u"Splitter"_s}};

    auto currentWidget = widget;
    while (currentWidget) {
        for (const auto classes : parentClasses.asKeyValueRange()) {
            if (currentWidget->inherits(classes.first)) {
                members.prepend(classes.second);
                break;
            }
        }
        currentWidget = currentWidget->parentWidget();
    }

    return members;
}

QMap<QString, LayoutItem> layoutMap(const Union::ElementList &elements, const QStyleOption *opt, const QStringList &subElements)
{
    QMap<QString, LayoutItem> map;
    QList<LayoutItem> items;

    if (subElements.empty()) {
        qWarning() << "No sublements given, returning empty map!" << elements << opt->type;
        return map;
    }

    // TODO: Go through all elements, create rectangles for them
    // Then place and resize those rectangles according to hierarchy
    // Use the original opt->rect as the main container
    // move any subelements in it according their given rules

    QRectF availableSpace = opt->rect;

    // Get spacing for main item
    auto properties = queryProperties(elements);
    int globalSpacing = 0;
    QMargins padding;
    if (properties->layout()->padding()) {
        padding = properties->layout()->padding()->toMargins().toMargins();
    }
    availableSpace = availableSpace.marginsRemoved(padding);
    if (subElements.count() > 1) {
        globalSpacing = properties->layout()->spacing().value_or(0);
    }
    auto currentHierarchy = elements;
    // this could be turned into its own method
    for (const auto &subElement : subElements) {
        // NOTE: Currently text and icon are part of the main element, but eventually
        // will be moved as their own elements
        if (subElement != u"Icon"_s && subElement != u"Text"_s) {
            auto unionElement = Union::Element::create();
            unionElement->setType(subElement);
            unionElement->setStates(statesFromOption(opt));
            unionElement->setHints(hintsFromOption(opt));
            unionElement->setColorSet(colorsetFromOption(opt));
            unionElement->setAttributes(attributesFromOption(opt));
            currentHierarchy.append(unionElement);
        }
        properties = queryProperties(currentHierarchy);
        Union::Properties::Alignment horizontalAlignment;
        Union::Properties::Alignment verticalAlignment;
        int order = 0;
        QRectF elementRect = availableSpace;
        // NOTE: For now icon and text are their own things, so check them separately.
        // in future this should be unnecessary.
        if (subElement == u"Icon"_s) {
            // Toolbutton can override the icon size
            if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
                elementRect.setWidth(toolButtonOption->iconSize.width());
                elementRect.setHeight(toolButtonOption->iconSize.height());
            } else {
                elementRect.setWidth(properties->icon()->width().value_or(0));
                elementRect.setHeight(properties->icon()->height().value_or(0));
            }
            horizontalAlignment = properties->icon()->alignment()->horizontal().value_or(Union::Properties::Alignment::Unspecified);
            verticalAlignment = properties->icon()->alignment()->vertical().value_or(Union::Properties::Alignment::Unspecified);
            order = properties->icon()->alignment()->order().value_or(0);
        } else if (subElement == u"Text"_s || subElement == u"ShortcutText"_s) {
            horizontalAlignment = properties->text()->alignment()->horizontal().value_or(Union::Properties::Alignment::Unspecified);
            verticalAlignment = properties->text()->alignment()->vertical().value_or(Union::Properties::Alignment::Unspecified);
            auto optiontext = textFromOption(opt);
            const int tabPosition(optiontext.indexOf(QLatin1Char('\t')));
            if (tabPosition >= 0) {
                QString accelerator(optiontext.mid(tabPosition + 1));
                if (subElement == u"ShortcutText"_s) {
                    optiontext = optiontext.mid(tabPosition + 1);
                } else {
                    optiontext = optiontext.left(tabPosition);
                }
            }
            // if we are a menuitem and have a shortcut, we need to split the text with /t and place them according their alignments
            elementRect = opt->fontMetrics.boundingRect(availableSpace.toRect(), textFlagsFromProperties(properties, true), optiontext);
            order = properties->text()->alignment()->order().value_or(0);
        } else {
            elementRect.setWidth(properties->layout()->width().value_or(0));
            elementRect.setHeight(properties->layout()->height().value_or(0));
            horizontalAlignment = properties->layout()->alignment()->horizontal().value_or(Union::Properties::Alignment::Unspecified);
            verticalAlignment = properties->layout()->alignment()->vertical().value_or(Union::Properties::Alignment::Unspecified);
            order = properties->layout()->alignment()->order().value_or(0);
        }
        LayoutItem item = LayoutItem();
        item.elementName = subElement;
        item.horizontalAlignment = horizontalAlignment;
        item.verticalAlignment = verticalAlignment;
        item.order = order;
        item.rect = elementRect;
        items.append(item);
    }

    // Sort the list according to order. Set any filled items as last
    std::sort(items.begin(), items.end(), [](const LayoutItem &lhs, const LayoutItem &rhs) {
        if (lhs.horizontalAlignment == rhs.horizontalAlignment || lhs.verticalAlignment == rhs.verticalAlignment) {
            if (lhs.horizontalAlignment == Union::Properties::Alignment::End || lhs.verticalAlignment == Union::Properties::Alignment::End) {
                return lhs.order > rhs.order;
            } else {
                return lhs.order < rhs.order;
            }
        }
        return false;
    });

    // Actual layouting starts here
    // QtWidgets containment is always within Widget, since we can't draw outside of a widget due
    // widgets limitations.

    int counter = 1;
    int spacing = globalSpacing;
    QRectF horizontalSpace = availableSpace;
    QRectF verticalSpace = availableSpace;

    // First, layout the start/end only
    for (auto &item : items) {
        // Skip spacing for last/only item
        if (counter >= items.count()) {
            spacing = 0;
        }

        auto itemWidth = item.rect.width() + spacing;
        auto itemHeight = item.rect.height() + spacing;
        switch (item.horizontalAlignment) {
        case Union::Properties::Alignment::StackFill:
        case Union::Properties::Alignment::StackCenter:
            qWarning() << "StackFill/StackCenter is not supported for horizontal alignment!";
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::Start:
            item.rect.moveLeft(horizontalSpace.left());
            horizontalSpace.setLeft(item.rect.left() + itemWidth);
            break;
        case Union::Properties::Alignment::Center:
            // Center is bit confusing. It is meant to center the drawing inside the rectangle,
            // so we do that for stackcenter/stackfill items.
            if (item.horizontalAlignment == Union::Properties::Alignment::Center
                && (item.verticalAlignment == Union::Properties::Alignment::StackCenter || item.verticalAlignment == Union::Properties::Alignment::StackFill)) {
                item.rect = centerRect(horizontalSpace.toRect(), item.rect.width(), item.rect.height());
            } else {
                // When layouting normally we need to move it to next to the other item anyway.
                if (items.count() > 1) {
                    item.rect.moveLeft(horizontalSpace.left());
                    horizontalSpace.setLeft(item.rect.left() + itemWidth);
                } else {
                    // For single items, we can just center it completely
                    item.rect.moveCenter(availableSpace.center());
                }
            }
            break;
        case Union::Properties::Alignment::End:
            item.rect.moveRight(horizontalSpace.right());
            horizontalSpace.setRight(item.rect.right() - itemWidth);
            break;
        default:
            break;
        }

        switch (item.verticalAlignment) {
        case Union::Properties::Alignment::Unspecified:
        case Union::Properties::Alignment::Start:
            item.rect.moveTop(verticalSpace.top());
            verticalSpace.setTop(item.rect.top() + itemHeight);
            break;
            // We can safely center the element within its rectangle here
        case Union::Properties::Alignment::Center:
            item.rect.moveCenter(QPoint(item.rect.center().x(), verticalSpace.center().y()));
            break;
        case Union::Properties::Alignment::End:
            item.rect.moveBottom(verticalSpace.bottom());
            verticalSpace.setBottom(item.rect.bottom() - itemHeight);
            break;
        default:
            break;
        }

        map[item.elementName] = item;
        counter++;
    }

    // Then, layout the fills and stacks
    counter = 0;
    spacing = globalSpacing;
    for (auto &item : items) {
        // Skip spacing for last/only item
        if (counter >= items.count()) {
            spacing = 0;
        }

        auto itemHeight = item.rect.height() + spacing;
        switch (item.horizontalAlignment) {
        case Union::Properties::Alignment::Fill:
            item.rect.moveLeft(horizontalSpace.left());
            item.rect.setRight(horizontalSpace.right());
            break;
        default:
            break;
        }

        switch (item.verticalAlignment) {
        case Union::Properties::Alignment::Fill:
            item.rect.setTop(verticalSpace.top());
            item.rect.setBottom(verticalSpace.bottom());
            break;
        case Union::Properties::Alignment::StackFill:
        case Union::Properties::Alignment::StackCenter:
            item.rect.moveTop(verticalSpace.top());
            verticalSpace.moveTop(item.rect.top() + itemHeight);
            break;
        default:
            break;
        }

        map[item.elementName] = item;
        counter++;
    }

    return map;
}

QString textFromOption(const QStyleOption *opt)
{
    if (const auto comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
        return comboBoxOption->currentText;
    }

    switch ((QStyleOption::OptionType)opt->type) {
    case QStyleOption::SO_Button:
        if (const auto option = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_ToolButton:
        if (const auto option = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_DockWidget:
        if (const auto option = qstyleoption_cast<const QStyleOptionDockWidget *>(opt)) {
            return option->title;
        }
        break;
    case QStyleOption::SO_Header:
        if (const auto option = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_MenuItem:
        if (const auto option = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_ProgressBar:
        if (const auto option = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_Tab:
        if (const auto option = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_ToolBox:
        if (const auto option = qstyleoption_cast<const QStyleOptionToolBox *>(opt)) {
            return option->text;
        }
        break;
    case QStyleOption::SO_ViewItem:
        if (const auto option = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            return option->text;
        }
        break;
    default:
        break;
    }
    return QString();
}

int textFlagsFromProperties(Union::Properties::StylePropertyGroup *properties, bool skipAlign)
{
    int textFlags = Qt::AlignVCenter;
    // Handle alignment case-by-case basis. Sometimes we want to just use default
    // alignleft and center, especially if we have an icon to work with.
    auto textAlign = QFlags(Qt::AlignAbsolute);
    if (!skipAlign) {
        textAlign = toQtAlignment(properties->text()->alignment());
    }
    auto textWrap = toQtWrapMode(properties->text()->wrapMode().value_or(Union::Properties::TextWrapMode::NoWrap));
    auto textElide = toQtElideMode(properties->text()->elide().value_or(Union::Properties::TextElide::Right));
    auto textColor = properties->text()->color();
    textFlags |= textAlign;
    // Do not add wrap flags if we get DontClip
    // This could be done better
    if (textWrap == Qt::TextDontClip) {
        textFlags |= textWrap;
    }
    textFlags |= textElide;
    textFlags |= Qt::TextShowMnemonic;
    return textFlags;
}

QRect centerRect(const QRect &rect, int width, int height)
{
    return QRect(rect.left() + (rect.width() - width) / 2, rect.top() + (rect.height() - height) / 2, width, height);
}

QStringList buildSubElementList(const QStyleOption *option, const QWidget *widget)
{
    QStringList childelements = {};
    if (const auto viewItemOption = qstyleoption_cast<const QStyleOptionViewItem *>(option)) {
        if (viewItemOption) {
            if (viewItemOption->features.testFlag(QStyleOptionViewItem::HasDisplay) && !viewItemOption->text.isEmpty()) {
                childelements.append(u"Text"_s);
            }
            if (viewItemOption->features.testFlag(QStyleOptionViewItem::HasDecoration) && !viewItemOption->icon.isNull()) {
                childelements.append(u"Icon"_s);
            }
            if (viewItemOption->features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
                childelements.append(u"CheckBox"_s);
            }
        }
    } else if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
        if (qobject_cast<const QPushButton *>(widget)) {
            if (buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) {
                childelements.append(u"Indicator"_s);
            }
            if (!buttonOption->icon.isNull()) {
                childelements.append(u"Icon"_s);
            }
            if (!buttonOption->text.isEmpty()) {
                childelements.append(u"Text"_s);
            }
        } else if (qobject_cast<const QCheckBox *>(widget) || qobject_cast<const QRadioButton *>(widget)) {
            childelements.append(u"Indicator"_s);
            if (!buttonOption->icon.isNull()) {
                childelements.append(u"Icon"_s);
            }
            if (!buttonOption->text.isEmpty()) {
                childelements.append(u"Text"_s);
            }
        }
    } else if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
        bool hasIndicator =
            toolButtonOption->features.testFlag(QStyleOptionToolButton::HasMenu) || toolButtonOption->features.testFlag(QStyleOptionToolButton::Menu);
        bool hasIcon = !toolButtonOption->icon.isNull();
        bool hasText = !toolButtonOption->text.isEmpty();
        if (hasIcon) {
            childelements.append(u"Icon"_s);
        }
        if (hasText) {
            childelements.append(u"Text"_s);
        }
        if (hasIndicator) {
            childelements.append(u"Indicator"_s);
        }
    } else if (const auto dockOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
        if (dockOption->closable) {
            childelements.append(u"CloseButton"_s);
        }
        if (dockOption->floatable) {
            childelements.append(u"FloatButton"_s);
        }
        if (!dockOption->title.isEmpty()) {
            childelements.append(u"Text"_s);
        }
        // Would check for icon too but the styleoption has no icon field!
    } else if (const auto titleBarOption = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
        if (!titleBarOption->text.isEmpty()
            && (titleBarOption->titleBarFlags.testFlag(Qt::WindowTitleHint) || titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint))) {
            childelements.append(u"Text"_s);
        }
        if (!titleBarOption->icon.isNull()) {
            childelements.append(u"Icon"_s);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowContextHelpButtonHint)) {
            childelements.append(u"HelpButton"_s);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowMinimizeButtonHint)) {
            childelements.append(u"MinimizeButton"_s);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowMaximizeButtonHint)) {
            childelements.append(u"MaximizeButton"_s);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowCloseButtonHint)) {
            childelements.append(u"CloseButton"_s);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint)) {
            childelements.append(u"SystemMenu"_s);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowShadeButtonHint)) {
            childelements.append(u"ShadeButton"_s);
        }
    }

    return childelements;
}

QIcon queryIcon(const QStyleOption *option, const QWidget *widget, const QString &defaultIconName, const QStringList &targetHierarchy)
{
    auto name = defaultIconName;
    auto elements = prepareElements(option, widget, targetHierarchy);
    auto props = queryProperties(elements);
    if (props && props->icon()) {
        name = props->icon()->name().value_or(name);
    }
    return QIcon::fromTheme(name);
}