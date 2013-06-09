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

#include "PipeArrow.h"

#include <EdgeStyle.h>

#include <QPainter>

#include <cmath>

class PipeArrowPrivate
{
    public:
};

PipeArrow::PipeArrow(tikz::EdgeStyle * style)
    : AbstractArrow(style)
    , d(new PipeArrowPrivate())
{
}

PipeArrow::~PipeArrow()
{
    delete d;
}

tikz::Arrow PipeArrow::type() const
{
    return tikz::PipeArrow;
}

QString PipeArrow::name() const
{
    return QObject::tr("|");
}

qreal PipeArrow::leftExtend() const
{
    // see: pgfcorearrows.code.tex
    return -0.25 * style()->penWidth();
}

qreal PipeArrow::rightExtend() const
{
    // see: pgfcorearrows.code.tex
    return 0.75 * style()->penWidth();
}

void PipeArrow::draw(QPainter* painter) const
{
    // see: pgfcorearrows.code.tex
    QPainterPath p = path();
    painter->save();
    QPen pen = painter->pen();
    pen.setWidthF(style()->penWidth());
    pen.setColor(Qt::black); // TODO: style()->penColor());
    pen.setCapStyle(Qt::SquareCap);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(p);
    painter->restore();
}

QPainterPath PipeArrow::path() const
{
    // see: pgfcorearrows.code.tex
    const qreal lineWidth = style()->penWidth();
    const qreal dima = 2 * 0.03527 + 1.5 * lineWidth;
    QPainterPath path;
    path.moveTo(QPointF(0.25 * lineWidth, -dima));
    path.lineTo(QPointF(0.25 * lineWidth,  dima));

    return path;
}

// kate: indent-width 4; replace-tabs on;