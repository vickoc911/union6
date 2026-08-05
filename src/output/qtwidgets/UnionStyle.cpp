// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#include "UnionStyle.h"
#include "StyleDrawing.h"
#include "StyleUtils.h"

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
    case QStyle::CE_PushButtonBevel: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            auto opt = *buttonOption;
            opt.rect = subElementRect(SE_PushButtonBevel, buttonOption, widget);
            drawElementBackground(painter, &opt, widget);
        }
    }
        return;
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
    case QStyle::CE_RadioButtonLabel:
    case QStyle::CE_CheckBoxLabel: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            layoutAndDrawIconTextIndicator(buttonOption, painter, widget, buttonOption->icon, buttonOption->text);
        }
    }
        return;
    case QStyle::CE_PushButtonLabel: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QIcon indicator;
            if (buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) {
                auto indicatorProps = queryProperties(prepareElements(buttonOption, widget, {u"Indicator"_s}));
                if (indicatorProps->icon()) {
                    indicator = QIcon::fromTheme(indicatorProps->icon()->name().value_or(u"arrow-down-symbolic"_s));
                }
            }
            layoutAndDrawIconTextIndicator(buttonOption, painter, widget, buttonOption->icon, buttonOption->text, indicator);
        }
    }
        return;
    case QStyle::CE_PushButton: {
        drawControl(CE_PushButtonBevel, option, painter, widget);
        drawControl(CE_PushButtonLabel, option, painter, widget);
    }
        return;
    case QStyle::CE_ToolButtonLabel: {
        const auto buttonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option);
        if (!buttonOption) {
            return;
        }
        auto text = buttonOption->text;
        auto icon = buttonOption->icon;
        // Skip text drawing for icon only buttons completely
        if (buttonOption->toolButtonStyle == Qt::ToolButtonIconOnly) {
            text = QString();
        } else if (buttonOption->toolButtonStyle == Qt::ToolButtonTextOnly) {
            icon = QIcon();
        }
        if (icon.isNull() && buttonOption->toolButtonStyle != Qt::ToolButtonTextOnly && buttonOption->features.testFlag(QStyleOptionToolButton::Arrow)) {
            QList<Union::Element::Ptr> elements = prepareElements(buttonOption, widget);
            bool hasText = !text.isEmpty();
            QStringList subElements = {u"Icon"_s};
            if (hasText) {
                subElements.append(u"Text"_s);
            }
            auto map = layoutMap(elements, buttonOption, subElements);
            if (hasText) {
                QRect textRect = map[u"Text"_s].rect.toRect();
                drawText(textRect, buttonOption, painter, text, widget);
            }
            QRect iconRect = map[u"Icon"_s].rect.toRect();
            auto subopt = *buttonOption;
            subopt.rect = iconRect;

            switch (buttonOption->arrowType) {
            case Qt::LeftArrow:
                drawPrimitive(PE_IndicatorArrowLeft, &subopt, painter, widget);
                break;
            case Qt::RightArrow:
                drawPrimitive(PE_IndicatorArrowRight, &subopt, painter, widget);
                break;
            case Qt::UpArrow:
                drawPrimitive(PE_IndicatorArrowUp, &subopt, painter, widget);
                break;
            case Qt::DownArrow:
                drawPrimitive(PE_IndicatorArrowDown, &subopt, painter, widget);
                break;
            default:
                break;
            }
        } else {
            layoutAndDrawIconTextIndicator(buttonOption, painter, widget, icon, text);
        }
    }
        return;
    case QStyle::CE_CheckBox: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *buttonOption;
            // Draw background
            auto bgElements = prepareElements(&subopt, widget);
            auto bgProps = queryProperties(bgElements);
            auto rect = backgroundRectangle(option, bgProps).toRect();
            drawBackground(painter, rect, bgProps);
            // Draw indicator
            subopt.rect = subElementRect(SE_CheckBoxIndicator, buttonOption, widget);
            drawPrimitive(PE_IndicatorCheckBox, &subopt, painter, widget);
            // Draw text
            subopt.rect = subElementRect(SE_CheckBoxContents, buttonOption, widget);
            drawControl(CE_CheckBoxLabel, &subopt, painter, widget);
        }
    }
        return;
    case QStyle::CE_RadioButton: {
        if (const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *buttonOption;
            // Draw background
            auto bgElements = prepareElements(&subopt, widget);
            auto bgProps = queryProperties(bgElements);
            auto rect = backgroundRectangle(option, bgProps).toRect();
            drawBackground(painter, rect, bgProps);
            // Draw indicator
            subopt.rect = subElementRect(SE_RadioButtonIndicator, buttonOption, widget);
            drawPrimitive(PE_IndicatorRadioButton, &subopt, painter, widget);
            // Draw text
            subopt.rect = subElementRect(SE_RadioButtonContents, buttonOption, widget);
            drawControl(CE_RadioButtonLabel, &subopt, painter, widget);
        }
    }
        return;
    case QStyle::CE_MenuItem: {
        if (const auto menuItemOption = qstyleoption_cast<const QStyleOptionMenuItem *>(option)) {
            if (menuItemOption->menuItemType == QStyleOptionMenuItem::Separator) {
                drawElementBackground(painter, menuItemOption, widget, {u"MenuSeparator"_s});
                // TODO we need to allow text drawing on separators?
                // if (!menuItemOption->text.isEmpty()){
                //    auto textrect = menuItemOption->rect;
                //    textrect.setHeight(menuItemOption->fontMetrics.height());
                //    drawText(textrect, menuItemOption, painter, menuItemOption->text, widget);
                //}
            } else {
                const auto elements = prepareElements(menuItemOption, widget, {u"MenuItem"_s});
                auto props = queryProperties(elements);
                drawElementBackground(painter, menuItemOption, widget, {u"MenuItem"_s});
                layoutAndDrawIconTextIndicator(menuItemOption, painter, widget, menuItemOption->icon, menuItemOption->text, QIcon(), {u"MenuItem"_s});
                if (menuItemOption->menuItemType == QStyleOptionMenuItem::SubMenu) {
                    if (props->layout() && props->icon()) {
                        auto map = layoutMap(elements, menuItemOption, {u"Arrow"_s});
                        auto rect = map[u"Arrow"_s].rect;
                        QIcon icon = QIcon::fromTheme(props->icon()->name().value_or(u"arrow-right-symbolic"_s));
                        drawIcon(rect.toRect(), menuItemOption, painter, icon, widget);
                    }
                }
            }
        }
    }
        return;
    case QStyle::CE_ToolBoxTabShape:
    case QStyle::CE_TabBarTabShape: {
        if (const auto tabOption = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            auto bgElements = prepareElements(tabOption, widget, {u"TabButton"_s});
            auto bgProps = queryProperties(bgElements);
            auto rect = backgroundRectangle(option, bgProps).toRect();
            drawBackground(painter, rect, bgProps);
        }
    }
        return;
    case QStyle::CE_ToolBoxTabLabel:
    case QStyle::CE_TabBarTabLabel: {
        if (const auto tabOption = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            // TODO: Rely on qcommonstyle for vertical tabs for now
            bool verticalTabs = tabOption->shape == QTabBar::RoundedEast || tabOption->shape == QTabBar::RoundedWest
                || tabOption->shape == QTabBar::TriangularEast || tabOption->shape == QTabBar::TriangularWest;
            if (verticalTabs) {
                QCommonStyle::drawControl(controlElement, tabOption, painter, widget);
            } else {
                auto subopt = *tabOption;
                subopt.rect = subElementRect(SE_TabBarTabText, tabOption, widget);
                layoutAndDrawIconTextIndicator(&subopt, painter, widget, tabOption->icon, tabOption->text);
            }
        }
    }
        return;
    case QStyle::CE_ToolBoxTab:
    case QStyle::CE_TabBarTab: {
        if (const auto tabOption = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            drawControl(CE_TabBarTabShape, tabOption, painter, widget);
            drawControl(CE_TabBarTabLabel, tabOption, painter, widget);
        }
    }
        return;
    case QStyle::CE_ItemViewItem: {
        const auto viewItemOption = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        if (!viewItemOption) {
            return;
        }
        // Dont allow drawing outside of the area
        painter->save();
        painter->setClipRect(option->rect);
        QStyleOptionViewItem subopt = *viewItemOption;
        // Draw background
        auto elements = prepareElements(&subopt, widget, {u"ItemViewItem"_s});
        auto props = queryProperties(elements);
        subopt.rect = backgroundRectangle(option, props).toRect();
        drawBackground(painter, subopt.rect, props);
        // Draw text
        if (subopt.features.testFlag(QStyleOptionViewItem::HasDisplay)) {
            subopt.rect = subElementRect(SE_ItemViewItemText, &subopt, widget);
            layoutAndDrawIconTextIndicator(&subopt, painter, widget, QIcon(), viewItemOption->text, QIcon(), {u"ItemViewItem"_s});
        }
        // Draw indicator
        if (subopt.features.testFlag(QStyleOptionViewItem::HasCheckIndicator)) {
            QStyleOptionButton checkbox;
            switch (subopt.checkState) {
            case Qt::Unchecked:
                checkbox.state.setFlag(State_Off);
                break;
            case Qt::PartiallyChecked:
                checkbox.state.setFlag(State_NoChange);
                break;
            case Qt::Checked:
                checkbox.state.setFlag(State_On);
                break;
            }
            checkbox.state.setFlag(State_Enabled, viewItemOption->state.testFlag(State_Enabled));
            checkbox.rect = subElementRect(SE_ItemViewItemCheckIndicator, viewItemOption, widget);
            drawPrimitive(PE_IndicatorCheckBox, &checkbox, painter, widget);
        } else if (subopt.features.testFlag(QStyleOptionViewItem::HasDecoration)) {
            subopt.rect = subElementRect(SE_ItemViewItemDecoration, viewItemOption, widget);
            layoutAndDrawIconTextIndicator(&subopt, painter, widget, viewItemOption->icon, QString(), QIcon(), {u"ItemViewItem"_s});
        }
        painter->restore();
    }
        return;
    case QStyle::CE_ProgressBarGroove: {
        if (const auto progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            auto subopt = *progressBarOption;
            auto groove = subElementRect(SE_ProgressBarGroove, progressBarOption, widget);
            subopt.rect = groove;
            drawElementBackground(painter, &subopt, widget);
        }
    }
        return;
    case QStyle::CE_ProgressBarContents: {
        const auto progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar *>(option);
        if (!progressBarOption) {
            return;
        }
        auto elements = prepareElements(progressBarOption, widget, {u"ProgressBar"_s, u"Track"_s});
        auto props = queryProperties(elements);
        auto progress = subElementRect(SE_ProgressBarContents, progressBarOption, widget);
        drawBackground(painter, progress, props);
    }
        return;
    case QStyle::CE_ProgressBarLabel: {
        if (const auto progressBarOption = qstyleoption_cast<const QStyleOptionProgressBar *>(option)) {
            if (progressBarOption->textVisible) {
                auto subopt = *progressBarOption;
                auto rect = subElementRect(SE_ProgressBarLabel, progressBarOption, widget);
                subopt.rect = rect;
                layoutAndDrawIconTextIndicator(&subopt, painter, widget, QIcon(), progressBarOption->text);
            }
        }
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
    case QStyle::CE_HeaderLabel:
        if (const auto header = qstyleoption_cast<const QStyleOptionHeader *>(option)) {
            auto props = queryProperties(prepareElements(header, widget, {u"HeaderViewDelegate"_s}));
            QIcon sortIndicator;
            switch (header->sortIndicator) {
            case QStyleOptionHeader::None:
                break;
            case QStyleOptionHeader::SortUp:
                if (props->icon()) {
                    sortIndicator = QIcon::fromTheme(props->icon()->name().value_or(u"arrow-up-symbolic"_s));
                }
                break;
            case QStyleOptionHeader::SortDown:
                if (props->icon()) {
                    sortIndicator = QIcon::fromTheme(props->icon()->name().value_or(u"arrow-down-symbolic"_s));
                }
                break;
            }
            layoutAndDrawIconTextIndicator(header, painter, widget, sortIndicator, header->text, QIcon(), {u"HeaderViewDelegate"_s});
        }
        return;
    case QStyle::CE_HeaderSection: {
        drawElementBackground(painter, option, widget, {u"HeaderViewDelegate"_s});
    }
        return;
    case QStyle::CE_Header:
        drawControl(CE_HeaderSection, option, painter, widget);
        drawControl(CE_HeaderLabel, option, painter, widget);
        return;
    case QStyle::CE_Splitter:
        drawElementBackground(painter, option, widget, {u"Splitter"_s});
        return;
    case QStyle::CE_SizeGrip:
    case QStyle::CE_RubberBand:
    case QStyle::CE_DockWidgetTitle:
    case QStyle::CE_ColumnViewGrip:
    case QStyle::CE_MenuEmptyArea:
    case QStyle::CE_HeaderEmptyArea:
    case QStyle::CE_MenuBarEmptyArea:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuHMargin:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_MenuScroller:
    // Scrollbar buttons are ignored for now since they do not exist in qtquick
    case QStyle::CE_ScrollBarAddLine:
    case QStyle::CE_ScrollBarSubLine:
    case QStyle::CE_ScrollBarAddPage:
    case QStyle::CE_ScrollBarSubPage:
    case QStyle::CE_ScrollBarFirst:
    case QStyle::CE_ScrollBarLast:
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
        const auto buttonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option);
        if (!buttonOption) {
            return;
        }
        const auto elements = prepareElements(buttonOption, widget);
        const auto properties = queryProperties(elements);
        auto rect = subControlRect(CC_ToolButton, option, SC_ToolButton, widget);
        drawBackground(painter, rect, properties);

        drawControl(CE_ToolButtonLabel, buttonOption, painter, widget);
        if (buttonOption->features.testFlag(QStyleOptionToolButton::Menu) || buttonOption->features.testFlag(QStyleOptionToolButton::HasMenu)) {
            auto indicatorRect = subControlRect(CC_ToolButton, option, SC_ToolButtonMenu, widget);
            const auto props = queryProperties(prepareElements(buttonOption, widget, {u"Indicator"_s}));
            if (props->icon()) {
                auto icon = QIcon::fromTheme(props->icon()->name().value_or(u"arrow-down-symbolic"_s));
                drawIcon(indicatorRect, option, painter, icon, widget);
                painter->setPen(Qt::cyan);
            }
        }
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
            // Take the button padding into account when drawing this
            if (props->layout() && props->layout()->padding()) {
                auto pad = props->layout()->padding()->toMargins().toMargins();
                indicatorRect.adjust(props->layout()->spacing().value_or(0), pad.top(), -pad.right(), -pad.bottom());
                auto center = indicatorRect.center();
                indicatorRect.setWidth(props->icon()->width().value_or(0));
                indicatorRect.setHeight(props->icon()->height().value_or(0));
                indicatorRect.moveCenter(center);
            }
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
    case QStyle::CC_TitleBar:
    case QStyle::CC_Dial:
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
        auto props = queryProperties(prepareElements(option, widget, {u"IndicatorArrowLeft"_s}));
        auto name = u"arrow-left-symbolic"_s;
        if (props->icon()) {
            name = props->icon()->name().value_or(name);
        }
        auto icon = QIcon::fromTheme(name);
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowUp: {
        auto props = queryProperties(prepareElements(option, widget, {u"IndicatorArrowUp"_s}));
        auto name = u"arrow-up-symbolic"_s;
        if (props->icon()) {
            name = props->icon()->name().value_or(name);
        }
        auto icon = QIcon::fromTheme(name);
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowRight: {
        auto props = queryProperties(prepareElements(option, widget, {u"IndicatorArrowRight"_s}));
        auto name = u"arrow-right-symbolic"_s;
        if (props->icon()) {
            name = props->icon()->name().value_or(name);
        }
        auto icon = QIcon::fromTheme(name);
        drawIcon(option->rect, option, painter, icon, widget);
        return;
    }
    case QStyle::PE_IndicatorArrowDown: {
        auto props = queryProperties(prepareElements(option, widget, {u"IndicatorArrowDown"_s}));
        auto name = u"arrow-down-symbolic"_s;
        if (props->icon()) {
            name = props->icon()->name().value_or(name);
        }
        auto icon = QIcon::fromTheme(name);
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
        break;
    case QStyle::PE_IndicatorBranch:
    case QStyle::PE_IndicatorButtonDropDown:
    case QStyle::PE_IndicatorItemViewItemCheck:
    case QStyle::PE_IndicatorDockWidgetResizeHandle:
    case QStyle::PE_IndicatorHeaderArrow:
    case QStyle::PE_IndicatorMenuCheckMark:
    case QStyle::PE_IndicatorProgressChunk:
    case QStyle::PE_IndicatorToolBarHandle:
    case QStyle::PE_IndicatorToolBarSeparator:
    case QStyle::PE_IndicatorTabTear:
    case QStyle::PE_IndicatorColumnViewArrow:
    case QStyle::PE_IndicatorItemViewItemDrop:
    case QStyle::PE_IndicatorTabClose:
    case QStyle::PE_IndicatorTabTearRight:
        break;
    default:
        drawElementBackground(painter, option, widget);
        return;
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
    case QStyle::CT_PushButton:
        if (const auto *buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            size = contentsSize.grownBy(padding);
            // TODO: currently only works on indicators at start/end
            if (buttonOption->features.testFlag(QStyleOptionButton::HasMenu)) {
                size.rwidth() += pixelMetric(PM_LayoutHorizontalSpacing, buttonOption, widget);
            }
        }
        break;
    case QStyle::CT_ToolButton: {
        if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            size = size.grownBy(padding);
            auto indicatorProps = queryProperties(prepareElements(option, widget, {u"Indicator"_s}));
            if (indicatorProps->layout()) {
                if (toolButtonOption->toolButtonStyle != Qt::ToolButtonTextUnderIcon) {
                    size.rwidth() += indicatorProps->layout()->width().value_or(0);
                } else {
                    size.rheight() += indicatorProps->layout()->height().value_or(0);
                }
            }
        }
    } break;
    case QStyle::CT_MenuItem: {
        const auto *menuItemOpt = qstyleoption_cast<const QStyleOptionMenuItem *>(option);
        if (!menuItemOpt) {
            return size;
        }
        // Handle separator separately (pun not intended)
        if (menuItemOpt->menuItemType == QStyleOptionMenuItem::Separator) {
            auto separatorProps = queryProperties(prepareElements(menuItemOpt, widget, {u"MenuSeparator"_s}));
            if (separatorProps->layout()) {
                int width = separatorProps->layout()->width().value_or(1);
                int height = separatorProps->layout()->height().value_or(1);
                QSize separatorSize(width, height);
                return separatorSize.grownBy(padding);
            }
        } else {
            auto menuProps = queryProperties(prepareElements(menuItemOpt, widget, {u"MenuItem"_s}));
            if (menuProps->layout()) {
                int width = menuProps->layout()->width().value_or(1);
                int height = menuProps->layout()->height().value_or(1);
                if (size.width() > width) {
                    width = size.width();
                }
                if (size.height() > height) {
                    height = size.height();
                }
                QSize itemSize(width, height);
                return itemSize.grownBy(padding);
            }
        }
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
        auto elements = prepareElements(option, widget, {u"TabButton"_s});
        auto props = queryProperties(elements);
        auto textRect = subElementRect(SE_TabBarTabText, option, widget);
        if (props->layout()) {
            if (props->layout()->width()) {
                textRect.setWidth(props->layout()->width().value_or(contentsSize.width()));
            }
            if (props->layout()->height()) {
                textRect.setHeight(props->layout()->height().value_or(contentsSize.height()));
            }
            if (props->layout()->padding()) {
                padding = props->layout()->padding()->toMargins().toMargins();
            }
        }
        return textRect.size().grownBy(padding);
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
    case QStyle::CT_ItemViewItem:
        size = size.grownBy(padding);
        break;
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
    case QStyle::SE_TreeViewDisclosureItem:
    case QStyle::SE_RadioButtonIndicator:
    case QStyle::SE_CheckBoxIndicator: {
        auto elements = prepareElements(option, widget);
        auto map = layoutMap(elements, option, {u"Indicator"_s});
        rect = map[u"Indicator"_s].rect.toRect();
    } break;
    case QStyle::SE_ItemViewItemText:
    case QStyle::SE_ItemViewItemDecoration:
    case QStyle::SE_ItemViewItemCheckIndicator: {
        const auto viewItemOption = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStringList childelements = buildSubElementList(viewItemOption, widget);
        if (childelements.isEmpty()) {
            return QRect();
        }
        auto elements = prepareElements(option, widget, {u"ItemViewItem"_s});
        auto map = layoutMap(elements, option, childelements);

        if (element == SE_ItemViewItemText) {
            rect = map[u"Text"_s].rect.toRect();
        }
        if (element == SE_ItemViewItemDecoration) {
            rect = map[u"Icon"_s].rect.toRect();
        }
        if (element == SE_ItemViewItemCheckIndicator) {
            rect = map[u"CheckBox"_s].rect.toRect();
        }
    } break;
    case QStyle::SE_PushButtonContents:
    case QStyle::SE_RadioButtonContents:
    case QStyle::SE_CheckBoxContents: {
        const auto buttonOption = qstyleoption_cast<const QStyleOptionButton *>(option);
        auto elements = prepareElements(buttonOption, widget);
        QStringList childelements = buildSubElementList(buttonOption, widget);
        if (childelements.isEmpty()) {
            return QRect();
        }
        auto map = layoutMap(elements, option, childelements);
        QRect unifiedRect;

        for (const auto &m : map) {
            // Contents will skip the indicator
            if (element == SE_PushButtonContents) {
                unifiedRect = unifiedRect.united(m.rect.toRect());
            } else {
                if (m.elementName != u"Indicator"_s) {
                    unifiedRect = unifiedRect.united(m.rect.toRect());
                }
            }
        }
        rect = unifiedRect;
    } break;

    case QStyle::SE_PushButtonBevel: {
        auto buttonElements = prepareElements(option, widget);
        auto props = queryProperties(buttonElements);
        rect = backgroundRectangle(option, props).toRect();
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
        auto elements = prepareElements(option, widget, {u"HeaderViewDelegate"_s});
        QStringList childelements = {u"Text"_s, u"Icon"_s};
        auto map = layoutMap(elements, option, childelements);
        auto mapItem = (element == SE_HeaderLabel) ? u"Text"_s : u"Icon"_s;
        rect = map[mapItem].rect.toRect();
    } break;
    case QStyle::SE_TabBarTabText: {
        if (const QStyleOptionTab *tabOption = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            bool verticalTabs = tabOption->shape == QTabBar::RoundedEast || tabOption->shape == QTabBar::RoundedWest
                || tabOption->shape == QTabBar::TriangularEast || tabOption->shape == QTabBar::TriangularWest;
            if (verticalTabs) {
                // TODO: Rely on qcommonstyle for vertical tabs for now
                return QCommonStyle::subElementRect(SE_TabBarTabText, tabOption, widget);
            }

            auto elements = prepareElements(option, widget, {u"TabButton"_s});
            auto map = layoutMap(elements, option, {u"Icon"_s, u"Text"_s});
            QRect unifiedRect;
            for (const auto &m : map) {
                unifiedRect = unifiedRect.united(m.rect.toRect());
            }
            rect = unifiedRect;
        }
    } break;
    case QStyle::SE_ProgressBarLabel: {
        // Copied and repurposed from Breeze
        const auto progressBarOption(qstyleoption_cast<const QStyleOptionProgressBar *>(option));
        if (!progressBarOption) {
            return QRect();
        }
        const bool textVisible(progressBarOption->textVisible);
        const bool busy(progressBarOption->minimum == 0 && progressBarOption->maximum == 0);
        if (!textVisible || busy) {
            return QRect();
        }

        auto props = queryProperties(prepareElements(option, widget));
        auto textFlags = textFlagsFromProperties(props, false);
        int textWidth = qMax(option->fontMetrics.size(textFlags, progressBarOption->text).width(), option->fontMetrics.size(textFlags, u"100%"_s).width());
        auto rect = centerRect(option->rect, textWidth, option->rect.height());
        rect.setLeft(rect.right() - textWidth + 1);
        rect = visualRect(option->direction, option->rect, rect);
        return rect;
    } break;
    case QStyle::SE_ProgressBarContents: {
        // Copied from Breeze
        const auto progressBarOption(qstyleoption_cast<const QStyleOptionProgressBar *>(option));
        if (!progressBarOption) {
            return QRect();
        }
        const auto rect(subElementRect(SE_ProgressBarGroove, progressBarOption, widget));
        const bool busy(progressBarOption->minimum == 0 && progressBarOption->maximum == 0);
        if (busy) {
            return rect;
        }
        const bool horizontal(progressBarOption->state.testFlag(QStyle::State_Horizontal));
        bool reverse = (horizontal && (option->direction == Qt::RightToLeft)) || !horizontal;
        if (progressBarOption->invertedAppearance) {
            reverse = !reverse;
        }
        const int progress(progressBarOption->progress - progressBarOption->minimum);
        const int steps(qMax(progressBarOption->maximum - progressBarOption->minimum, 1));
        const qreal position = qreal(progress) / qreal(steps);
        const int indicatorSize(position * (horizontal ? rect.width() : rect.height()));
        QRect indicatorRect;
        if (horizontal) {
            indicatorRect = QRect(rect.left() + (reverse ? rect.width() - indicatorSize : 0), rect.y(), indicatorSize, rect.height());
        } else {
            indicatorRect = QRect(rect.x(), reverse ? (rect.bottom() - indicatorSize + 1) : rect.top(), rect.width(), indicatorSize);
        }
        return indicatorRect;
    } break;
    case QStyle::SE_ProgressBarGroove: {
        // Copied and repurposed from Breeze
        const auto progressBarOption(qstyleoption_cast<const QStyleOptionProgressBar *>(option));
        if (!progressBarOption) {
            return QRect();
        }
        auto props = queryProperties(prepareElements(option, widget));
        qreal width = 0;
        qreal height = 0;
        if (props->layout()) {
            width = props->layout()->width().value_or(width);
            height = props->layout()->height().value_or(height);
        }
        rect = option->rect;
        rect.setHeight(height);
        rect.setWidth(width);
        if (progressBarOption->state.testFlag(QStyle::State_Horizontal)) {
            rect = centerRect(option->rect, width, height);
        } else {
            rect = centerRect(option->rect, height, width);
        }
        return visualRect(option->direction, option->rect, rect);
    } break;
    // Follow defaults
    case QStyle::SE_TabWidgetTabContents:
    case QStyle::SE_ToolBoxTabContents:
    case QStyle::SE_TabBarTabLeftButton:
    case QStyle::SE_TabBarTabRightButton:
    case QStyle::SE_DockWidgetCloseButton:
    case QStyle::SE_DockWidgetFloatButton:
    case QStyle::SE_DockWidgetIcon:
    case QStyle::SE_DockWidgetTitleBarText:
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
    case QStyle::SE_PushButtonFocusRect:
    case QStyle::SE_ComboBoxFocusRect:
    case QStyle::SE_SliderFocusRect:
    case QStyle::SE_ItemViewItemFocusRect:
    case QStyle::SE_CheckBoxLayoutItem:
    case QStyle::SE_ComboBoxLayoutItem:
    case QStyle::SE_DateTimeEditLayoutItem:
    case QStyle::SE_FrameLayoutItem:
    case QStyle::SE_GroupBoxLayoutItem:
    case QStyle::SE_LabelLayoutItem:
    case QStyle::SE_SpinBoxLayoutItem:
    case QStyle::SE_SliderLayoutItem:
    case QStyle::SE_ProgressBarLayoutItem:
    case QStyle::SE_PushButtonLayoutItem:
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
        if (const auto *toolButtonOption = qstyleoption_cast<const QStyleOptionToolButton *>(option)) {
            // Background
            if (subControl == SC_ToolButton) {
                auto elements = prepareElements(toolButtonOption, widget);
                const auto properties = queryProperties(elements);
                return visualRect(toolButtonOption->direction, toolButtonOption->rect, backgroundRectangle(toolButtonOption, properties).toRect());
            }
            // Menu button background
            if (subControl == SC_ToolButtonMenu) {
                bool hasIndicator =
                    toolButtonOption->features.testFlag(QStyleOptionToolButton::HasMenu) || toolButtonOption->features.testFlag(QStyleOptionToolButton::Menu);
                if (!hasIndicator) {
                    return QRect();
                }
                auto subElements = buildSubElementList(toolButtonOption, widget);
                if (subElements.isEmpty()) {
                    return QRect();
                }
                auto map = layoutMap(prepareElements(toolButtonOption, widget), toolButtonOption, subElements);
                return visualRect(toolButtonOption->direction, toolButtonOption->rect, map[u"Indicator"_s].rect.toRect());
            }
        }
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
            if (sliderOption->minimum == sliderOption->maximum) {
                return groove;
            }
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

    if (complexControl == CC_TitleBar) { }
    if (complexControl == CC_Dial) { }
    if (complexControl == CC_MdiControls) { }
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
    case QStyle::PM_TabBarScrollButtonWidth: {
        auto elements = prepareElements(option, widget, {u"TabScrollButton"_s});
        if (elements.isEmpty()) {
            return defaultMetric;
        }
        auto properties = queryProperties(elements);
        if (!properties) {
            return defaultMetric;
        }
        return properties->layout()->width().value_or(defaultMetric);
    } break;
    case QStyle::PM_TitleBarButtonSize:
    case QStyle::PM_MenuPanelWidth:
    case QStyle::PM_SplitterWidth: {
        if (properties->layout()) {
            return properties->layout()->width().value_or(defaultMetric);
        }
    } break;
    case QStyle::PM_SpinBoxSliderHeight:
    case QStyle::PM_TitleBarHeight:
    case QStyle::PM_MenuScrollerHeight:
    case QStyle::PM_TabBarBaseHeight: {
        if (properties->layout()) {
            return properties->layout()->height().value_or(defaultMetric);
        }
    } break;
    case QStyle::PM_TreeViewIndentation: {
        auto elements = prepareElements(option, widget, {u"Indentation"_s});
        if (elements.isEmpty()) {
            return defaultMetric;
        }
        auto properties = queryProperties(elements);
        if (!properties) {
            return defaultMetric;
        }
        if (properties->layout()) {
            return properties->layout()->width().value_or(defaultMetric);
        }
    } break;

    case QStyle::PM_MessageBoxIconSize:
    case QStyle::PM_ListViewIconSize:
    case QStyle::PM_SmallIconSize:
    case QStyle::PM_ButtonIconSize:
    case QStyle::PM_IconViewIconSize:
    case QStyle::PM_ToolBarIconSize:
    case QStyle::PM_ProgressBarChunkWidth:
    case QStyle::PM_LargeIconSize:
    case QStyle::PM_TabBarIconSize:
    case QStyle::PM_TitleBarButtonIconSize:
    case QStyle::PM_LineEditIconSize:
    case QStyle::PM_SliderTickmarkOffset:
    case QStyle::PM_SliderSpaceAvailable:
    case QStyle::PM_MaximumDragDistance:
    case QStyle::PM_MenuTearoffHeight:
    case QStyle::PM_DockWidgetSeparatorExtent:
    case QStyle::PM_TabBarTabOverlap:
    case QStyle::PM_DockWidgetHandleExtent:
    case QStyle::PM_TabBarBaseOverlap:
    case QStyle::PM_DialogButtonsSeparator:
    case QStyle::PM_DialogButtonsButtonWidth:
    case QStyle::PM_DialogButtonsButtonHeight:
    case QStyle::PM_MdiSubWindowMinimizedWidth:
    case QStyle::PM_HeaderMarkSize:
    case QStyle::PM_HeaderGripMargin:
    case QStyle::PM_DockWidgetTitleMargin:
    case QStyle::PM_DockWidgetTitleBarButtonMargin:
    case QStyle::PM_SizeGripSize:
    case QStyle::PM_TextCursorWidth:
    case QStyle::PM_SubMenuOverlap:
    case QStyle::PM_HeaderDefaultSectionSizeHorizontal:
    case QStyle::PM_HeaderDefaultSectionSizeVertical:
    case QStyle::PM_CustomBase:
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

    const bool enabled = opt->state.testFlag(QStyle::State_Enabled);
    QList<Union::Element::Ptr> elements = prepareElements(opt, widget);
    auto properties = queryProperties(elements);
    QColor penColor;
    if (overrideColor.isValid()) {
        penColor = overrideColor;
    } else {
        // TODO: hide mnemonics if requested
        auto textColor = properties->text()->color();
        penColor = opt->palette.text().color();
        if (textColor) {
            penColor = textColor->toQColor();
        }
    }

    painter->save();
    painter->setPen(penColor);
    drawItemText(painter, rect, textFlagsFromProperties(properties, true), opt->palette, enabled, text);
    painter->restore();
}

void UnionStyle::drawIcon(const QRect &rect, const QStyleOption *opt, QPainter *painter, const QIcon &icon, const QWidget *widget, const QColor &overrideColor)
    const
{
    QList<Union::Element::Ptr> elements = prepareElements(opt, widget);
    const bool enabled = opt->state.testFlag(QStyle::State_Enabled);
    auto properties = queryProperties(elements);

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
    } else if (properties->icon() && properties->icon()->color().has_value()) {
        auto iconColor = properties->icon()->color();
        penColor = iconColor->toQColor();
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
