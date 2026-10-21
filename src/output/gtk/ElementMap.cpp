// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "ElementMap.h"

#include <properties/BorderPropertyGroup.h>
#include <properties/CornersPropertyGroup.h>
#include <properties/LayoutPropertyGroup.h>
#include <properties/LinePropertyGroup.h>
#include <properties/SizePropertyGroup.h>

using namespace Qt::StringLiterals;
using namespace Union::Properties;

namespace Union::Gtk
{

using State = Union::Element::State;

static constexpr auto Hovered = State::Hovered;
static constexpr auto Pressed = State::Pressed;
static constexpr auto Checked = State::Checked;
static constexpr auto Disabled = State::Disabled;
static constexpr auto VisualFocus = State::VisualFocus;
static constexpr auto ActiveFocus = State::ActiveFocus;
static constexpr auto Highlighted = State::Highlighted;

static const StateVariant Base{};
static const StateVariant Backdrop{.pseudo = u":backdrop"_s, .backdrop = true};

static StateVariant state(const QString &pseudo, Union::Element::States states)
{
    return StateVariant{.pseudo = pseudo, .states = states};
}

static StateVariant hint(const QString &pseudo, const QStringList &hints)
{
    return StateVariant{.pseudo = pseudo, .hints = hints};
}

static const QList ButtonStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":active"_s, Pressed),
    state(u":checked"_s, Checked),
    state(u":checked:hover"_s, Checked | Hovered),
    state(u":checked:active"_s, Checked | Pressed),
    state(u":focus-visible"_s, VisualFocus),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
    Backdrop,
};

// GTK entries show focus any time they have keyboard focus
static const QList EntryStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":focus-within"_s, ActiveFocus | VisualFocus),
    state(u":disabled"_s, Disabled),
    Backdrop,
};

static const QList CheckStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":checked"_s, Checked),
    state(u":checked:hover"_s, Checked | Hovered),
    state(u":focus-visible"_s, VisualFocus),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
    Backdrop,
};

static const QList RowStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":active"_s, Pressed),
    state(u":selected"_s, Highlighted),
    state(u":selected:hover"_s, Highlighted | Hovered),
    state(u":disabled"_s, Disabled),
};

// Rows only react to the pointer if they're activatable
static const QList ActivatableRowStates = {
    Base,
    state(u".activatable:hover"_s, Hovered),
    state(u".activatable:active"_s, Pressed),
    state(u":selected"_s, Highlighted),
    state(u".activatable:selected:hover"_s, Highlighted | Hovered),
    state(u":disabled"_s, Disabled),
};

// GTK marks the combo box button as checked while the popup is open
static const QList ComboStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":active"_s, Pressed),
    state(u":checked"_s, Pressed),
    state(u":focus-visible"_s, VisualFocus),
    state(u":disabled"_s, Disabled),
    Backdrop,
};

// Checks & radios that aren't in a check button (menus, tree views) carry the states themselves
static const QList IndicatorStates = {
    Base,
    state(u":checked"_s, Checked),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
};

static const QList CheckIndicatorStates = {
    Base,
    state(u":checked"_s, Checked),
    hint(u":indeterminate"_s, {u"indeterminate"_s}),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
};

static const QList CheckBoxIndicatorStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":checked"_s, Checked),
    state(u":checked:hover"_s, Checked | Hovered),
    state(u":focus-visible"_s, VisualFocus),
    hint(u":indeterminate"_s, {u"indeterminate"_s}),
    StateVariant{.pseudo = u":indeterminate:disabled"_s, .states = Disabled, .hints = {u"indeterminate"_s}},
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
    Backdrop,
};

// Header buttons don't react to the pointer. These undo the states from the button rules
static const QList HeaderStates = {
    Base,
    state(u":hover"_s, {}),
    state(u":active"_s, {}),
    state(u":focus-visible"_s, {}),
    state(u":disabled"_s, Disabled),
};

static const QList BaseAndBackdrop = {Base, Backdrop};
static const QList BaseAndDisabled = {Base, state(u":disabled"_s, Disabled)};

static ElementSpec element(const QString &type, const QStringList &hints = {}, const QVariantMap &attributes = {})
{
    return ElementSpec{type, hints, attributes};
}

static qreal layoutValue(const std::shared_ptr<StylePropertyGroup> &properties, std::optional<qreal> (LayoutPropertyGroup::*getter)() const)
{
    if (!properties || !properties->layout()) {
        return 0.0;
    }
    return (properties->layout()->*getter)().value_or(0.0);
}

static qreal paddingSum(const std::shared_ptr<StylePropertyGroup> &properties, Qt::Orientation orientation)
{
    if (!properties || !properties->layout() || !properties->layout()->padding()) {
        return 0.0;
    }
    const auto margins = properties->layout()->padding()->toMargins();
    return orientation == Qt::Horizontal ? margins.left() + margins.right() : margins.top() + margins.bottom();
}

// GTK centres slider and switch handles in the trough. Negative margins let a handle that's bigger than the trough overflow it
static AdjustFunction overflowAdjust(ElementPath troughPath, Qt::Orientation orientation)
{
    return [troughPath, orientation](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto size = orientation == Qt::Horizontal ? &LayoutPropertyGroup::height : &LayoutPropertyGroup::width;
        const auto trough = context.query(troughPath);
        const auto handle = context.query(troughPath + ElementPath{element(u"Handle"_s)});
        const auto overflow = (layoutValue(handle, size) - layoutValue(trough, size)) / 2.0;
        if (overflow > 0.0) {
            declarations.set(u"margin"_s, orientation == Qt::Horizontal ? toPx(-overflow) + u" 0"_s : u"0 "_s + toPx(-overflow));
        }
    };
}

