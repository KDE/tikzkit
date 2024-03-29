/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2014 Dominik Haumann <dhaumann@kde.org>
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

#include "PropertyBrowser.h"
#include "ValuePropertyManager.h"
#include "ValueSpinBoxFactory.h"
#include "OpacityEditorFactory.h"
#include "OpacityPropertyManager.h"
#include "UidPropertyManager.h"

#include <tikz/core/Uid.h>
#include <tikz/core/Document.h>
#include <tikz/core/Node.h>
#include <tikz/core/Path.h>
#include <tikz/core/Style.h>
#include <tikz/core/PropertyManager.h>
#include <tikz/core/UndoSetProperty.h>

#include <tikz/ui/View.h>
#include <tikz/ui/NodeItem.h>
#include <tikz/ui/PathItem.h>
#include <tikz/ui/Document.h>

#include <QDebug>
#include <QVBoxLayout>
#include <QPointer>
#include <QMetaProperty>

#include <QtTreePropertyBrowser>
#include <QtDoublePropertyManager>
#include <QtDoubleSpinBoxFactory>
#include <QtBoolPropertyManager>
#include <QtColorPropertyManager>
#include <QtColorEditorFactory>

#include <QFile>
#include <QJsonDocument>

tikz::core::Style * styleForItem(const tikz::core::Uid & uid)
{
    if (! uid.isValid()) {
        return nullptr;
    }

    auto e = uid.entity();
    switch (e->entityType()) {
        case tikz::EntityType::Document: return qvariant_cast<tikz::core::Style*>(uid.entity()->property("style"));
        case tikz::EntityType::Path: return qvariant_cast<tikz::core::Uid>(uid.entity()->property("style")).entity<tikz::core::Style>();
        case tikz::EntityType::Node: return qvariant_cast<tikz::core::Uid>(uid.entity()->property("style")).entity<tikz::core::Style>();
        case tikz::EntityType::Style: return uid.entity<tikz::core::Style>();
    }

    return nullptr;
}

namespace tikz {
namespace ui {

class PropertyBrowserPrivate
{
public:
    QtTreePropertyBrowser * browser = nullptr;
    ValuePropertyManager * valueManager = nullptr;
    QtBoolPropertyManager * boolManager = nullptr;
    QtColorPropertyManager * colorManager = nullptr;
    OpacityPropertyManager * opacityManager = nullptr;
    QtDoublePropertyManager * doubleManager = nullptr;
    QtEnumPropertyManager * enumManager = nullptr;
    QtStringPropertyManager * stringManager = nullptr;
    UidPropertyManager * uidManager = nullptr;

    QPointer<View> view = nullptr;
    tikz::core::Uid uid;
    QHash<QtProperty *, QString> propertyMap;

public:
    void addProperty(QtProperty *property, const QString & name)
    {
        Q_ASSERT(! propertyMap.contains(property));
        propertyMap[property] = name;
//         QtBrowserItem *item =
        browser->addProperty(property);
//         browser->setExpanded(item, idToExpanded[name]);
    }

    void addSubProperty(QtProperty *parent, QtProperty *property, const QString & name)
    {
        Q_ASSERT(! propertyMap.contains(property));
        propertyMap[property] = name;
        parent->addSubProperty(property);
//         browser->setExpanded(item, idToExpanded[name]);
    }

