// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "UnionStyle.h"
#include "BackgroundDrawing.h"
#include "SharedNames.h"
#include "StyleUtils.h"
#include "elements/ButtonElement.h"
#include "elements/CheckElement.h"
#include "elements/ComboBoxElement.h"
#include "elements/FrameElement.h"
#include "elements/GroupBoxElement.h"
#include "elements/HeaderElement.h"
#include "elements/ItemViewElement.h"
#include "elements/LineEditElement.h"
#include "elements/MenuElement.h"
#include "elements/MenuItemElement.h"
#include "elements/ProgressBarElement.h"
#include "elements/ScrollBarElement.h"
#include "elements/SliderElement.h"
#include "elements/SpinBoxElement.h"
#include "elements/TabElement.h"
#include "elements/TitleBarElement.h"
#include "elements/ToolButtonElement.h"

#include <ElementQuery.h>
#include <QApplication>
#include <StyleRegistry.h>

#include <QApplication>
#include <QBitmap>
#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFormLayout>
#include <QGraphicsItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QGroupBox>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMainWindow>
#include <QMdiArea>
#include <QMenu>
#include <QMenuBar>
#include <QMetaEnum>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QStackedLayout>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTableView>
#include <QTextBrowser>
#include <QTextEdit>
#include <QToolBar>
#include <QToolBox>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>
#include <QWidgetAction>

using namespace Qt::StringLiterals;

UnionStyle::UnionStyle()
    : QCommonStyle()
    , m_showMnemonics(false)
{
    qApp->installEventFilter(this);
    Union::StyleRegistry::instance()->load();
}

void UnionStyle::drawControl(QStyle::ControlElement controlElement, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // Make lines not look completely terrible on fractional scales
    painter->setRenderHint(QPainter::Antialiasing, true);
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());
    switch (controlElement) {
    case QStyle::CE_ComboBoxLabel:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            ev->drawIcon(painter);
            if (!ev->isEditable()) {
                ev->drawText(painter);
            }
        }
        return;
    case QStyle::CE_PushButtonBevel:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::CE_PushButtonLabel:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->drawIcon(painter);
            ev->drawText(painter);
            ev->drawIndicator(painter);
        }
        return;
    case QStyle::CE_PushButton: {
        drawControl(CE_PushButtonBevel, option, painter, widget);
        drawControl(CE_PushButtonLabel, option, painter, widget);
    }
        return;
    case QStyle::CE_ToolButtonLabel:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->drawIcon(painter);
            ev->drawText(painter);
            ev->drawIndicator(painter);
        }
        return;
    case QStyle::CE_CheckBoxLabel:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::CheckBox)) {
            ev->drawText(painter);
        }
        return;
    case QStyle::CE_CheckBox:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::CheckBox)) {
            ev->drawBackground(painter);
            ev->drawIndicator(painter);
            drawControl(CE_CheckBoxLabel, option, painter, widget);
        }
        return;
    case QStyle::CE_RadioButtonLabel:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::RadioButton)) {
            ev->drawText(painter);
        }
        return;
    case QStyle::CE_RadioButton:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::RadioButton)) {
            ev->drawBackground(painter);
            ev->drawIndicator(painter);
            drawControl(CE_RadioButtonLabel, option, painter, widget);
        }
        return;
    case QStyle::CE_MenuItem:
        if (auto ev = cachedElement<MenuItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CE_TabBarTabShape:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::CE_TabBarTabLabel:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            // TODO: handle vertical tabs
            if (ev->isVertical()) {
                QCommonStyle::drawControl(controlElement, option, painter, widget);
            } else {
                ev->drawIcon(painter);
                ev->drawText(painter);
            }
        }
        return;
    case QStyle::CE_TabBarTab:
        drawControl(CE_TabBarTabShape, option, painter, widget);
        drawControl(CE_TabBarTabLabel, option, painter, widget);
        return;
    case QStyle::CE_ItemViewItem:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CE_ProgressBarGroove:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::CE_ProgressBarContents:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            ev->drawIndicator(painter);
        }
        return;
    case QStyle::CE_ProgressBarLabel:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            ev->drawText(painter);
        }
        return;
    case QStyle::CE_ProgressBar:
        drawControl(CE_ProgressBarGroove, option, painter, widget);
        drawControl(CE_ProgressBarContents, option, painter, widget);
        drawControl(CE_ProgressBarLabel, option, painter, widget);
        return;
    case QStyle::CE_ScrollBarSlider:
        drawElementBackground(painter, option, widget, {ElementString::Handle});
        return;
    case QStyle::CE_ShapedFrame:
        drawElementBackground(painter, option, widget, {ElementString::Frame});
        return;
    case QStyle::CE_FocusFrame:
        drawElementBackground(painter, option, widget, {ElementString::FocusFrame});
        return;
    case QStyle::CE_ToolBar:
        drawElementBackground(painter, option, widget);
        return;
    case QStyle::CE_MenuBarItem:
        drawElementBackground(painter, option, widget, {ElementString::MenuItem});
        return;
    case QStyle::CE_HeaderLabel:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            ev->drawIcon(painter);
            ev->drawText(painter);
        }
        return;
    case QStyle::CE_HeaderSection:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::CE_Header:
        drawControl(CE_HeaderSection, option, painter, widget);
        drawControl(CE_HeaderLabel, option, painter, widget);
        return;
    case QStyle::CE_Splitter:
        drawElementBackground(painter, option, widget, {ElementString::Splitter});
        return;
    case QStyle::CE_RubberBand:
        drawElementBackground(painter, option, widget, {ElementString::RubberBand});
        return;
    case QStyle::CE_SizeGrip:
        drawElementBackground(painter, option, widget, {ElementString::SizeGrip});
        return;
    case QStyle::CE_DockWidgetTitle:
        if (const auto dockOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            auto textRect = subElementRect(SE_DockWidgetTitleBarText, option, widget);
            drawText(textRect, dockOption, painter, dockOption->title, widget);
        }
        return;
    // Ignored
    case QStyle::CE_MenuEmptyArea:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuHMargin:
        return;
    // Scrollbar buttons are also ignored for now since they do not exist in qtquick
    case QStyle::CE_ScrollBarAddLine:
    case QStyle::CE_ScrollBarSubLine:
    case QStyle::CE_ScrollBarAddPage:
    case QStyle::CE_ScrollBarSubPage:
    case QStyle::CE_ScrollBarFirst:
    case QStyle::CE_ScrollBarLast:
        return;
    // TODO: Turn these into their own element
    case QStyle::CE_ToolBoxTabShape:
    case QStyle::CE_ToolBoxTabLabel:
    case QStyle::CE_ToolBoxTab:
        break;
    // Rely on QCommonStyle
    case QStyle::CE_MenuScroller:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_HeaderEmptyArea:
    case QStyle::CE_MenuBarEmptyArea:
    case QStyle::CE_ColumnViewGrip: // Undocumented??
    case QStyle::CE_CustomBase:
        break;
    }

    QCommonStyle::drawControl(controlElement, option, painter, widget);
}

