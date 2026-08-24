// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

#pragma once

#include "qtwidgets_logging.h"
#include <Element.h>
#include <properties/SizePropertyGroup.h>
#include <properties/StylePropertyGroup.h>

#include <QMargins>
#include <QPainterPath>

class QStyleOption;

struct LayoutItem {
    QString elementName;
    int order;
    Union::Properties::Alignment horizontalAlignment;
    Union::Properties::Alignment verticalAlignment;
    QRectF rect;
};

const char property_union_member_list[] = "_union_member_list";

/*!
 * \brief Translate the state from QStyleOption to Union::Element states.
 */
Union::Element::States statesFromOption(const QStyleOption *option);
QStringList hintsFromOption(const QStyleOption *option);
QVariantMap attributesFromOption(const QStyleOption *option);

Qt::Alignment toQtAlignment(Union::Properties::AlignmentPropertyGroup *alignmentGroup);
Qt::TextElideMode toQtElideMode(Union::Properties::TextElide elideMode);
Qt::TextFlag toQtWrapMode(Union::Properties::TextWrapMode wrapMode);

/*!
 * \brief Returns the background rectangle of an option, but removes its insets according to the properties first.
 */
QRectF backgroundRectangle(const QStyleOption *option, const Union::Properties::StylePropertyGroup *properties);

/*!
 * \brief Prepares elements for a widget. Sometimes we cannot decipher the specific item from widget alone, such as itemviews.
 * In those cases you may need to manually choose a target hierarchy, such as {"ItemViewItem"}
 */
Union::ElementList prepareElements(const QStyleOption *opt, const QWidget *widget = nullptr, QStringList targetHierarchy = {});

/*!
 * \brief Queries the properties from list of elements. The properties match to the last element in the list,
 * inheriting anything it needs from its parents.
 */
Union::Properties::StylePropertyGroup *queryProperties(const Union::ElementList &elements);

/*!
 * \brief Matches the widget name/class to a matching CSS element name, and sets up
 * property "_union_member_list" to the widget. This can be used to get the whole parental
 * hierarchy of the widget
 */
QStringList widgetToElementHierarchy(const QWidget *widget);

/*!
 * \brief Layouts list of elements, then returns a map of LayoutItems that contain information such as rectangles.
 * It will take list of elements, such as Button and any potential parents it has.
 * Then it uses the subElements stringlist to construct a layout: For example button is the container, then
 * Text, Icon and Indicator are the items to be layouted within the button container.
 * Currently only one container is used, which is the rectangle of the parent of the subElements.
 */
QMap<QString, LayoutItem> layoutMap(const Union::ElementList &elements, const QStyleOption *opt, const QStringList &subElements);

/*!
 * \brief Helper function to get text from any QStyleOption that has a field with QString (text/title)
 */
QString textFromOption(const QStyleOption *opt);

/*!
 * \brief Returns flags for text drawing purposes. If using LayoutMap for alignment, it's best to
 * skip the Qt alignment by setting skipAlign to true.
 */
int textFlagsFromProperties(Union::Properties::StylePropertyGroup *properties, bool skipAlign);

/*!
 * \brief Centers a rectangle depending on width and height. Copied from Breeze.
 */
QRectF centerRect(const QRectF &rect, int width, int height);

/*!
 * \brief Helper function for getting correct icon from properties.
 */

QIcon queryIcon(const QStyleOption *option, const QWidget *widget, const QString &defaultIconName, const QStringList &targetHierarchy = {});

/*!
 * \brief Helper function to query the size of the element from properties.
 */
QSizeF querySize(const QStyleOption *option, const QWidget *widget, const QStringList &targetHierarchy = {});

/*!
 * \brief Tries to match styleOption type to a potential element.
 * Used when widget is null.
 */
QString styleOptionToElementName(const QStyleOption *option);
