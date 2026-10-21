// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "ThemeAssets.h"

#include <QFile>
#include <QStack>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "gtk_logging.h"

using namespace Qt::StringLiterals;
using namespace Union::Gtk;

static const QStringList ShapeElements = {
    u"path"_s,
    u"rect"_s,
    u"circle"_s,
    u"ellipse"_s,
    u"line"_s,
    u"polyline"_s,
    u"polygon"_s,
};

struct Paint {
    QString fill;
    QString stroke;
};

static bool isVisible(const QString &paint)
{
    return !paint.isEmpty() && paint != u"none" && paint != u"transparent";
}

// the style attribute wins over the presentation attribute
static std::optional<QString> paintProperty(const QXmlStreamAttributes &attributes, const QString &property)
{
    const auto declarations = attributes.value(u"style").toString().split(u';', Qt::SkipEmptyParts);
    for (const auto &declaration : declarations) {
        if (declaration.section(u':', 0, 0).trimmed() == property) {
            return declaration.section(u':', 1).trimmed();
        }
    }
    if (attributes.hasAttribute(property)) {
        return attributes.value(property).toString().trimmed();
    }
    return std::nullopt;
}

// GTK picks the colour of each shape by its classes
static std::optional<QByteArray> toSymbolicSvg(const QByteArray &svg)
{
    QXmlStreamReader reader(svg);
    reader.setNamespaceProcessing(false);

    QByteArray output;
    QXmlStreamWriter writer(&output);

    // fill & stroke are inherited, by default shapes are filled with no stroke
    QStack<Paint> inherited;
    inherited.push(Paint{u"black"_s, u"none"_s});

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            auto attributes = reader.attributes();
            const Paint paint{
                .fill = paintProperty(attributes, u"fill"_s).value_or(inherited.top().fill),
                .stroke = paintProperty(attributes, u"stroke"_s).value_or(inherited.top().stroke),
            };
            inherited.push(paint);

            const auto name = reader.qualifiedName().toString();
            if (ShapeElements.contains(name)) {
                const bool filled = isVisible(paint.fill);
                auto classes = attributes.value(u"class").toString().split(u' ', Qt::SkipEmptyParts);
                classes.append(filled ? u"foreground-fill"_s : u"transparent-fill"_s);
                if (isVisible(paint.stroke)) {
                    classes.append(u"foreground-stroke"_s);
                    // GTK 3 fills the shape too unless it has "stroke"
                    if (!filled) {
                        classes.append(u"stroke"_s);
                    }
                }
                classes.removeDuplicates();

                attributes.removeIf([](const QXmlStreamAttribute &attribute) {
                    return attribute.qualifiedName() == u"class";
                });
                attributes.append(u"class"_s, classes.join(u' '));
            }

            writer.writeStartElement(name);
            writer.writeAttributes(attributes);
        } else if (reader.isEndElement()) {
            writer.writeEndElement();
            inherited.pop();
        } else if (!reader.isStartDocument()) {
            writer.writeCurrentToken(reader);
        }
    }

    if (reader.hasError()) {
        return std::nullopt;
    }

    return output;
}

static std::optional<QByteArray> readImage(const std::filesystem::path &source)
{
    QFile file(source);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(UNION_GTK) << "Could not read image" << source.string() << file.errorString();
        return std::nullopt;
    }
    return file.readAll();
}

std::optional<QString> ThemeAssets::addImage(const std::filesystem::path &source)
{
    const auto contents = readImage(source);
    if (!contents) {
        return std::nullopt;
    }

    const auto name = u"assets/"_s + QString::fromStdString(source.filename().string());
    m_files[name] = contents.value();
    return name;
}

std::optional<QString> ThemeAssets::addSymbolicImage(const std::filesystem::path &source)
{
    const auto contents = readImage(source);
    if (!contents) {
        return std::nullopt;
    }

    const auto symbolic = toSymbolicSvg(contents.value());
    if (!symbolic) {
        qCWarning(UNION_GTK) << "Could not read" << source.string() << "as an SVG to recolour it";
        return std::nullopt;
    }

    // GTK only recolours files named *-symbolic.svg
    const auto name = u"assets/%1-symbolic.svg"_s.arg(QString::fromStdString(source.stem().string()));
    m_files[name] = symbolic.value();
    return name;
}

const std::map<QString, QByteArray> &ThemeAssets::files() const
{
    return m_files;
}
