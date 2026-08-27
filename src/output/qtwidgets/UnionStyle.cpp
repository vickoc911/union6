// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "UnionStyle.h"
#include "BackgroundDrawing.h"
#include "SharedNames.h"
#include "StyleUtils.h"
#include "elements/AbstractElement.h"
#include "elements/ButtonElement.h"
#include "elements/CheckElement.h"
#include "elements/ComboBoxElement.h"
#include "elements/DockWidgetElement.h"
#include "elements/FocusElement.h"
#include "elements/FrameElement.h"
#include "elements/GroupBoxElement.h"
#include "elements/HeaderElement.h"
#include "elements/IndicatorElement.h"
#include "elements/ItemViewElement.h"
#include "elements/LineEditElement.h"
#include "elements/MenuBarElement.h"
#include "elements/MenuBarItemElement.h"
#include "elements/MenuElement.h"
#include "elements/MenuItemElement.h"
#include "elements/ProgressBarElement.h"
#include "elements/RadioButtonElement.h"
#include "elements/RubberBandElement.h"
#include "elements/ScrollAreaCornerElement.h"
#include "elements/ScrollBarElement.h"
#include "elements/SizeGripElement.h"
#include "elements/SliderElement.h"
#include "elements/SpinBoxElement.h"
#include "elements/SplitterElement.h"
#include "elements/StatusBarElement.h"
#include "elements/TabBarElement.h"
#include "elements/TabCloseButtonElement.h"
#include "elements/TabElement.h"
#include "elements/TabWidgetElement.h"
#include "elements/TitleBarElement.h"
#include "elements/ToolBarElement.h"
#include "elements/ToolBoxTabElement.h"
#include "elements/ToolButtonElement.h"
#include "elements/ToolTipElement.h"
#include "elements/TreeViewElement.h"
#include "elements/WidgetElement.h"

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
#include <QRubberBand>
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
#include <qstyle.h>
#include <qstyleoption.h>

