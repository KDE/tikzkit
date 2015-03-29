/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "Style.h"

#include "Document.h"
#include "VisitorHelpers.h"

#include <QSet>

namespace tikz {
namespace core {

// NOTE: these strings have to match the Q_PROPERTY strings, otherise
//       serialization will not work correctly.
static constexpr char s_penStyle[] = "penStyle";
static constexpr char s_lineWidth[] = "lineWidth";
static constexpr char s_doubleLine[] = "doubleLine";
static constexpr char s_innerLineWidth[] = "innerLineWidth";
static constexpr char s_penOpacity[] = "penOpacity";
static constexpr char s_fillOpacity[] = "fillOpacity";
static constexpr char s_penColor[] = "penColor";
static constexpr char s_innerLineColor[] = "innerLineColor";
static constexpr char s_fillColor[] = "fillColor";
static constexpr char s_rotation[] = "rotation";

/**
 * Private data and helper functions of class Style.
 */
class StylePrivate
{
public:
    // uniq id, or -1
    qint64 id = -1;

    // parent / child hierarchy
    Style * parent = nullptr;
    QVector<Style *> children;

    // config reference counter
    int refCounter = 0;

    // list of set properties
    QSet<QString> properties;

    // line style
    PenStyle penStyle = SolidLine;

    // line width
    tikz::Value lineWidth = tikz::Value::semiThick();

    // double lines
    bool doubleLine = false;
    tikz::Value innerLineWidth = tikz::Value::semiThick();

    // colors
    QColor penColor = Qt::black;
    QColor innerLineColor = Qt::white;
    QColor fillColor = Qt::transparent;

    qreal penOpacity = 1.0;
    qreal fillOpacity = 1.0;

