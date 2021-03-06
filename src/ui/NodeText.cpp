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

#include "NodeText.h"
#include "NodeText_p.h"
#include "NodeItem.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include <QDebug>

namespace tikz {
namespace ui {

NodeText::NodeText(NodeItem* node)
    : QGraphicsItem(node)
    , d(new NodeTextPrivate(node, this))
{
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    QObject::connect(node->node(), SIGNAL(textChanged(QString)), &d->texGenerator, SLOT(generateImage(QString)));
}

NodeText::~NodeText()
{
    delete d;
}

QRectF NodeText::boundingRect() const
{
    return textRect().adjusted(-0.1, -0.1, 0.1, 0.1);
}

void NodeText::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->scale(1.0, -1.0);
//     painter->drawRect(textRect());
    d->svgRenderer.render(painter, textRect());

    painter->restore();
}

QRectF NodeText::textRect() const
{
    if (d->svgRenderer.isValid()) {
        QRectF rect = d->svgRenderer.viewBoxF();
        rect.setSize(QSizeF(rect.width(), rect.height()));
        rect.moveCenter(QPointF(0.0, 0.0));
        return rect;
    } else {
        return QRectF(0.0, 0.0, 0.0, 0.0);
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
