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

#include "CircleShape.h"
#include "NodeItem.h"

#include <tikz/core/Style.h>
#include <tikz/core/tikz.h>

#include <QDebug>
#include <cmath>

namespace tikz {
namespace ui {

class CircleShapePrivate
{
    public:
};

CircleShape::CircleShape(NodeItem * node)
    : AbstractShape(node)
    , d(new CircleShapePrivate())
{
}

CircleShape::~CircleShape()
{
    delete d;
}

tikz::Shape CircleShape::type() const
{
    return tikz::Shape::ShapeCircle;
}

void CircleShape::adjustShapeRect(const QRectF & textRect, QRectF & shapeRect) const
{
    // calculate radius of textRect
    const qreal w = textRect.width() / 2.0 + node()->style()->innerSep().toPoint();
    const qreal h = textRect.height() / 2.0 + node()->style()->innerSep().toPoint();
    const qreal r = std::sqrt(w * w + h * h);

    // make sure the circle around textRect is contained in shapeRect
    if (2.0 * r > shapeRect.width()) {
        shapeRect.setWidth(2.0 * r);
    }
    if (2.0 * r > shapeRect.height()) {
        shapeRect.setHeight(2.0 * r);
    }
}

QPainterPath CircleShape::shape() const
{
    const qreal r = qMax(node()->shapeRect().width(),
                         node()->shapeRect().height()) / 2.0;

    QPainterPath path;
    path.addEllipse(QPointF(0, 0), r, r);
    return path;
}

QPainterPath CircleShape::outline() const
{
    const qreal lw = node()->style()->penWidth().toPoint() / 2;
    const qreal r = qMax(node()->shapeRect().width(),
                         node()->shapeRect().height()) / 2.0 + lw;

    QPainterPath path;
    path.addEllipse(QPointF(0, 0), r, r);
    return path;
}

QStringList CircleShape::supportedAnchors() const
{
    // by default, just return NoAnchor
    const QStringList anchors = QStringList()
        << QString()
        << QStringLiteral("center")
        << QStringLiteral("north")
        << QStringLiteral("north east")
        << QStringLiteral("east")
        << QStringLiteral("south east")
        << QStringLiteral("south")
        << QStringLiteral("south west")
        << QStringLiteral("west")
        << QStringLiteral("north west");
    return anchors;
}

QPointF CircleShape::anchorPos(const QString & anchor) const
{
    if (anchor.isEmpty()) {
        return QPointF(0, 0);
    }

    const qreal r = node()->style()->outerSep().toPoint() +
                    qMax(node()->shapeRect().width(),
                         node()->shapeRect().height()) / 2.0;

    if (anchor == QStringLiteral("center")) {
        return QPointF(0, 0);
    } else if (anchor == QStringLiteral("north")) {
        return QPointF(0, r);
    } else if (anchor == QStringLiteral("north east")) {
        return QPointF(r, r) * 0.70710678;
    } else if (anchor == QStringLiteral("east")) {
        return QPointF(r, 0);
    } else if (anchor == QStringLiteral("south east")) {
        return QPointF(r, -r) * 0.70710678;
    } else if (anchor == QStringLiteral("south")) {
        return QPointF(0, -r);
    } else if (anchor == QStringLiteral("south west")) {
        return QPointF(-r, -r) * 0.70710678;
    } else if (anchor == QStringLiteral("west")) {
        return QPointF(-r, 0);
    } else if (anchor == QStringLiteral("north west")) {
        return QPointF(-r, r) * 0.70710678;
    }

    tikz::warn("The shape 'circle' does not support anchor '" + anchor + "'.");

    return QPointF(0, 0);
}

QPointF CircleShape::contactPoint(const QString & anchor, qreal rad) const
{
    if (! anchor.isEmpty()) {
        return anchorPos(anchor);
    }

    const qreal r = node()->style()->outerSep().toPoint() +
                    qMax(node()->shapeRect().width(),
                         node()->shapeRect().height()) / 2.0;

    return QPointF(r * std::cos(rad), r * std::sin(rad));
}

}
}

// kate: indent-width 4; replace-tabs on;