    qreal rotation = 0.0;
};

Style::Style()
    : d(new StylePrivate())
{
}

Style::Style(qint64 id, Document* tikzDocument)
    : QObject(tikzDocument)
    , d(new StylePrivate())
{
    d->id = id;
}

Style::Style(const QJsonObject & json, Document* tikzDocument)
    : QObject(tikzDocument)
    , d(new StylePrivate())
{
    using namespace internal;

    beginConfig();

    if (json.contains("parent-style-id")) {
        const qint64 styleId = json["parent-style-id"].toString().toLongLong();
        d->parent = tikzDocument->style()->findStyle(styleId);
    }

    if (json.contains("style-id")) {
        d->id = json["style-id"].toString().toLongLong();
    }

    if (json.contains("pen-color")) {
        setPenColor(json["pen-color"].toString());
    }

    if (json.contains("fill-color")) {
        setFillColor(json["fill-color"].toString());
    }

    if (json.contains("pen-opacity")) {
        setPenOpacity(json["pen-opacity"].toDouble());
    }

    if (json.contains("fill-opacity")) {
        setFillOpacity(json["fill-opacity"].toDouble());
    }

    if (json.contains("pen-style")) {
        setPenStyle(penStyleFromString(json["pen-style"].toString()));
    }

    if (json.contains("line-width")) {
        setLineWidth(Value::fromString(json["line-width"].toString()));
    }
    // FIXME line type

    if (json.contains("double-line")) {
        setDoubleLine(json["double-line"].toBool());

        if (json.contains("inner-line-width")) {
            setInnerLineWidth(Value::fromString(json["inner-line-width"].toString()));
        }

        // FIXME line type

        if (json.contains("double-line-color")) {
            setInnerLineColor(json["double-line-color"].toString());
        }
    }

    if (json.contains("rotation")) {
        setRotation(json["rotation"].toDouble());
    }

    endConfig();
}

Style::~Style()
{
    // unregister all child styles
    foreach (Style * style, d->children) {
        style->setParentStyle(d->parent);
    }
    Q_ASSERT(d->children.size() == 0);

    // avoid unnecessary propagation of the changed() signal
    disconnect(this, SIGNAL(changed()), 0, 0);

    // now: remove from parent's child list, if needed
    setParentStyle(0);
}

qint64 Style::id() const
{
    return d->id;
}

void Style::setStyle(const Style& other)
{
    if (this == &other) {
        return;
    }

    // start configuration
    beginConfig();

    // backup properties not to copy
    const qint64 id = d->id;
    Style * parent = d->parent;
    const int refCounter = d->refCounter;

    // perform copy of everything
    *d = *other.d;

    // restore persistend properties
    d->id = id;
    d->parent = parent;
    d->refCounter = refCounter;

    // end configuration
    endConfig();
}

QJsonObject Style::toJson() const
{
    using namespace tikz::core::internal;

    QJsonObject json;

    if (parentStyle()) {
        json["parent-style-id"] = QString::number(parentStyle()->id());
    }

    json["style-id"] = QString::number(id());

    if (penColorSet()) {
        json["pen-color"] = penColor().name();
    }

    if (fillColorSet()) {
        json["fill-color"] = fillColor().name();
    }

    if (penOpacitySet()) {
        json["pen-opacity"] = penOpacity();
    }

    if (fillOpacitySet()) {
        json["fill-opacity"] = fillOpacity();
    }

    if (penStyleSet()) {
        json["pen-style"] = penStyleToString(penStyle());
    }

    if (lineWidthSet()) {
        json["line-width"] = lineWidth().toString();
    }

    // FIXME line type

    if (doubleLineSet()) {
        json["double-line"] = "true";

        if (innerLineWidthSet()) {
            json["inner-line-width"] = innerLineWidth().toString();
        }

        // FIXME line width

        if (innerLineColorSet()) {
            json["double-line-color"] = innerLineColor().name();
        }
    }

    if (rotationSet()) {
        json["rotation"] = rotation();
    }

    return json;
}

Style *Style::parentStyle() const
{
    return d->parent;
}

void Style::setParentStyle(Style *parent)
{
    Q_ASSERT(parent != this);

    if (d->parent != parent) {
        beginConfig();
        if (d->parent) {
            // disconnect all signals (e.g. changed())
            disconnect(d->parent, 0, this, 0);

            // remove this in old parent's children list
            Q_ASSERT(d->parent->d->children.contains(this));
            d->parent->d->children.remove(d->parent->d->children.indexOf(this));
        }
        d->parent = parent;
        if (d->parent) {
            // forward changed() signal
            connect(d->parent, SIGNAL(changed()), this, SIGNAL(changed()));

            // interst us into the new parent's children list
            Q_ASSERT(! d->parent->d->children.contains(this));
            d->parent->d->children.append(this);
        }
        endConfig();
    }
}

bool Style::hasChildStyles() const
{
    return d->children.size() > 0;
}

Style * Style::findStyle(qint64 styleId) const
{
    if (d->id == styleId) {
        return const_cast<Style*>(this);
    }

    for (const auto style : d->children) {
        auto ptr = style->findStyle(styleId);
        if (ptr) {
            return ptr;
        }
    }

    return nullptr;
}

void Style::beginConfig()
{
    Q_ASSERT(d->refCounter >= 0);
    ++d->refCounter;
}

void Style::endConfig()
{
    Q_ASSERT(d->refCounter > 0);

    --d->refCounter;
    if (d->refCounter == 0) {
        emit changed();
    }
}

void Style::addProperty(const QString & property)
{
    d->properties.insert(property);
}

void Style::removeProperty(const QString & property)
{
    d->properties.remove(property);
}

bool Style::propertySet(const QString & property) const
{
    return d->properties.contains(property);
}

PenStyle Style::penStyle() const
{
    if (propertySet(s_penStyle)) {
        return d->penStyle;
    }

    if (parentStyle()) {
        return parentStyle()->penStyle();
    }

    return SolidLine;
}

bool Style::penStyleSet() const
{
    return propertySet(s_penStyle);
}

void Style::setPenStyle(tikz::PenStyle style)
{
    if (!propertySet(s_penStyle) || d->penStyle != style) {
        beginConfig();
        addProperty(s_penStyle);
        d->penStyle = style;
        endConfig();
    }
}

void Style::unsetPenStyle()
{
    if (propertySet(s_penStyle)) {
        beginConfig();
        removeProperty(s_penStyle);
        d->penStyle = SolidLine;
        endConfig();
    }
}

tikz::Value Style::penWidth() const
{
    if (!doubleLine()) {
        return lineWidth();
    } else {
        const Value width = lineWidth();
        const Value innerWidth = innerLineWidth();
        return 2.0 * width + innerWidth;
    }
}

bool Style::lineWidthSet() const
{
    return propertySet(s_lineWidth);
}

void Style::setLineWidth(const tikz::Value & width)
{
    if (!propertySet(s_lineWidth)
        || d->lineWidth != width
    ) {
        beginConfig();
        addProperty(s_lineWidth);
        d->lineWidth = width;
        endConfig();
    }
}

tikz::Value Style::lineWidth() const
{
    if (propertySet(s_lineWidth)) {
        return d->lineWidth;
    }

    if (parentStyle()) {
        return parentStyle()->lineWidth();
    }

    return tikz::Value::semiThick();
}

void Style::unsetLineWidth()
{
    if (propertySet(s_lineWidth)) {
        beginConfig();
        removeProperty(s_lineWidth);
        d->lineWidth = tikz::Value::semiThick();
        endConfig();
    }
}

bool Style::doubleLine() const
{
    if (propertySet(s_doubleLine)) {
        return d->doubleLine;
    }

    if (parentStyle()) {
        return parentStyle()->doubleLine();
    }

    return false;
}

bool Style::doubleLineSet() const
{
    return propertySet(s_doubleLine);
}

void Style::setDoubleLine(bool enabled)
{
    if (!propertySet(s_doubleLine) || d->doubleLine != enabled) {
        beginConfig();
        addProperty(s_doubleLine);
        d->doubleLine = enabled;
        endConfig();
    }
}

void Style::unsetDoubleLine()
{
    if (propertySet(s_doubleLine)) {
        beginConfig();
        removeProperty(s_doubleLine);
        d->doubleLine = false;
        endConfig();
    }
}

tikz::Value Style::innerLineWidth() const
{
    if (doubleLine()) {
        if (propertySet(s_innerLineWidth)) {
            return d->innerLineWidth;
        }

        if (parentStyle() && parentStyle()->doubleLine()) {
            return parentStyle()->innerLineWidth();
        }

        return tikz::Value::semiThick();
    }

    return tikz::Value(0.0);
}

bool Style::innerLineWidthSet() const
{
    return propertySet(s_innerLineWidth);
}

void Style::setInnerLineWidth(const tikz::Value & width)
{
    if (!propertySet(s_innerLineWidth) || d->innerLineWidth != width) {
        beginConfig();
        addProperty(s_innerLineWidth);
        d->innerLineWidth = width;
        endConfig();
    }
}

void Style::unsetInnerLineWidth()
{
    if (propertySet(s_innerLineWidth)) {
        beginConfig();
        removeProperty(s_innerLineWidth);
        d->innerLineWidth = tikz::Value::semiThick();
        endConfig();
    }
}

qreal Style::penOpacity() const
{
    if (propertySet(s_penOpacity)) {
        return d->penOpacity;
    }

    if (parentStyle()) {
        return parentStyle()->penOpacity();
    }

    return 1.0;
}

void Style::setPenOpacity(qreal opacity)
{
    if (!propertySet(s_penOpacity) || d->penOpacity != opacity) {
        beginConfig();
        addProperty(s_penOpacity);
        d->penOpacity = opacity;
        endConfig();
    }
}

bool Style::penOpacitySet() const
{
    return propertySet(s_penOpacity);
}

void Style::unsetPenOpacity()
{
    if (propertySet(s_penOpacity)) {
        beginConfig();
        removeProperty(s_penOpacity);
        d->penOpacity = 1.0;
        endConfig();
    }
}

qreal Style::fillOpacity() const
{
    if (propertySet(s_fillOpacity)) {
        return d->fillOpacity;
    }

    if (parentStyle()) {
        return parentStyle()->fillOpacity();
    }

    return 1.0;
}

bool Style::fillOpacitySet() const
{
    return propertySet(s_fillOpacity);
}

void Style::setFillOpacity(qreal opacity)
{
    if (!propertySet(s_fillOpacity) || d->fillOpacity != opacity) {
        beginConfig();
        addProperty(s_fillOpacity);
        d->fillOpacity = opacity;
        endConfig();
    }
}

void Style::unsetFillOpacity()
{
    if (propertySet(s_fillOpacity)) {
        beginConfig();
        removeProperty(s_fillOpacity);
        d->fillOpacity = 1.0;
        endConfig();
    }
}

QColor Style::penColor() const
{
    if (propertySet(s_penColor)) {
        return d->penColor;
    }

    if (parentStyle()) {
        return parentStyle()->penColor();
    }

    return Qt::black;
}

bool Style::penColorSet() const
{
    return propertySet(s_penColor);
}

QColor Style::innerLineColor() const
{
    if (propertySet(s_innerLineColor)) {
        return d->innerLineColor;
    }

    if (parentStyle()) {
        return parentStyle()->innerLineColor();
    }

    return Qt::white;
}

bool Style::innerLineColorSet() const
{
    return propertySet(s_innerLineColor);
}

QColor Style::fillColor() const
{
    if (propertySet(s_fillColor)) {
        return d->fillColor;
    }

    if (parentStyle()) {
        return parentStyle()->fillColor();
    }

    return Qt::transparent;
}

bool Style::fillColorSet() const
{
    return propertySet(s_fillColor);
}

void Style::setPenColor(const QColor & color)
{
    if (!propertySet(s_penColor) || d->penColor != color) {
        beginConfig();
        addProperty(s_penColor);
        d->penColor = color;
        endConfig();
    }
}

void Style::unsetPenColor()
{
    if (propertySet(s_penColor)) {
        beginConfig();
        removeProperty(s_penColor);
        d->penColor = Qt::black;
        endConfig();
    }
}

void Style::setInnerLineColor(const QColor & color)
{
    if (!propertySet(s_innerLineColor) || d->innerLineColor != color) {
        beginConfig();
        addProperty(s_innerLineColor);
        d->innerLineColor = color;
        endConfig();
    }
}

void Style::unsetInnerLineColor()
{
    if (propertySet(s_innerLineColor)) {
        beginConfig();
        removeProperty(s_innerLineColor);
        d->innerLineColor = Qt::white;
        endConfig();
    }
}

void Style::setFillColor(const QColor & color)
{
    if (!propertySet(s_fillColor) || d->fillColor != color) {
        beginConfig();
        addProperty(s_fillColor);
        d->fillColor = color;
        endConfig();
    }
}

void Style::unsetFillColor()
{
    if (propertySet(s_fillColor)) {
        beginConfig();
        removeProperty(s_fillColor);
        d->fillColor = Qt::transparent;
        endConfig();
    }
}

qreal Style::rotation() const
{
    if (propertySet(s_rotation)) {
        return d->rotation;
    }

    if (parentStyle()) {
        return parentStyle()->rotation();
    }

    return 0.0;
}

bool Style::rotationSet() const
{
    return propertySet(s_rotation);
}

void Style::setRotation(qreal angle)
{
    if (!propertySet(s_rotation) || d->rotation != angle) {
        beginConfig();
        addProperty(s_rotation);
        d->rotation = angle;
        endConfig();
    }
}

void Style::unsetRotation()
{
    if (propertySet(s_rotation)) {
        beginConfig();
        removeProperty(s_rotation);
        d->rotation = 0.0;
        endConfig();
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
