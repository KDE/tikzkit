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

#include "BezierCurve.h"

#include <cmath>

BezierCurve::BezierCurve()
{
}

BezierCurve::BezierCurve(const QPointF& p1,
                         const QPointF& p2,
                         const QPointF& c1,
                         const QPointF& c2)
    : m_p1(p1)
    , m_p2(p2)
    , m_c1(c1)
    , m_c2(c2)
{
}

BezierCurve::~BezierCurve()
{
}

void BezierCurve::setP1(const QPointF& p1)
{
    m_p1 = p1;
}

void BezierCurve::setP2(const QPointF& p2)
{
    m_p2 = p2;
}

void BezierCurve::setC1(const QPointF& c1)
{
    m_c1 = c1;
}

void BezierCurve::setC2(const QPointF& c2)
{
    m_c2 = c2;
}

QPointF BezierCurve::p1() const
{
    return m_p1;
}

QPointF BezierCurve::p2() const
{
    return m_p2;
}

QPointF BezierCurve::c1() const
{
    return m_c1;
}

QPointF BezierCurve::c2() const
{
    return m_c2;
}

inline static qreal interpolateBezierCurve(qreal t,
                                           qreal p1,
                                           qreal p2,
                                           qreal c1,
                                           qreal c2)
{
    return (-p1 + 3*c1 -3*c2 + p2) * t*t*t + (3*p1 - 6*c1 + 3*c2) * t*t + (-3*p1 + 3*c1) * t + p1;
}

inline static QPointF interpolateBezierCurve(qreal t,
                                             const QPointF& p1,
                                             const QPointF& p2,
                                             const QPointF& c1,
                                             const QPointF& c2)
{
    return (-p1 + 3*c1 -3*c2 + p2) * t*t*t + (3*p1 - 6*c1 + 3*c2) * t*t + (-3*p1 + 3*c1) * t + p1;
}

QPointF BezierCurve::pointAtPercent(qreal t) const
{
    Q_ASSERT(t >= 0.0);
    Q_ASSERT(t <= 1.0);

    return interpolateBezierCurve(t, m_p1, m_p2, m_c1, m_c2);
}

QPainterPath BezierCurve::toPath(int samplePoints) const
{
    QPainterPath path;
    path.moveTo(m_p1);
    for (int i = 1; i <= samplePoints; ++i) {
        path.lineTo(pointAtPercent(static_cast<qreal>(i) / samplePoints));
    }
    return path;
}

QPainterPath BezierCurve::toPath(qreal t1, qreal t2, int samplePoints) const
{
    Q_ASSERT(t1 >= 0);
    Q_ASSERT(t1 <= 1);
    Q_ASSERT(t2 >= 0);
    Q_ASSERT(t2 <= 1);

    QPainterPath path;
    path.moveTo(pointAtPercent(t1));

    const qreal tau = (t2 - t1) / samplePoints;
    for (int i = 1; i <= samplePoints; ++i) {
        path.lineTo(pointAtPercent(t1 + i * tau));
    }
    return path;
}

BezierCurve BezierCurve::subPath(qreal t1, qreal t2) const
{
    Q_ASSERT(t1 >= 0.0);
    Q_ASSERT(t1 < 1.0);
    Q_ASSERT(t2 > 0.0);
    Q_ASSERT(t2 <= 1.0);
    Q_ASSERT(t1 < t2);

    // copy this curve
    BezierCurve c(*this);

    // adapt start, if requried
    if (t1 > 0.0) {
        const QPointF x0 = c.m_p1 + t1 * (c.m_c1 - c.m_p1);
        const QPointF x1 = c.m_c1 + t1 * (c.m_c2 - c.m_c1);
        const QPointF x2 = c.m_c2 + t1 * (c.m_p2 - c.m_c2);

        const QPointF x3 = x0 + t1 * (x1 - x0);
        const QPointF x4 = x1 + t1 * (x2 - x1);

        c.m_p1 = x3 + t1 * (x4 - x3);
        c.m_c1 = x4;
        c.m_c2 = x2;
    }

    // adapt end, if requried
    if (t2 < 1.0) {
        const QPointF x0 = c.m_p1 + t2 * (c.m_c1 - c.m_p1);
        const QPointF x1 = c.m_c1 + t2 * (c.m_c2 - c.m_c1);
        const QPointF x2 = c.m_c2 + t2 * (c.m_p2 - c.m_c2);

        const QPointF x3 = x0 + t2 * (x1 - x0);
        const QPointF x4 = x1 + t2 * (x2 - x1);

        c.m_c1 = x0;
        c.m_c2 = x3;
        c.m_p2 = x3 + t2 * (x4 - x3);
    }

    return c;
}

qreal BezierCurve::intersect(const QPainterPath & path)
{
    const int samplePoints = 50;

    const qreal tau = 1.0 / samplePoints;

    for (int a = 0; a < samplePoints; ++a) {
        QLineF line1(pointAtPercent(a * tau),
                     pointAtPercent((a + 1) * tau));
        for (int b = 0; b < samplePoints; ++b) {
            QLineF line2(path.pointAtPercent(b * tau),
                         path.pointAtPercent((b + 1) * tau));

            QPointF crossing;
            if (QLineF::BoundedIntersection == line1.intersects(line2, &crossing)) {
                return a * tau;
            }
        }
    }

    return 0.0;
}

// kate: indent-width 4; replace-tabs on;
