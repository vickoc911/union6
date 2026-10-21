// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "ThemeGenerator.h"

#include <algorithm>
#include <cmath>
#include <tuple>

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QSet>
#include <utility>

#include <Element.h>
#include <ElementQuery.h>
#include <Style.h>

#include "ColorTranslator.h"
#include "ThemeAssets.h"

#include "gtk_logging.h"

using namespace Qt::StringLiterals;
using namespace Union::Gtk;
using namespace Union::Properties;

using State = Union::Element::State;

static QString pseudoForVersion(const QString &pseudo, GtkVersion version)
{
    if (version == GtkVersion::Gtk4) {
        return pseudo;
    }
    auto result = pseudo;
    result.replace(u":focus-visible"_s, u":focus"_s);
    result.replace(u":focus-within"_s, u":focus"_s);
    return result;
}

static QString readResource(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(UNION_GTK) << "Could not read" << path << file.errorString();
        return QString{};
    }
    return QString::fromUtf8(file.readAll());
}

static QString readStylesheet(const QString &name, GtkVersion version)
{
    return readResource(u":/union-gtk/common/"_s + name) + u"\n"_s + readResource(u":/union-gtk/gtk%1/"_s.arg(int(version)) + name);
}

static QString describePath(const ElementPath &path)
{
    QStringList parts{u"ApplicationWindow"_s};
    for (const auto &element : path) {
        auto part = element.type;
        for (const auto &hint : element.hints) {
            part += u'.' + hint;
        }
        for (auto it = element.attributes.cbegin(); it != element.attributes.cend(); ++it) {
            part += u"[%1=%2]"_s.arg(it.key(), it.value().toString());
        }
        parts.append(part);
    }
    return parts.join(u" > "_s);
}

static QString applyPseudo(const QString &selector, const QString &pseudo, int marker)
{
    static const QString markerText = u"{}"_s;

    QStringList result;
    const auto alternatives = selector.split(u',');
    for (const auto &alternative : alternatives) {
        auto text = alternative.trimmed();
        const auto count = text.count(markerText);
        if (count == 0) {
            result.append(text + pseudo);
            continue;
        }

        const auto target = marker < 0 || marker >= count ? count - 1 : marker;
        QString output;
        qsizetype position = 0;
        for (int index = 0; index < count; ++index) {
            const auto next = text.indexOf(markerText, position);
            output += text.mid(position, next - position);
            if (index == target) {
                output += pseudo;
            }
            position = next + markerText.size();
        }
        output += text.mid(position);
        result.append(output);
    }

    return result.join(u",\n"_s);
}

// Tokens are tagged with their marker so we can tell when a variant is a more specific version of another
static QSet<QString> pseudoTokens(const QString &pseudo, int marker)
{
    static const QRegularExpression tokenExpression(uR"((:[a-z-]+(?:\([^)]*\))?|\.[A-Za-z0-9_-]+))"_s);

    QSet<QString> result;
    for (const auto &match : tokenExpression.globalMatch(pseudo)) {
        result.insert(QString::number(marker) + match.captured(1));
    }
    return result;
}

// :not(x) counts as x like in GTK. Our selectors never use ids
static int selectorSpecificity(const QString &selector)
{
    static const QRegularExpression classExpression(uR"((\.[\w-]+|:(?!not\()[\w-]+))"_s);
    static const QRegularExpression elementExpression(uR"((?:^|[\s>+~(])([a-z][\w-]*))"_s);
    static const QRegularExpression pseudoExpression(uR"(:[\w-]+)"_s);

    auto alternative = selector.section(u',', 0, 0).trimmed();
    alternative.remove(u"{}"_s);

    const auto classes = alternative.count(classExpression);
    auto withoutPseudo = alternative;
    withoutPseudo.remove(pseudoExpression);
    const auto elements = withoutPseudo.count(elementExpression);

    return classes * 100 + elements;
}

static std::optional<Union::Color> firstVisibleLine(const BorderPropertyGroup *group)
{
    if (!group) {
        return std::nullopt;
    }

    for (const auto line : {group->top(), group->right(), group->bottom(), group->left()}) {
        if (!line || line->style().value_or(LineStyle::Solid) == LineStyle::None || line->size().value_or(0.0) <= 0.0) {
            continue;
        }
        if (auto color = resolveColor(line->color()); color && color->alpha() > 0) {
            return line->color();
        }
    }

    return std::nullopt;
}