// GTK sizes a switch by its track, so the handle stuck out of it & rows taller than the track stretched it.
// Union sizes it by the handle, so add the overhang as a transparent border the background doesn't paint
static AdjustFunction trackBorderAdjust(ElementPath troughPath)
{
    return [troughPath](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto trough = context.query(troughPath);
        const auto handle = context.query(troughPath + ElementPath{element(u"Handle"_s)});
        const auto overflow = (layoutValue(handle, &LayoutPropertyGroup::height) - layoutValue(trough, &LayoutPropertyGroup::height)) / 2.0;
        if (overflow <= 0.0) {
            return;
        }

        declarations.set(u"border-style"_s, u"solid"_s);
        declarations.set(u"border-color"_s, u"transparent"_s);
        declarations.set(u"border-width"_s, toPx(overflow) + u" 0"_s);
        declarations.set(u"background-clip"_s, u"padding-box"_s);

        // corners are measured on the outside of the border, so they need the overhang only vertically
        if (trough && trough->corners()) {
            const auto radius = trough->corners()->radii().topLeft;
            declarations.set(u"border-radius"_s, toPx(radius) + u" / "_s + toPx(radius + overflow));
        }
    };
}

static AdjustFunction scrollbarSliderAdjust(ElementPath scrollbarPath, Qt::Orientation orientation)
{
    return [scrollbarPath, orientation](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto scrollbar = context.query(scrollbarPath);
        const auto crossAxis = orientation == Qt::Vertical ? Qt::Horizontal : Qt::Vertical;
        auto size = layoutValue(scrollbar, &LayoutPropertyGroup::width);
        if (orientation == Qt::Horizontal && !scrollbarPath.last().hints.contains(u"noninteractive"_s)) {
            size = layoutValue(scrollbar, &LayoutPropertyGroup::height);
        }
        const auto thickness = std::max(1.0, size - paddingSum(scrollbar, crossAxis));
        declarations.set(orientation == Qt::Vertical ? u"min-width"_s : u"min-height"_s, toPx(thickness));
    };
}

// the slider handle sticks out of the trough, scales need room for it on the cross axis
static AdjustFunction scalePaddingAdjust(ElementSpec slider, Qt::Orientation orientation)
{
    return [slider, orientation](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto size = orientation == Qt::Horizontal ? &LayoutPropertyGroup::height : &LayoutPropertyGroup::width;
        const auto handle = layoutValue(context.query({slider, element(u"Handle"_s)}), size);
        const auto padding = toPx(std::max(0.0, (handle - layoutValue(context.query({slider}), size)) / 2.0));
        declarations.set(u"padding"_s,
                         orientation == Qt::Horizontal ? shorthand(padding, u"0"_s, padding, u"0"_s) : shorthand(u"0"_s, padding, u"0"_s, padding));
    };
}

// tick marks keep the Union tick mark margin from the trough, marks.top is on the other side so it's mirrored
static AdjustFunction marksAdjust(ElementPath tickMark, Qt::Orientation orientation, bool before)
{
    return [tickMark, orientation, before](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto properties = context.query(tickMark);
        const auto margins = properties && properties->layout() ? properties->layout()->margins() : nullptr;
        if (!margins) {
            return;
        }

        const auto margin = toPx((orientation == Qt::Horizontal ? margins->top() : margins->left()).value_or(0.0));
        if (orientation == Qt::Horizontal) {
            declarations.set(before ? u"margin-bottom"_s : u"margin-top"_s, margin);
        } else {
            declarations.set(before ? u"margin-right"_s : u"margin-left"_s, margin);
        }
    };
}

// GTK 3 draws the arrow as big as its box, so the box is the icon size and the rest of the length is padding
static AdjustFunction scrollArrowAdjust(Qt::Orientation orientation)
{
    return [orientation](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto button = context.query({element(u"TabScrollButton"_s)});
        const auto icon = button && button->icon() ? button->icon()->width().value_or(0.0) : 0.0;
        const auto length = layoutValue(button, orientation == Qt::Horizontal ? &LayoutPropertyGroup::width : &LayoutPropertyGroup::height);
        if (icon <= 0.0 || length <= icon) {
            return;
        }

        const auto padding = toPx((length - icon) / 2.0);
        declarations.set(orientation == Qt::Horizontal ? u"min-width"_s : u"min-height"_s, toPx(icon));
        declarations.set(u"padding"_s,
                         orientation == Qt::Horizontal ? shorthand(u"0"_s, padding, u"0"_s, padding) : shorthand(padding, u"0"_s, padding, u"0"_s));
    };
}

// GTK stretches buttons to the height of their box, visible on tab close button hover
static AdjustFunction circleAdjust(ElementPath path)
{
    return [path](DeclarationList &declarations, const RuleContext &context, const StateVariant &variant) {
        const auto properties = context.query(path, variant.states);
        const auto color = declarations.value(u"background-color"_s);
        if (!properties || !properties->corners() || !color || color == u"transparent") {
            return;
        }

        const auto size = std::min(layoutValue(properties, &LayoutPropertyGroup::width), layoutValue(properties, &LayoutPropertyGroup::height));
        const auto radii = properties->corners()->radii();
        if (size <= 0.0 || std::min({radii.topLeft, radii.topRight, radii.bottomRight, radii.bottomLeft}) < size / 2.0) {
            return;
        }

        declarations.set(u"background-color"_s, u"transparent"_s);
        declarations.set(u"background-image"_s, u"radial-gradient(circle closest-side, %1 calc(100% - 1px), transparent)"_s.arg(color.value()));
        declarations.remove(u"border-radius"_s);
    };
}