// Complex controls are bit annoying. We may need to manually handle some things to make sure they work correctly
void UnionStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
    // Make lines not look completely terrible on fractional scales
    painter->setRenderHint(QPainter::Antialiasing, true);
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());
    switch (control) {
    case QStyle::CC_ToolButton:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CC_GroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CC_ComboBox:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            ev->drawBackground(painter);
            // Do not draw the text and icon again, as its being handled by QStyle in CE_ComboBoxLabel
            ev->drawIndicator(painter);
        }
        return;
    case QStyle::CC_SpinBox:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CC_ScrollBar:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CC_Slider:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    case QStyle::CC_TitleBar:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            ev->draw(painter);
        }
        return;
    // Rely on QCommonStyle
    case QStyle::CC_Dial: // TODO: need to make the dial from scratch to get this to work :(
    case QStyle::CC_MdiControls:
    case QStyle::CC_CustomBase:
        break;
    }

    QCommonStyle::drawComplexControl(control, option, painter, widget);
}

QStyle::SubControl
UnionStyle::hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &point, const QWidget *widget) const
{
    switch (control) {
    // Make scrollbar behave like in QtQuick
    case CC_ScrollBar: {
        auto grooveRect = subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget);
        if (grooveRect.contains(point)) {
            const auto sliderRect = subControlRect(CC_ScrollBar, option, SC_ScrollBarSlider, widget);
            const auto precedes = [](const QStyleOptionComplex *option, QPoint point, QRect rect) {
                if (option->state & QStyle::State_Horizontal) {
                    if (option->direction == Qt::LeftToRight) {
                        return point.x() < rect.right();
                    } else {
                        return point.x() > rect.x();
                    }
                } else {
                    return point.y() < rect.y();
                }
            };
            if (sliderRect.contains(point)) {
                return SC_ScrollBarSlider;
            } else if (precedes(option, point, sliderRect)) {
                return SC_ScrollBarSubPage;
            } else {
                return SC_ScrollBarAddPage;
            }
        }
    }
    default:
        return QCommonStyle::hitTestComplexControl(control, option, point, widget);
    }
}

void UnionStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // Make lines not look completely terrible on fractional scales
    painter->setRenderHint(QPainter::Antialiasing, true);
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());

    switch (element) {
    case QStyle::PE_FrameStatusBarItem:
        drawElementBackground(painter, option, widget, {ElementString::Item});
        return;
    case QStyle::PE_FrameMenu:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_Widget:
        // Relates to PE_Frame
        drawElementBackground(painter, option, widget, {ElementString::Panel});
        return;
        // Standalone elements
    case QStyle::PE_PanelLineEdit:
        // For spinboxes and comboboxes, we do not want to draw this element
        // TODO: maybe this should be handleable by the CSS
        if (!widget || widget->parentWidget()->inherits("QComboBox") || widget->parentWidget()->inherits("QAbstractSpinBox")) {
            return;
        }
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::PE_PanelItemViewItem:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::PE_PanelItemViewRow:
        drawElementBackground(painter, option, widget, {ElementString::ItemViewRow});
        return;
    case QStyle::PE_PanelScrollAreaCorner:
        drawElementBackground(painter, option, widget, {ElementString::ScrollAreaCorner});
        return;
    case QStyle::PE_PanelTipLabel:
        drawElementBackground(painter, option, widget, {ElementString::ToolTip});
        return;
    case QStyle::PE_FrameFocusRect:
        drawElementBackground(painter, option, widget, {ElementString::FocusFrame});
        return;
        // Indicators
    case QStyle::PE_IndicatorCheckBox:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::CheckBox)) {
            ev->drawIndicator(painter);
        }
        return;
    case QStyle::PE_IndicatorRadioButton:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::RadioButton)) {
            ev->drawIndicator(painter);
        }
        return;
    case QStyle::PE_IndicatorArrowLeft: {
        const auto icon = queryIcon(option, widget, u"arrow-left-symbolic"_s, {ElementString::IndicatorArrowLeft});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowUp: {
        const auto icon = queryIcon(option, widget, u"arrow-up-symbolic"_s, {ElementString::IndicatorArrowUp});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowRight: {
        const auto icon = queryIcon(option, widget, u"arrow-right-symbolic"_s, {ElementString::IndicatorArrowRight});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowDown: {
        const auto icon = queryIcon(option, widget, u"arrow-down-symbolic"_s, {ElementString::IndicatorArrowDown});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorSpinPlus:
    case QStyle::PE_IndicatorSpinMinus:
    case QStyle::PE_IndicatorSpinUp:
    case QStyle::PE_IndicatorSpinDown: {
        auto up = (element == PE_IndicatorSpinUp);
        auto spinboxElements = prepareElements(option, widget);
        auto element = Union::Element::create();
        element->setType(ElementString::Indicator);
        element->setStates(statesFromOption(option));
        auto hints = hintsFromOption(option);
        // Use the constrained look for now
        hints.append(up ? u"Increase"_s : u"Decrease"_s);
        element->setHints(hints);
        element->setAttributes(attributesFromOption(option));
        spinboxElements.append(element);
        auto props = queryProperties(spinboxElements);
        drawBackgroundRectangle(painter, option->rect, props);
        if (props->icon()) {
            auto icon = QIcon::fromTheme(props->icon()->name().value_or(up ? u"arrow-up-symbolic"_s : u"arrow-down-symbolic"_s));
            drawIcon(option->rect, option, painter, icon, widget);
        }
    }
        return;
    case QStyle::PE_FrameLineEdit:
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_FrameButtonBevel:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_PanelButtonCommand:
    case QStyle::PE_PanelButtonBevel:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::PE_FrameDefaultButton:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_FrameButtonTool:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_PanelButtonTool:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::PE_FrameGroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_Frame:
        if (auto ev = cachedElement<FrameElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->drawFrame(painter);
        }
        return;
    case QStyle::PE_FrameDockWidget:
    case QStyle::PE_FrameTabWidget:
    case QStyle::PE_FrameWindow:
    case QStyle::PE_FrameTabBarBase:
    case QStyle::PE_PanelMenuBar:
    case QStyle::PE_PanelToolBar:
    case QStyle::PE_PanelStatusBar:
        drawElementBackground(painter, option, widget);
        return;
    case QStyle::PE_PanelMenu:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            ev->drawBackground(painter);
        }
        return;
    case QStyle::PE_IndicatorBranch: {
        auto defaultIconName = QString();
        if (option->state.testFlag(State_Children)) {
            if (option->state.testFlag(QStyle::State_Item)) {
                defaultIconName = u"arrow-right-symbolic"_s;
            }
            if (option->state.testFlag(QStyle::State_Open)) {
                defaultIconName = u"arrow-down-symbolic"_s;
            }
        }
        const auto icon = queryIcon(option, widget, defaultIconName, {ElementString::IndicatorBranch});
        auto size = querySize(option, widget, {ElementString::TreeViewDelegate, ElementString::Indicator});
        auto rect = centerRect(option->rect, size.width(), size.height());
        drawIcon(rect, option, painter, icon, widget);
    }
        return;
    case QStyle::PE_IndicatorButtonDropDown: {
        const auto icon = queryIcon(option, widget, u"arrow-down-symbolic"_s, {ElementString::IndicatorButtonDropDown});
        drawIcon(option->rect, option, painter, icon, widget);
    }
        return;
    case QStyle::PE_IndicatorMenuCheckMark:
    case QStyle::PE_IndicatorItemViewItemCheck:
        drawPrimitive(PE_IndicatorCheckBox, option, painter, widget);
        return;
    case QStyle::PE_IndicatorHeaderArrow: {
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            ev->drawIcon(painter);
        } else {
            // Fallback
            if (option->state.testFlags(State_UpArrow)) {
                drawPrimitive(PE_IndicatorArrowUp, option, painter, widget);
            } else if (option->state.testFlags(State_DownArrow)) {
                drawPrimitive(PE_IndicatorArrowDown, option, painter, widget);
            }
        }
    }
        return;
    case QStyle::PE_IndicatorProgressChunk:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            ev->drawChunk(painter);
        }
        return;
    case QStyle::PE_IndicatorToolBarHandle:
        drawElementBackground(painter, option, widget, {ElementString::IndicatorToolBarHandle});
        return;
    case QStyle::PE_IndicatorToolBarSeparator:
        drawElementBackground(painter, option, widget, {ElementString::IndicatorToolBarSeparator});
        return;
    case QStyle::PE_IndicatorColumnViewArrow:
        drawPrimitive(PE_IndicatorArrowRight, option, painter, widget);
        return;
    case QStyle::PE_IndicatorTabClose: {
        drawElementBackground(painter, option, widget, {ElementString::Tab, ElementString::CloseButton});
        const auto icon = queryIcon(option, widget, u"tab-close-symbolic"_s, {ElementString::IndicatorTabClose});
        drawIcon(option->rect, option, painter, icon, widget);
    }
        return;
    // Handle with QCommonStyle for now
    case QStyle::PE_IndicatorTabTear:
    case QStyle::PE_IndicatorTabTearRight:
    case QStyle::PE_IndicatorItemViewItemDrop:
    case QStyle::PE_IndicatorDockWidgetResizeHandle:
    case QStyle::PE_CustomBase:
        break;
    }

    QCommonStyle::drawPrimitive(element, option, painter, widget);
}