static std::optional<qreal> fromPx(const QString &value)
{
    static const QRegularExpression lengthExpression(uR"(^(-?\d+(?:\.\d+)?)px$)"_s);
    const auto match = lengthExpression.match(value);
    if (!match.hasMatch()) {
        return std::nullopt;
    }
    return match.captured(1).toDouble();
}

// $name becomes the metric and -$name its negative
static QString substituteMetrics(const QString &css, const QHash<QString, QString> &metrics)
{
    static const QRegularExpression reference(uR"((-?)\$([a-z][a-z0-9-]*))"_s);

    QStringList lines;
    for (const auto &line : css.split(u'\n')) {
        QString result;
        qsizetype position = 0;
        bool missing = false;
        for (const auto &match : reference.globalMatch(line)) {
            const auto name = match.captured(2);
            if (!metrics.contains(name)) {
                qCWarning(UNION_GTK) << "Unknown metric" << name << "in static stylesheet";
            }
            auto value = metrics.value(name);
            if (value.isEmpty()) {
                missing = true;
                break;
            }
            if (!match.captured(1).isEmpty() && value != u"0") {
                value = value.startsWith(u'-') ? value.mid(1) : u'-' + value;
            }
            result += line.mid(position, match.capturedStart() - position) + value;
            position = match.capturedEnd();
        }
        if (!missing) {
            lines.append(result + line.mid(position));
        }
    }
    return lines.join(u'\n');
}

static bool writeIfChanged(const QString &path, const QByteArray &contents)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly) && file.readAll() == contents) {
        return false;
    }
    file.close();

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) || file.write(contents) != contents.size()) {
        qCWarning(UNION_GTK) << "Could not write" << path << file.errorString();
        return false;
    }
    return true;
}

// true if anything changed
static bool writeStylesheet(const QString &directory, const GeneratedStylesheet &sheet)
{
    QDir(directory).mkpath(u"assets"_s);

    bool changed = writeIfChanged(directory + u"/gtk.css"_s, sheet.css.toUtf8());
    // GTK falls back to its built-in theme if there's no dark variant
    changed |= writeIfChanged(directory + u"/gtk-dark.css"_s, "@import url(\"gtk.css\");\n");
    for (const auto &asset : sheet.assets) {
        changed |= writeIfChanged(directory + u'/' + asset.first, asset.second);
    }

    // Clean up images left over from an older version of the style
    QDir assets(directory + u"/assets"_s);
    for (const auto &name : assets.entryList(QDir::Files)) {
        if (!sheet.assets.contains(u"assets/"_s + name)) {
            changed |= assets.remove(name);
        }
    }

    return changed;
}

struct EmittedRule {
    QSet<QString> tokens;
    int specificity = 0;
    int order = 0;
    DeclarationList declarations;
};

ThemeGenerator::ThemeGenerator(std::shared_ptr<Union::Style> style)
    : m_style(std::move(style))
{
}

std::shared_ptr<StylePropertyGroup> ThemeGenerator::query(const ElementPath &path, Union::Element::States states) const
{
    return queryVariant(path, -1, StateVariant{.states = states});
}

std::shared_ptr<StylePropertyGroup> ThemeGenerator::queryVariant(const ElementPath &path, int anchor, const StateVariant &variant) const
{
    ElementList elements;

    auto root = Union::Element::create();
    root->setType(u"ApplicationWindow"_s);
    if (variant.backdrop) {
        root->setHints({u"inactive"_s});
    }
    elements.append(root);

    const auto anchorIndex = anchor < 0 ? path.size() - 1 : anchor;
    for (qsizetype index = 0; index < path.size(); ++index) {
        const auto &spec = path.at(index);
        auto element = Union::Element::create();
        element->setType(spec.type);
        element->setAttributes(spec.attributes);

        auto hints = spec.hints;
        if (index == anchorIndex) {
            element->setStates(variant.states);
            hints.append(variant.hints);
        }
        element->setHints(hints);
        elements.append(element);
    }

    if (path.isEmpty()) {
        root->setStates(variant.states);
        root->setHints(root->hints() + variant.hints);
    }

    Union::ElementQuery elementQuery(m_style);
    elementQuery.setElements(elements);
    elementQuery.execute();
    if (!elementQuery.properties()) {
        return nullptr;
    }

    auto result = std::make_shared<StylePropertyGroup>();
    StylePropertyGroup::resolveProperties(elementQuery.properties(), result.get());
    return result;
}

