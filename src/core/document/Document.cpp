/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2015 Dominik Haumann <dhaumann@kde.org>
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

#include "Document.h"
#include "Node.h"
#include "EdgePath.h"
#include "EllipsePath.h"
#include "Style.h"

#include "Transaction.h"
#include "UndoManager.h"
#include "UndoFactory.h"
#include "UndoGroup.h"
#include "UndoCreateEntity.h"
#include "UndoDeleteEntity.h"
#include "UndoSetProperty.h"

#include "Visitor.h"
#include "SerializeVisitor.h"
#include "DeserializeVisitor.h"
#include "TikzExportVisitor.h"

#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QUrl>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace tikz {
namespace core {

// helper: remove \r and \n from visible document name (see Kate bug #170876)
inline static QString removeNewLines(const QString &str)
{
    QString tmp(str);
    return tmp.replace(QLatin1String("\r\n"), QLatin1String(" "))
           .replace(QLatin1Char('\r'), QLatin1Char(' '))
           .replace(QLatin1Char('\n'), QLatin1Char(' '));
}

class DocumentPrivate
{
    public:
        // Document this private instance belongs to
        Document * q = nullptr;

        // the Document's current url
        QUrl url;
        // undo manager
        UndoManager * undoManager = nullptr;
        // flag whether operations should add undo items or not
        bool undoActive = false;

        Unit preferredUnit = Unit::Centimeter;

        // global document style options
        Style * style = nullptr;

        // Entity list, contains Nodes and Paths
        QVector<Entity *> entities;

        // Node lookup map
        QHash<Uid, Entity *> entityMap;

        // the document-wide unique ids start at 1.
        // Id 0 is reserved for the Document Uid, see Document constructor.
        qint64 nextId = 1;

        // helper to get a document-wide unique id
        qint64 uniqueId()
        {
            return nextId++;
        }

