/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
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

#ifndef TIKZ_CORE_ENTITY_H
#define TIKZ_CORE_ENTITY_H

#include "tikz_export.h"
#include "tikz.h"
#include "Uid.h"

#include <memory>
#include <QObject>
#include <QJsonObject>

namespace tikz {
namespace core {

class Document;
class EntityPrivate;

class TIKZCORE_EXPORT Entity : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 uid READ uid)

    public:
        /**
         * Default constructor.
         * Creates a default entity with uid set to -1 and no associated Document.
         */
        Entity();

        /**
         * Constructor that deserializes the entity from @p json.
         */
        Entity(const QJsonObject & json, Document* doc);

        /**
         * Virtual destructor.
         */
        virtual ~Entity();

        /**
         * Unique entity identifier.
         * If the entity id is -1 (or negative), the entity is not unique,
         * implying the entity is not managed by any tikz::core::Document.
         */
        qint64 uid() const;

        /**
         * Returns the EntityType.
         */
        virtual tikz::EntityType entityType() const = 0;

        /**
         * Serialize the style to a JSON object.
         */
        virtual QJsonObject toJson() const;

        /**
         * Returns the document, if passed to the constructor.
         */
        Document * document() const;

    //
    // config methods
    //
    public:
        /**
         * Start changing properties.
         * This call is ref-counted. For each beginConfig() you finally
         * have to call endConfig().
         */
        void beginConfig();

        /**
         * End of changing properties.
         * This will emit changed(), if the number of calls of endConfig()
         * matches the calls the one of beginConfig(), i.e. the ref-counter is zero.
         *
         * Using beginConfig() and endConfig() allows to change multiple
         * config values, while still only emitting the changed() signal only once.
         */
        void endConfig();

        /**
         * Returns whether beginConfig() was called without an endConfig() yet.
         */
        bool configActive() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever the style changes.
         * This includes changes in the parent style that possibly influence
         * the appearance of this style.
         */
        void changed();

    //
    // internal to tikz::Document
    //
    protected:
        friend class Document;

        /**
         * Associate this style with @p id to the document @p tikzDocument.
         */
        Entity(qint64 id, Document* tikzDocument);

    private:
        std::unique_ptr<EntityPrivate> const d;
};

}
}
#endif // TIKZ_CORE_ENTITY_H

// kate: indent-width 4; replace-tabs on;
