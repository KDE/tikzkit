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

#include "EllipsePath.h"
#include "UndoSetEllipsePos.h"
#include "Node.h"
#include "Document.h"

#include <QDebug>

namespace tikz {
namespace core {

class EllipsePathPrivate
{
    public:
        // meta node this ellipse
        MetaPos::Ptr pos;
};

EllipsePath::EllipsePath(qint64 id, Document* doc)
    : Path(id, doc)
    , d(new EllipsePathPrivate())
{
    d->pos = doc->createMetaPos();

    connect(d->pos->notificationObject(), SIGNAL(changed(tikz::core::MetaPos*)),
            this, SLOT(emitChangedIfNeeded()));
}

EllipsePath::~EllipsePath()
{
    delete d;
}

Path::Type EllipsePath::type() const
{
    return Path::Ellipse;
}

void EllipsePath::deconstruct()
{
    // just set both the start and end pos to (0, 0).
    // undo (i.e., creating the node again), will then restore the initial
    // connections correctly.
    beginConfig();
    setPos(QPointF(0, 0));
    endConfig();
}

void EllipsePath::detachFromNode(Node * node)
{
    Q_ASSERT(node != 0);

    // disconnect from node, if currently attached
    if (d->pos->node() == node) {
        auto newPos = metaPos();
        newPos->setNode(0);
        setMetaPos(newPos);
    }
    Q_ASSERT(d->pos->node() != node);
}

void EllipsePath::setNode(Node* node)
{
    auto newPos = metaPos();
    newPos->setNode(node);
    setMetaPos(newPos);

    Q_ASSERT(d->pos->node() == node);
}

Node* EllipsePath::node() const
{
    return d->pos->node();
}

QPointF EllipsePath::pos() const
{
    return d->pos->pos();
}

void EllipsePath::setPos(const QPointF& pos)
{
    auto newPos = metaPos();
    newPos->setPos(pos);
    setMetaPos(newPos);

    Q_ASSERT(d->pos->pos() == pos);
}

tikz::core::MetaPos::Ptr EllipsePath::metaPos() const
{
    tikz::core::MetaPos::Ptr pos(document()->createMetaPos());
    *pos = *d->pos;
    return pos;
}

void EllipsePath::setMetaPos(const tikz::core::MetaPos::Ptr & pos)
{
    if (*d->pos == *pos) {
        return;
    }

    if (document()->undoActive()) {
        beginConfig();
        auto oldNode = node();
        *d->pos = *pos;
        auto newNode = node();
        if (oldNode != newNode) {
            emit nodeChanged(newNode);
        }
        endConfig();
    } else {
        document()->undoManager()->push(
            new UndoSetEllipsePos(this, pos, document()));
    }
}

tikz::Anchor EllipsePath::anchor() const
{
    return d->pos->anchor();
}

void EllipsePath::setAnchor(tikz::Anchor anchor)
{
    auto newPos = metaPos();
    newPos->setAnchor(anchor);
    setMetaPos(newPos);

    Q_ASSERT(d->pos->anchor() == anchor);
}

}
}

// kate: indent-width 4; replace-tabs on;