    void addProperty(QObject * object, const QString & name)
    {
        const auto info = tikz::core::propertyManager().info(name);

        if (!info.isValid()) {
            qDebug() << "Unknown property:" << name;
            return;
        }

        QVariant prop = object->property(name.toLatin1().data());
        if (!prop.isValid()) {
            qDebug() << "Unknown property type:" << name;
            return;
        }

        QtProperty *property = nullptr;

        const QString type = info.type();
        if (type == "string") {
            property = stringManager->addProperty(info.title());
            stringManager->setValue(property, prop.toString());
        }
        else if (type == "uid") {
            property = uidManager->addProperty(info.title());
            uidManager->setValue(property, prop.value<tikz::core::Uid>());
        }
        else if (type == "value") {
            property = valueManager->addProperty(info.title());
            valueManager->setRange(property, tikz::Value(0, tikz::Unit::Millimeter), tikz::Value(10, tikz::Unit::Millimeter));
            valueManager->setMinimum(property, tikz::Value::fromString(info.minimum().toString()));
            valueManager->setMaximum(property, tikz::Value::fromString(info.maximum().toString()));
            valueManager->setSingleStep(property, info.singleStep());
            valueManager->setValue(property, prop.value<tikz::Value>());
        }
        else if (type == "bool") {
            property = boolManager->addProperty(info.title());
            boolManager->setValue(property, prop.toBool());
        }
        else if (type == "color") {
            property = colorManager->addProperty(info.title());
            colorManager->setValue(property, prop.value<QColor>());
        }
        else if (type == "opacity") {
            property = opacityManager->addProperty(info.title());
            opacityManager->setValue(property, prop.toDouble());
        }
        else if (type == "double") {
            property = doubleManager->addProperty(info.title());
            doubleManager->setMinimum(property, info.minimum().toDouble());
            doubleManager->setMaximum(property, info.maximum().toDouble());
            doubleManager->setSingleStep(property, info.singleStep());
            doubleManager->setValue(property, prop.toDouble());
        }
        else if (type == "enum") {
            // find out property type
            auto metaProp = object->metaObject()->property(object->metaObject()->indexOfProperty(name.toLatin1().data()));
            if (metaProp.userType() == QMetaType::UnknownType) {
                qDebug() << "Unknown enum type";
            }

            // remove namespace, if applicable
            QString typeName = metaProp.typeName();
            if (typeName.startsWith("tikz::")) {
                typeName.remove(0, 6);
            }

            const int enumIndex = tikz::staticMetaObject.indexOfEnumerator(typeName.toLatin1().data());
            const QMetaEnum metaEnum = tikz::staticMetaObject.enumerator(enumIndex);
            QStringList enumNames;
            for (int i = 0; i < metaEnum.keyCount(); ++i) {
                enumNames << metaEnum.key(i);
            }
            property = enumManager->addProperty(info.title());
            enumManager->setEnumNames(property, enumNames);
            enumManager->setValue(property, prop.toInt());
        }
        else {
            qDebug() << "Unknown property type:" << name;
        }
        if (property) {
            const QString modifiedFunction = info.modifiedFunction();
            if (!modifiedFunction.isEmpty()) {
                bool isModified = false;
                const bool ok = QMetaObject::invokeMethod(object,
                                                          modifiedFunction.toLatin1().data(),
                                                          Qt::DirectConnection,
                                                          Q_RETURN_ARG(bool, isModified)
                );
                if (ok) {
                    property->setModified(isModified);
                } else {
                    tikz::debug("properties.json: Invalid modified getter " + modifiedFunction);
                }
            }
            addProperty(property, name);
        }
    }

