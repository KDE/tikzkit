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

#ifndef TIKZ_DOCUMENT_H
#define TIKZ_DOCUMENT_H

#include "tikz_export.h"
#include "tikz.h"

#include <QObject>
#include <QVector>

class QUndoStack;
class QPointF;

namespace tikz
{

class DocumentPrivate;
class Style;
class Node;
class Edge;
class NodeStyle;
class EdgeStyle;

class TIKZCORE_EXPORT Document : public QObject
{
    Q_OBJECT

    public:
        /**
         * Default constructor.
         */
        Document(QObject * parent = 0);

        /**
         * Destructor
         */
        virtual ~Document();

    //
    // serialization
    //
    public:
        /**
         * Serialize the Document with all its nodes and edges to a JSON string.
         */
        QByteArray toJson() const;

    //
    // Undo / redo management
    //
    public:
        /**
         * Get the undo stack of this document.
         */
        QUndoStack * undoManager();

        /**
         * Begin undo group @p name.
         * Each beginUndoGroup() must have a matching endUndoGroup().
         * The calls may be nested.
         */
        void beginUndoGroup(const QString & name);

        /**
         * End undo group @p name.
         */
        void endUndoGroup();

        /**
         * Check whether the document is in a modified state.
         */
        bool isModified() const;

    //
    // Node, edge and style management
    //
    public:
        /**
         * Get the global document style.
         * This is the global document style. All other styles fall back to
         * this style, if a property is not set.
         */
        Style * style() const;

        /**
         * Get the list of nodes of the tikz document.
         */
        QVector<Node*> nodes() const;

        /**
         * Get the list of edges of the tikz document.
         */
        QVector<Edge*> edges() const;

        /**
         * Get the node with @p id.
         * @param id unique id of the node
         * @return null, if the id is -1, otherwise a valid pointer to the node
         */
        Node * nodeFromId(qint64 id);

        /**
         * Get the edge with @p id.
         * @param id unique id of the edge
         * @return null, if the id is -1, otherwise a valid pointer to the node
         */
        Edge * edgeFromId(qint64 id);

    //
    // Node and edge creation
    //
    public:
        /**
         * Creates a new node associated with this document.
         * If the node is not needed anymore, just delete it.
         */
        Node * createNode();

        /**
         * Creates a new edge associated with this document.
         * If the edge is not needed anymore, just delete it.
         */
        Edge * createEdge();

        /**
         * Remove @p node from the document by deleting the node object.
         * Afterwards, the pointer is invalid.
         * @param node node to delete
         */
        void deleteNode(Node * node);

        /**
         * Remove @p edge from the document by deleting the edge object.
         * Afterwards, the pointer is invalid.
         * @param edge edge to delete
         */
        void deleteEdge(Edge * edge);

    //
    // Node manipulation with undo/redo support
    //
    public:
        /**
         * Set the position of @p node to @p pos.
         */
        void setNodePos(Node * node, const QPointF & pos);

        /**
         * Set the text of @p node to @p text.
         */
        void setNodeText(Node * node, const QString & text);

        /**
         * Set the style of @p node to @p style.
         */
        void setNodeStyle(Node * node, const NodeStyle & style);

    //
    // internal: Undo / redo items manipulate with ID
    //
    protected:
        /**
         * Create a new node associated with this document with @p id.
         */
        virtual Node * createNode(qint64 id);

        /**
         * Create a new edge associated with this document with @p id.
         */
        virtual Edge * createEdge(qint64 id);

        /**
         * Delete node @p id associated with this document.
         */
        virtual void deleteNode(qint64 id);

        /**
         * Delete edge @p id associated with this document.
         */
        virtual void deleteEdge(qint64 id);

    private:
        DocumentPrivate * const d;

        // friend classing
        friend class UndoCreateNode;
        friend class UndoDeleteNode;
        friend class UndoCreateEdge;
        friend class UndoDeleteEdge;
};

}

#endif // TIKZ_DOCUMENT_H

// kate: indent-width 4; replace-tabs on;
