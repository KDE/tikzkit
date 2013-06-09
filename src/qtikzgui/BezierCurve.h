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

#ifndef TIKZGUI_BEZIER_CURVE_H
#define TIKZGUI_BEZIER_CURVE_H

#include <QPointF>
#include <QPainterPath>

class BezierCurve
{
    public:
        /**
         * Default constructor
         */
        BezierCurve();

        /**
         * Default constructor
         */
        BezierCurve(const QPointF& p1,
                    const QPointF& p2,
                    const QPointF& c1,
                    const QPointF& c2);

        /**
         * Default constructor
         */
        virtual ~BezierCurve();

        void setP1(const QPointF& p1);
        void setP2(const QPointF& p2);
        void setC1(const QPointF& c1);
        void setC2(const QPointF& c2);

        QPointF p1() const;
        QPointF p2() const;
        QPointF c1() const;
        QPointF c2() const;

        QPointF pointAtPercent(qreal t) const;

        QPainterPath toPath(int samplePoints = 50) const;
        QPainterPath toPath(qreal t1, qreal t2, int samplePoints = 50) const;

        qreal intersect(const QPainterPath & path);

    private:
        QPointF m_p1;
        QPointF m_p2;
        QPointF m_c1;
        QPointF m_c2;
};

#endif // TIKZGUI_BEZIER_CURVE_H

// kate: indent-width 4; replace-tabs on;
