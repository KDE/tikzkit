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

#include "TikzNode.h"
#include <tikz/core/NodeStyle.h>
#include "NodeText.h"
#include "PaintHelper.h"
#include "AbstractShape.h"
#include "TikzDocument.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTextLayout>
#include <QGraphicsTextItem>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QPainterPath>
#include <QPixmap>

#include <QDebug>

#include <cmath>

namespace tikz {
namespace ui {

class TikzNodePrivate
{
    TikzNode* q;

    public:
        TikzNodePrivate(TikzNode * tikzNode) : q(tikzNode) {}

        tikz::core::Node* node;
        NodeText* textItem;
        AbstractShape * shape;

        bool itemChangeRunning : 1;
        bool dirty : 1;
        QPainterPath shapePath;

    public:
        void updateCache()
        {
            if (!dirty) return;
            dirty = false;

            if (node->style()->shape() != shape->type()) {
                delete shape;
                shape = createShape(node->style()->shape(), q);
            }

            shapePath = shape->shape();

            QTransform trans;
            trans.scale(node->style()->scale(), node->style()->scale());
            shapePath = trans.map(shapePath);

            q->setRotation(node->style()->rotation());
        }
};

TikzNode::TikzNode(tikz::core::Node * node, QGraphicsItem * parent)
    : TikzItem(parent)
    , d(new TikzNodePrivate(this))
{
    d->dirty = true;
    d->node = node;
    d->shape = new AbstractShape(this);
    d->itemChangeRunning = false;

    connect(d->node, SIGNAL(changed()), this, SLOT(styleChanged()));

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    d->textItem = new NodeText(this);
    d->textItem->setPos(boundingRect().center());

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    slotSetPos(node->pos());
}

TikzNode::~TikzNode()
{
    delete d->shape;
    delete d;
}

TikzDocument * TikzNode::document() const
{
    Q_ASSERT(qobject_cast<TikzDocument*>(d->node->document()) != nullptr);
    return qobject_cast<TikzDocument*>(d->node->document());
}

int TikzNode::type() const
{
    return UserType + 2;
}

tikz::core::Node * TikzNode::node()
{
    return d->node;
}

qint64 TikzNode::id() const
{
    return d->node->id();
}

tikz::core::NodeStyle* TikzNode::style() const
{
    return d->node->style();
}

QVector<tikz::Anchor> TikzNode::supportedAnchors() const
{
    // make sure cache is up-to-date
    d->updateCache();

    return d->shape->supportedAnchors();
}

QPointF TikzNode::anchor(tikz::Anchor anchor) const
{
    // make sure cache is up-to-date
    d->updateCache();

    QTransform trans;
    trans.scale(style()->scale(), style()->scale());
    return trans.map(d->shape->anchorPos(anchor));
}

QPointF TikzNode::contactPoint(tikz::Anchor anchor, qreal rad) const
{
    // make sure cache is up-to-date
    d->updateCache();

    // adapt angle to account for the self rotation of this node
    rad -= rotation() * M_PI / 180.0;

    QTransform trans;
    trans.scale(style()->scale(), style()->scale());
    return trans.map(d->shape->contactPoint(anchor, rad));
}

QRectF TikzNode::shapeRect() const
{
    d->updateCache();

    const QRectF textRect = d->textItem->textRect();
    const qreal innerSep = style()->innerSep();

    qreal w = textRect.width() + 2.0 * innerSep;
    qreal h = textRect.height() + 2.0 * innerSep;

    // extend rect, if minimum size is set
    if (w < style()->minimumWidth()) {
        w = style()->minimumWidth();
    }
    if (h < style()->minimumHeight()) {
        h = style()->minimumHeight();
    }

    QRectF rect(0.0, 0.0, w, h);
    d->shape->adjustShapeRect(textRect, rect);

    // center shape rect at (0, 0)
    rect.moveCenter(QPointF(0, 0));

    return rect;
}

void TikzNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    // debugging: bounding rect
//     painter->drawRect(boundingRect());

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    d->updateCache();

    PaintHelper sh(painter, style());
    QPen p = sh.pen();
    QBrush brush(style()->fillColor());
    painter->setPen(p);
    painter->setBrush(brush);

    painter->drawPath(d->shapePath);

    // draw double line, if wanted
    if (style()->doubleLine()) {
        painter->setBrush(Qt::NoBrush);
        p.setWidthF(style()->innerLineWidth());
        p.setColor(Qt::white);
        painter->setPen(p);
        painter->drawPath(d->shapePath);
    }


    painter->restore();
}

QRectF TikzNode::boundingRect() const
{
    // make sure cache is up-to-date
    d->updateCache();

    QRectF br = d->shapePath.boundingRect();
    br.adjust(-0.2, -0.2, 0.2, 0.2);

    return br;
}

QPainterPath TikzNode::shape() const
{
    // make sure cache is up-to-date
    d->updateCache();

    return d->shapePath;
}

QVariant TikzNode::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if (change == ItemPositionChange && scene() && !d->itemChangeRunning) {
        d->itemChangeRunning = true;
        QPointF newPos = value.toPointF();
        setPos(newPos);
        d->node->setPos(newPos);
        d->itemChangeRunning = false;
    }

    return QGraphicsObject::itemChange(change, value);
}

void TikzNode::slotSetPos(const QPointF& pos)
{
    if (d->itemChangeRunning) return;

    // the tikz::core::Node position changed.
    // propagate this to this TikzNode::setPos().
    setPos(pos);
}

void TikzNode::styleChanged()
{
    prepareGeometryChange();
    d->dirty = true;
    if (d->node->pos() != pos()) slotSetPos(d->node->pos());
    emit changed();
}

}
}

// kate: indent-width 4; replace-tabs on;