QString ThemeGenerator::palette() const
{
    auto background = [this](const ElementPath &path, Union::Element::States states = {}, bool backdrop = false) -> std::optional<Union::Color> {
        const auto properties = queryVariant(path, -1, StateVariant{.states = states, .backdrop = backdrop});
        return properties && properties->background() ? properties->background()->color() : std::nullopt;
    };
    auto text = [this](const ElementPath &path, Union::Element::States states = {}, bool backdrop = false) -> std::optional<Union::Color> {
        const auto properties = queryVariant(path, -1, StateVariant{.states = states, .backdrop = backdrop});
        return properties && properties->text() ? properties->text()->color() : std::nullopt;
    };
    auto border = [this](const ElementPath &path, bool backdrop = false) {
        const auto properties = queryVariant(path, -1, StateVariant{.backdrop = backdrop});
        return properties ? firstVisibleLine(properties->border()) : std::nullopt;
    };

    const ElementPath window;
    const ElementPath view{{u"TextField"_s}};
    const ElementPath selection{{u"ItemDelegate"_s}};
    const ElementPath button{{u"Button"_s}};
    const ElementPath frame{{u"Frame"_s}};
    const ElementPath label{{u"Label"_s}};
    auto message = [](const QString &type) {
        return ElementPath{{u"InlineMessage"_s, {}, {{u"type"_s, type}}}};
    };
    // No links in the style, use the colour scheme's
    auto scheme = [](const QString &role) {
        return Union::Color::custom(u"kcolorscheme"_s, {u"active"_s, u"view"_s, u"foreground"_s, role});
    };

    const QList<std::pair<QStringList, std::optional<Union::Color>>> colors = {
        {{u"theme_bg_color"_s}, background(window)},
        {{u"theme_fg_color"_s}, text(window)},
        {{u"theme_base_color"_s, u"content_view_bg"_s, u"text_view_bg"_s}, background(view)},
        {{u"theme_text_color"_s}, text(view)},
        {{u"theme_selected_bg_color"_s}, background(selection, State::Highlighted)},
        {{u"theme_selected_fg_color"_s}, text(selection, State::Highlighted)},
        {{u"borders"_s}, border(frame)},
        {{u"unfocused_borders"_s}, border(frame, true)},
        {{u"insensitive_fg_color"_s}, text(label, State::Disabled)},
        {{u"insensitive_bg_color"_s}, background(button, State::Disabled)},
        {{u"insensitive_base_color"_s}, background(view, State::Disabled)},
        {{u"theme_unfocused_bg_color"_s}, background(window, {}, true)},
        {{u"theme_unfocused_fg_color"_s}, text(window, {}, true)},
        {{u"theme_unfocused_base_color"_s}, background(view, {}, true)},
        {{u"theme_unfocused_text_color"_s}, text(view, {}, true)},
        {{u"theme_unfocused_selected_bg_color"_s}, background(selection, State::Highlighted, true)},
        {{u"theme_unfocused_selected_fg_color"_s}, text(selection, State::Highlighted, true)},
        {{u"success_color"_s}, border(message(u"positive"_s))},
        {{u"warning_color"_s}, border(message(u"warning"_s))},
        {{u"error_color"_s}, border(message(u"error"_s))},
        {{u"link_color"_s}, scheme(u"link"_s)},
        {{u"link_visited_color"_s}, scheme(u"visited"_s)},
    };

    QString result;
    for (const auto &entry : colors) {
        const auto expression = toColorExpression(entry.second);
        if (!expression) {
            qCWarning(UNION_GTK) << "No value for named color" << entry.first.first();
            continue;
        }
        for (const auto &name : entry.first) {
            result += u"@define-color %1 %2;\n"_s.arg(name, expression.value());
        }
    }
    return result;
}