static QMarginsF borderWidths(const std::shared_ptr<StylePropertyGroup> &properties)
{
    const auto border = properties ? properties->border() : nullptr;
    if (!border) {
        return {};
    }
    const auto size = [](const LinePropertyGroup *line) {
        return line ? line->size().value_or(0.0) : 0.0;
    };
    return QMarginsF(size(border->left()), size(border->top()), size(border->right()), size(border->bottom()));
}

// Union places spin box indicators from the edge of the whole spin box but GTK puts the buttons inside its
// padding, which apps like GIMP change. Only keep the padding before the text & size it with min-height
static AdjustFunction spinBoxAdjust(ElementPath spinBox)
{
    return [spinBox](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto properties = context.query(spinBox);
        if (!properties || !properties->layout() || !properties->layout()->padding()) {
            return;
        }

        const auto border = borderWidths(properties);
        const auto padding = properties->layout()->padding()->toMargins();
        declarations.set(u"padding"_s, shorthand(u"0"_s, u"0"_s, u"0"_s, toPx(padding.left() - border.left())));
        declarations.set(u"min-height"_s, toPx(layoutValue(properties, &LayoutPropertyGroup::height) - border.top() - border.bottom()));
    };
}

static AdjustFunction spinButtonAdjust(ElementPath spinBox, ElementPath indicator, bool atEnd)
{
    return [spinBox, indicator, atEnd](DeclarationList &declarations, const RuleContext &context, const StateVariant &variant) {
        declarations.set(u"padding"_s, u"0"_s);
        declarations.set(u"box-shadow"_s, declarations.value(u"box-shadow"_s).value_or(u"none"_s));

        const auto border = borderWidths(context.query(spinBox));
        const auto properties = context.query(indicator, variant.states);
        const auto margins = properties && properties->layout() && properties->layout()->margins() ? properties->layout()->margins()->toMargins() : QMarginsF{};
        const auto end = atEnd ? toPx(margins.right() - border.right()) : u"0"_s;
        declarations.set(u"margin"_s, shorthand(toPx(margins.top() - border.top()), end, toPx(margins.bottom() - border.bottom()), u"0"_s));
    };
}

// only the sides of the labels padding from the button
static AdjustFunction sidePaddingAdjust(ElementPath path)
{
    return [path](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto properties = context.query(path);
        if (!properties || !properties->layout() || !properties->layout()->padding()) {
            return;
        }

        const auto padding = properties->layout()->padding()->toMargins();
        declarations.set(u"padding"_s, shorthand(u"0"_s, toPx(padding.right()), u"0"_s, toPx(padding.left())));
    };
}