    template <typename T>
    void setValue(QtProperty * property, const T & val)
    {
        // if items are inserted, the slot valueChanged() is also called.
        // In this case, the item is not yet registered in the map. Hence,
        // we just return.
        if (! propertyMap.contains(property)) {
            return;
        }

        auto style = styleForItem(uid);
        if (style) {
            const QString name = propertyMap[property];
            style->document()->addUndoItem(new tikz::core::UndoSetProperty(style->uid(), name, val));
        }
    }
};





PropertyBrowser::PropertyBrowser(QWidget *parent)
    : QWidget(parent)
    , d(new PropertyBrowserPrivate)
{
    d->browser = new QtTreePropertyBrowser(this);
    auto vboxLayout = new QVBoxLayout(this);
    vboxLayout->setContentsMargins(0, 0, 0, 0);
    vboxLayout->addWidget(d->browser);

    // setup property managers
    d->valueManager = new ValuePropertyManager(this);
    d->boolManager = new QtBoolPropertyManager(this);
    d->colorManager = new QtColorPropertyManager(this);
    d->opacityManager = new OpacityPropertyManager(this);
    d->doubleManager = new QtDoublePropertyManager(this);
    d->enumManager = new QtEnumPropertyManager(this);
    d->stringManager = new QtStringPropertyManager(this);
    d->uidManager = new UidPropertyManager(this);

    // setup propertybrowser
    d->browser->setFactoryForManager(d->valueManager, new ValueSpinBoxFactory(this));
    d->browser->setFactoryForManager(d->boolManager, new QtCheckBoxFactory(this));
    d->browser->setFactoryForManager(d->colorManager, new QtColorEditorFactory(this));
    d->browser->setFactoryForManager(d->opacityManager, new OpacityEditorFactory(this));
    d->browser->setFactoryForManager(d->doubleManager, new QtDoubleSpinBoxFactory(this));
    d->browser->setFactoryForManager(d->enumManager, new QtEnumEditorFactory(this));
    d->browser->setFactoryForManager(d->stringManager, new QtLineEditFactory(this));
//  d->uidManager: Uids are read-only, no editor required

    d->browser->setPropertiesWithoutValueMarked(true);
    d->browser->setRootIsDecorated(false);
    d->browser->setResizeMode(QtTreePropertyBrowser::ResizeToContents);

    // setup connections
    connect(d->valueManager, SIGNAL(valueChanged(QtProperty*, const tikz::Value &)),
            this, SLOT(valueChanged(QtProperty*, const tikz::Value &)));
    connect(d->boolManager, SIGNAL(valueChanged(QtProperty*, bool)),
            this, SLOT(valueChanged(QtProperty*, bool)));
    connect(d->colorManager, SIGNAL(valueChanged(QtProperty*, const QColor &)),
            this, SLOT(valueChanged(QtProperty*, const QColor &)));
    connect(d->opacityManager, SIGNAL(valueChanged(QtProperty*, qreal)),
            this, SLOT(valueChanged(QtProperty*, qreal)));
    connect(d->doubleManager, SIGNAL(valueChanged(QtProperty*, double)),
            this, SLOT(doubleValueChanged(QtProperty*, double)));
    connect(d->enumManager, SIGNAL(valueChanged(QtProperty*, int)),
            this, SLOT(enumValueChanged(QtProperty*, int)));
    connect(d->stringManager, SIGNAL(valueChanged(QtProperty*, QString)),
            this, SLOT(valueChanged(QtProperty*, QString)));
    // d->uidManager: read-only, no connection required
}

PropertyBrowser::~PropertyBrowser()
{
    delete d;
}

void PropertyBrowser::setView(tikz::ui::View * view)
{
    if (d->view == view) {
        return;
    }

    if (d->view) {
        disconnect(d->view, SIGNAL(selectionChanged(tikz::ui::View*)), this, SLOT(updateCurrentItem()));
    }

    d->view = view;
    if (d->view) {
        connect(d->view, SIGNAL(selectionChanged(tikz::ui::View*)), this, SLOT(updateCurrentItem()));
    }

    updateCurrentItem();
}

void PropertyBrowser::setItem(const tikz::core::Uid & uid)
{
    if (d->uid == uid) {
        return;
    }

    d->uid = uid;

    // clear all managers and mappings
    d->valueManager->clear();
    d->boolManager->clear();
    d->colorManager->clear();
    d->opacityManager->clear();
    d->doubleManager->clear();
    d->enumManager->clear();
    d->stringManager->clear();
    d->uidManager->clear();
    d->propertyMap.clear();

    auto style = styleForItem(d->uid);
    if (style) {
        auto metaObj = style->metaObject();
        while (metaObj) {
            for (int i = metaObj->propertyOffset(); i < metaObj->propertyCount(); ++i) {
                d->addProperty(style, metaObj->property(i).name());
            }
            metaObj = metaObj->superClass();
        }
    }
}

void PropertyBrowser::valueChanged(QtProperty *property, const tikz::Value & val)
{
    d->setValue(property, val);
}

void PropertyBrowser::valueChanged(QtProperty *property, bool val)
{
    d->setValue(property, val);
}

void PropertyBrowser::valueChanged(QtProperty *property, const QColor & val)
{
    d->setValue(property, val);
}

void PropertyBrowser::valueChanged(QtProperty *property, int val)
{
    d->setValue(property, val);
}

void PropertyBrowser::valueChanged(QtProperty *property, double val)
{
    d->setValue(property, val);
}

void PropertyBrowser::doubleValueChanged(QtProperty *property, double val)
{
    d->setValue(property, val);
}

void PropertyBrowser::enumValueChanged(QtProperty *property, int val)
{
    d->setValue(property, val);
}

void PropertyBrowser::valueChanged(QtProperty *property, const QString & val)
{
    d->setValue(property, val);
}

void PropertyBrowser::updateCurrentItem()
{
    auto tikzItems = d->view->selectedItems();
    if (tikzItems.size() != 1) {
        if (d->view) {
            setItem(d->view->document()->uid());
        } else {
            setItem(tikz::core::Uid());
        }
        return;
    }

    auto item = tikzItems.front();
    if (qobject_cast<NodeItem*>(item)) {
        setItem(qobject_cast<NodeItem*>(item)->uid());
    } else if (qobject_cast<PathItem*>(item)) {
        setItem(qobject_cast<PathItem*>(item)->uid());
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
