/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2014 Dominik Haumann <dhaumann@kde.org>
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

#ifndef TIKZ_UI_DOCUMENT_PRIVATE_H
#define TIKZ_UI_DOCUMENT_PRIVATE_H

#include <QObject>
#include <QVector>
#include <QHash>

class TikzNode;
class QGraphicsView;

namespace tikz {
namespace core {
    class Document;
    class Node;
    class Path;
}

namespace ui {

class PathItem;

class TikzDocument;
class TikzScene;


class TikzDocumentPrivate : public QObject
{
    Q_OBJECT

    public:
        /**
         * Default constructor.
         */
        TikzDocumentPrivate(TikzDocument * tikzDocument);

    //
    // data types
    //
    public:
        /**
         * The associated TikzDocument.
         */
        TikzDocument * tikzDoc; // FIXME: tikzDoc needed nowhere! remove?

        /**
         * List of TikzNode%s.
         */
        QVector<TikzNode*> nodes;

        /**
         * List of tikz::ui::PathItem%s.
         */
        QVector<tikz::ui::PathItem *> paths;

        /**
         * Node lookup map
         */
        QHash<qint64, TikzNode*> nodeMap;

        /**
         * Edge lookup map
         */
        QHash<qint64, tikz::ui::PathItem *> pathMap;

        /**
         * Graphics scene for the document.
         */
        TikzScene * scene;

        /**
         * List of graphics views.
         */
        QVector<QGraphicsView*> views;
};

}
}

#endif // TIKZ_UI_DOCUMENT_PRIVATE_H

// kate: indent-width 4; replace-tabs on;