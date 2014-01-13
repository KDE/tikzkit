/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2014 Dominik Haumann <dhaumann@kde.org>
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

#ifndef TIKZ_UI_ELLIPSE_SHAPE_H
#define TIKZ_UI_ELLIPSE_SHAPE_H

#include "AbstractShape.h"

namespace tikz {
namespace ui {

class EllipseShapePrivate;

class EllipseShape : public AbstractShape
{
    public:
        /**
         * Default constructor
         */
        EllipseShape(TikzNode * node);

        /**
         * Default constructor
         */
        virtual ~EllipseShape();

    //
    // Shape properties
    //
    public:
        /**
         * Returns the type of this shape.
         */
        virtual tikz::Shape type() const;

        /**
         * Reimplemented such that the ellipse inside @p shapeRect completely
         * includes the @p textRect.
         */
        virtual void adjustShapeRect(const QRectF & textRect, QRectF & shapeRect) const;

        /**
         * Returns the painter path of this shape.
         * This path is also used to draw the shape.
         */
        virtual QPainterPath shape() const;

        /**
         * Returns a list of anchors the shape supports.
         */
        virtual QVector<tikz::Anchor> supportedAnchors() const;

        /**
         * Returns the position of @p anchor in local node coordinates.
         */
        virtual QPointF anchorPos(tikz::Anchor anchor) const;

        /**
         * Returns the contact point for @p anchor and angle @p rad.
         */
        virtual QPointF contactPoint(tikz::Anchor anchor, qreal rad) const;

    private:
        EllipseShapePrivate * const d;
};

}
}

#endif // TIKZ_UI_ELLIPSE_SHAPE_H

// kate: indent-width 4; replace-tabs on;