QHash<QString, QString> ThemeGenerator::metrics(ThemeAssets &assets) const
{
    struct Metric {
        QString name;
        ElementPath path;
        Features features;
        QString property;
        Union::Element::States states = {};
    };

    const ElementPath button{{u"Button"_s}};
    auto heading = [](const QString &level) {
        return ElementPath{{u"Heading"_s, {u"primary"_s, level}}};
    };
    auto arrow = [](const QString &direction) {
        return ElementPath{{u"IndicatorArrow"_s + direction}};
    };

    const QList<Metric> metrics = {
        {u"radius"_s, button, Feature::Corners, u"border-radius"_s},
        {u"round-button-radius"_s, {{u"RoundButton"_s}}, Feature::Corners, u"border-radius"_s},
        {u"border-width"_s, {{u"Frame"_s}}, Feature::Border, u"border-width"_s},
        {u"focus-width"_s, {{u"TextField"_s}}, Feature::Outline, u"outline-width"_s, State::VisualFocus},
        {u"icon-size"_s, {{u"SmallIconSize"_s}}, Feature::MinWidth, u"min-width"_s},
        {u"icon-size-large"_s, {{u"LargeIconSize"_s}}, Feature::MinWidth, u"min-width"_s},
        {u"spacing"_s, button, Feature::Spacing, u"border-spacing"_s},
        {u"spacing-medium"_s, {{u"DialogButtonBox"_s}}, Feature::Spacing, u"border-spacing"_s},
        {u"spacing-large"_s, {{u"Pane"_s}}, Feature::Padding, u"padding"_s},
        {u"indicator-spacing"_s, {{u"CheckDelegate"_s}}, Feature::Spacing, u"border-spacing"_s},
        {u"scrollbar-size"_s, {{u"ScrollBar"_s, {u"vertical"_s}}}, Feature::MinWidth, u"min-width"_s},
        {u"progress-thickness"_s, {{u"ProgressBar"_s, {u"horizontal"_s}}}, Feature::MinHeight, u"min-height"_s},
        {u"heading-1-size"_s, heading(u"level-1"_s), Feature::Font, u"font-size"_s},
        {u"heading-2-size"_s, heading(u"level-2"_s), Feature::Font, u"font-size"_s},
        {u"heading-3-size"_s, heading(u"level-3"_s), Feature::Font, u"font-size"_s},
        {u"heading-4-size"_s, heading(u"level-4"_s), Feature::Font, u"font-size"_s},
        {u"heading-weight"_s, heading(u"level-1"_s), Feature::Font, u"font-weight"_s},
        {u"dim-opacity"_s, {{u"Heading"_s, {u"secondary"_s}}}, Feature::Opacity, u"opacity"_s},
        {u"busy-icon"_s, {{u"BusyIndicator"_s}}, Feature::IconSource, u"-gtk-icon-source"_s},
        {u"arrow-up"_s, arrow(u"Up"_s), Feature::IconSource, u"-gtk-icon-source"_s},
        {u"arrow-down"_s, arrow(u"Down"_s), Feature::IconSource, u"-gtk-icon-source"_s},
        {u"arrow-left"_s, arrow(u"Left"_s), Feature::IconSource, u"-gtk-icon-source"_s},
        {u"arrow-right"_s, arrow(u"Right"_s), Feature::IconSource, u"-gtk-icon-source"_s},
    };

    QHash<QString, QString> result;
    for (const auto &metric : metrics) {
        const auto properties = queryVariant(metric.path, -1, StateVariant{.states = metric.states});
        auto value = translate(properties.get(), {.features = metric.features, .borderMode = BorderMode::Real, .assets = &assets}).value(metric.property);

        // Make font sizes relative to the app font, with the same pt to px
        // conversion as the CSS input
        if (value && metric.property == u"font-size") {
            const auto length = fromPx(value.value());
            const auto fontSize = QGuiApplication::font().pointSizeF() * 1.333;
            value = length ? std::optional(QString::number(std::round(length.value() / fontSize * 100.0)) + u'%') : std::nullopt;
        }

        if (!value) {
            qCDebug(UNION_GTK) << "No value for metric" << metric.name << "in" << describePath(metric.path);
        }
        result.insert(metric.name, value.value_or(QString{}));
    }
    return result;
}

