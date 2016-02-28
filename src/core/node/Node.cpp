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

#include "Node.h"
#include "NodeStyle.h"
#include "Document.h"
#include "Visitor.h"
#include "MetaPos.h"

#include "UndoSetNodePos.h"
#include "UndoSetNodeText.h"
#include "UndoSetNodeStyle.h"

#include <cmath>

#include <QUndoStack>
#include <QDebug>

namespace tikz {
namespace core {

class NodePrivate
{
    public:
        NodePrivate(Document * doc)
            : pos(doc)
        {}

        // node position
        MetaPos pos;

        // node text
        QString text;

        // this node's style
        NodeStyle style;
};

Node::Node(const Uid & uid, Document* doc)
    : Entity(uid, doc)
    , d(new NodePrivate(doc))
{
    d->style.setParentStyle(doc->style());

    connect(&d->style, SIGNAL(changed()), this, SLOT(emitChangedIfNeeded()));
}

Node::~Node()
{
    delete d;
}

tikz::EntityType Node::entityType() const
{
    return EntityType::Node;
}

bool Node::accept(Visitor & visitor)
{
    visitor.visit(this);
    return true;
}

void Node::setPos(const tikz::Pos & pos)
{
    auto newPos = metaPos();
    newPos.setPos(pos);
    setMetaPos(newPos);

    Q_ASSERT(d->pos.pos() == pos);
}

tikz::Pos Node::pos() const
{
    return d->pos.pos();
}

void Node::setMetaPos(const tikz::core::MetaPos & pos)
{
    if (d->pos == pos) {
        return;
    }

    if (document()->undoActive()) {
        ConfigTransaction transaction(this);
        d->pos = pos;
    } else {
        document()->addUndoItem(new UndoSetNodePos(uid(), pos, document()));
    }
}

const tikz::core::MetaPos & Node::metaPos() const
{
    return d->pos;
}

void Node::setText(const QString& text)
{
    // only continue when change is required
    if (d->text == text) {
        return;
    }

    if (document()->undoActive()) {
        ConfigTransaction transaction(this);
        d->text = text;
        emit textChanged(d->text);
    } else {
        // create new undo item, push will call ::redo()
        document()->addUndoItem(new UndoSetNodeText(uid(), text, document()));

        // now the text should be updated
        Q_ASSERT(d->text == text);
    }
}

QString Node::text() const
{
    return d->text;
}

NodeStyle* Node::style() const
{
    return &d->style;
}

void Node::setStyle(const NodeStyle & style)
{
    // TODO: room for optimization: if style did not change, abort

    if (document()->undoActive()) {
        ConfigTransaction transaction(this);
        d->style.setStyle(&style);
    } else {
    // create new undo item, push will call ::redo()
    document()->addUndoItem(new UndoSetNodeStyle(uid(), style, document()));

    // now the text should be updated
//     Q_ASSERT(d->style == style); // same as above
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
