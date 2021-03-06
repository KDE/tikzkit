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

#include "AbstractArrow.h"

#include <tikz/core/Style.h>

#include <QObject>
#include <QPainterPathStroker>

class AbstractArrowPrivate
{
    public:
        tikz::core::Style* style;
};

AbstractArrow::AbstractArrow(tikz::core::Style* style)
    : d(new AbstractArrowPrivate())
{
    d->style = style;
}

AbstractArrow::~AbstractArrow()
{
    delete d;
}

tikz::core::Style* AbstractArrow::style() const
{
    return d->style;
}

tikz::Arrow AbstractArrow::type() const
{
    return tikz::Arrow::NoArrow;
}

QString AbstractArrow::name() const
{
    return QObject::tr("none");
}

qreal AbstractArrow::leftExtend() const
{
    return 0.0;
}

qreal AbstractArrow::rightExtend() const
{
    return 0.0;
}

void AbstractArrow::draw(QPainter* painter) const
{
    Q_UNUSED(painter);
}

QPainterPath AbstractArrow::path() const
{
    return QPainterPath();
}

QPainterPath AbstractArrow::contour(qreal width) const
{
    QPainterPath arrowPath = path();

    // if path is empty, return immediately
    if (arrowPath.isEmpty()) {
        return QPainterPath();
    }

    QPainterPathStroker stroker;
    stroker.setJoinStyle(Qt::RoundJoin);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setWidth(width + style()->penWidth().toPoint());

    return stroker.createStroke(arrowPath);
}

#include "ToArrow.h"
#include "StealthArrow.h"
#include "LatexArrow.h"
#include "PipeArrow.h"
#include "StealthTickArrow.h"

AbstractArrow *createArrow(tikz::Arrow type, tikz::core::Style* style)
{
    switch (type) {
        case tikz::Arrow::NoArrow: return new AbstractArrow(style);
        case tikz::Arrow::ToArrow: return new ToArrow(style);
        case tikz::Arrow::ReversedToArrow: return new ReversedToArrow(style);
        case tikz::Arrow::StealthArrow: return new StealthArrow(style);
        case tikz::Arrow::ReversedStealthArrow: return new ReversedStealthArrow(style);
        case tikz::Arrow::LatexArrow: return new LatexArrow(style);
        case tikz::Arrow::ReversedLatexArrow: return new ReversedLatexArrow(style);
        case tikz::Arrow::PipeArrow: return new PipeArrow(style);
        case tikz::Arrow::StealthTickArrow: return new StealthTickArrow(style);
        case tikz::Arrow::ReversedStealthTickArrow: return new ReversedStealthTickArrow(style);
        default: break;
    }
    return new AbstractArrow(style);
}

// kate: indent-width 4; replace-tabs on;
