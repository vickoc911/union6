// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "StyleUtils.h"
#include "SharedNames.h"
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
#include <qstyleoption.h>

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

    states.setFlag(Union::Element::State::Hovered, option->state.testFlag(QStyle::State_MouseOver));
    states.setFlag(Union::Element::State::ActiveFocus, option->state.testFlag(QStyle::State_HasFocus));
    states.setFlag(Union::Element::State::VisualFocus,
                   option->state.testFlag(QStyle::State_KeyboardFocusChange) && option->state.testFlag(QStyle::State_HasFocus));
    states.setFlag(Union::Element::State::Disabled, !option->state.testFlag(QStyle::State_Enabled));
    states.setFlag(Union::Element::State::Highlighted, option->state.testFlag(QStyle::State_Selected));

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
            if (optionButton->state.testFlag(QStyle::State_NoChange)) {
                hints.append(u"no-change"_s);
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

            if (optionViewItem->state.testFlag(QStyle::State_Open)) {
                hints.append(u"expanded"_s);
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
        if (const auto opt = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            if (opt->checked) {
                hints.append(u"with-submenu"_s);
            }
            if (opt->menuItemType == QStyleOptionMenuItem::Separator && !opt->text.isEmpty()) {
                hints.append(u"with-title"_s);
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
    case QStyleOption::SO_ViewItem: {
        if (const auto optionViewItem = static_cast<const QStyleOptionViewItem *>(option)) {
            QVariantMap map;
            if (optionViewItem->decorationPosition == QStyleOptionViewItem::Top) {
                map[u"display"_s] = QVariant(u"text-under-icon"_s);
            }
            if (optionViewItem->decorationPosition == QStyleOptionViewItem::Bottom) {
                map[u"display"_s] = QVariant(u"text-below-icon"_s);
            }
            if (optionViewItem->decorationPosition == QStyleOptionViewItem::Left) {
                map[u"display"_s] = QVariant(u"text-after-icon"_s);
            }
            if (optionViewItem->decorationPosition == QStyleOptionViewItem::Right) {
                map[u"display"_s] = QVariant(u"text-before-icon"_s);
            }
            return map;
        }
    }
    case QStyleOption::SO_MenuItem:
    case QStyleOption::SO_TabBarBase:
    case QStyleOption::SO_Default:
    case QStyleOption::SO_FocusRect:
    case QStyleOption::SO_Button:
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
            elementTypes = widgetToElementHierarchy(widget);
        }
    } else {
        elementTypes = {styleOptionToElementName(opt)};
    }

    // Meld duplicate elements that appear next to each other
    if (elementTypes.isEmpty()) {
        elementTypes.append(targetHierarchy);
    } else {
        for (const auto &target : targetHierarchy) {
            if (elementTypes.last() != target) {
                elementTypes.append(target);
            }
        }
    }

    for (const auto &elementType : elementTypes) {
        auto unionElement = Union::Element::create();
        unionElement->setType(elementType);
        unionElement->setStates(statesFromOption(opt));
        unionElement->setHints(hintsFromOption(opt));
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

QStringList widgetToElementHierarchy(const QWidget *widget)
{
    if (!widget) {
        return QStringList();
    }
    QStringList members;
    // We will have to check what items the widget inherits from,
    // as far as I know there is no better way to do this.
    const QMap<const char *, QString> parentClasses = {{"QCheckBox", ElementString::CheckBox},
                                                       {"QRadioButton", ElementString::RadioButton},
                                                       {"QPushButton", ElementString::Button},
                                                       {"QToolButton", ElementString::ToolButton},
                                                       {"QDial", ElementString::Dial},
                                                       {"QScrollBar", ElementString::ScrollBar},
                                                       {"QSlider", ElementString::Slider},
                                                       {"QAbstractSpinBox", ElementString::SpinBox},
                                                       {"QComboBox", ElementString::ComboBox},
                                                       {"QDialog", ElementString::Dialog},
                                                       {"QDialogButtonBox", ElementString::DialogButtonBox},
                                                       {"QDockWidget", ElementString::Dock},
                                                       {"QFocusFrame", ElementString::FocusFrame},
                                                       {"QFrame", ElementString::Frame},
                                                       {"QGroupBox", ElementString::GroupBox},
                                                       {"QKeySequenceEdit", ElementString::KeySequenceEdit},
                                                       {"QLineEdit", ElementString::TextField},
                                                       {"QMainWindow", ElementString::ApplicationWindow},
                                                       {"QMdiSubWinow", ElementString::MdiSubWindow},
                                                       {"QMenu", ElementString::Menu},
                                                       {"QMenuBar", ElementString::MenuBar},
                                                       {"QProgressBar", ElementString::ProgressBar},
                                                       {"QRubberBand", ElementString::RubberBand},
                                                       {"QSizeGrip", ElementString::SizeGrip},
                                                       {"QSplitterHandle", ElementString::SplitterHandle},
                                                       {"QStatusBar", ElementString::StatusBar},
                                                       {"QTabBar", ElementString::TabBar},
                                                       {"QTabWidget", ElementString::TabWidget},
                                                       {"QToolBar", ElementString::ToolBar},
                                                       {"QAbstractScrollArea", ElementString::ScrollArea},
                                                       {"QListView", ElementString::ListView},
                                                       {"QTreeView", ElementString::TreeViewDelegate},
                                                       {"QSplitter", ElementString::Splitter}};

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

QString styleOptionToElementName(const QStyleOption *option)
{
    if (!option) {
        return ElementString::Widget; // Default items that have no styleoption.
    }
    switch ((QStyleOption::OptionType)option->type) {
    case QStyleOption::SO_Default:
        return ElementString::Widget;
    case QStyleOption::SO_FocusRect:
        return ElementString::FocusFrame;
    case QStyleOption::SO_Button:
        return ElementString::Button;
    case QStyleOption::SO_Tab:
        return ElementString::Tab;
    case QStyleOption::SO_MenuItem:
        return ElementString::MenuItem;
    case QStyleOption::SO_Frame:
    case QStyleOption::SO_TabWidgetFrame:
        return ElementString::Frame;
    case QStyleOption::SO_ProgressBar:
        return ElementString::ProgressBar;
    case QStyleOption::SO_ToolBox:
        return ElementString::ToolBox;
    case QStyleOption::SO_Header:
        return ElementString::Header;
    case QStyleOption::SO_DockWidget:
        return ElementString::DockWidget;
    case QStyleOption::SO_ViewItem:
        return ElementString::ItemViewItem;
    case QStyleOption::SO_TabBarBase:
        return ElementString::TabBar;
    case QStyleOption::SO_RubberBand:
        return ElementString::RubberBand;
    case QStyleOption::SO_ToolBar:
        return ElementString::ToolBar;
    case QStyleOption::SO_GraphicsItem:
        return ElementString::GraphicsItem;
    case QStyleOption::SO_Slider:
        return ElementString::Slider;
    case QStyleOption::SO_SpinBox:
        return ElementString::SpinBox;
    case QStyleOption::SO_ToolButton:
        return ElementString::ToolButton;
    case QStyleOption::SO_ComboBox:
        return ElementString::ComboBox;
    case QStyleOption::SO_TitleBar:
        return ElementString::TitleBar;
    case QStyleOption::SO_GroupBox:
        return ElementString::GroupBox;
    case QStyleOption::SO_SizeGrip:
        return ElementString::SizeGrip;
    case QStyleOption::SO_CustomBase:
        // Just return "Widget"
        break;
    // Handle complex cases by casting in case they resolve to one of these
    case QStyleOption::SO_Complex:
    case QStyleOption::SO_ComplexCustomBase: {
        if (qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            return ElementString::ComboBox;
        }
        if (qstyleoption_cast<const QStyleOptionGroupBox *>(option)) {
            return ElementString::GroupBox;
        }
        if (qstyleoption_cast<const QStyleOptionSizeGrip *>(option)) {
            return ElementString::SizeGrip;
        }
        if (qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            return ElementString::Slider;
        }
        if (qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
            return ElementString::SpinBox;
        }
        if (qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            return ElementString::TitleBar;
        }
        if (qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            return ElementString::ToolButton;
        }
    } break;
    }
    return ElementString::Widget;
}

QMap<QString, LayoutItem> layoutMap(const Union::ElementList &elements, const QStyleOption *opt, const QStringList &subElementList)
{
    QMap<QString, LayoutItem> map;
    QList<LayoutItem> items;
    QStringList subElements = subElementList;

    // If subelement list is empty, just use default widget item.
    // This ensures any custom components get layouted too.
    if (subElements.empty()) {
        qDebug() << "No sublements given, using Widget placeholder for" << elements << opt->type << opt->styleObject;
        subElements = {ElementString::Widget};
    }

    // TODO: Go through all elements, create rectangles for them
    // Then place and resize those rectangles according to hierarchy
    // Use the original opt->rect as the main container
    // move any subelements in it according their given rules

    // Get spacing for main item
    auto properties = queryProperties(elements);
    QRectF availableSpace = backgroundRectangle(opt, properties);
    int globalSpacing = 0;
    QMarginsF padding;
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
        if (subElement != ElementString::Icon && subElement != ElementString::Text) {
            auto unionElement = Union::Element::create();
            unionElement->setType(subElement);
            unionElement->setStates(statesFromOption(opt));
            unionElement->setHints(hintsFromOption(opt));
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
        if (subElement == ElementString::Icon) {
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
        } else if (subElement == ElementString::Text || subElement == ElementString::ShortcutText) {
            horizontalAlignment = properties->text()->alignment()->horizontal().value_or(Union::Properties::Alignment::Unspecified);
            verticalAlignment = properties->text()->alignment()->vertical().value_or(Union::Properties::Alignment::Unspecified);
            auto optiontext = textFromOption(opt);
            // if we are a menuitem and have a shortcut, we need to split the text with /t and place them according their alignments
            const int tabPosition(optiontext.indexOf(QLatin1Char('\t')));
            if (tabPosition >= 0) {
                QString accelerator(optiontext.mid(tabPosition + 1));
                if (subElement == ElementString::ShortcutText) {
                    optiontext = optiontext.mid(tabPosition + 1);
                } else {
                    optiontext = optiontext.left(tabPosition);
                }
            }
            // When layouting, ensure we take mnemonics into account
            auto textFlags = textFlagsFromProperties(properties, true);
            textFlags |= Qt::TextShowMnemonic;
            auto fontMetrics = opt->fontMetrics;
            if (properties->text() && properties->text()->font().has_value()) {
                fontMetrics = QFontMetrics(properties->text()->font().value());
            }
            elementRect = fontMetrics.boundingRect(availableSpace.toRect(), textFlags, optiontext);
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
            qCWarning(UNION_QTWIDGETS) << "StackFill/StackCenter is not supported for horizontal alignment!";
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
    if (textWrap != Qt::TextDontClip) {
        textFlags |= textWrap;
    }
    textFlags |= textElide;
    return textFlags;
}

QRectF centerRect(const QRectF &rect, int width, int height)
{
    return QRect(rect.left() + (rect.width() - width) / 2, rect.top() + (rect.height() - height) / 2, width, height);
}