QString ThemeGenerator::rules(GtkVersion version, ThemeAssets &assets) const
{
    QString result = u"/*\n * Rules generated from the Union style\n */\n\n"_s;

    QHash<QString, QList<EmittedRule>> emitted;
    int order = 0;

    const auto specs = ruleSpecs(version);
    for (const auto &spec : specs) {
        TranslateOptions options{
            .features = spec.features,
            .borderMode = spec.borderMode,
            .assets = &assets,
            .version = version,
        };

        // Backdrop has the same specificity as the states, so put it first and let
        // them win
        auto variants = spec.variants;
        std::stable_partition(variants.begin(), variants.end(), [](const StateVariant &variant) {
            return variant.pseudo.isEmpty();
        });
        if (!variants.isEmpty()) {
            std::stable_partition(variants.begin() + 1, variants.end(), [](const StateVariant &variant) {
                return variant.backdrop;
            });
        }

        const auto baseSpecificity = selectorSpecificity(spec.selector);
        const auto translatedProperties = featureProperties(spec.features);

        QString block;
        QList<EmittedRule> own;

        for (const auto &variant : std::as_const(variants)) {
            const auto unionAnchor = variant.unionAnchor >= 0 ? variant.unionAnchor : spec.stateAnchor;
            const auto gtkMarker = variant.gtkMarker >= 0 ? variant.gtkMarker : spec.gtkMarker;

            auto properties = queryVariant(spec.path, unionAnchor, variant);
            auto declarations = translate(properties.get(), options);
            if (spec.adjust) {
                spec.adjust(declarations, *this, variant);
            }

            const auto pseudo = pseudoForVersion(variant.pseudo, version);
            const auto tokens = pseudoTokens(pseudo, gtkMarker);
            const auto specificity = baseSpecificity + int(tokens.size()) * 100;

            // Skip anything the less specific rules (ours or the parents') already
            // set
            QList<const EmittedRule *> applicable;
            for (const auto &parent : spec.parents) {
                for (const auto &rule : std::as_const(emitted[parent])) {
                    if (tokens.contains(rule.tokens)) {
                        applicable.append(&rule);
                    }
                }
            }
            for (const auto &rule : std::as_const(own)) {
                if (rule.tokens != tokens && tokens.contains(rule.tokens)) {
                    applicable.append(&rule);
                }
            }
            std::sort(applicable.begin(), applicable.end(), [](const EmittedRule *first, const EmittedRule *second) {
                return std::tie(first->specificity, first->order) < std::tie(second->specificity, second->order);
            });

            DeclarationList reference;
            for (const auto rule : std::as_const(applicable)) {
                if (std::tie(rule->specificity, rule->order) < std::tie(specificity, order)) {
                    reference.merge(rule->declarations);
                }
            }
            if (spec.partial) {
                for (const auto &entry : reference.entries()) {
                    if (!translatedProperties.contains(entry.property) && !declarations.contains(entry.property)) {
                        reference.remove(entry.property);
                    }
                }
            }

            const auto output = declarations.difference(reference);
            own.append(EmittedRule{.tokens = tokens, .specificity = specificity, .order = order++, .declarations = output});

            if (!output.isEmpty()) {
                block += applyPseudo(spec.selector, pseudo, gtkMarker) + u" {\n"_s + output.toString() + u"}\n\n"_s;
            }
        }

        emitted.insert(spec.selector, own);

        if (!block.isEmpty()) {
            result += u"/* %1 */\n\n"_s.arg(describePath(spec.path)) + block;
        }
    }

    return result;
}

GeneratedStylesheet ThemeGenerator::generate(GtkVersion version) const
{
    ThemeAssets assets;

    const auto metrics = this->metrics(assets);

    QString css = u"/* Generated from the Union style \"%1\", do not edit. */\n\n"_s.arg(m_style->id());
    css += palette() + u"\n"_s;
    css += substituteMetrics(readStylesheet(u"base.css"_s, version), metrics) + u"\n"_s;
    css += rules(version, assets);
    css += substituteMetrics(readStylesheet(u"overrides.css"_s, version), metrics);

    return GeneratedStylesheet{
        .css = css,
        .assets = assets.files(),
    };
}

bool ThemeGenerator::writeTheme(const QString &themeDirectory) const
{
    bool changed = false;
    for (const auto version : {GtkVersion::Gtk3, GtkVersion::Gtk4}) {
        changed |= writeStylesheet(themeDirectory + u"/gtk-%1.0"_s.arg(int(version)), generate(version));
    }
    return changed;
}