#include "ElementCache.h"

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
    const auto hash = qHashMulti(QHashSeed::globalSeed(), controlElement, option, widget);
    auto drawEnums = DrawEnums();
    drawEnums.ControlElement = controlElement;

    switch (controlElement) {
    case QStyle::CE_PushButton:
    case QStyle::CE_PushButtonBevel:
    case QStyle::CE_PushButtonLabel:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_CheckBox:
    case QStyle::CE_CheckBoxLabel:
        if (auto ev = cachedElement<CheckElement, QStyleOptionButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_RadioButton:
    case QStyle::CE_RadioButtonLabel:
        if (auto ev = cachedElement<RadioButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_TabBarTab:
    case QStyle::CE_TabBarTabShape:
    case QStyle::CE_TabBarTabLabel:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ProgressBar:
    case QStyle::CE_ProgressBarGroove:
    case QStyle::CE_ProgressBarContents:
    case QStyle::CE_ProgressBarLabel:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_MenuItem:
        if (auto ev = cachedElement<MenuItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_MenuBarEmptyArea:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_MenuBarItem:
        if (auto ev = cachedElement<MenuBarItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ToolButtonLabel:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_Header:
    case QStyle::CE_HeaderSection:
    case QStyle::CE_HeaderLabel:
    case QStyle::CE_HeaderEmptyArea:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_SizeGrip:
        if (auto ev = cachedElement<SizeGripElement, QStyleOptionSizeGrip>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_Splitter:
        if (auto ev = cachedElement<SplitterElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_RubberBand:
        if (auto ev = cachedElement<RubberBandElement, QStyleOptionRubberBand>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_DockWidgetTitle:
        if (auto ev = cachedElement<DockWidgetElement, QStyleOptionDockWidget>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ScrollBarAddLine:
    case QStyle::CE_ScrollBarSubLine:
    case QStyle::CE_ScrollBarAddPage:
    case QStyle::CE_ScrollBarSubPage:
    case QStyle::CE_ScrollBarSlider:
    case QStyle::CE_ScrollBarFirst:
    case QStyle::CE_ScrollBarLast:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_FocusFrame:
    case QStyle::CE_ShapedFrame:
        if (auto ev = cachedElement<FrameElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ComboBoxLabel:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ToolBar:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ToolBoxTabShape:
    case QStyle::CE_ToolBoxTabLabel:
    case QStyle::CE_ToolBoxTab:
        if (auto ev = cachedElement<ToolBoxTabElement, QStyleOptionToolBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CE_ItemViewItem:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    // Rely on QCommonStyle
    case QStyle::CE_MenuScroller:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuHMargin:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_MenuEmptyArea:
    case QStyle::CE_ColumnViewGrip: // undocumented
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
    const auto hash = qHashMulti(QHashSeed::globalSeed(), control, option, widget);
    auto drawEnums = DrawEnums();
    drawEnums.ComplexControl = control;
    switch (control) {
    case QStyle::CC_ToolButton:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CC_GroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CC_ComboBox:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CC_SpinBox:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CC_ScrollBar:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CC_Slider:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::CC_TitleBar:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
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
            const auto precedes = [](const QStyleOptionComplex *option, QPoint point, QRectF rect) {
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
    const auto hash = qHashMulti(QHashSeed::globalSeed(), element, option, widget);
    auto drawEnums = DrawEnums();
    drawEnums.PrimitiveElement = element;
    switch (element) {
    case QStyle::PE_PanelStatusBar:
    case QStyle::PE_FrameStatusBarItem:
        if (auto ev = cachedElement<StatusBarElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_PanelMenu:
    case QStyle::PE_FrameMenu:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_Widget:
        if (auto ev = cachedElement<WidgetElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameTabWidget:
        if (auto ev = cachedElement<TabWidgetElement, QStyleOptionTabWidgetFrame>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameTabBarBase:
        if (auto ev = cachedElement<TabBarElement, QStyleOptionTabBarBase>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameLineEdit:
    case QStyle::PE_PanelLineEdit:
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_PanelItemViewItem:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_PanelScrollAreaCorner:
        if (auto ev = cachedElement<ScrollAreaCornerElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_PanelTipLabel:
        if (auto ev = cachedElement<ToolTipElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameFocusRect:
        if (auto ev = cachedElement<FocusElement, QStyleOptionFocusRect>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
            ;
        }
        return;
    case QStyle::PE_IndicatorCheckBox:
        if (auto ev = cachedElement<CheckElement, QStyleOptionButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorRadioButton:
        if (auto ev = cachedElement<RadioButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorArrowLeft:
    case QStyle::PE_IndicatorArrowUp:
    case QStyle::PE_IndicatorArrowRight:
    case QStyle::PE_IndicatorArrowDown:
    case QStyle::PE_IndicatorButtonDropDown:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorSpinPlus:
    case QStyle::PE_IndicatorSpinMinus:
    case QStyle::PE_IndicatorSpinUp:
    case QStyle::PE_IndicatorSpinDown:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameDefaultButton:
    case QStyle::PE_FrameButtonBevel:
    case QStyle::PE_PanelButtonCommand:
    case QStyle::PE_PanelButtonBevel:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameButtonTool:
    case QStyle::PE_PanelButtonTool:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameGroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_FrameDockWidget:
    case QStyle::PE_FrameWindow:
    case QStyle::PE_Frame:
        if (auto ev = cachedElement<FrameElement, QStyleOptionFrame>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_PanelMenuBar:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorToolBarHandle:
    case QStyle::PE_IndicatorToolBarSeparator:
    case QStyle::PE_PanelToolBar:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorBranch:
        if (auto ev = cachedElement<TreeViewElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorMenuCheckMark:
    case QStyle::PE_IndicatorItemViewItemCheck:
        drawPrimitive(PE_IndicatorCheckBox, option, painter, widget);
        return;
    case QStyle::PE_IndicatorHeaderArrow: {
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
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
            ev->draw(painter, drawEnums);
        }
        return;
    case QStyle::PE_IndicatorColumnViewArrow:
        drawPrimitive(PE_IndicatorArrowRight, option, painter, widget);
        return;
    case QStyle::PE_IndicatorTabClose:
        if (auto ev = cachedElement<TabCloseButtonElement, QStyleOption>(hash, option, widget)) {
            ev->draw(painter, drawEnums);
        }
        return;
    // Handle with QCommonStyle for now
    case QStyle::PE_PanelItemViewRow:
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
    const auto hash = qHashMulti(QHashSeed::globalSeed(), contentsType, option, contentsSize, widget);
    switch (contentsType) {
    case QStyle::CT_PushButton: {
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
    } break;
    case QStyle::CT_ToolButton:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_MenuItem:
        if (auto ev = cachedElement<MenuItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_ComboBox:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_TabBarTab:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_Slider:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_ItemViewItem:
        if (auto ev = cachedElement<ItemViewElement, QStyleOptionViewItem>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_SpinBox:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_ScrollBar:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_CheckBox:
        if (auto ev = cachedElement<CheckElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_RadioButton:
        if (auto ev = cachedElement<RadioButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_GroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_ProgressBar:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_HeaderSection:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_LineEdit:
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_Menu:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_MenuBar:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_MenuBarItem:
        if (auto ev = cachedElement<MenuBarItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_SizeGrip:
        if (auto ev = cachedElement<RubberBandElement, QStyleOptionRubberBand>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_Splitter:
        if (auto ev = cachedElement<SplitterElement, QStyleOption>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_TabWidget:
        if (auto ev = cachedElement<TabWidgetElement, QStyleOptionTabWidgetFrame>(hash, option, widget)) {
            return ev->contentsSize(contentsSize).toSize();
        }
        break;
    case QStyle::CT_DialogButtons:
    case QStyle::CT_MdiControls:
    case QStyle::CT_CustomBase:
        break;
    }
    return QCommonStyle::sizeFromContents(contentsType, option, contentsSize, widget);
}

QRect UnionStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    const auto hash = qHashMulti(QHashSeed::globalSeed(), element, option, widget);

    auto cached = ElementCache::element<QStyle::SubElement,
                                        QStyle::SE_ItemViewItemText,
                                        QStyle::SE_ItemViewItemDecoration,
                                        QStyle::SE_ItemViewItemCheckIndicator,
                                        // QStyle::SE_RadioButtonContents,
                                        // QStyle::SE_RadioButtonIndicator,
                                        // QStyle::SE_CheckBoxIndicator,
                                        // QStyle::SE_CheckBoxContents,
                                        QStyle::SE_PushButtonFocusRect,
                                        QStyle::SE_PushButtonContents,
                                        QStyle::SE_PushButtonBevel,
                                        QStyle::SE_LineEditContents,
                                        QStyle::SE_HeaderArrow,
                                        QStyle::SE_HeaderLabel //,
                                        // QStyle::SE_TabBarTabText,
                                        // QStyle::SE_ProgressBarLabel,
                                        // QStyle::SE_ProgressBarContents,
                                        // QStyle::SE_ProgressBarGroove,
                                        // QStyle::SE_DockWidgetTitleBarText,
                                        // QStyle::SE_DockWidgetCloseButton,
                                        // QStyle::SE_DockWidgetFloatButton,
                                        // QStyle::SE_DockWidgetIcon,
                                        // QStyle::SE_ToolBoxTabContents
                                        >(element, hash, this, option, widget);
    if (cached) {
        return cached->subElementRect(element).toRect();
    }

    return QCommonStyle::subElementRect(element, option, widget);
}

QRect UnionStyle::subControlRect(ComplexControl complexControl, const QStyleOptionComplex *option, SubControl subControl, const QWidget *widget) const
{
    const auto hash = qHashMulti(QHashSeed::globalSeed(), complexControl, option, subControl, widget);
    switch (complexControl) {
    case QStyle::CC_ToolButton:
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
        }
        break;
    case QStyle::CC_ComboBox:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
        }
        break;
    case QStyle::CC_SpinBox:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
        }
        break;
    case QStyle::CC_ScrollBar:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
        }
        break;
    case QStyle::CC_Slider:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
        }
        break;
    case QStyle::CC_GroupBox:
        if (auto ev = cachedElement<GroupBoxElement, QStyleOptionGroupBox>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
        }
        break;
    case QStyle::CC_TitleBar:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            return ev->subControlRect(subControl).toRect();
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
    const auto hash = qHashMulti(QHashSeed::globalSeed(), metric, option, widget);
    switch (metric) {
    // Don't shift button text when sunken
    case QStyle::PM_TabBarTabShiftHorizontal:
    case QStyle::PM_TabBarTabShiftVertical:
    case QStyle::PM_ButtonShiftHorizontal:
    case QStyle::PM_ButtonShiftVertical:
    case QStyle::PM_TabBar_ScrollButtonOverlap:
        return 0;
    // Don't allow overlap
    case QStyle::PM_ScrollView_ScrollBarOverlap:
        return 0;
    // Due to how QWidgets works, we just return the highest value
    // since there is no support for returning padding per edge
    // In QStyle "Margin" means "Padding" apparently.
    case QStyle::PM_MenuBarVMargin:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return qMax(ev->padding().left(), ev->padding().right());
        }
        break;
    case QStyle::PM_MenuBarHMargin:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return qMax(ev->padding().top(), ev->padding().bottom());
        }
        break;
    case QStyle::PM_FocusFrameVMargin:
        if (auto ev = cachedElement<FocusElement, QStyleOptionFocusRect>(hash, option, widget)) {
            return qMax(ev->padding().left(), ev->padding().right());
        }
        break;
    case QStyle::PM_FocusFrameHMargin:
        if (auto ev = cachedElement<FocusElement, QStyleOptionFocusRect>(hash, option, widget)) {
            return qMax(ev->padding().top(), ev->padding().bottom());
        }
        break;
    case QStyle::PM_MenuHMargin:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            return qMax(ev->padding().left(), ev->padding().right());
        }
        break;
    case QStyle::PM_MenuVMargin:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            return qMax(ev->padding().top(), ev->padding().bottom());
        }
        break;
    // According to the docs: The size of the margin between the sort indicator and the text.
    // So return spacing here.
    case QStyle::PM_HeaderMargin:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->spacing();
        }
        break;
    case QStyle::PM_LineEditIconMargin:
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            auto margins = ev->iconPadding();
            return (margins.left() + margins.right() + margins.top() + margins.bottom()) / 4;
        }
        break;
    case QStyle::PM_ButtonMargin:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->averagePadding();
        }
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            return ev->averagePadding();
        }
        break;
    case QStyle::PM_ToolBarSeparatorExtent:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->separatorExtent();
        }
        break;
    case QStyle::PM_ToolTipLabelFrameWidth:
        if (auto ev = cachedElement<ToolTipElement, QStyleOptionFrame>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_ToolBarFrameWidth:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_MenuDesktopFrameWidth:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_MenuBarPanelWidth:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_DefaultFrameWidth:
        if (auto ev = cachedElement<FrameElement, QStyleOptionFrame>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_SpinBoxFrameWidth:
        if (auto ev = cachedElement<SpinBoxElement, QStyleOptionSpinBox>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_ComboBoxFrameWidth:
        if (auto ev = cachedElement<ComboBoxElement, QStyleOptionComboBox>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_DockWidgetFrameWidth:
        if (auto ev = cachedElement<DockWidgetElement, QStyleOptionDockWidget>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_ButtonDefaultIndicator:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            return ev->averageBorderSize();
        }
        break;
    case QStyle::PM_IndicatorWidth:
        if (auto ev = cachedElement<CheckElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->indicatorSize().width();
        }
        break;
    case QStyle::PM_IndicatorHeight:
        if (auto ev = cachedElement<CheckElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->indicatorSize().height();
        }
        break;
    case QStyle::PM_ExclusiveIndicatorWidth:
        if (auto ev = cachedElement<RadioButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->indicatorSize().width();
        }
        break;
    case QStyle::PM_ExclusiveIndicatorHeight:
        if (auto ev = cachedElement<RadioButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->indicatorSize().height();
        }
        break;
    case QStyle::PM_MenuButtonIndicator:
        if (auto ev = cachedElement<MenuItemElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->indicatorSize().width();
        }
        break;
    case QStyle::PM_TabCloseIndicatorWidth:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            return ev->indicatorSize().width();
        }
        break;
    case QStyle::PM_TabCloseIndicatorHeight:
        if (auto ev = cachedElement<TabElement, QStyleOptionTab>(hash, option, widget)) {
            return ev->indicatorSize().height();
        }
        break;
    case QStyle::PM_SliderLength:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->height();
        }
        break;
    case QStyle::PM_ScrollBarExtent:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->extent();
        }
        break;
    case QStyle::PM_SliderThickness:
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->controlThickness();
        }
        break;
    case QStyle::PM_SliderControlThickness:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->controlThickness();
        }
        if (auto ev = cachedElement<SliderElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->controlThickness();
        }
        break;
    case QStyle::PM_ToolBarHandleExtent:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->handleExtent();
        }
        break;
    case QStyle::PM_ToolBarExtensionExtent:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->extensionExtent();
        }
        break;
    case QStyle::PM_LayoutLeftMargin:
    case QStyle::PM_LayoutTopMargin:
    case QStyle::PM_LayoutRightMargin:
    case QStyle::PM_LayoutBottomMargin: {
        if (auto ev = cachedElement<AbstractElement, QStyleOption>(hash, option, widget)) {
            ev->layout();
            auto margins = ev->padding();
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
    case QStyle::PM_LayoutHorizontalSpacing:
    case QStyle::PM_LayoutVerticalSpacing:
        if (auto ev = cachedElement<AbstractElement, QStyleOption>(hash, option, widget)) {
            return ev->spacing();
        }
        break;
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
        if (auto ev = cachedElement<CheckElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->spacing();
        }
        break;
    case QStyle::PM_RadioButtonLabelSpacing:
        if (auto ev = cachedElement<RadioButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->spacing();
        }
        break;
    case QStyle::PM_MenuBarItemSpacing:
        if (auto ev = cachedElement<MenuBarElement, QStyleOptionMenuItem>(hash, option, widget)) {
            return ev->spacing();
        }
        break;
    case QStyle::PM_ToolBarItemSpacing:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->spacing();
        }
        break;
    case QStyle::PM_ToolBarItemMargin:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->averagePadding();
        }
        break;
    case QStyle::PM_TabBarScrollButtonWidth:
        if (auto ev = cachedElement<TabBarElement, QStyleOptionTabBarBase>(hash, option, widget)) {
            return ev->scrollButtonWidth();
        }
        break;
    case QStyle::PM_MenuPanelWidth:
        if (auto ev = cachedElement<MenuElement, QStyleOption>(hash, option, widget)) {
            return ev->width();
        }
        break;
    case QStyle::PM_SplitterWidth:
        if (auto ev = cachedElement<SplitterElement, QStyleOption>(hash, option, widget)) {
            return ev->width();
        }
        break;
    case QStyle::PM_TitleBarHeight:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            return ev->height();
        }
        break;
    case QStyle::PM_TitleBarButtonSize:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            return ev->buttonWidth();
        }
        break;
    case QStyle::PM_TitleBarButtonIconSize:
        if (auto ev = cachedElement<TitleBarElement, QStyleOptionTitleBar>(hash, option, widget)) {
            return ev->iconSize().width();
        }
        break;
    case QStyle::PM_TabBarBaseHeight:
        if (auto ev = cachedElement<TabBarElement, QStyleOptionTabBarBase>(hash, option, widget)) {
            return ev->height();
        }
        break;
    case QStyle::PM_ProgressBarChunkWidth:
        if (auto ev = cachedElement<ProgressBarElement, QStyleOptionProgressBar>(hash, option, widget)) {
            return ev->chunkWidth();
        }
        break;
    case QStyle::PM_ScrollBarSliderMin:
        if (auto ev = cachedElement<ScrollBarElement, QStyleOptionSlider>(hash, option, widget)) {
            return ev->minimumSize();
        }
        break;
    case QStyle::PM_ButtonIconSize:
        if (auto ev = cachedElement<ButtonElement, QStyleOptionButton>(hash, option, widget)) {
            return ev->iconSize().width();
        }
        if (auto ev = cachedElement<ToolButtonElement, QStyleOptionToolButton>(hash, option, widget)) {
            return ev->iconSize().width();
        }
        break;
    case QStyle::PM_TabBarIconSize:
        if (auto ev = cachedElement<TabBarElement, QStyleOptionTabBarBase>(hash, option, widget)) {
            return ev->iconSize().width();
        }
        break;
    case QStyle::PM_LineEditIconSize:
        if (auto ev = cachedElement<LineEditElement, QStyleOptionFrame>(hash, option, widget)) {
            return ev->iconSize().width();
        }
        break;
    case QStyle::PM_ToolBarIconSize:
        if (auto ev = cachedElement<ToolBarElement, QStyleOptionToolBar>(hash, option, widget)) {
            return ev->iconSize().width();
        }
        break;
    case QStyle::PM_SizeGripSize:
        if (auto ev = cachedElement<SizeGripElement, QStyleOptionSizeGrip>(hash, option, widget)) {
            return ev->width();
        }
        break;
    case QStyle::PM_HeaderDefaultSectionSizeHorizontal:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->width();
        }
        break;
    case QStyle::PM_HeaderDefaultSectionSizeVertical:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->height();
        }
        break;
    case QStyle::PM_HeaderMarkSize:
        if (auto ev = cachedElement<HeaderElement, QStyleOptionHeader>(hash, option, widget)) {
            return ev->indicatorSize().width();
        }
        break;
    case QStyle::PM_TreeViewIndentation:
        if (auto ev = cachedElement<TreeViewElement, QStyleOption>(hash, option, widget)) {
            return ev->indentation();
        }
        break;
    case QStyle::PM_ListViewIconSize:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            return ev->listViewIconSize();
        }
        break;
    case QStyle::PM_SmallIconSize:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            return ev->smallIconSize();
        }
        break;
    case QStyle::PM_IconViewIconSize:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            return ev->iconViewIconSize();
        }
        break;
    case QStyle::PM_LargeIconSize:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            return ev->largeIconSize();
        }
        break;
    case QStyle::PM_MessageBoxIconSize:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            return ev->messageBoxIconSize();
        }
        break;
    case QStyle::PM_TextCursorWidth:
        if (auto ev = cachedElement<IndicatorElement, QStyleOption>(hash, option, widget)) {
            return ev->textCursorWidth();
        }
        break;
    // Unimplemented, use QCommonStyle for now
    case QStyle::PM_MenuScrollerHeight:
    case QStyle::PM_SpinBoxSliderHeight:
    case QStyle::PM_ScrollView_ScrollBarSpacing:
    case QStyle::PM_MdiSubWindowFrameWidth:
    case QStyle::PM_MaximumDragDistance:
    case QStyle::PM_SliderTickmarkOffset:
    case QStyle::PM_SliderSpaceAvailable:
    case QStyle::PM_DockWidgetSeparatorExtent:
    case QStyle::PM_DockWidgetHandleExtent:
    case QStyle::PM_TabBarTabOverlap:
    case QStyle::PM_TabBarBaseOverlap:
    case QStyle::PM_MenuTearoffHeight:
    case QStyle::PM_DialogButtonsSeparator: // Deprecated
    case QStyle::PM_DialogButtonsButtonWidth: // Deprecated
    case QStyle::PM_DialogButtonsButtonHeight: // Deprecated
    case QStyle::PM_MdiSubWindowMinimizedWidth:
    case QStyle::PM_HeaderGripMargin:
    case QStyle::PM_DockWidgetTitleMargin:
    case QStyle::PM_DockWidgetTitleBarButtonMargin:
    case QStyle::PM_SubMenuOverlap:
    case QStyle::PM_CustomBase:
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

QIcon UnionStyle::unionIcon(Union::Properties::StylePropertyGroup *properties, const QString &defaultName) const
{
    if (properties && properties->icon()) {
        auto name = properties->icon()->name().value_or(defaultName);
        if (!name.isEmpty()) {
            QColor color = standardPalette().text().color();
            if (properties->icon()->color()) {
                color = properties->icon()->color()->toQColor();
            }
            return Union::StyleRegistry::instance()->platform()->platformIcon(name, color);
        }
    }
    return QIcon::fromTheme(defaultName);
}