QSize UnionStyle::sizeFromContents(QStyle::ContentsType contentsType, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());
    switch (contentsType) {
    case QStyle::CT_PushButton: {
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
    } break;
    case QStyle::CT_ToolButton:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ;
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_MenuItem:
        if (auto ev = cachedElement<MenuItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_ComboBox:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_TabBarTab:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_Slider:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_ItemViewItem:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_SpinBox:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_ScrollBar:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_CheckBox:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::CheckBox)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_RadioButton:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::RadioButton)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_GroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_ProgressBar:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_HeaderSection:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_LineEdit:
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    case QStyle::CT_Menu:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            return ev->contentsSize(contentsSize);
        }
        break;
    // Use defaults from qcommonstyle
    case QStyle::CT_MenuBar:
    case QStyle::CT_TabWidget:
    case QStyle::CT_Splitter:
    case QStyle::CT_MenuBarItem:
    case QStyle::CT_SizeGrip:
    case QStyle::CT_DialogButtons:
    case QStyle::CT_MdiControls:
    case QStyle::CT_CustomBase:
        break;
    }
    return QCommonStyle::sizeFromContents(contentsType, option, contentsSize, widget);
}

QRect UnionStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect;
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());

    switch (element) {
    case QStyle::SE_ItemViewItemText:
    case QStyle::SE_ItemViewItemDecoration:
    case QStyle::SE_ItemViewItemCheckIndicator:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            return ev->subElementRect(element);
        }
        break;
    case QStyle::SE_RadioButtonContents:
    case QStyle::SE_RadioButtonIndicator:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::RadioButton)) {
            return ev->subElementRect(element);
        }
        break;
    case QStyle::SE_CheckBoxIndicator:
    case QStyle::SE_CheckBoxContents:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::CheckBox)) {
            return ev->subElementRect(element);
        }
        break;
    case QStyle::SE_PushButtonFocusRect:
    case QStyle::SE_PushButtonContents:
    case QStyle::SE_PushButtonBevel: {
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->subElementRect(element);
        }
    } break;
    case QStyle::SE_LineEditContents:
    case QStyle::SE_ShapedFrameContents:
    case QStyle::SE_FrameContents: {
        auto frameElements = prepareElements(option, widget);
        auto props = queryProperties(frameElements);
        int frameWidth = pixelMetric(PM_DefaultFrameWidth, option, widget);
        rect = backgroundRectangle(option, props).toRect().adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);
    } break;
    case QStyle::SE_HeaderArrow:
    case QStyle::SE_HeaderLabel:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->subElementRect(element);
        }
        break;
    case QStyle::SE_TabBarTabText:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            // TODO: handle vertical tabs
            if (ev->isVertical()) {
                return QCommonStyle::subElementRect(element, option, widget);
            }
            return ev->subElementRect(element);
        }
        break;
    case QStyle::SE_ProgressBarLabel:
    case QStyle::SE_ProgressBarContents:
    case QStyle::SE_ProgressBarGroove:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            return ev->subElementRect(element);
        }
        break;
    case QStyle::SE_DockWidgetTitleBarText:
    case QStyle::SE_DockWidgetCloseButton:
    case QStyle::SE_DockWidgetFloatButton:
    case QStyle::SE_DockWidgetIcon: {
        if (const auto dockOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            QStringList childelements;
            if (dockOption->closable) {
                childelements.append(ElementString::CloseButton);
            }
            if (dockOption->floatable) {
                childelements.append(ElementString::FloatButton);
            }
            if (!dockOption->title.isEmpty()) {
                childelements.append(ElementString::Text);
            }
            if (childelements.isEmpty()) {
                return QRect();
            }
            auto elements = prepareElements(option, widget, {ElementString::DockWidget});
            auto map = layoutMap(elements, option, childelements);

            if (element == SE_DockWidgetTitleBarText) {
                rect = map[ElementString::Text].rect.toRect();
            }
            if (element == SE_DockWidgetFloatButton) {
                rect = map[ElementString::FloatButton].rect.toRect();
            }
            if (element == SE_DockWidgetCloseButton) {
                rect = map[ElementString::CloseButton].rect.toRect();
            }
            // The styleoption has no icon, yet there is whole thing for an icon? Wtf.
            if (element == SE_DockWidgetIcon) {
                rect = map[ElementString::Icon].rect.toRect();
            }
        }
    } break;
    // Follow defaults
    case QStyle::SE_TreeViewDisclosureItem:
    case QStyle::SE_TabWidgetTabContents:
    case QStyle::SE_ToolBoxTabContents:
    case QStyle::SE_TabBarTabLeftButton:
    case QStyle::SE_TabBarTabRightButton:
    case QStyle::SE_ToolBarHandle:
    case QStyle::SE_TabWidgetLeftCorner:
    case QStyle::SE_TabWidgetRightCorner:
    case QStyle::SE_TabWidgetTabBar:
    case QStyle::SE_TabWidgetTabPane:
    case QStyle::SE_TabBarTearIndicator:
    case QStyle::SE_TabBarTearIndicatorRight:
    case QStyle::SE_TabBarScrollRightButton:
    case QStyle::SE_TabBarScrollLeftButton:
    case QStyle::SE_CustomBase:
    case QStyle::SE_CheckBoxClickRect:
    case QStyle::SE_CheckBoxFocusRect:
    case QStyle::SE_RadioButtonFocusRect:
    case QStyle::SE_RadioButtonClickRect:
    case QStyle::SE_ComboBoxFocusRect:
    case QStyle::SE_SliderFocusRect:
    case QStyle::SE_ItemViewItemFocusRect:
    case QStyle::SE_PushButtonLayoutItem:
    case QStyle::SE_CheckBoxLayoutItem:
    case QStyle::SE_ComboBoxLayoutItem:
    case QStyle::SE_DateTimeEditLayoutItem:
    case QStyle::SE_FrameLayoutItem:
    case QStyle::SE_GroupBoxLayoutItem:
    case QStyle::SE_LabelLayoutItem:
    case QStyle::SE_SpinBoxLayoutItem:
    case QStyle::SE_SliderLayoutItem:
    case QStyle::SE_ProgressBarLayoutItem:
    case QStyle::SE_RadioButtonLayoutItem:
    case QStyle::SE_TabWidgetLayoutItem:
    case QStyle::SE_ToolButtonLayoutItem:
        return QCommonStyle::subElementRect(element, option, widget);
    }

    return visualRect(option->direction, option->rect, rect);
}