// Parent rules need to come first
static void appendRules(QList<RuleSpec> &specs)
{
    specs.append({
        .selector = u"window.background"_s,
        .path = {},
        .features = Feature::Background | Feature::TextColor | Feature::Font,
        .variants = BaseAndBackdrop,
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"headerbar"_s,
        .path = {element(u"ToolBar"_s, {u"header"_s})},
        .features = Feature::Background | Feature::Border | Feature::TextColor | Feature::Padding | Feature::MinHeight,
        .variants = BaseAndBackdrop,
        .borderMode = BorderMode::Real,
    });

    // toolbar nodes in GTK 3, just a class in GTK 4
    specs.append({
        .selector = u".toolbar, toolbar"_s,
        .path = {element(u"ToolBar"_s)},
        .features = Feature::Background | Feature::TextColor | Feature::Padding | Feature::MinHeight,
        .variants = BaseAndBackdrop,
    });

    specs.append({
        .selector = u"actionbar > revealer > box"_s,
        .path = {element(u"ToolBar"_s, {u"footer"_s})},
        .features = Feature::Background | Feature::Border | Feature::TextColor | Feature::Padding | Feature::MinHeight,
        .variants = BaseAndBackdrop,
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"button"_s,
        .path = {element(u"Button"_s)},
        .features = Feature::Control,
        .variants = ButtonStates,
    });

    specs.append({
        .selector = u"button.flat"_s,
        .path = {element(u"Button"_s, {u"flat"_s})},
        .features = Feature::Control,
        .variants = ButtonStates,
        .parents = {u"button"_s},
    });

    // GIMP shrink the padding of every button, do this so stack switchers keep their side padding on the label
    specs.append({
        .selector = u"stackswitcher > button > label"_s,
        .path = {element(u"Button"_s)},
        .features = Feature::None,
        .variants = {Base},
        .adjust = sidePaddingAdjust({element(u"Button"_s)}),
    });

    specs.append({
        .selector = u".toolbar button, toolbar button"_s,
        .path = {element(u"ToolBar"_s), element(u"ToolButton"_s)},
        .features = Feature::Control,
        .variants = ButtonStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"headerbar button"_s,
        .path = {element(u"ToolBar"_s, {u"header"_s}), element(u"ToolButton"_s)},
        .features = Feature::Control,
        .variants = ButtonStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"combobox button.combo"_s,
        .path = {element(u"ComboBox"_s)},
        .features = Feature::Control,
        .variants = ComboStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"combobox arrow"_s,
        .path = {element(u"ComboBox"_s), element(u"Indicator"_s)},
        .features = Feature::IconSource | Feature::MinSize,
        .variants = {Base},
    });

    specs.append({
        .selector = u"entry"_s,
        .path = {element(u"TextField"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding | Feature::MinHeight | Feature::Outline,
        .variants = EntryStates,
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"spinbutton:not(.vertical)"_s,
        .path = {element(u"SpinBox"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding | Feature::MinHeight | Feature::Outline,
        .variants = EntryStates,
        .borderMode = BorderMode::Real,
        .adjust = spinBoxAdjust({element(u"SpinBox"_s)}),
    });

    // Union spin boxes don't stack the buttons around the text, vertical ones just get the frame
    specs.append({
        .selector = u"spinbutton.vertical"_s,
        .path = {element(u"SpinBox"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Outline,
        .variants = EntryStates,
        .borderMode = BorderMode::Real,
    });

    // GTK puts both buttons at the end so they both use the Union increase button
    // corners are squared unless they're hovered or pressed.
    const ElementPath spinBox{element(u"SpinBox"_s)};
    const ElementPath increase{element(u"SpinBox"_s), element(u"Indicator"_s, {u"increase"_s})};
    const auto spinButton = [&](const QString &selector, bool atEnd) {
        return RuleSpec{
            .selector = selector,
            .path = increase,
            .features =
                Feature::Background | Feature::Border | Feature::Corners | Feature::TextColor | Feature::IconColor | Feature::IconSource | Feature::MinWidth,
            .variants =
                {
                    Base,
                    state(u":hover"_s, Hovered),
                    state(u":active"_s, Pressed),
                    state(u":disabled"_s, Disabled),
                    Backdrop,
                },
            .parents = atEnd ? QStringList{u"button"_s, u"spinbutton:not(.vertical) > button"_s} : QStringList{u"button"_s},
            .adjust = spinButtonAdjust(spinBox, increase, atEnd),
        };
    };
    specs.append(spinButton(u"spinbutton:not(.vertical) > button"_s, false));
    specs.append(spinButton(u"spinbutton:not(.vertical) > button.up"_s, true));

    specs.append({
        .selector = u"spinbutton:not(.vertical) > button.down"_s,
        .path = {element(u"SpinBox"_s), element(u"Indicator"_s, {u"decrease"_s})},
        .features = Feature::IconSource,
        .variants = {Base},
        .parents = {u"spinbutton:not(.vertical) > button"_s},
        .partial = true,
    });

    // Vertical spin buttons stack +/- around the text with no frame of their
    // own, so they only get the indicator look while hovered or pressed
    specs.append({
        .selector = u"spinbutton.vertical > button"_s,
        .path = {element(u"SpinBox"_s), element(u"Indicator"_s)},
        .features = Feature::Background | Feature::Border | Feature::Corners | Feature::TextColor | Feature::IconColor,
        .variants =
            {
                Base,
                state(u":hover"_s, Hovered),
                state(u":active"_s, Pressed),
                state(u":disabled"_s, Disabled),
                Backdrop,
            },
        .parents = {u"button"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"checkbutton"_s,
        .path = {element(u"CheckBox"_s)},
        .features = Feature::TextColor | Feature::Font | Feature::Padding,
        .variants = {Base, state(u":disabled"_s, Disabled), Backdrop},
    });

    specs.append({
        .selector = u"check"_s,
        .path = {element(u"CheckBox"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = CheckIndicatorStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"radio"_s,
        .path = {element(u"RadioButton"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = IndicatorStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"checkbutton{} > check"_s,
        .path = {element(u"CheckBox"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline | Feature::IconSource,
        .variants = CheckBoxIndicatorStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"switch"_s,
        .path = {element(u"Switch"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize,
        .variants = CheckStates,
        .stateAnchor = 0,
        .adjust = trackBorderAdjust({element(u"Switch"_s), element(u"Indicator"_s)}),
    });

    specs.append({
        .selector = u"switch{} > slider"_s,
        .path = {element(u"Switch"_s), element(u"Indicator"_s), element(u"Handle"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline,
        .variants = CheckStates,
        .stateAnchor = 0,
        .adjust = overflowAdjust({element(u"Switch"_s), element(u"Indicator"_s)}, Qt::Horizontal),
    });

    for (const auto orientation : {Qt::Horizontal, Qt::Vertical}) {
        const auto name = orientation == Qt::Horizontal ? u"horizontal"_s : u"vertical"_s;
        const auto minSize = orientation == Qt::Horizontal ? Feature::MinHeight : Feature::MinWidth;
        const auto slider = element(u"Slider"_s, {name});

        specs.append({
            .selector = u"scale.%1"_s.arg(name),
            .path = {slider},
            .features = Feature::None,
            .variants = {Base},
            .adjust = scalePaddingAdjust(slider, orientation),
        });

        for (const bool before : {true, false}) {
            const auto marks = before ? u"top"_s : u"bottom"_s;
            specs.append({
                .selector = u"scale.%1 > marks.%2"_s.arg(name, marks),
                .path = {slider, element(u"TickMark"_s)},
                .features = Feature::None,
                .variants = {Base},
                .adjust = marksAdjust({slider, element(u"TickMark"_s)}, orientation, before),
            });
        }

        specs.append({
            .selector = u"scale.%1 trough"_s.arg(name),
            .path = {slider},
            .features = Feature::Background | Feature::Border | Feature::Corners | minSize,
            .variants = {Base, state(u":disabled"_s, Disabled), Backdrop},
        });

        specs.append({
            .selector = u"scale.%1{} trough > slider{}"_s.arg(name),
            .path = {slider, element(u"Handle"_s)},
            .features = Feature::Frame | Feature::MinSize | Feature::Outline,
            .variants =
                {
                    Base,
                    state(u":hover"_s, Hovered),
                    StateVariant{.pseudo = u":active"_s, .states = Pressed, .unionAnchor = 0},
                    StateVariant{.pseudo = u":focus-visible"_s, .states = VisualFocus, .unionAnchor = 0, .gtkMarker = 0},
                    StateVariant{.pseudo = u":disabled"_s, .states = Disabled, .unionAnchor = 0},
                    Backdrop,
                },
            .adjust = overflowAdjust({slider}, orientation),
        });

        specs.append({
            .selector = u"progressbar.%1 > trough"_s.arg(name),
            .path = {element(u"ProgressBar"_s, {name})},
            .features = Feature::Background | Feature::Border | Feature::Corners | Feature::MinSize,
            .variants = BaseAndDisabled,
        });

        specs.append({
            .selector = u"scale.%1 > marks > mark > indicator"_s.arg(name),
            .path = {slider, element(u"TickMark"_s)},
            .features = Feature::Background | Feature::MinSize,
            .variants = BaseAndDisabled,
            .stateAnchor = 0,
        });

        specs.append({
            .selector = u"levelbar.%1 > trough"_s.arg(name),
            .path = {element(u"ProgressBar"_s, {name})},
            .features = Feature::Background | Feature::Border | Feature::Corners,
            .variants = BaseAndDisabled,
        });

        const auto progressBar = element(u"ProgressBar"_s, {name});
        specs.append({
            .selector = u"levelbar.%1 > trough > block"_s.arg(name),
            .path = {progressBar, element(u"Chunk"_s)},
            .features = Feature::Corners | Feature::MinSize,
            .variants = {Base},
            .adjust =
                [progressBar, orientation](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
                    const auto bar = context.query({progressBar});
                    if (orientation == Qt::Horizontal) {
                        declarations.set(u"min-height"_s, toPx(layoutValue(bar, &LayoutPropertyGroup::height)));
                    } else {
                        declarations.set(u"min-width"_s, toPx(layoutValue(bar, &LayoutPropertyGroup::width)));
                        declarations.set(u"min-height"_s, toPx(layoutValue(context.query({progressBar, element(u"Chunk"_s)}), &LayoutPropertyGroup::width)));
                    }
                },
        });

        const auto scrollbar = element(u"ScrollBar"_s, {name});
        specs.append({
            .selector = u"scrollbar.%1"_s.arg(name),
            .path = {scrollbar},
            .features = Feature::Background | Feature::Border | Feature::Padding,
            .variants = {Base},
            .borderMode = BorderMode::Real,
        });

        specs.append({
            .selector = u"scrollbar.%1 slider"_s.arg(name),
            .path = {scrollbar, element(u"Handle"_s)},
            .features = Feature::Background | Feature::Border | Feature::Corners,
            .variants =
                {
                    Base,
                    state(u":hover"_s, Hovered),
                    state(u":active"_s, Pressed),
                    state(u":disabled"_s, Disabled),
                },
            .adjust = scrollbarSliderAdjust({scrollbar}, orientation),
        });

        const auto indicator = element(u"ScrollBar"_s, {name, u"noninteractive"_s});
        specs.append({
            .selector = u"scrollbar.%1.overlay-indicator:not(.hovering)"_s.arg(name),
            .path = {indicator},
            .features = Feature::Background | Feature::Border | Feature::Padding,
            .variants = {Base},
            .borderMode = BorderMode::Real,
        });

        specs.append({
            .selector = u"scrollbar.%1.overlay-indicator:not(.hovering) slider"_s.arg(name),
            .path = {indicator, element(u"Handle"_s)},
            .features = Feature::Background | Feature::Border | Feature::Corners,
            .variants = {Base},
            .adjust = scrollbarSliderAdjust({indicator}, orientation),
        });
    }

    specs.append({
        .selector = u"scale{} trough > highlight"_s,
        .path = {element(u"Slider"_s), element(u"Fill"_s)},
        .features = Feature::Background | Feature::Border | Feature::Corners,
        .variants = BaseAndDisabled,
        .stateAnchor = 0,
    });

    // Scale fill is a range, like the buffered part of a video
    specs.append({
        .selector = u"scale trough > fill"_s,
        .path = {element(u"ProgressBar"_s)},
        .features = Feature::Background,
        .variants = {Base},
    });

    specs.append({
        .selector = u"progressbar{} > trough > progress"_s,
        .path = {element(u"ProgressBar"_s), element(u"Track"_s)},
        .features = Feature::Background | Feature::Border | Feature::Corners,
        .variants = BaseAndDisabled,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"levelbar{} > trough > block.filled, levelbar{} > trough > block.high, levelbar{} > trough > block.full"_s,
        .path = {element(u"ProgressBar"_s), element(u"Track"_s)},
        .features = Feature::Background | Feature::Border,
        .variants = BaseAndDisabled,
        .stateAnchor = 0,
    });

    for (const auto &direction : {u"top"_s, u"bottom"_s, u"left"_s, u"right"_s}) {
        const auto attributes = QVariantMap{{u"direction"_s, direction}};

        // the line goes under the tabs so the open tab joins the page
        specs.append({
            .selector = u"notebook > header.%1"_s.arg(direction),
            .path = {element(u"TabBar"_s, {}, attributes)},
            .features = Feature::Background | Feature::Border,
            .variants = BaseAndBackdrop,
        });

        specs.append({
            .selector = u"notebook > header.%1 > tabs > tab"_s.arg(direction),
            .path = {element(u"TabBar"_s, {}, attributes), element(u"TabButton"_s, {}, attributes)},
            .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding,
            .variants =
                {
                    Base,
                    hint(u":first-child"_s, {u"first"_s}),
                    hint(u":last-child"_s, {u"last"_s}),
                    hint(u":only-child"_s, {u"first"_s, u"last"_s}),
                    state(u":hover"_s, Hovered),
                    state(u":checked"_s, Checked),
                    state(u":checked:hover"_s, Checked | Hovered),
                    state(u":disabled"_s, Disabled),
                    Backdrop,
                },
        });
    }

    // scroll arrows only get the length along the tab bar so they don't make the tabs taller
    specs.append({
        .selector = u"notebook > header.top > tabs > arrow, notebook > header.bottom > tabs > arrow"_s,
        .path = {element(u"TabScrollButton"_s)},
        .features = Feature::MinWidth | Feature::IconSize,
        .variants = {Base},
        .adjust = scrollArrowAdjust(Qt::Horizontal),
    });

    specs.append({
        .selector = u"notebook > header.left > tabs > arrow, notebook > header.right > tabs > arrow"_s,
        .path = {element(u"TabScrollButton"_s)},
        .features = Feature::MinHeight | Feature::IconSize,
        .variants = {Base},
        .adjust = scrollArrowAdjust(Qt::Vertical),
    });

    const ElementPath closeButton = {element(u"TabBar"_s), element(u"TabButton"_s), element(u"CloseButton"_s)};
    specs.append({
        .selector = u"notebook > header > tabs > tab button.flat"_s,
        .path = closeButton,
        .features = Feature::Background | Feature::Corners | Feature::TextColor | Feature::MinSize,
        .variants = {Base, state(u":hover"_s, Hovered), state(u":active"_s, Pressed), state(u":disabled"_s, Disabled)},
        .parents = {u"button"_s, u"button.flat"_s},
        .adjust = circleAdjust(closeButton),
    });

    specs.append({
        .selector = u"notebook > stack:not(:only-child)"_s,
        .path = {element(u"TabWidget"_s)},
        .features = Feature::Background,
        .variants = BaseAndBackdrop,
    });

    // only notebooks with a border get the page frame, overrides.css sorts out the tab side
    specs.append({
        .selector = u"notebook.frame > stack:not(:only-child)"_s,
        .path = {element(u"TabWidget"_s)},
        .features = Feature::Border | Feature::Corners | Feature::Padding,
        .variants = BaseAndBackdrop,
        .borderMode = BorderMode::Real,
        .parents = {u"notebook > stack:not(:only-child)"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"popover.menu modelbutton"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding | Feature::MinSize,
        .variants =
            {
                Base,
                state(u":hover"_s, Hovered),
                state(u":selected"_s, Hovered),
                state(u":active"_s, Pressed),
                state(u":disabled"_s, Disabled),
            },
    });

    specs.append({
        .selector = u"popover.menu modelbutton{} > accelerator"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s), element(u"ShortcutText"_s)},
        .features = Feature::TextColor,
        .variants = BaseAndDisabled,
        .gtkMarker = 0,
    });

    specs.append({
        .selector = u"popover.menu modelbutton check{}"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = IndicatorStates,
        .stateAnchor = 1,
    });

    specs.append({
        .selector = u"popover.menu modelbutton radio{}"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s, {u"exclusive"_s}), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = IndicatorStates,
        .stateAnchor = 1,
    });

    specs.append({
        .selector = u".navigation-sidebar > row"_s,
        .path = {element(u"ItemDelegate"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding | Feature::Margin,
        .variants = RowStates,
    });

    // GTK rows react to the pointer, so they're the hover list delegates.
    specs.append({
        .selector = u"list > row"_s,
        .path = {element(u"ItemDelegate"_s, {u"inside-list"_s, u"hover-enabled"_s})},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding | Feature::Margin,
        .variants = ActivatableRowStates,
    });

    specs.append({
        .selector = u"list > row:last-child"_s,
        .path = {element(u"ItemDelegate"_s, {u"inside-list"_s, u"hover-enabled"_s, u"last-in-list"_s})},
        .features = Feature::Padding | Feature::Margin,
        .variants = {Base},
        .parents = {u"list > row"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"treeview.view"_s,
        .path = {element(u"TreeViewDelegate"_s)},
        .features = Feature::Background | Feature::TextColor,
        .variants = {Base, state(u":selected"_s, Highlighted), state(u":disabled"_s, Disabled), Backdrop},
    });

    // GTK 3 only has tree views, GTK 4 has column views too
    specs.append({
        .selector = u"columnview > header > button, treeview.view header button"_s,
        .path = {element(u"HeaderViewDelegate"_s)},
        .features = Feature::Background | Feature::Border | Feature::TextColor | Feature::Font | Feature::Padding | Feature::Corners | Feature::Shadow,
        .variants = HeaderStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"popover.menu modelbutton arrow:not(.left)"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s), element(u"Arrow"_s)},
        .features = Feature::IconSource | Feature::MinSize,
        .variants = {Base},
    });

    specs.append({
        .selector = u"menubar"_s,
        .path = {element(u"MenuBar"_s)},
        .features = Feature::Background | Feature::TextColor,
        .variants = BaseAndBackdrop,
    });

    // Suggested & destructive actions don't exist in Union. Use a selected item and an error message instead
    // FIXME: define the hover and pressed colours of these in breeze union, until then they look the same in every state
    const auto accentStates = [](Union::Element::States base) {
        return QList{
            state(QString{}, base),
            state(u":hover"_s, base | Hovered),
            state(u":active"_s, base | Pressed),
            state(u":checked"_s, base | Pressed),
        };
    };

    specs.append({
        .selector = u"button.suggested-action:not(:disabled){}, headerbar button.suggested-action:not(:disabled){}"_s,
        .path = {element(u"ItemDelegate"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::IconColor,
        .variants = accentStates(Highlighted),
        .parents = {u"button"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"button.destructive-action:not(:disabled){}, headerbar button.destructive-action:not(:disabled){}"_s,
        .path = {element(u"InlineMessage"_s, {}, {{u"type"_s, u"error"_s}})},
        .features = Feature::Background | Feature::Border | Feature::Shadow | Feature::TextColor | Feature::IconColor,
        .variants = accentStates({}),
        .parents = {u"button"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"popover.menu separator"_s,
        .path = {element(u"Menu"_s), element(u"MenuSeparator"_s)},
        .features = Feature::Background | Feature::Margin,
        .variants = {Base},
    });

    specs.append({
        .selector = u"headerbar separator.vertical"_s,
        .path = {element(u"ToolBar"_s, {u"header"_s}), element(u"ToolSeparator"_s, {u"vertical"_s})},
        .features = Feature::Background | Feature::MinWidth | Feature::Margin,
        .variants = {Base},
    });

    specs.append({
        .selector = u".toolbar separator.vertical, actionbar separator.vertical, toolbar separator"_s,
        .path = {element(u"ToolBar"_s), element(u"ToolSeparator"_s, {u"vertical"_s})},
        .features = Feature::Background | Feature::MinWidth | Feature::Margin,
        .variants = {Base},
    });

    specs.append({
        .selector = u"scrolledwindow > junction"_s,
        .path = {element(u"ScrollAreaCorner"_s)},
        .features = Feature::Background,
        .variants = BaseAndBackdrop,
    });
}

static void appendGtk4Rules(QList<RuleSpec> &specs)
{
    specs.append({
        .selector = u"popover.emoji-picker emoji"_s,
        .path = {element(u"ItemDelegate"_s)},
        .features = Feature::Background,
        .variants = {state(u":hover"_s, Hovered), state(u":focus"_s, Hovered)},
    });

    specs.append({
        .selector = u"checkbutton{} > radio"_s,
        .path = {element(u"RadioButton"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline | Feature::IconSource,
        .variants = CheckStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"dropdown > button"_s,
        .path = {element(u"ComboBox"_s)},
        .features = Feature::Control,
        .variants = ComboStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"dropdown arrow, menubutton arrow.down"_s,
        .path = {element(u"ComboBox"_s), element(u"Indicator"_s)},
        .features = Feature::IconSource | Feature::MinSize,
        .variants = {Base},
    });

    specs.append({
        .selector = u"entry{} > text > placeholder"_s,
        .path = {element(u"TextField"_s), element(u"Text"_s, {u"placeholder"_s})},
        .features = Feature::TextColor,
        .variants = BaseAndDisabled,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"popover > contents"_s,
        .path = {element(u"Popup"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding,
        .variants = {Base},
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"popover > arrow"_s,
        .path = {element(u"Popup"_s)},
        .features = Feature::Background | Feature::Border,
        .variants = {Base},
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"popover.menu > contents"_s,
        .path = {element(u"Menu"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding,
        .variants = {Base},
        .borderMode = BorderMode::Real,
        .parents = {u"popover > contents"_s},
    });

    specs.append({
        .selector = u"popover.menu > arrow"_s,
        .path = {element(u"Menu"_s)},
        .features = Feature::Background | Feature::Border,
        .variants = {Base},
        .borderMode = BorderMode::Real,
        .parents = {u"popover > arrow"_s},
    });

    specs.append({
        .selector = u"listview > row"_s,
        .path = {element(u"ItemDelegate"_s, {u"inside-list"_s, u"hover-enabled"_s})},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding | Feature::Margin,
        .variants = ActivatableRowStates,
    });

    specs.append({
        .selector = u"listview > row:last-child"_s,
        .path = {element(u"ItemDelegate"_s, {u"inside-list"_s, u"hover-enabled"_s, u"last-in-list"_s})},
        .features = Feature::Padding | Feature::Margin,
        .variants = {Base},
        .parents = {u"listview > row"_s},
        .partial = true,
    });

    // column views are tables in Union, the cells are the delegates so the rows don't get padding
    specs.append({
        .selector = u"columnview > listview > row"_s,
        .path = {element(u"ItemDelegate"_s, {u"inside-table"_s})},
        .features = Feature::Frame | Feature::TextColor,
        .variants = ActivatableRowStates,
        .parents = {u"listview > row"_s},
    });

    specs.append({
        .selector = u"columnview > listview > row > cell"_s,
        .path = {element(u"ItemDelegate"_s, {u"inside-table"_s})},
        .features = Feature::Padding,
        .variants = {Base},
    });

    specs.append({
        .selector = u"gridview > child"_s,
        .path = {element(u"ItemDelegate"_s, {}, {{u"display"_s, u"text-under-icon"_s}})},
        .features = Feature::Frame | Feature::TextColor,
        .variants = {Base, state(u":hover"_s, Hovered), state(u":selected"_s, Highlighted), state(u":disabled"_s, Disabled)},
    });

    specs.append({
        .selector = u"dropdown > popover > contents"_s,
        .path = {element(u"Popup"_s, {u"combobox"_s})},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding,
        .variants = {Base},
        .borderMode = BorderMode::Real,
        .parents = {u"popover > contents"_s},
    });

    // GTK selects drop-down rows on hover, all of it is the combo box highlight
    specs.append({
        .selector = u"dropdown > popover listview > row"_s,
        .path = {element(u"Popup"_s, {u"combobox"_s}), element(u"ItemDelegate"_s)},
        .features = Feature::Frame | Feature::TextColor,
        .variants =
            {
                Base,
                state(u".activatable:hover"_s, Highlighted),
                state(u":selected"_s, Highlighted),
                state(u".activatable:selected:hover"_s, Highlighted),
                state(u":disabled"_s, Disabled),
            },
        .parents = {u"listview > row"_s},
    });

    specs.append({
        .selector = u"dropdown > popover listview > row > box"_s,
        .path = {element(u"Popup"_s, {u"combobox"_s}), element(u"ItemDelegate"_s)},
        .features = Feature::Spacing,
        .variants = {Base},
    });

    specs.append({
        .selector = u"menubar > item"_s,
        .path = {element(u"MenuBar"_s), element(u"MenuBarItem"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding | Feature::MinHeight,
        .variants = {Base, state(u":hover"_s, Hovered), state(u":selected"_s, Highlighted)},
    });

    // Window controls fill the header bar height, the image is what you actually see
    // TODO: define the hover and click states in breeze union (colours, backdrop, border)
    specs.append({
        .selector = u"windowcontrols > button > image"_s,
        .path = {element(u"TitleBar"_s), element(u"CloseButton"_s)},
        .features = Feature::MinSize,
        .variants = {Base},
        .adjust =
            [](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
                const auto titleBar = context.query({element(u"TitleBar"_s)});
                if (titleBar && titleBar->icon() && titleBar->icon()->width().value_or(0.0) > 0.0) {
                    declarations.set(u"-gtk-icon-size"_s, toPx(titleBar->icon()->width().value()));
                }
            },
    });

    specs.append({
        .selector = u"windowcontrols > button{} > image"_s,
        .path = {element(u"ToolBar"_s, {u"header"_s}), element(u"ToolButton"_s)},
        .features = Feature::Background | Feature::Border | Feature::Corners | Feature::TextColor,
        .variants = {Base, state(u":hover"_s, Hovered), state(u":active"_s, Pressed), Backdrop},
    });
}

static void appendGtk3Rules(QList<RuleSpec> &specs)
{
    specs.append({
        .selector = u"radiobutton"_s,
        .path = {element(u"RadioButton"_s)},
        .features = Feature::TextColor | Feature::Font | Feature::Padding,
        .variants = {Base, state(u":disabled"_s, Disabled), Backdrop},
    });

    specs.append({
        .selector = u"radiobutton{} > radio"_s,
        .path = {element(u"RadioButton"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline | Feature::IconSource,
        .variants = CheckStates,
        .stateAnchor = 0,
    });

    // GTK 3 stack sidebars are missing the navigation-sidebar class
    specs.append({
        .selector = u"stacksidebar list > row"_s,
        .path = {element(u"ItemDelegate"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding | Feature::Margin,
        .variants = ActivatableRowStates,
        .parents = {u"list > row"_s},
    });

    // No contents node on GTK 3 popovers
    specs.append({
        .selector = u"popover.background"_s,
        .path = {element(u"Popup"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding,
        .variants = {Base},
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"popover.background.menu"_s,
        .path = {element(u"Menu"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding,
        .variants = {Base},
        .borderMode = BorderMode::Real,
        .parents = {u"popover.background"_s},
    });

    specs.append({
        .selector = u"menu, .menu, .context-menu"_s,
        .path = {element(u"Menu"_s)},
        .features = Feature::Background | Feature::Border | Feature::TextColor | Feature::Padding,
        .variants = {Base},
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"menu menuitem, .menu menuitem, .context-menu menuitem"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding | Feature::MinHeight,
        .variants = {Base, state(u":hover"_s, Hovered), state(u":disabled"_s, Disabled)},
    });

    specs.append({
        .selector = u"menuitem{} accelerator"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s), element(u"ShortcutText"_s)},
        .features = Feature::TextColor,
        .variants = BaseAndDisabled,
        .gtkMarker = 0,
    });

    specs.append({
        .selector = u"menuitem arrow"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s), element(u"Arrow"_s)},
        .features = Feature::IconSource | Feature::MinSize,
        .variants = {Base},
    });

    specs.append({
        .selector = u"menuitem check{}"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = IndicatorStates,
        .stateAnchor = 1,
    });

    specs.append({
        .selector = u"menuitem radio{}"_s,
        .path = {element(u"Menu"_s), element(u"MenuItem"_s, {u"exclusive"_s}), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = IndicatorStates,
        .stateAnchor = 1,
    });

    specs.append({
        .selector = u"menu separator, .menu separator, .context-menu separator"_s,
        .path = {element(u"Menu"_s), element(u"MenuSeparator"_s)},
        .features = Feature::Background | Feature::Margin,
        .variants = {Base},
    });

    specs.append({
        .selector = u"menubar > menuitem"_s,
        .path = {element(u"MenuBar"_s), element(u"MenuBarItem"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Padding | Feature::MinHeight,
        .variants = {Base, state(u":hover"_s, Hovered)},
    });

    // Title buttons aren't stretched in GTK 3 so they need the size set directly
    specs.append({
        .selector = u"headerbar button.titlebutton, .titlebar button.titlebutton"_s,
        .path = {element(u"TitleBar"_s), element(u"CloseButton"_s)},
        .features = Feature::MinSize,
        .variants = {Base},
    });
}

QList<RuleSpec> ruleSpecs(GtkVersion version)
{
    QList<RuleSpec> specs;
    appendRules(specs);
    if (version == GtkVersion::Gtk4) {
        appendGtk4Rules(specs);
    } else {
        appendGtk3Rules(specs);
    }
    return specs;
}
}
