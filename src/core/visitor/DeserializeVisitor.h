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

#ifndef TIKZ_DESERIALIZE_VISITOR_H
#define TIKZ_DESERIALIZE_VISITOR_H

#include "Visitor.h"
#include <QJsonObject>

namespace tikz {
namespace core {

class Document;
class Style;
class Node;
class Path;

/**
 * Deserializes a tikz::core::Document from a json file.
 */
class DeserializeVisitor : public Visitor
{
    public:
        /**
         * Default constructor.
         */
        DeserializeVisitor();

        /**
         * Destructor
         */
        virtual ~DeserializeVisitor();

    //
    // extra functions
    //
    public:
        /**
         * Load the tikz::Document from the file @p filename.
         * @return true on success, otherwise false
         */
        bool load(const QString & filename);

    //
    // Visitor pattern
    //
    public:
        /**
         * Serializes document @p doc document.
         */
        void visit(Document * doc) override;

        /**
         * Serializes @p node.
         */
        void visit(Node * node) override;

        /**
         * Serializes @p path.
         */
        void visit(Path * path) override;

        /**
         * Serializes @p style.
         */
        void visit(Style * style) override;

    //
    // private data
    //
    private:
        QJsonObject m_root;
        QJsonObject m_nodes;
        QJsonObject m_paths;
        QJsonObject m_styles;
};

}
}

#endif // TIKZ_DESERIALIZE_VISITOR_H

// kate: indent-width 4; replace-tabs on;