QRect UnionStyle::subControlRect(ComplexControl complexControl, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());
    switch (complexControl) {
    case QStyle::CC_ToolButton:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_ComboBox:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_SpinBox:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_ScrollBar:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_Slider:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_GroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_TitleBar:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            return ev->subControlRect(subControl);
        }
        break;
    case QStyle::CC_Dial:
    case QStyle::CC_MdiControls:
    case QStyle::CC_CustomBase:
        break;
    }

    // TODO: Leave Dial and MDIControls to QCommonStyle for now
    return QCommonStyle::subControlRect(complexControl, option, subControl, widget);
}

int UnionStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    // We do not use elements here since *any* element can ask for these values.
    int defaultMetric = QCommonStyle::pixelMetric(metric, option, widget);
    auto elements = prepareElements(option, widget);
    if (elements.isEmpty()) {
        return defaultMetric;
    }
    auto properties = queryProperties(elements);
    if (!properties) {
        return defaultMetric;
    }
    const auto hash = qHash(option, QHashSeed::globalSeed()) + qHash(widget, QHashSeed::globalSeed());

    switch (metric) {
    // Don't shift button text when sunken
    case QStyle::PM_TabBarTabShiftHorizontal:
    case QStyle::PM_TabBarTabShiftVertical:
    case QStyle::PM_ButtonShiftHorizontal:
    case QStyle::PM_ButtonShiftVertical:
    case QStyle::PM_TabBar_ScrollButtonOverlap:
        return 0;
    case QStyle::PM_ScrollView_ScrollBarOverlap:
        return 0;
    // Due to how QWidgets works, we just return the average padding size now
    // since there is no support for returning padding per edge
    // In QStyle "Margin" means "Padding" apparently.
    case QStyle::PM_ToolBarItemMargin:
    case QStyle::PM_MenuBarVMargin:
    case QStyle::PM_MenuBarHMargin:
    case QStyle::PM_FocusFrameVMargin:
    case QStyle::PM_FocusFrameHMargin:
    case QStyle::PM_MenuHMargin:
    case QStyle::PM_MenuVMargin:
    case QStyle::PM_HeaderMargin:
    case QStyle::PM_LineEditIconMargin:
    case QStyle::PM_ButtonMargin: {
        if (properties->layout() && properties->layout()->padding()) {
            auto margins = properties->layout()->padding()->toMargins();
            auto avg = (margins.left() + margins.right() + margins.top() + margins.bottom()) / 4;
            return avg;
        }
    } break;
    case QStyle::PM_ToolBarSeparatorExtent:
    case QStyle::PM_ToolTipLabelFrameWidth:
    case QStyle::PM_ToolBarFrameWidth:
    case QStyle::PM_MenuDesktopFrameWidth:
    case QStyle::PM_MenuBarPanelWidth:
    case QStyle::PM_DefaultFrameWidth:
    case QStyle::PM_SpinBoxFrameWidth:
    case QStyle::PM_ComboBoxFrameWidth:
    case QStyle::PM_DockWidgetFrameWidth:
    case QStyle::PM_MdiSubWindowFrameWidth:
    case QStyle::PM_ButtonDefaultIndicator: {
        if (properties->border()) {
            auto borderSizes = properties->border()->sizes();
            auto avg = (borderSizes.left() + borderSizes.right() + borderSizes.top() + borderSizes.bottom()) / 4;
            return avg;
        }
    } break;
    case QStyle::PM_IndicatorWidth:
    case QStyle::PM_ExclusiveIndicatorWidth:
    case QStyle::PM_MenuButtonIndicator:
    case QStyle::PM_IndicatorHeight:
    case QStyle::PM_TabCloseIndicatorWidth:
    case QStyle::PM_TabCloseIndicatorHeight:
    case QStyle::PM_ExclusiveIndicatorHeight: {
        auto elements = prepareElements(option, widget, {ElementString::Indicator});
        if (elements.isEmpty()) {
            return defaultMetric;
        }
        auto properties = queryProperties(elements);
        if (!properties) {
            return defaultMetric;
        }
        if (properties->layout()) {
            if (metric == PM_IndicatorHeight || metric == PM_ExclusiveIndicatorHeight || metric == PM_TabCloseIndicatorHeight) {
                return properties->layout()->height().value_or(defaultMetric);
            }
            return properties->layout()->width().value_or(defaultMetric);
        }
    } break;
    case QStyle::PM_ScrollBarSliderMin: {
        return 40;
    } break;
    case QStyle::PM_SliderThickness:
    case QStyle::PM_SliderLength:
    case QStyle::PM_ScrollBarExtent: {
        auto scrollbaroption = qstyleoption_cast<const QStyleOptionSlider *>(option);
        if (scrollbaroption && properties->layout()) {
            if (scrollbaroption->orientation == Qt::Horizontal) {
                return properties->layout()->height().value_or(defaultMetric);
            } else {
                return properties->layout()->width().value_or(defaultMetric);
            }
        }
    } break;
    case QStyle::PM_SliderControlThickness: {
        auto scrollbaroption = qstyleoption_cast<const QStyleOptionSlider *>(option);
        auto elements = prepareElements(option, widget, {ElementString::Handle});
        if (elements.isEmpty()) {
            return defaultMetric;
        }
        auto properties = queryProperties(elements);
        if (!properties) {
            return defaultMetric;
        }
        if (scrollbaroption && properties->layout()) {
            QSize size(properties->layout()->width().value_or(defaultMetric), properties->layout()->height().value_or(defaultMetric));
            if (properties->layout()->padding()) {
                size = size.shrunkBy(properties->layout()->padding()->toMargins().toMargins());
            }
            if (scrollbaroption->orientation == Qt::Horizontal) {
                return size.height();
            } else {
                return size.width();
            }
        }
    } break;
    case QStyle::PM_ToolBarHandleExtent:
    case QStyle::PM_ToolBarExtensionExtent: {
        auto toolBarOption = qstyleoption_cast<const QStyleOptionToolBar *>(option);
        QStringList childelements;
        if (metric == PM_ToolBarHandleExtent) {
            childelements.append(ElementString::Handle);
        }
        if (metric == PM_ToolBarExtensionExtent) {
            childelements.append(ElementString::Extension);
        }
        auto elements = prepareElements(option, widget, childelements);
        if (elements.isEmpty()) {
            return defaultMetric;
        }
        auto properties = queryProperties(elements);
        if (!properties) {
            return defaultMetric;
        }
        if (toolBarOption && properties->layout()) {
            if (toolBarOption->state & QStyle::State_Horizontal) {
                return properties->layout()->width().value_or(defaultMetric);
            } else {
                return properties->layout()->height().value_or(defaultMetric);
            }
        }
    } break;
    case QStyle::PM_LayoutLeftMargin:
    case QStyle::PM_LayoutTopMargin:
    case QStyle::PM_LayoutRightMargin:
    case QStyle::PM_LayoutBottomMargin: {
        if (properties->layout() && properties->layout()->padding()) {
            auto margins = properties->layout()->padding()->toMargins();
            if (metric == PM_LayoutLeftMargin) {
                return margins.left();
            }
            if (metric == PM_LayoutTopMargin) {
                return margins.top();
            }
            if (metric == PM_LayoutBottomMargin) {
                return margins.bottom();
            }
            if (metric == PM_LayoutRightMargin) {
                return margins.right();
            }
        }
    } break;
    case QStyle::PM_TabBarTabHSpace:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            return ev->hSpace();
        }
        break;
    case QStyle::PM_TabBarTabVSpace:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            return ev->vSpace();
        }
        break;
    // Currently we only have one spacing value
    case QStyle::PM_CheckBoxLabelSpacing:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::CheckBox)) {
            return ev->labelSpacing();
        }
        break;
    case QStyle::PM_RadioButtonLabelSpacing:
        if (auto ev = cachedCheckElement(hash, option, widget, CheckElement::Type::RadioButton)) {
            return ev->labelSpacing();
        }
        break;
    case QStyle::PM_ScrollView_ScrollBarSpacing:
    case QStyle::PM_MenuBarItemSpacing:
    case QStyle::PM_ToolBarItemSpacing:
    case QStyle::PM_LayoutHorizontalSpacing:
    case QStyle::PM_LayoutVerticalSpacing: {
        if (properties->layout()) {
            return properties->layout()->spacing().value_or(defaultMetric);
        }
    } break;
    case QStyle::PM_TabBarScrollButtonWidth:
        return querySize(option, widget, {ElementString::TabScrollButton}).width();
    case QStyle::PM_MenuPanelWidth:
    case QStyle::PM_SplitterWidth:
        return querySize(option, widget).width();
    case QStyle::PM_TitleBarHeight:
        return querySize(option, widget, {ElementString::TitleBar}).height();
    case QStyle::PM_TitleBarButtonSize:
    case QStyle::PM_TitleBarButtonIconSize:
        return querySize(option, widget, {ElementString::TitleBar, ElementString::NormalButton}).width();
    case QStyle::PM_SpinBoxSliderHeight:
    case QStyle::PM_MenuScrollerHeight:
    case QStyle::PM_TabBarBaseHeight:
        return querySize(option, widget).height();
    case QStyle::PM_TreeViewIndentation:
        return querySize(option, widget, {ElementString::TreeViewDelegate, ElementString::Indentation}).width();
    case QStyle::PM_SmallIconSize:
        return querySize(option, widget, {ElementString::SmallIconSize}).width();
    case QStyle::PM_TabBarIconSize:
        return querySize(option, widget, {ElementString::TabBarIconSize}).width();
    case QStyle::PM_LineEditIconSize:
        return querySize(option, widget, {ElementString::LineEditIconSize}).width();
    case QStyle::PM_ListViewIconSize:
        return querySize(option, widget, {ElementString::ListViewIconSize}).width();
    case QStyle::PM_ButtonIconSize:
        return querySize(option, widget, {ElementString::ButtonIconSize}).width();
    case QStyle::PM_ToolBarIconSize:
        return querySize(option, widget, {ElementString::ToolBarIconSize}).width();
    case QStyle::PM_HeaderMarkSize:
        return querySize(option, widget, {ElementString::HeaderMarkSize}).width();
    case QStyle::PM_IconViewIconSize:
        return querySize(option, widget, {ElementString::IconViewIconSize}).width();
    case QStyle::PM_LargeIconSize:
        return querySize(option, widget, {ElementString::LargeIconSize}).width();
    case QStyle::PM_MessageBoxIconSize:
        return querySize(option, widget, {ElementString::MessageBoxIconSize}).width();
    case QStyle::PM_SizeGripSize:
        return querySize(option, widget, {ElementString::SizeGripSize}).width();
    case QStyle::PM_TextCursorWidth:
        return querySize(option, widget, {ElementString::TextCursorWidth}).width();
    case QStyle::PM_HeaderDefaultSectionSizeHorizontal:
        return querySize(option, widget, {ElementString::HeaderDefaultSectionSize}).width();
    case QStyle::PM_HeaderDefaultSectionSizeVertical:
        return querySize(option, widget, {ElementString::HeaderDefaultSectionSize}).height();
    case QStyle::PM_ProgressBarChunkWidth:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            return ev->chunkWidth();
        }
        break;
    default:
        break;
    };
    return QCommonStyle::pixelMetric(metric, option, widget);
}

