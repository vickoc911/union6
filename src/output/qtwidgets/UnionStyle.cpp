// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "UnionStyle.h"
#include "BackgroundDrawing.h"
#include "StyleUtils.h"
#include "elements/ButtonElement.h"
#include "elements/CheckElement.h"
#include "elements/HeaderElement.h"
#include "elements/ItemViewElement.h"
#include "elements/MenuItemElement.h"
#include "elements/ProgressBarElement.h"
#include "elements/TabElement.h"
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
/*
 * How this works:
 * We polish the items which should get the hierarchy information
 * Create the needed areas/rectangles/sizes from the layoutmap
 * Utilize those areas for drawing the elements in right places
 */

UnionStyle::UnionStyle()
    : QCommonStyle()
{
    Union::StyleRegistry::instance()->load();
}

void UnionStyle::drawControl(QStyle::ControlElement controlElement, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // Make lines not look completely terrible on fractional scales
    painter->setRenderHint(QPainter::Antialiasing, true);

    switch (controlElement) {
    case QStyle::CE_ComboBoxLabel: {
        if (const auto comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox *>(option)) {
            if (!comboBoxOption->editable) {
                auto subopt = *comboBoxOption;
                subopt.rect = subControlRect(CC_ComboBox, comboBoxOption, SC_ComboBoxEditField, widget);
                layoutAndDrawIconTextIndicator(&subopt, painter, widget, comboBoxOption->currentIcon, comboBoxOption->currentText);
            }
        }
    }
        return;
    case QStyle::CE_PushButtonBevel: {
        auto ev = ButtonElement::create(option, this, widget);
        ev->drawBg(painter);
    }
        return;
    case QStyle::CE_PushButtonLabel: {
        auto ev = ButtonElement::create(option, this, widget);
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
    case QStyle::CE_ToolButtonLabel: {
        auto ev = ToolButtonElement::create(option, this, widget);
        ev->drawIcon(painter);
        ev->drawText(painter);
        ev->drawIndicator(painter);
    }
        return;
    case QStyle::CE_CheckBoxLabel: {
        auto ev = CheckElement::create(CheckElement::Type::CheckBox, option, this, widget);
        ev->drawText(painter);
    }
        return;
    case QStyle::CE_CheckBox: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            auto ev = CheckElement::create(CheckElement::Type::CheckBox, option, this, widget);
            ev->drawBg(painter);
            ev->drawIndicator(painter);
            drawControl(CE_CheckBoxLabel, buttonOption, painter, widget);
        }
    }
        return;
    case QStyle::CE_RadioButtonLabel: {
        auto ev = CheckElement::create(CheckElement::Type::RadioButton, option, this, widget);
        ev->drawText(painter);
    }
        return;
    case QStyle::CE_RadioButton: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            auto ev = CheckElement::create(CheckElement::Type::RadioButton, option, this, widget);
            ev->drawBg(painter);
            ev->drawIndicator(painter);
            drawControl(CE_RadioButtonLabel, buttonOption, painter, widget);
        }
    }
        return;
    case QStyle::CE_MenuItem: {
        auto ev = MenuItemElement::create(option, this, widget);
        ev->draw(painter);
    }
        return;
    case QStyle::CE_ToolBoxTabShape:
    case QStyle::CE_ToolBoxTabLabel:
    case QStyle::CE_ToolBoxTab:
        return;
    case QStyle::CE_TabBarTabShape: {
        auto ev = TabElement::create(option, this, widget);
        ev->drawBg(painter);
    }
        return;
    case QStyle::CE_TabBarTabLabel: {
        auto ev = TabElement::create(option, this, widget);
        // TODO: handle vertical tabs
        if (ev->isVertical()) {
            QCommonStyle::drawControl(CE_TabBarTabLabel, option, painter, widget);
        } else {
            ev->drawIcon(painter);
            ev->drawText(painter);
        }
    }
        return;
    case QStyle::CE_TabBarTab: {
        if (const auto tabOption = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            drawControl(CE_TabBarTabShape, tabOption, painter, widget);
            drawControl(CE_TabBarTabLabel, tabOption, painter, widget);
        }
    }
        return;
    case QStyle::CE_ItemViewItem: {
        auto ev = ItemViewElement::create(option, this, widget);
        ev->draw(painter);
    }
        return;
    case QStyle::CE_ProgressBarGroove: {
        auto ev = ProgressBarElement::create(option, this, widget);
        ev->drawGroove(painter);
    }
        return;
    case QStyle::CE_ProgressBarContents: {
        auto ev = ProgressBarElement::create(option, this, widget);
        ev->drawTrack(painter);
    }
        return;
    case QStyle::CE_ProgressBarLabel: {
        auto ev = ProgressBarElement::create(option, this, widget);
        ev->drawText(painter);
    }
        return;
    case QStyle::CE_ProgressBar:
        drawControl(CE_ProgressBarGroove, option, painter, widget);
        drawControl(CE_ProgressBarContents, option, painter, widget);
        drawControl(CE_ProgressBarLabel, option, painter, widget);
        return;
    case QStyle::CE_ScrollBarSlider:
        drawElementBackground(painter, option, widget, {u"Handle"_s});
        return;
    case QStyle::CE_ShapedFrame:
        drawElementBackground(painter, option, widget, {u"Frame"_s});
        return;
    case QStyle::CE_FocusFrame:
        drawElementBackground(painter, option, widget, {u"FocusFrame"_s});
        return;
    case QStyle::CE_ToolBar:
        drawElementBackground(painter, option, widget);
        return;
    case QStyle::CE_MenuBarItem:
        drawElementBackground(painter, option, widget, {u"MenuBarItem"_s});
        return;
    case QStyle::CE_HeaderLabel: {
        auto ev = HeaderElement::create(option, this, widget);
        ev->drawIcon(painter);
        ev->drawText(painter);
    }
        return;
    case QStyle::CE_HeaderSection: {
        auto ev = HeaderElement::create(option, this, widget);
        ev->drawBg(painter);
    }
        return;
    case QStyle::CE_Header:
        drawControl(CE_HeaderSection, option, painter, widget);
        drawControl(CE_HeaderLabel, option, painter, widget);
        return;
    case QStyle::CE_Splitter:
        drawElementBackground(painter, option, widget, {u"Splitter"_s});
        return;
    case QStyle::CE_RubberBand:
        drawElementBackground(painter, option, widget, {u"RubberBand"_s});
        return;
    case QStyle::CE_SizeGrip:
        drawElementBackground(painter, option, widget, {u"SizeGrip"_s});
        return;
    case QStyle::CE_DockWidgetTitle: {
        if (const auto dockOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            auto textRect = subElementRect(SE_DockWidgetTitleBarText, option, widget);
            drawText(textRect, dockOption, painter, dockOption->title, widget);
        }
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

    switch (control) {
    case QStyle::CC_ToolButton: {
        auto ev = ToolButtonElement(option, this, widget);
        ev.draw(painter);
    }
        return;
    case QStyle::CC_GroupBox: {
        const auto groupBoxOption = qstyleoption_cast<const QStyleOptionGroupBox *>(option);
        if (!groupBoxOption) {
            return;
        }
        const auto elements = prepareElements(groupBoxOption, widget);
        const auto properties = queryProperties(elements);
        auto rect = backgroundRectangle(groupBoxOption, properties).toRect();
        if (groupBoxOption->subControls.testFlag(QStyle::SC_GroupBoxFrame)) {
            drawBackground(painter, rect, properties);
        }
        if ((groupBoxOption->subControls & QStyle::SC_GroupBoxLabel) && !groupBoxOption->text.isEmpty()) {
            QRect textRect = subControlRect(CC_GroupBox, option, SC_GroupBoxLabel, widget);
            QColor textColor = properties->text()->color().value().toQColor();

            painter->setPen(textColor);
            drawItemText(painter,
                         textRect,
                         textFlagsFromProperties(properties, false),
                         groupBoxOption->palette,
                         groupBoxOption->state & State_Enabled,
                         groupBoxOption->text,
                         textColor.isValid() ? QPalette::NoRole : QPalette::WindowText);
        }

        if (auto groupBox = qobject_cast<const QGroupBox *>(widget)) {
            if (groupBox->isCheckable()) {
                QStyleOptionButton checkbox;
                checkbox.rect = subControlRect(CC_GroupBox, option, SC_GroupBoxCheckBox, widget);
                checkbox.state = groupBoxOption->state;
                drawPrimitive(PE_IndicatorCheckBox, &checkbox, painter, widget);
            }
        }
    }
        return;
    case QStyle::CC_ComboBox: {
        const auto comboBoxOption = qstyleoption_cast<const QStyleOptionComboBox *>(option);
        if (!comboBoxOption) {
            return;
        }
        drawElementBackground(painter, comboBoxOption, widget);
        drawControl(CE_ComboBoxLabel, comboBoxOption, painter, widget);
        auto indicatorRect = subControlRect(CC_ComboBox, comboBoxOption, SC_ComboBoxArrow, widget);
        const auto props = queryProperties(prepareElements(comboBoxOption, widget, {u"Indicator"_s}));
        if (props->icon()) {
            auto icon = QIcon::fromTheme(props->icon()->name().value_or(u"arrow-down-symbolic"_s));
            drawIcon(indicatorRect, option, painter, icon, widget);
        }
    }
        return;
    case QStyle::CC_SpinBox: {
        const auto spinBoxOpt = qstyleoption_cast<const QStyleOptionSpinBox *>(option);
        if (!spinBoxOpt) {
            return;
        }
        auto elements = prepareElements(spinBoxOpt, widget);
        drawElementBackground(painter, spinBoxOpt, widget);
        // For spinbox we need to manually create the indicator buttons
        if (spinBoxOpt->buttonSymbols != QAbstractSpinBox::NoButtons) {
            bool arrows = (spinBoxOpt->buttonSymbols == QAbstractSpinBox::UpDownArrows);
            // Increase
            auto up = *spinBoxOpt;
            up.rect = subControlRect(CC_SpinBox, spinBoxOpt, SC_SpinBoxUp, widget);
            drawPrimitive(arrows ? PE_IndicatorSpinUp : PE_IndicatorSpinPlus, &up, painter, widget);
            // Decrease
            auto down = *spinBoxOpt;
            down.rect = subControlRect(CC_SpinBox, spinBoxOpt, SC_SpinBoxDown, widget);
            drawPrimitive(arrows ? PE_IndicatorSpinDown : PE_IndicatorSpinMinus, &down, painter, widget);
        }
    }
        return;
    case QStyle::CC_ScrollBar: {
        // Draw background, then let QCommonStyle handle rest;
        auto scrollbarProps = queryProperties(prepareElements(option, widget));
        // If scrollbar has no background color, QStyle gets confused and doesnt draw anything,
        // causing visual glitches. In those cases, use the ApplicationWindow background color.
        if (scrollbarProps->background() && scrollbarProps->background()->color().has_value()) {
            drawBackground(painter, option->rect, scrollbarProps);
        } else {
            drawBackground(painter, option->rect, queryProperties(prepareElements(option, widget, {u"ApplicationWindow"_s})));
        }
        auto rect = subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget);
        drawBackground(painter, rect, scrollbarProps);
        QCommonStyle::drawComplexControl(control, option, painter, widget);
    }
        return;
    case QStyle::CC_Slider: {
        const auto sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option);
        if (!sliderOption) {
            return;
        }
        // Background
        auto grooveRect = subControlRect(CC_Slider, sliderOption, SC_SliderGroove, widget);
        drawBackground(painter, grooveRect, queryProperties(prepareElements(sliderOption, widget)));

        const qreal p = sliderOption->sliderValue;
        const qreal min = sliderOption->minimum;
        const qreal max = sliderOption->maximum;
        const qreal percentage = (p - min) / (max - min);

        auto progress = grooveRect;

        const bool horizontal = sliderOption->state.testFlag(QStyle::State_Horizontal);
        const bool inverted(sliderOption->upsideDown);
        bool reverse = horizontal && option->direction == Qt::RightToLeft;
        if (inverted) {
            reverse = !reverse;
        }

        if (horizontal) {
            const qreal progressWidth = percentage * grooveRect.width();
            if (reverse) {
                progress.setLeft(grooveRect.right() - progressWidth);
            } else {
                progress.setWidth(progressWidth);
            }
        } else {
            const qreal progressHeight = percentage * grooveRect.height();
            if (reverse) {
                progress.setTop(grooveRect.bottom() - progressHeight);
            } else {
                progress.setHeight(progressHeight);
            }
        }

        drawBackground(painter, progress, queryProperties(prepareElements(sliderOption, widget, {u"Fill"_s})));

        // Tickmark drawing is copied from breeze
        // TODO: theyre still bit off, not exactly at the center of the handle
        if (sliderOption->subControls.testFlag(SC_SliderTickmarks)) {
            auto tickmarkElements = prepareElements(sliderOption, widget, {u"Tickmark"_s});
            auto props = queryProperties(tickmarkElements);
            if (!props->layout()) {
                return;
            }
            auto rect = option->rect;
            const int tickPosition(sliderOption->tickPosition);
            const int available(pixelMetric(PM_SliderSpaceAvailable, option, widget));
            int interval = sliderOption->tickInterval;

            if (interval < 1) {
                interval = sliderOption->pageStep;
            }
            if (interval >= 1) {
                const int fudge(pixelMetric(PM_SliderLength, option, widget) / 2);
                int current(sliderOption->minimum);

                int tickwidth = props->layout()->width().value_or(2);
                int tickheight = props->layout()->height().value_or(8);

                // store tick lines
                QList<QLine> tickLines;
                if (horizontal) {
                    if (tickPosition & QSlider::TicksAbove) {
                        tickLines.append(QLine(rect.left(), grooveRect.top() - tickwidth, rect.left(), grooveRect.top() - tickwidth - tickheight));
                    }
                    if (tickPosition & QSlider::TicksBelow) {
                        tickLines.append(QLine(rect.left(), grooveRect.bottom() + tickwidth, rect.left(), grooveRect.bottom() + tickwidth + tickheight));
                    }

                } else {
                    if (tickPosition & QSlider::TicksAbove) {
                        tickLines.append(QLine(grooveRect.left() - tickwidth, rect.top(), grooveRect.left() - tickwidth - tickheight, rect.top()));
                    }
                    if (tickPosition & QSlider::TicksBelow) {
                        tickLines.append(QLine(grooveRect.right() + tickwidth, rect.top(), grooveRect.right() + tickwidth + tickheight, rect.top()));
                    }
                }

                // colors
                const auto reverse(option->direction == Qt::RightToLeft);
                while (current <= sliderOption->maximum) {
                    // adjust color
                    tickmarkElements.last()->setHint(u"active"_s, current <= sliderOption->sliderPosition);
                    auto props = queryProperties(tickmarkElements);
                    const auto color = props->background()->color()->toQColor();
                    painter->setPen(color);

                    // calculate positions and draw lines
                    const int position(sliderPositionFromValue(sliderOption->minimum, sliderOption->maximum, current, available, inverted) + fudge);
                    for (const QLine &tickLine : std::as_const(tickLines)) {
                        if (horizontal) {
                            painter->drawLine(tickLine.translated(reverse ? (rect.width() - position) : position, 0));
                        } else {
                            painter->drawLine(tickLine.translated(0, position));
                        }
                    }
                    // go to next position
                    current += interval;
                }
            }
        }

        auto handle = subControlRect(CC_Slider, sliderOption, SC_SliderHandle, widget);
        drawBackground(painter, handle, queryProperties(prepareElements(sliderOption, widget, {u"Handle"_s})));
    }
        return;
    case QStyle::CC_TitleBar: {
        const auto titleBarOption = qstyleoption_cast<const QStyleOptionTitleBar *>(option);
        if (!titleBarOption) {
            return;
        }
        drawElementBackground(painter, titleBarOption, widget, {u"TitleBar"_s});
        auto map = layoutMap(prepareElements(titleBarOption, widget, {u"TitleBar"_s}), titleBarOption, buildSubElementList(titleBarOption, widget));
        if (!titleBarOption->text.isEmpty()
            && (titleBarOption->titleBarFlags.testFlag(Qt::WindowTitleHint) || titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint))) {
            drawText(map[u"Text"_s].rect.toRect(), titleBarOption, painter, titleBarOption->text, widget);
        }
        if (!titleBarOption->icon.isNull()) {
            drawIcon(map[u"Icon"_s].rect.toRect(), titleBarOption, painter, titleBarOption->icon, widget);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowContextHelpButtonHint)) {
            const auto icon = queryIcon(titleBarOption, widget, u"help-contextual-symbolic"_s, {u"TitleBar"_s, u"HelpButton"_s});
            drawIcon(map[u"HelpButton"_s].rect.toRect(), titleBarOption, painter, icon, widget);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowMinimizeButtonHint)) {
            const auto icon = queryIcon(titleBarOption, widget, u"window-minimize-symbolic"_s, {u"TitleBar"_s, u"MinimizeButton"_s});
            drawIcon(map[u"MinimizeButton"_s].rect.toRect(), titleBarOption, painter, icon, widget);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowMaximizeButtonHint)) {
            const auto icon = queryIcon(titleBarOption, widget, u"window-maximize-symbolic"_s, {u"TitleBar"_s, u"MaximizeButton"_s});
            drawIcon(map[u"MaximizeButton"_s].rect.toRect(), titleBarOption, painter, icon, widget);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowCloseButtonHint)) {
            const auto icon = queryIcon(titleBarOption, widget, u"window-close-symbolic"_s, {u"TitleBar"_s, u"CloseButton"_s});
            drawIcon(map[u"CloseButton"_s].rect.toRect(), titleBarOption, painter, icon, widget);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowSystemMenuHint)) {
            const auto icon = queryIcon(titleBarOption, widget, u"application-menu-symbolic"_s, {u"TitleBar"_s, u"SystemMenu"_s});
            drawIcon(map[u"SystemMenu"_s].rect.toRect(), titleBarOption, painter, icon, widget);
        }
        if (titleBarOption->titleBarFlags.testFlag(Qt::WindowShadeButtonHint)) {
            const auto icon = queryIcon(titleBarOption, widget, u"window-shade-symbolic"_s, {u"TitleBar"_s, u"ShadeButton"_s});
            drawIcon(map[u"ShadeButton"_s].rect.toRect(), titleBarOption, painter, icon, widget);
        }
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

    switch (element) {
    case QStyle::PE_FrameStatusBarItem:
        drawElementBackground(painter, option, widget, {u"Item"_s});
        return;
    case QStyle::PE_FrameMenu:
        drawElementBackground(painter, option, widget, {u"Menu"_s});
        return;
    case QStyle::PE_Widget:
        // Relates to PE_Frame
        drawElementBackground(painter, option, widget, {u"Panel"_s});
        return;
        // Standalone elements
    case QStyle::PE_PanelLineEdit:
        // For spinboxes and comboboxes, we do not want to draw this element
        // TODO: remove if this is not needed after making the complex controls
        if (widget->parentWidget()->inherits("QComboBox") || widget->parentWidget()->inherits("QAbstractSpinBox")) {
            return;
        }
        drawElementBackground(painter, option, widget);
        return;
    case QStyle::PE_PanelItemViewItem:
        drawElementBackground(painter, option, widget, {u"ItemViewItem"_s});
        return;
    case QStyle::PE_PanelItemViewRow:
        drawElementBackground(painter, option, widget, {u"ItemViewRow"_s});
        return;
    case QStyle::PE_PanelScrollAreaCorner:
        drawElementBackground(painter, option, widget, {u"ScrollAreaCorner"_s});
        return;
    case QStyle::PE_PanelTipLabel:
        drawElementBackground(painter, option, widget, {u"ToolTip"_s});
        return;
    case QStyle::PE_FrameFocusRect:
        drawElementBackground(painter, option, widget, {u"FocusFrame"_s});
        return;
        // Indicators
    case QStyle::PE_IndicatorCheckBox:
        drawElementBackground(painter, option, widget, {u"CheckBox"_s, u"Indicator"_s});
        return;
    case QStyle::PE_IndicatorRadioButton:
        drawElementBackground(painter, option, widget, {u"RadioButton"_s, u"Indicator"_s});
        return;
    case QStyle::PE_IndicatorArrowLeft: {
        const auto icon = queryIcon(option, widget, u"arrow-left-symbolic"_s, {u"IndicatorArrowLeft"_s});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowUp: {
        const auto icon = queryIcon(option, widget, u"arrow-up-symbolic"_s, {u"IndicatorArrowUp"_s});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowRight: {
        const auto icon = queryIcon(option, widget, u"arrow-right-symbolic"_s, {u"IndicatorArrowRight"_s});
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowDown: {
        const auto icon = queryIcon(option, widget, u"arrow-down-symbolic"_s, {u"IndicatorArrowDown"_s});
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
        element->setType(u"Indicator"_s);
        element->setStates(statesFromOption(option));
        auto hints = hintsFromOption(option);
        // Use the constrained look for now
        hints.append(up ? u"Increase"_s : u"Decrease"_s);
        element->setHints(hints);
        element->setColorSet(colorsetFromOption(option));
        element->setAttributes(attributesFromOption(option));
        spinboxElements.append(element);
        auto props = queryProperties(spinboxElements);
        drawBackground(painter, option->rect, props);
        if (props->icon()) {
            auto icon = QIcon::fromTheme(props->icon()->name().value_or(up ? u"arrow-up-symbolic"_s : u"arrow-down-symbolic"_s));
            drawIcon(option->rect, option, painter, icon, widget);
        }
    }
        return;
    case QStyle::PE_FrameLineEdit:
        drawElementBackground(painter, option, widget, {u"TextField"_s});
        return;
    case QStyle::PE_Frame:
    case QStyle::PE_FrameDefaultButton:
    case QStyle::PE_FrameDockWidget:
    case QStyle::PE_FrameGroupBox:
    case QStyle::PE_FrameTabWidget:
    case QStyle::PE_FrameWindow:
    case QStyle::PE_FrameButtonBevel:
    case QStyle::PE_FrameButtonTool:
    case QStyle::PE_FrameTabBarBase:
    case QStyle::PE_PanelButtonCommand:
    case QStyle::PE_PanelButtonBevel:
    case QStyle::PE_PanelButtonTool:
    case QStyle::PE_PanelMenuBar:
    case QStyle::PE_PanelToolBar:
    case QStyle::PE_PanelStatusBar:
    case QStyle::PE_PanelMenu:
        drawElementBackground(painter, option, widget);
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
        const auto icon = queryIcon(option, widget, defaultIconName, {u"IndicatorBranch"_s});
        auto size = querySize(option, widget, {u"TreeViewDelegate"_s, u"Indicator"_s});
        auto rect = centerRect(option->rect, size.width(), size.height());
        drawIcon(rect, option, painter, icon, widget);
    }
        return;
    case QStyle::PE_IndicatorButtonDropDown: {
        const auto icon = queryIcon(option, widget, u"arrow-down-symbolic"_s, {u"IndicatorButtonDropDown"_s});
        drawIcon(option->rect, option, painter, icon, widget);
    }
        return;
    case QStyle::PE_IndicatorMenuCheckMark:
    case QStyle::PE_IndicatorItemViewItemCheck:
        drawPrimitive(PE_IndicatorCheckBox, option, painter, widget);
        return;
    case QStyle::PE_IndicatorHeaderArrow: {
        if (qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            auto ev = HeaderElement::create(option, this, widget);
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
        drawElementBackground(painter, option, widget, {u"IndicatorProgressChunk"_s});
        return;
    case QStyle::PE_IndicatorToolBarHandle:
        drawElementBackground(painter, option, widget, {u"IndicatorToolBarHandle"_s});
        return;
    case QStyle::PE_IndicatorToolBarSeparator:
        drawElementBackground(painter, option, widget, {u"IndicatorToolBarSeparator"_s});
        return;
    case QStyle::PE_IndicatorColumnViewArrow:
        drawPrimitive(PE_IndicatorArrowRight, option, painter, widget);
        return;
    case QStyle::PE_IndicatorTabClose: {
        drawElementBackground(painter, option, widget, {u"TabButton"_s, u"CloseButton"_s});
        const auto icon = queryIcon(option, widget, u"tab-close-symbolic"_s, {u"IndicatorTabClose"_s});
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
    // TODO use subelement rects to build the thing if possible
    QSize size = QCommonStyle::sizeFromContents(contentsType, option, contentsSize, widget);
    QSize minimumSize(contentsSize.width(), contentsSize.height());
    auto elements = prepareElements(option, widget);
    if (elements.isEmpty()) {
        return size;
    }
    auto properties = queryProperties(elements);
    if (!properties) {
        return size;
    }
    QMargins padding;
    if (properties->layout()) {
        auto frameWidth = pixelMetric(PM_DefaultFrameWidth, option, widget);
        auto width = properties->layout()->width().value_or(1) + frameWidth;
        auto height = properties->layout()->height().value_or(1) + frameWidth;
        minimumSize = QSize(width, height);
        if (properties->layout()->padding()) {
            padding = properties->layout()->padding()->toMargins().toMargins();
        }
        if (properties->layout()->inset()) {
            padding += properties->layout()->inset()->toMargins().toMargins();
        }
    }
    switch (contentsType) {
    case QStyle::CT_PushButton: {
        auto ev = ButtonElement::create(option, this, widget);
        return ev->contentsSize(size);
    } break;
    case QStyle::CT_ToolButton: {
        auto ev = ToolButtonElement::create(option, this, widget);
        return ev->contentsSize(size);
    } break;
    case QStyle::CT_MenuItem: {
        auto ev = MenuItemElement::create(option, this, widget);
        return ev->contentsSize(size);
    } break;
    // Use defaults from qcommonstyle
    case QStyle::CT_ComboBox: {
        auto elements = prepareElements(option, widget);
        QStringList childelements = {u"Icon"_s, u"Text"_s, u"Indicator"_s};
        auto map = layoutMap(elements, option, childelements);
        QRect unifiedRect;
        for (const auto &m : map) {
            unifiedRect = unifiedRect.united(m.rect.toRect());
        }
        // Follow the contents width
        if (unifiedRect.width() < contentsSize.width()) {
            unifiedRect.setWidth(contentsSize.width());
        }
        size = unifiedRect.size().grownBy(padding);
        size.rwidth() += pixelMetric(PM_LayoutLeftMargin, option, widget);
    } break;
    case QStyle::CT_TabBarTab: {
        auto ev = TabElement::create(option, this, widget);
        return ev->contentsSize(size);
    } break;
    case QStyle::CT_Slider: {
        auto sliderOpt = qstyleoption_cast<const QStyleOptionSlider *>(option);
        if (sliderOpt) {
            QRegion r;
            auto grooveRect = subControlRect(CC_Slider, sliderOpt, SC_SliderGroove, widget);
            auto tickRect = subControlRect(CC_Slider, sliderOpt, SC_SliderTickmarks, widget);
            auto handleRect = subControlRect(CC_Slider, sliderOpt, SC_SliderHandle, widget);
            r.setRects({grooveRect, tickRect, handleRect});
            size = r.boundingRect().size().grownBy(padding);
            return size;
        }
    } break;
    case QStyle::CT_ItemViewItem: {
        auto ev = ItemViewElement::create(option, this, widget);
        size = ev->contentsSize(size);
    } break;
    case QStyle::CT_TabWidget:
    case QStyle::CT_Splitter:
    case QStyle::CT_MenuBar:
    case QStyle::CT_LineEdit:
    case QStyle::CT_GroupBox:
    case QStyle::CT_CheckBox:
    case QStyle::CT_RadioButton:
    case QStyle::CT_ProgressBar:
    case QStyle::CT_MenuBarItem:
    case QStyle::CT_Menu:
    case QStyle::CT_ScrollBar:
    case QStyle::CT_SpinBox:
    case QStyle::CT_SizeGrip:
    case QStyle::CT_DialogButtons:
    case QStyle::CT_HeaderSection:
    case QStyle::CT_MdiControls:
    case QStyle::CT_CustomBase:
        break;
    }

    if (size.width() < minimumSize.width()) {
        size.setWidth(minimumSize.width());
    }
    if (size.height() < minimumSize.height()) {
        size.setHeight(minimumSize.height());
    }
    return size;
}

QRect UnionStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    QRect rect;
    switch (element) {
    case QStyle::SE_TreeViewDisclosureItem: {
        auto elements = prepareElements(option, widget);
        auto map = layoutMap(elements, option, {u"Indicator"_s});
        rect = map[u"Indicator"_s].rect.toRect();
    } break;
    case QStyle::SE_ItemViewItemText:
    case QStyle::SE_ItemViewItemDecoration:
    case QStyle::SE_ItemViewItemCheckIndicator: {
        auto ev = ItemViewElement::create(option, this, widget);
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_RadioButtonContents:
    case QStyle::SE_RadioButtonIndicator: {
        auto ev = CheckElement::create(CheckElement::Type::RadioButton, option, this, widget);
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_CheckBoxIndicator:
    case QStyle::SE_CheckBoxContents: {
        auto ev = CheckElement::create(CheckElement::Type::CheckBox, option, this, widget);
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_PushButtonFocusRect:
    case QStyle::SE_PushButtonContents:
    case QStyle::SE_PushButtonBevel: {
        auto ev = ButtonElement::create(option, this, widget);
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_ShapedFrameContents:
    case QStyle::SE_LineEditContents:
    case QStyle::SE_FrameContents: {
        auto frameElements = prepareElements(option, widget);
        auto props = queryProperties(frameElements);
        int frameWidth = pixelMetric(PM_DefaultFrameWidth, option, widget);
        rect = backgroundRectangle(option, props).toRect().adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);
    } break;
    case QStyle::SE_HeaderArrow:
    case QStyle::SE_HeaderLabel: {
        auto ev = HeaderElement::create(option, this, widget);
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_TabBarTabText: {
        auto ev = TabElement::create(option, this, widget);
        // TODO: handle vertical tabs
        if (ev->isVertical()) {
            return QCommonStyle::subElementRect(element, option, widget);
        }
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_ProgressBarLabel:
    case QStyle::SE_ProgressBarContents:
    case QStyle::SE_ProgressBarGroove: {
        auto ev = ProgressBarElement::create(option, this, widget);
        return ev->subElementRect(element);
    } break;
    case QStyle::SE_DockWidgetTitleBarText:
    case QStyle::SE_DockWidgetCloseButton:
    case QStyle::SE_DockWidgetFloatButton:
    case QStyle::SE_DockWidgetIcon: {
        if (const auto dockOption = qstyleoption_cast<const QStyleOptionDockWidget *>(option)) {
            QStringList childelements = buildSubElementList(dockOption, widget);
            if (childelements.isEmpty()) {
                return QRect();
            }
            auto elements = prepareElements(option, widget, {u"DockWidget"_s});
            auto map = layoutMap(elements, option, childelements);

            if (element == SE_DockWidgetTitleBarText) {
                rect = map[u"Text"_s].rect.toRect();
            }
            if (element == SE_DockWidgetFloatButton) {
                rect = map[u"FloatButton"_s].rect.toRect();
            }
            if (element == SE_DockWidgetCloseButton) {
                rect = map[u"CloseButton"_s].rect.toRect();
            }
            // The styleoption has no icon, yet there is whole thing for an icon? Wtf.
            if (element == SE_DockWidgetIcon) {
                rect = map[u"Icon"_s].rect.toRect();
            }
        }
    } break;
    // Follow defaults
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
    if (complexControl == CC_ToolButton) {
        auto ev = ToolButtonElement::create(option, this, widget);
        return ev->subControlRect(subControl);
    }

    if (complexControl == CC_ComboBox) {
        // cast option and check
        const auto comboBoxOption(qstyleoption_cast<const QStyleOptionComboBox *>(option));
        if (!comboBoxOption) {
            return QCommonStyle::subControlRect(CC_ComboBox, option, subControl, widget);
        }
        switch (subControl) {
        case SC_ComboBoxFrame:
        case SC_ComboBoxListBoxPopup:
            return option->rect;

        case SC_ComboBoxArrow: {
            auto elements = prepareElements(option, widget);
            auto map = layoutMap(elements, option, {u"Indicator"_s});
            auto rect = map[u"Indicator"_s].rect;
            const int spacing(pixelMetric(PM_LayoutVerticalSpacing, option, widget));
            rect = rect.adjusted(-spacing, 0, spacing, 0);
            return visualRect(option->direction, option->rect, rect.toRect());
        }

        case SC_ComboBoxEditField: {
            QRect labelRect;
            auto rect = option->rect;
            auto indicatorRect = subControlRect(CC_ComboBox, option, SC_ComboBoxArrow, widget);
            labelRect = QRect(rect.left(), rect.top(), rect.width() - indicatorRect.width(), rect.height());
            return visualRect(option->direction, option->rect, labelRect);
        }

        default:
            break;
        }
    }
    if (complexControl == CC_GroupBox) {
        switch (subControl) {
        case SC_GroupBoxLabel: {
            auto elements = prepareElements(option, widget);
            auto map = layoutMap(elements, option, {u"GroupBox"_s, u"Text"_s});
            auto rect = map[u"Text"_s].rect.toRect();
            return visualRect(option->direction, option->rect, rect);
        }
        case SC_GroupBoxContents: {
            auto elements = prepareElements(option, widget);
            auto map = layoutMap(elements, option, {u"GroupBox"_s, u"Text"_s});
            auto rect = map[u"GroupBox"_s].rect.toRect();
            return visualRect(option->direction, option->rect, rect);
        }
        case SC_GroupBoxCheckBox: {
            auto elements = prepareElements(option, widget);
            auto map = layoutMap(elements, option, {u"GroupBox"_s, u"Icon"_s});
            auto rect = map[u"Icon"_s].rect.toRect();
            return visualRect(option->direction, option->rect, rect);
        }
        case SC_GroupBoxFrame: {
            return option->rect;
        }
        default:
            break;
        }
    }

    if (complexControl == CC_ScrollBar) {
        // Copied from Breeze
        auto rect = option->rect;
        if (widget) {
            rect = widget->visibleRegion().boundingRect();
        }
        if (subControl == SC_ScrollBarSlider) {
            auto sliderOption = qstyleoption_cast<const QStyleOptionSlider *>(option);
            if (!sliderOption) {
                return QCommonStyle::subControlRect(complexControl, option, subControl, widget);
            }
            const bool horizontal = (sliderOption->state.testFlag(State_Horizontal));
            auto groove = visualRect(option->direction, rect, subControlRect(CC_ScrollBar, option, SC_ScrollBarGroove, widget));

            int space(horizontal ? groove.width() : groove.height());
            int thickness = 0;
            QMargins padding;
            auto props = queryProperties(prepareElements(option, widget, {u"Slider"_s}));
            if (props->layout()) {
                auto scrollProps = queryProperties(prepareElements(option, widget));
                if (scrollProps->layout() && scrollProps->layout()->padding()) {
                    padding = scrollProps->layout()->padding()->toMargins().toMargins();
                }
                if (sliderOption->orientation == Qt::Horizontal) {
                    thickness = props->layout()->height().value_or(0);
                } else {
                    thickness = props->layout()->width().value_or(0);
                }
            }

            // Return early with just padding changes
            if (sliderOption->minimum == sliderOption->maximum) {
                if (horizontal) {
                    auto rect = QRect(groove.left(), groove.top(), groove.width(), groove.height());
                    return visualRect(option->direction, rect, rect.adjusted(padding.left(), thickness, -padding.right(), -thickness));
                } else {
                    auto rect = QRect(groove.left(), groove.top(), groove.width(), groove.height());
                    return visualRect(option->direction, rect, rect.adjusted(thickness, padding.top(), -thickness, -padding.bottom()));
                }
            }

            int sliderSize = space * qreal(sliderOption->pageStep) / (sliderOption->maximum - sliderOption->minimum + sliderOption->pageStep);
            sliderSize = qMax(sliderSize, qMax(thickness, pixelMetric(PM_ScrollBarSliderMin, option, widget)));
            sliderSize = qMin(sliderSize, space);
            space -= sliderSize;
            if (space <= 0) {
                return groove;
            }
            int pos = qRound(qreal(sliderOption->sliderPosition - sliderOption->minimum) / (sliderOption->maximum - sliderOption->minimum) * space);
            if (sliderOption->upsideDown) {
                pos = space - pos;
            }
            if (horizontal) {
                auto rect = QRect(groove.left() + pos, groove.top(), sliderSize, groove.height());
                return visualRect(option->direction, rect, rect.adjusted(padding.left(), thickness, -padding.right(), -thickness));
            } else {
                auto rect = QRect(groove.left(), groove.top() + pos, groove.width(), sliderSize);
                return visualRect(option->direction, rect, rect.adjusted(thickness, padding.top(), -thickness, -padding.bottom()));
            }
        } else if (subControl == SC_ScrollBarGroove) {
            return visualRect(option->direction, option->rect, rect);
        } else {
            return QRect();
        }
    }

    if (complexControl == CC_Slider) {
        // cast option and check
        const auto sliderOption(qstyleoption_cast<const QStyleOptionSlider *>(option));
        if (!sliderOption) {
            return QCommonStyle::subControlRect(complexControl, option, subControl, widget);
        }
        // Copied from Breeze
        const bool horizontal(sliderOption->orientation == Qt::Horizontal);
        auto rect(sliderOption->rect);
        auto frameWidth = pixelMetric(PM_DefaultFrameWidth, option, widget);
        if (widget) {
            rect = widget->visibleRegion().boundingRect();
        }

        switch (subControl) {
        case SC_SliderHandle: {
            auto props = queryProperties(prepareElements(option, widget, {u"Handle"_s}));
            int handleHeight = 1;
            int handleWidth = 1;
            if (props->layout()) {
                handleHeight = props->layout()->height().value_or(6);
                handleWidth = props->layout()->width().value_or(6);
            }

            QRect handleRect(centerRect(rect, handleWidth, handleHeight));
            const int sliderPos = sliderPositionFromValue(sliderOption->minimum,
                                                          sliderOption->maximum,
                                                          sliderOption->sliderPosition,
                                                          (horizontal ? (rect.width() - handleWidth) : (rect.height() - handleHeight)),
                                                          sliderOption->upsideDown);
            if (horizontal) {
                handleRect.moveLeft(rect.x() + sliderPos);
            } else {
                handleRect.moveTop(rect.y() + sliderPos);
            }
            handleRect = visualRect(option->direction, rect, handleRect);
            return handleRect;
        }

        case SC_SliderGroove: {
            auto props = queryProperties(prepareElements(option, widget));
            int grooveHeight = 1;
            int grooveWidth = 1;
            if (props->layout()) {
                grooveHeight = props->layout()->height().value_or(6);
                grooveWidth = props->layout()->width().value_or(6);
            }

            auto grooveRect = rect.adjusted(frameWidth, frameWidth, -frameWidth, -frameWidth);

            // centering
            if (horizontal) {
                grooveRect = centerRect(rect, grooveRect.width(), grooveHeight);
            } else {
                grooveRect = centerRect(rect, grooveWidth, grooveRect.height());
            }
            return visualRect(option->direction, rect, grooveRect);
        }

        default:
            return QCommonStyle::subControlRect(complexControl, option, subControl, widget);
        }
    }

    /* Use what qcommonstyle provides for now
        if (cc == CC_SpinBox){
            switch (sc) {
                case            SC_SpinBoxUp:
                case            SC_SpinBoxDown :
                case            SC_SpinBoxFrame:
                case            SC_SpinBoxEditField:
                default:
                    break;
            }
        }
    */

    if (complexControl == CC_TitleBar) {
        if (const auto titleBar = qstyleoption_cast<const QStyleOptionTitleBar *>(option)) {
            auto elements = prepareElements(option, widget, {u"TitleBar"_s});
            auto subElements = buildSubElementList(titleBar, widget);
            auto map = layoutMap(elements, option, subElements);
            switch (subControl) {
            case SC_TitleBarSysMenu:
                return map[u"SystemMenu"_s].rect.toRect();
            case SC_TitleBarMinButton:
                return map[u"MinimizeButton"_s].rect.toRect();
            case SC_TitleBarMaxButton:
                return map[u"MaximizeButton"_s].rect.toRect();
            case SC_TitleBarCloseButton:
                return map[u"CloseButton"_s].rect.toRect();
            case SC_TitleBarNormalButton:
                return map[u"NormalButton"_s].rect.toRect();
            case SC_TitleBarShadeButton:
            case SC_TitleBarUnshadeButton:
                return map[u"ShadeButton"_s].rect.toRect();
            case SC_TitleBarContextHelpButton:
                return map[u"HelpButton"_s].rect.toRect();
            case SC_TitleBarLabel:
                return map[u"Text"_s].rect.toRect();
                break;
            default:
                break;
            }
        }
    }
    // Leave Dial and MDIControls to QCommonStyle for now
    return QCommonStyle::subControlRect(complexControl, option, subControl, widget);
}

int UnionStyle::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    int defaultMetric = QCommonStyle::pixelMetric(metric, option, widget);
    auto elements = prepareElements(option, widget);
    if (elements.isEmpty()) {
        return defaultMetric;
    }
    auto properties = queryProperties(elements);
    if (!properties) {
        return defaultMetric;
    }

    switch (metric) {
    // Don't shift button text when sunken
    case QStyle::PM_TabBarTabShiftHorizontal:
    case QStyle::PM_TabBarTabShiftVertical:
    case QStyle::PM_ButtonShiftHorizontal:
    case QStyle::PM_ButtonShiftVertical:
    case QStyle::PM_TabBar_ScrollButtonOverlap:
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
        auto elements = prepareElements(option, widget, {u"Indicator"_s});
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
        auto elements = prepareElements(option, widget, {u"Handle"_s});
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
            childelements.append(u"Handle"_s);
        }
        if (metric == PM_ToolBarExtensionExtent) {
            childelements.append(u"Extension"_s);
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
    // Currently we only have one spacing value
    case QStyle::PM_CheckBoxLabelSpacing:
    case QStyle::PM_ScrollView_ScrollBarSpacing:
    case QStyle::PM_RadioButtonLabelSpacing:
    case QStyle::PM_TabBarTabHSpace:
    case QStyle::PM_TabBarTabVSpace:
    case QStyle::PM_MenuBarItemSpacing:
    case QStyle::PM_ToolBarItemSpacing:
    case QStyle::PM_LayoutHorizontalSpacing:
    case QStyle::PM_LayoutVerticalSpacing: {
        if (properties->layout()) {
            return properties->layout()->spacing().value_or(defaultMetric);
        }
    } break;
    case QStyle::PM_TabBarScrollButtonWidth:
        return querySize(option, widget, {u"TabScrollButton"_s}).width();
    case QStyle::PM_MenuPanelWidth:
    case QStyle::PM_SplitterWidth:
        return querySize(option, widget).width();
    case QStyle::PM_TitleBarHeight:
        return querySize(option, widget, {u"TitleBar"_s}).height();
    case QStyle::PM_TitleBarButtonSize:
    case QStyle::PM_TitleBarButtonIconSize:
        return querySize(option, widget, {u"TitleBar"_s, u"NormalButton"_s}).width();
    case QStyle::PM_SpinBoxSliderHeight:
    case QStyle::PM_MenuScrollerHeight:
    case QStyle::PM_TabBarBaseHeight:
        return querySize(option, widget).height();
    case QStyle::PM_TreeViewIndentation:
        return querySize(option, widget, {u"TreeViewDelegate"_s, u"Indentation"_s}).width();
    case QStyle::PM_SmallIconSize:
        return querySize(option, widget, {u"SmallIconSize"_s}).width();
    case QStyle::PM_TabBarIconSize:
        return querySize(option, widget, {u"TabBarIconSize"_s}).width();
    case QStyle::PM_LineEditIconSize:
        return querySize(option, widget, {u"LineEditIconSize"_s}).width();
    case QStyle::PM_ListViewIconSize:
        return querySize(option, widget, {u"ListViewIconSize"_s}).width();
    case QStyle::PM_ButtonIconSize:
        return querySize(option, widget, {u"ButtonIconSize"_s}).width();
    case QStyle::PM_ToolBarIconSize:
        return querySize(option, widget, {u"ToolBarIconSize"_s}).width();
    case QStyle::PM_HeaderMarkSize:
        return querySize(option, widget, {u"HeaderMarkSize"_s}).width();
    case QStyle::PM_IconViewIconSize:
        return querySize(option, widget, {u"IconViewIconSize"_s}).width();
    case QStyle::PM_LargeIconSize:
        return querySize(option, widget, {u"LargeIconSize"_s}).width();
    case QStyle::PM_MessageBoxIconSize:
        return querySize(option, widget, {u"MessageBoxIconSize"_s}).width();
    case QStyle::PM_SizeGripSize:
        return querySize(option, widget, {u"SizeGripSize"_s}).width();
    case QStyle::PM_TextCursorWidth:
        return querySize(option, widget, {u"TextCursorWidth"_s}).width();
    case QStyle::PM_HeaderDefaultSectionSizeHorizontal:
        return querySize(option, widget, {u"HeaderDefaultSectionSize"_s}).width();
    case QStyle::PM_HeaderDefaultSectionSizeVertical:
        return querySize(option, widget, {u"HeaderDefaultSectionSize"_s}).height();
    case QStyle::PM_ProgressBarChunkWidth:
        return querySize(option, widget, {u"ProgressBarChunkWidth"_s}).width();
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
    default:
        return QCommonStyle::styleHint(hint, option, widget, returnData);
    }
}

void UnionStyle::polish(QApplication *application)
{
    QCommonStyle::polish(application);

    // Set global window color
    auto element = Union::Element::create();
    element->setType(u"ApplicationWindow"_s);

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

    widget->setProperty(property_union_member_list, setupMemberList(widget));

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

void UnionStyle::layoutAndDrawIconTextIndicator(const QStyleOption *opt,
                                                QPainter *painter,
                                                const QWidget *widget,
                                                const QIcon &icon,
                                                const QString &text,
                                                const QIcon &indicator,
                                                const QStringList &children) const
{
    QList<Union::Element::Ptr> elements = prepareElements(opt, widget, children);

    QString itemText = text;
    QString shortcutText;
    bool hasIcon = !icon.isNull();
    bool hasText = !itemText.isEmpty();
    bool hasIndicator = !indicator.isNull();

    QStringList subElements;
    if (hasIcon) {
        subElements.append(u"Icon"_s);
    }
    if (hasText) {
        subElements.append(u"Text"_s);
        const int tabPosition(itemText.indexOf(QLatin1Char('\t')));
        if (tabPosition >= 0) {
            subElements.append(u"ShortcutText"_s);
            shortcutText = itemText.mid(tabPosition + 1);
            itemText = itemText.left(tabPosition);
        }
    }
    if (hasIndicator) {
        subElements.append(u"Indicator"_s);
    }
    // Nothing to draw, just return
    if (subElements.isEmpty()) {
        return;
    }
    auto map = layoutMap(elements, opt, subElements);

    if (hasText) {
        QRect textRect = map[u"Text"_s].rect.toRect();
        drawText(textRect, opt, painter, itemText, widget);
    }

    if (hasIcon) {
        QRect iconRect = map[u"Icon"_s].rect.toRect();
        drawIcon(iconRect, opt, painter, icon, widget);
    }

    if (hasIndicator) {
        QRect indicatorRect = map[u"Indicator"_s].rect.toRect();
        drawIcon(indicatorRect, opt, painter, indicator, widget);
    }

    // ShortcutText is just like a regular text element but handled with different name
    // and has different coloration, so override the default colors
    if (!shortcutText.isEmpty()) {
        QRect textRect = map[u"ShortcutText"_s].rect.toRect();
        auto shortcutHierarchy = children;
        shortcutHierarchy.append(u"ShortcutText"_s);
        const auto properties = queryProperties(prepareElements(opt, widget, shortcutHierarchy));
        QColor overrideColor;
        if (properties->text() && properties->text()->color().has_value()) {
            overrideColor = properties->text()->color()->toQColor();
        }
        drawText(textRect, opt, painter, shortcutText, widget, overrideColor);
    }
}
