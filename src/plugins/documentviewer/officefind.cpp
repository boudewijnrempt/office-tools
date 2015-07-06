/*
 * This file is part of Meego Office UI for KOffice
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "officefind.h"

#include <QVariant>
#include <KoTextDocumentLayout.h>
#include <KoTextLayoutRootArea.h>
#include <KoShape.h>

OfficeFind::OfficeFind()
: KoFindText(QList<QTextDocument*>())
{
}

OfficeFind::~OfficeFind()
{
}

void OfficeFind::find(const QString &pattern)
{
    KoFindText::find(pattern);
    updateCanvasAll(matches());
}

void OfficeFind::findNext()
{
    updateCanvas(currentMatch());
    KoFindText::findNext();
    updateCanvas(currentMatch());
}

void OfficeFind::findPrevious()
{
    updateCanvas(currentMatch());
    KoFindText::findPrevious();
    updateCanvas(currentMatch());
}

void OfficeFind::finished()
{
    updateCanvasAll(matches());
    KoFindText::finished();
}

void OfficeFind::updateCanvas(const KoFindMatch &match)
{
    if (!match.isValid() || !match.location().canConvert<QTextCursor>() || !match.container().canConvert<QTextDocument*>() ) {
        return;
    }

    QTextDocument *doc = match.container().value<QTextDocument *>();
    if (doc) {
        KoTextDocumentLayout *documentLayout = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
        if (documentLayout) {
            QTextCursor cursor = match.location().value<QTextCursor>();
            KoTextLayoutRootArea *area = documentLayout->rootAreaForPosition(cursor.position());
            if (area) {
                KoShape *shape = area->associatedShape();
                if (shape) {
                    shape->update();
                }
            }
        }
    }
}

void OfficeFind::updateCanvasAll(const KoFindMatchList &matches)
{
    QSet<KoShape*> shapes;
    for (int i = 0; i < matches.count(); ++i) {
        const KoFindMatch &match = matches.at(i);
        if (!match.isValid() || !match.location().canConvert<QTextCursor>() || !match.container().canConvert<QTextDocument*>() ) {
            return;
        }
        QTextDocument *doc = match.container().value<QTextDocument *>();
        if (doc) {
            KoTextDocumentLayout *documentLayout = qobject_cast<KoTextDocumentLayout*>(doc->documentLayout());
            if (documentLayout) {
                QTextCursor cursor = match.location().value<QTextCursor>();
                KoTextLayoutRootArea *area = documentLayout->rootAreaForPosition(cursor.position());
                if (area) {
                    KoShape *shape = area->associatedShape();
                    if (shape) {
                        shapes.insert(shape);
                    }
                }
            }
        }
    }
    foreach (KoShape *shape, shapes) {
        shape->update();
    }
}