// Copied from Breeze.
// TODO: Make these adjustable!
int UnionStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    switch (hint) {
    case SH_RubberBand_Mask: {
        if (auto mask = qstyleoption_cast<QStyleHintReturnMask *>(returnData)) {
            mask->region = option->rect;

            /*
             * need to check on widget before removing inner region
             * in order to still preserve rubberband in MainWindow and QGraphicsView
             * in QMainWindow because it looks better
             * in QGraphicsView because the painting fails completely otherwise
             */
            if (widget
                && (qobject_cast<const QAbstractItemView *>(widget->parent()) || qobject_cast<const QGraphicsView *>(widget->parent())
                    || qobject_cast<const QMainWindow *>(widget->parent()))) {
                return true;
            }

            // also check if widget's parent is some itemView viewport
            if (widget && widget->parent() && qobject_cast<const QAbstractItemView *>(widget->parent()->parent())
                && static_cast<const QAbstractItemView *>(widget->parent()->parent())->viewport() == widget->parent()) {
                return true;
            }

            // mask out center
            mask->region -= option->rect.adjusted(1, 1, -1, -1);

            return true;
        }
        return false;
    }
    case SH_ComboBox_ListMouseTracking:
        return true;
    case SH_MenuBar_MouseTracking:
        return true;
    case SH_Menu_Scrollable:
        return true;
    case SH_Menu_MouseTracking:
        return true;
    case SH_Menu_SubMenuPopupDelay:
        return 150;
    case SH_Menu_SloppySubMenus:
        return true;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case SH_Widget_Animate:
        return StyleConfigData::animationsEnabled();
#endif
    case SH_Menu_SupportsSections:
        return true;
    case SH_Widget_Animation_Duration:
        return 150;
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return true;
    case SH_GroupBox_TextLabelVerticalAlignment:
        return Qt::AlignVCenter;
    case SH_TabBar_Alignment:
        return Qt::AlignLeft;
    case SH_ToolBox_SelectedPageTitleBold:
        return false;
    case SH_ScrollBar_MiddleClickAbsolutePosition:
        return true;
    case SH_ScrollView_FrameOnlyAroundContents:
        return false;
    case SH_FormLayoutFormAlignment:
        return Qt::AlignLeft | Qt::AlignTop;
    case SH_FormLayoutLabelAlignment:
        return Qt::AlignRight;
    case SH_FormLayoutFieldGrowthPolicy:
        return QFormLayout::ExpandingFieldsGrow;
    case SH_FormLayoutWrapPolicy:
        return QFormLayout::DontWrapRows;
    case SH_MessageBox_TextInteractionFlags:
        return Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse;
    case SH_ProgressDialog_CenterCancelButton:
        return false;
    case SH_MessageBox_CenterButtons:
        return false;
    case SH_FocusFrame_AboveWidget:
        return true;
    case SH_FocusFrame_Mask:
        return false;
    case SH_RequestSoftwareInputPanel:
        return RSIP_OnMouseClick;
    case SH_TitleBar_NoBorder:
        return true;
    case SH_DockWidget_ButtonsHaveFrame:
        return false;
    case SH_ScrollBar_LeftClickAbsolutePosition:
        return true;
    case SH_UnderlineShortcut:
        return true;
    default:
        return QCommonStyle::styleHint(hint, option, widget, returnData);
    }
}