        QString docName = QString("Untitled");

//
// helper functions
//
public:
    void updateDocumentName() {
        if (! url.isEmpty() && docName == removeNewLines(url.fileName())) {
            return;
        }

        QString newName = removeNewLines(url.fileName());

        if (newName.isEmpty()) {
            newName = "Untitled";
        }

        if (newName != docName) {
            docName = newName;
            Q_EMIT q->documentNameChanged(q);
        }
    }
};

Document::Document(QObject * parent)
    : Entity(Uid(0, this))
    , d(new DocumentPrivate())
{
    // the Document's ownership is maintained elsewhere. Since the Entity
    // does not allow passing the ownership, we need to do this explicitly here.
    setParent(parent);

    d->q = this;
    d->undoManager = new UndoManager(this);
    d->style = new Style(Uid(d->uniqueId(), this));

    // Debugging:
    d->style->setLineWidth(tikz::Value::veryThick());

    connect(d->undoManager, SIGNAL(cleanChanged(bool)), this, SIGNAL(modifiedChanged()));
}

Document::~Document()
{
    // clear Document contents
    close();

    // make sure things are really gone
    Q_ASSERT(d->entityMap.isEmpty());
    Q_ASSERT(d->entities.isEmpty());

    delete d;
}

tikz::EntityType Document::entityType() const
{
    return tikz::EntityType::Document;
}

bool Document::accept(Visitor & visitor)
{
    // visit this document
    visitor.visit(this);

    // visit all styles
    for (auto entity : d->entities) {
        auto style = qobject_cast<Style *>(entity);
        if (style) {
            style->accept(visitor);
        }
    }

    // visit all nodes
    for (auto entity : d->entities) {
        auto node = qobject_cast<Node *>(entity);
        if (node) {
            node->accept(visitor);
        }
    }

    // visit all paths
    for (auto entity : d->entities) {
        auto path = qobject_cast<Path *>(entity);
        if (path) {
            path->accept(visitor);
        }
    }

    return true;
}

void Document::close()
{
    // tell the world that all Nodes and Paths are about to be deleted
    Q_EMIT aboutToClear();

    // free all node and path data
    qDeleteAll(d->entities);
    d->entities.clear();
    d->entityMap.clear();

    // reset unique id counter
    d->nextId = 1;

    // reinitialize document style
    delete d->style;
    d->style = new Style(Uid(d->uniqueId(), this));

    // clear undo stack
    d->undoManager->clear();

    // unnamed document
    d->url.clear();

    // keep the document name up-to-date
    d->updateDocumentName();

    // propagate change() signal from style
    connect(d->style, &ConfigObject::changed, this, &ConfigObject::emitChangedIfNeeded);
}

bool Document::load(const QUrl & fileurl)
{
    // first start a clean document
    close();

    // open file + read all json contents
    QFile file(fileurl.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
         return false;
    }

    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = json.object();

    // read history and replay
    UndoFactory factory(this);
    QJsonArray history = root["history"].toArray();
    for (auto action : history) {
        QJsonObject entry = action.toObject();
        Transaction transaction(this, entry["text"].toString());
        QJsonArray items = entry["items"].toArray();
        for (auto item : items) {
            QJsonObject joItem = item.toObject();
            const QString type = joItem["type"].toString();
            UndoItem * undoItem = factory.createItem(type);
            if (undoItem) {
                undoItem->load(joItem);
                addUndoItem(undoItem);
            }
        }
    }

    if (root.contains("preferred-unit")) {
        setPreferredUnit(toEnum<Unit>(root["preferred-unit"].toString()));
    }

    // now make sure the next free uniq id is valid by finding the maximum
    // used id, and then add "+1".
    auto keys = d->entityMap.keys();
    if (keys.size()) {
        d->nextId = std::max_element(keys.begin(), keys.end())->id() + 1;
    }

    // keep the document name up-to-date
    d->updateDocumentName();

    // mark this state as unmodified
    d->undoManager->setClean();

    return true;
}

bool Document::reload()
{
    if (!d->url.isEmpty()) {
        return load(d->url);
    }
    return false;
}

bool Document::save()
{
    return saveAs(d->url);
}

bool Document::saveAs(const QUrl & targetUrl)
{
    SerializeVisitor v;
    accept(v);
    v.save(targetUrl.path());
    return true;

    const bool urlChanged = d->url.toLocalFile() != targetUrl.toLocalFile();

    if (targetUrl.isLocalFile()) {

        // first serialize to json document
        QJsonArray jsonHistory;
        for (auto group : d->undoManager->undoGroups()) {
            QJsonArray groupItems;
            for (auto item : group->undoItems()) {
                groupItems.append(item->save());
            }

            QJsonObject jsonGroup;
            jsonGroup["text"] = group->text();
            jsonGroup["items"] = groupItems;
            jsonHistory.append(jsonGroup);
        }

        QJsonObject json;
        json["history"] = jsonHistory;
        json["preferred-unit"] = toString(preferredUnit());

        // now save data
        QFile file(targetUrl.toLocalFile());
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        // write json to text stream
        QTextStream stream(&file);
        QJsonDocument jsonDoc(json);
        stream << jsonDoc.toJson();

        if (urlChanged) {
            d->url = targetUrl;
            // keep the document name up-to-date
            d->updateDocumentName();
        }

        // mark this state as unmodified
        d->undoManager->setClean();

        return true;
    }

    return false;
}

QUrl Document::url() const
{
    return d->url;
}

QString Document::documentName() const
{
    return d->docName;
}

bool Document::isEmptyBuffer() const
{
    return d->url.isEmpty()
        && ! isModified()
        && d->entities.isEmpty();
}

QString Document::tikzCode()
{
    TikzExportVisitor tev;
    accept(tev);

    return tev.tikzCode();
}

void Document::addUndoItem(tikz::core::UndoItem * undoItem)
{
    d->undoManager->addUndoItem(undoItem);
}

void Document::beginTransaction(const QString & name)
{
    // track changes
    beginConfig();

    // pass call to undo mananger
    d->undoManager->startTransaction(name);
}

void Document::cancelTransaction()
{
    d->undoManager->cancelTransaction();
}

void Document::finishTransaction()
{
    // first pass call to undo mananger
    d->undoManager->commitTransaction();

    // notify world about changes
    endConfig();
}

bool Document::transactionRunning() const
{
    return d->undoManager->transactionActive();
}

bool Document::setUndoActive(bool active)
{
    const bool lastState = d->undoActive;
    d->undoActive = active;
    return lastState;
}

bool Document::undoActive() const
{
    return d->undoActive;
}

bool Document::isModified() const
{
    return ! d->undoManager->isClean();
}

bool Document::undoAvailable() const
{
    return d->undoManager->undoAvailable();
}

bool Document::redoAvailable() const
{
    return d->undoManager->redoAvailable();
}

QAbstractItemModel * Document::historyModel() const
{
    return d->undoManager;
}

void Document::undo()
{
    const bool undoWasAvailable = undoAvailable();
    const bool redoWasAvailable = redoAvailable();

    d->undoManager->undo();

    const bool undoNowAvailable = undoAvailable();
    const bool redoNowAvailable = redoAvailable();

    if (undoWasAvailable != undoNowAvailable) {
        Q_EMIT undoAvailableChanged(undoNowAvailable);
    }

    if (redoWasAvailable != redoNowAvailable) {
        Q_EMIT redoAvailableChanged(redoNowAvailable);
    }
}

void Document::redo()
{
    const bool undoWasAvailable = undoAvailable();
    const bool redoWasAvailable = redoAvailable();

    d->undoManager->redo();

    const bool undoNowAvailable = undoAvailable();
    const bool redoNowAvailable = redoAvailable();

    if (undoWasAvailable != undoNowAvailable) {
        Q_EMIT undoAvailableChanged(undoNowAvailable);
    }

    if (redoWasAvailable != redoNowAvailable) {
        Q_EMIT redoAvailableChanged(redoNowAvailable);
    }
}

tikz::Pos Document::scenePos(const MetaPos & pos) const
{
    const auto node = pos.node();
    if (!node) {
        return pos.pos();
    }

    return node->pos();
}

void Document::setPreferredUnit(tikz::Unit unit)
{
    if (d->preferredUnit != unit) {
        d->preferredUnit = unit;
        Q_EMIT preferredUnitChanged(d->preferredUnit);
    }
}

tikz::Unit Document::preferredUnit() const
{
    return d->preferredUnit;
}

Style * Document::style() const
{
    return d->style;
}

QVector<Uid> Document::nodes() const
{
    QVector <Uid> nodeList;
    auto it = d->entityMap.cbegin();
    while (it != d->entityMap.cend()) {
        if (qobject_cast<Node *>(it.value())) {
            nodeList.append(it.key());
        }
        ++it;
    }
    return nodeList;
}

QVector<Uid> Document::paths() const
{
    QVector <Uid> pathList;
    auto it = d->entityMap.cbegin();
    while (it != d->entityMap.cend()) {
        if (qobject_cast<Path *>(it.value())) {
            pathList.append(it.key());
        }
        ++it;
    }
    return pathList;
}

Entity * Document::createEntity(tikz::EntityType type)
{
    // create new node, push will call ::redo()
    const Uid uid(d->uniqueId(), this);
    addUndoItem(new UndoCreateEntity(uid, type, this));

    // now the node should be in the map
    const auto it = d->entityMap.find(uid);
    if (it != d->entityMap.end()) {
        return *it;
    }

    // requested id not in map, this is a bug, since UndoCreateEntity should
    // call createEntity(uid, type) that inserts the Entity
    Q_ASSERT(false);

    return nullptr;
}

Entity * Document::createEntity(const Uid & uid, EntityType type)
{
    Q_ASSERT(uid.isValid());
    Q_ASSERT(uid.document() == this);
    Q_ASSERT(!d->entityMap.contains(uid));

    // create new node
    Entity * e = nullptr;
    switch (type) {
        case EntityType::Document: Q_ASSERT(false); break;
        case EntityType::Style: {
            e = new Style(uid);
            e->setObjectName("Style " + uid.toString());
            ((Style*)e)->setParentStyle(style()->uid());
            break;
        }
        case EntityType::Node: {
            e = new Node(uid);
            e->setObjectName("Node " + uid.toString());
            break;
        }
        case EntityType::Path: {
            e = new EdgePath(PathType::Line, uid); // FIXME: only EdgePath right now
            e->setObjectName("Path " + uid.toString());
            break;
        }
    }

    Q_ASSERT(e);
    d->entities.append(e);

    // insert entity into hash map
    d->entityMap.insert(uid, e);

    // propagate changed signal
    connect(e, &ConfigObject::changed, this, &ConfigObject::emitChangedIfNeeded);

    return e;
}

void Document::deleteEntity(Entity * e)
{
    // valid input?
    Q_ASSERT(e != nullptr);
    Q_ASSERT(d->entityMap.contains(e->uid()));

    // get id
    const Uid uid = e->uid();

    // start undo group
    d->undoManager->startTransaction("Remove entity");

    // make sure no edge points to the deleted node
    if (auto nodeEntity = qobject_cast<Node*>(e)) {
        for (auto entity : d->entities) {
            if (auto path = qobject_cast<Path *>(entity)) {
                path->detachFromNode(nodeEntity);
            }

            // TODO: a path might require the node?
            //       in that case, maybe delete the path as well?
        }
    }

    // delete node, push will call ::redo()
    addUndoItem(new UndoDeleteEntity(uid, this));

    // end undo group
    d->undoManager->commitTransaction();

    // node really removed?
    Q_ASSERT(!d->entityMap.contains(uid));
}

void Document::deleteEntity(const Uid & uid)
{
    // valid input?
    Q_ASSERT(uid.isValid());
    Q_ASSERT(d->entityMap.contains(uid));

    // get entity
    auto it = d->entityMap.find(uid);
    if (it != d->entityMap.end()) {
        const auto entity = *it;

        // unregister entity
        d->entityMap.erase(it);
        Q_ASSERT(d->entities.contains(entity));
        d->entities.erase(std::find(d->entities.begin(), d->entities.end(), entity));

        // truly delete node
        delete entity;
    }
}

Node * Document::createNode()
{
    Transaction transaction(this, "Create Node");

    // create node style
    auto nodeStyle = createEntity(tikz::EntityType::Style);

    // create node
    auto node = createEntity<Node>(tikz::EntityType::Node);

    // set the node style
    addUndoItem(new UndoSetProperty(node->uid(), "style", nodeStyle->uid()));

    return node;
}

Path * Document::createPath()
{
    Transaction transaction(this, "Create Path");

    // create path style
    auto pathStyle = createEntity(tikz::EntityType::Style);

    // create path
    auto path = createEntity<Path>(tikz::EntityType::Path);

    // set the path style
    addUndoItem(new UndoSetProperty(path->uid(), "style", pathStyle->uid()));

    return path;
}

Path * Document::createPath(PathType type, const Uid & uid)
{
    Q_ASSERT(uid.isValid());

    // create new path
    Path* path = nullptr;
    switch(type) {
        case PathType::Line:
        case PathType::HVLine:
        case PathType::VHLine:
        case PathType::BendCurve:
        case PathType::InOutCurve:
        case PathType::BezierCurve: {
            path = new EdgePath(type, uid);
            break;
        }
        case PathType::Ellipse:
            path = new EllipsePath(uid);
            break;
        default:
            Q_ASSERT(false);
    }

    // register path
    d->entities.append(path);

    // insert path into hash map
    d->entityMap.insert(uid, path);

    // propagate changed signal
    connect(path, &ConfigObject::changed, this, &ConfigObject::emitChangedIfNeeded);

    return path;
}

Entity * Document::entity(const tikz::core::Uid & uid) const
{
    if (uid.document() != this) {
        return nullptr;
    }

    // Uid 0 alreay refers to this Document
    if (uid.id() == 0) {
        return const_cast<Document*>(this);
    }

    // Uid 1 alreay refers to this Document's Style
    if (uid.id() == 1) {
        return d->style;
    }

    // all other entities are in the entity list
    const auto it = d->entityMap.find(uid);
    if (it != d->entityMap.end()) {
        return *it;
    }

    return nullptr;
}

QVector<Uid> Document::entities() const
{
    return QVector<Uid>::fromList(d->entityMap.keys());
}


}
}

// kate: indent-width 4; replace-tabs on;
