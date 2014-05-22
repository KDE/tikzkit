/* This file is part of the TikZKit project
 *
 * Copyright (C) 2014 Dominik Haumann <dhaumann@kde.org>
 * Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 or the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */
#include "DocumentManager.h"

#include <tikz/ui/Editor.h>
#include <tikz/ui/View.h>

#include <QUrl>

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
{
    createDocument();
}

DocumentManager::~DocumentManager()
{
}

tikz::ui::Document *DocumentManager::createDocument()
{
    auto doc = tikz::ui::Editor::instance()->createDocument(this);

    m_documents.append(doc);

    // we have a new document, show it the world
    emit documentCreated(doc);

    // return our new document
    return doc;
}

tikz::ui::Document *DocumentManager::findDocument(const QUrl &url) const
{
    QUrl u(url.adjusted(QUrl::NormalizePathSegments));

    foreach(tikz::ui::Document * it, m_documents) {
        if (it->url() == u) {
            return it;
        }
    }

    return nullptr;
}

tikz::ui::Document *DocumentManager::openUrl(const QUrl &url)
{
    // special handling: if only one unmodified empty buffer in the list,
    // keep this buffer in mind to close it after opening the new url
    tikz::ui::Document *doc = nullptr;
    if (documentList().count() == 1
        && documentList().at(0)->isEmptyBuffer())
    {
        doc = documentList().first();
    }

    //
    // create new document
    //
    QUrl u(url.adjusted(QUrl::NormalizePathSegments));


    // always new document if url is empty...
    if (!u.isEmpty()) {
        doc = findDocument(u);
    }

    if (!doc) {
        doc = createDocument();

        if (!u.isEmpty()) {
            doc->load(u);
        }
    }

    return doc;
}

bool DocumentManager::closeDocument(tikz::ui::Document *doc, bool closeUrl)
{
    doc->close();

    if (closeUrl) {
        //FIXME: needed?
//         KateApp::self()->emitDocumentClosed(QString::number((qptrdiff)doc));

        // document will be deleted, soon
        emit aboutToDeleteDocument(doc);

        // really delete the document
        delete m_documents.takeAt(m_documents.indexOf(doc));

        // document is gone, emit our signals
        emit documentDeleted(doc);
    }

    /**
     * never ever empty the whole document list
     * do this before documentsDeleted is emited, to have no flicker
     */
    if (m_documents.isEmpty()) {
        createDocument();
    }

    return true;
}


bool DocumentManager::closeAllDocuments(bool closeUrl)
{
    foreach (auto doc, m_documents) {
        closeDocument(doc, closeUrl);
    }
    return true;
}

/**
 * Find all modified documents.
 * @return Return the list of all modified documents.
 */
QList<tikz::ui::Document *> DocumentManager::modifiedDocumentList()
{
    QList<tikz::ui::Document *> modified;
    foreach(tikz::ui::Document * doc, m_documents) {
        if (doc->isModified()) {
            modified.append(doc);
        }
    }
    return modified;
}

bool DocumentManager::queryCloseDocuments(MainWindow * w)
{
#if 0
    int docCount = m_documents.count();
    foreach(tikz::ui::Document * doc, m_documents) {
        if (doc->url().isEmpty() && doc->isModified()) {
            int msgres = QMessageBox::warningYesNoCancel(w,
                         tr("<p>The document '%1' has been modified, but not saved.</p>"
                            "<p>Do you want to save your changes or discard them?</p>", doc->documentName()),
                         tr("Close Document"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            if (msgres == QMessageBox::Cancel) {
                return false;
            }

            if (msgres == QMessageBox::Yes) {
                KEncodingFileDialog::Result r = KEncodingFileDialog::getSaveUrlAndEncoding(doc->encoding(), QUrl(), QString(), w, i18n("Save As"));

                doc->setEncoding(r.encoding);

                if (!r.URLs.isEmpty()) {
                    QUrl tmp = r.URLs.first();

                    if (!doc->saveAs(tmp)) {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        } else {
            if (!doc->queryClose()) {
                return false;
            }
        }
    }

    // document count changed while queryClose, abort and notify user
    if (m_documents.count() > docCount) {
        KMessageBox::information(w,
                                 i18n("New file opened while trying to close Kate, closing aborted."),
                                 i18n("Closing Aborted"));
        return false;
    }
#endif
    return true;
}

void DocumentManager::saveAll()
{
    foreach(tikz::ui::Document * doc, m_documents)
    if (doc->isModified()) {
        doc->save();
    }
}

void DocumentManager::reloadAll()
{
    // reload all docs that are NOT modified on disk
    foreach(tikz::ui::Document * doc, m_documents) {
        doc->load(doc->url());
    }
}

// kate: indent-width 4; replace-tabs on;