void UnionStyle::polish(QApplication *application)
{
    QCommonStyle::polish(application);

    // Set global window color
    auto element = Union::Element::create();
    element->setType(ElementString::ApplicationWindow);

    const auto style = Union::StyleRegistry::instance()->defaultStyle();
    const auto matches = style->matches({element});
    const auto properties = matches.first()->properties();

    QPalette palette;
    if (properties->background() && properties->background()->color()) {
        palette.setColor(QPalette::Window, properties->background()->color().value().toQColor());
    }

    application->setPalette(palette);
}

void UnionStyle::polish(QWidget *widget)
{
    // WA_Hover setup stolen from Breeze
    // enable mouse over effects for all necessary widgets
    if (qobject_cast<QAbstractItemView *>(widget) || qobject_cast<QAbstractSpinBox *>(widget) || qobject_cast<QCheckBox *>(widget)
        || qobject_cast<QComboBox *>(widget) || qobject_cast<QDial *>(widget) || qobject_cast<QLineEdit *>(widget) || qobject_cast<QPushButton *>(widget)
        || qobject_cast<QRadioButton *>(widget) || qobject_cast<QScrollBar *>(widget) || qobject_cast<QSlider *>(widget)
        || qobject_cast<QSplitterHandle *>(widget) || qobject_cast<QTabBar *>(widget) || qobject_cast<QTextEdit *>(widget)
        || qobject_cast<QToolButton *>(widget) || widget->inherits("KTextEditor::View")) {
        widget->setAttribute(Qt::WA_Hover);
    }
    if (auto itemView = qobject_cast<QAbstractItemView *>(widget)) {
        // enable mouse over effects in the viewport of the itemview
        itemView->viewport()->setAttribute(Qt::WA_Hover);

    } else if (auto groupBox = qobject_cast<QGroupBox *>(widget)) {
        // checkable group boxes
        if (groupBox->isCheckable()) {
            groupBox->setAttribute(Qt::WA_Hover);
        }

    } else if (qobject_cast<QAbstractButton *>(widget) && qobject_cast<QDockWidget *>(widget->parent())) {
        widget->setAttribute(Qt::WA_Hover);

    } else if (qobject_cast<QAbstractButton *>(widget) && qobject_cast<QToolBox *>(widget->parent())) {
        widget->setAttribute(Qt::WA_Hover);
    }
    // enable mouse over effect in sunken scrollareas that support focus
    if (auto scrollArea = qobject_cast<QAbstractScrollArea *>(widget)) {
        if (scrollArea->frameShadow() == QFrame::Sunken && scrollArea->focusPolicy() & Qt::StrongFocus) {
            scrollArea->setAttribute(Qt::WA_Hover);
        }
    }
    if (qobject_cast<QScrollBar *>(widget)) {
        // remove opaque painting for scrollbars
        widget->setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

    widget->setProperty(property_union_member_list, widgetToElementHierarchy(widget));

    QCommonStyle::polish(widget);
}

void UnionStyle::drawText(const QRect &rect,
                          const QStyleOption *opt,
                          QPainter *painter,
                          const QString &text,
                          const QWidget *widget,
                          const QColor &overrideColor) const
{
    if (text.isEmpty()) {
        return;
    }

    int textFlags = Qt::AlignLeading | Qt::AlignVCenter;
    const bool enabled = opt->state.testFlag(QStyle::State_Enabled);
    QList<Union::Element::Ptr> elements = prepareElements(opt, widget);
    QColor penColor;
    if (overrideColor.isValid()) {
        penColor = overrideColor;
    } else {
        // TODO: hide mnemonics if requested
        if (!elements.isEmpty()) {
            auto properties = queryProperties(elements);
            auto textColor = properties->text()->color();
            penColor = opt->palette.text().color();
            if (textColor) {
                penColor = textColor->toQColor();
            }
            textFlags = textFlagsFromProperties(properties, true);
        }
    }

    painter->save();
    painter->setPen(penColor);
    drawItemText(painter, rect, textFlags, opt->palette, enabled, text);
    painter->restore();
}

void UnionStyle::drawItemText(QPainter *painter,
                              const QRect &rect,
                              int flags,
                              const QPalette &pal,
                              bool enabled,
                              const QString &text,
                              QPalette::ColorRole textRole) const
{
    flags |= m_showMnemonics ? Qt::TextShowMnemonic : Qt::TextHideMnemonic;
    QCommonStyle::drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void UnionStyle::drawIcon(const QRect &rect, const QStyleOption *opt, QPainter *painter, const QIcon &icon, const QWidget *widget, const QColor &overrideColor)
    const
{
    QList<Union::Element::Ptr> elements = prepareElements(opt, widget);
    const bool enabled = opt->state.testFlag(QStyle::State_Enabled);

    const QPalette activePalette = opt->palette;
    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
    auto iconSize = rect.size();
    // Toolbutton can override the regular icon size
    if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
        // However avoid resizing any icon (like indicators) inside toolbutton, only the main icon
        if (toolButtonOption->icon.name() == icon.name()) {
            iconSize = toolButtonOption->iconSize;
        }
    }
    const QPixmap pixmap = icon.pixmap(iconSize, dpr, enabled ? QIcon::Normal : QIcon::Disabled);

    QColor penColor = opt->palette.text().color(); // Use text color as fallback
    if (overrideColor.isValid()) {
        penColor = overrideColor;
    } else if (!elements.isEmpty()) {
        auto properties = queryProperties(elements);
        if (properties->icon() && properties->icon()->color().has_value()) {
            auto iconColor = properties->icon()->color();
            penColor = iconColor->toQColor();
        }
    }

    painter->save();
    painter->setPen(penColor);
    drawItemPixmap(painter, rect, Qt::AlignCenter, pixmap);
    painter->restore();
}

void UnionStyle::setMnemonics(bool enabled)
{
    if (m_showMnemonics != enabled) {
        m_showMnemonics = enabled;
        const auto widgets = qApp->topLevelWidgets();
        for (QWidget *widget : widgets) {
            widget->update();
        }
    }
}

bool UnionStyle::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Alt) {
            setMnemonics(true);
        }
        break;

    case QEvent::KeyRelease:
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Alt) {
            setMnemonics(false);
        }
        break;

    case QEvent::ApplicationStateChange:
        setMnemonics(false);
        break;

    default:
        break;
    }
    return QCommonStyle::eventFilter(object, event);
}