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

#include "TikzDocument.h"
#include "TikzDocument_p.h"

#include "Node.h"
#include "Edge.h"
#include "Style.h"
#include "Document.h"

#include "TikzNode.h"
#include "TikzEdge.h"
#include "TikzScene.h"

#include <QDebug>

//BEGIN private data
TikzDocumentPrivate::TikzDocumentPrivate(TikzDocument * tikzDocument)
    : QObject(tikzDocument)
{
    tikzDoc = tikzDocument;
}
//END



TikzDocument::TikzDocument(QObject * parent)
    : tikz::Document(parent)
{
    d = new TikzDocumentPrivate(this);
    d->doc = new tikz::Document(d);
    d->scene = new TikzScene(d);
}

TikzDocument::~TikzDocument()
{
    foreach (TikzEdge * edge, d->edges) {
        deleteEdge(edge->id());
    }

    foreach (TikzNode* node, d->nodes) {
        deleteNode(node->id());
    }

    // NOTE: d is deleted via QObject parent/child hierarchy
}

QGraphicsView * TikzDocument::createView()
{
}

TikzNode * TikzDocument::createTikzNode()
{
    // create node
    tikz::Node * node = Document::createNode();
    Q_ASSERT(d->nodeMap.contains(node->id()));

    return d->nodeMap[node->id()];
}

TikzEdge * TikzDocument::createTikzEdge()
{
    // create edge
    tikz::Edge * edge = Document::createEdge();
    Q_ASSERT(d->edgeMap.contains(edge->id()));

    return d->edgeMap[edge->id()];
}

void TikzDocument::deleteTikzNode(TikzNode * node)
{
    // delete node from id
    const int id = node->id();
    Q_ASSERT(d->nodeMap.contains(id));
    deleteNode(node->id());
    Q_ASSERT(! d->nodeMap.contains(id));
}

void TikzDocument::deleteTikzEdge(TikzEdge * edge)
{
    // delete edge from id
    const int id = edge->id();
    Q_ASSERT(d->edgeMap.contains(id));
    deleteEdge(edge->id());
    Q_ASSERT(! d->edgeMap.contains(id));
}

tikz::Node * TikzDocument::createNode(qint64 id)
{
    // create node by tikz::Document
    tikz::Node * node = Document::createNode(id);
    Q_ASSERT(id == node->id());
    Q_ASSERT(! d->nodeMap.contains(id));

    // create GUI item
    TikzNode * tikzNode = new TikzNode(node);
    d->nodes.append(tikzNode);
    d->nodeMap.insert(id, tikzNode);

    // add to graphics scene
    d->scene->addItem(tikzNode);

    return node;
}

tikz::Edge * TikzDocument::createEdge(qint64 id)
{
    tikz::Edge * edge = tikz::Document::createEdge(id);
    Q_ASSERT(id == edge->id());
    Q_ASSERT(! d->edgeMap.contains(id));

    // create GUI item
    TikzEdge * tikzEdge = new TikzEdge(edge);
    d->edges.append(tikzEdge);
    d->edgeMap.insert(id, tikzEdge);

    // add to graphics scene
    d->scene->addItem(tikzEdge);

    return edge;
}

void TikzDocument::deleteNode(qint64 id)
{
    Q_ASSERT(d->nodeMap.contains(id));

    // get TikzNode
    TikzNode * tikzNode = d->nodeMap[id];

    // remove from scene
    d->scene->removeItem(tikzNode);

    const int index = d->nodes.indexOf(tikzNode);
    Q_ASSERT(index >= 0);

    // delete item
    d->nodeMap.remove(id);
    d->nodes.remove(index);
    delete tikzNode;

    tikz::Document::deleteNode(id);
}

void TikzDocument::deleteEdge(qint64 id)
{
    Q_ASSERT(d->edgeMap.contains(id));

    // get TikzEdge
    TikzEdge * tikzEdge = d->edgeMap[id];

    // remove from scene
    d->scene->removeItem(tikzEdge);

    const int index = d->edges.indexOf(tikzEdge);
    Q_ASSERT(index >= 0);

    // delete item
    d->edgeMap.remove(id);
    d->edges.remove(index);
    delete tikzEdge;

    tikz::Document::deleteEdge(id);
}

// kate: indent-width 4; replace-tabs on;
