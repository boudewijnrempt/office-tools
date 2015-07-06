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

#include "documentlistmodel.h"
#include <QStringList>
#include <QSettings>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QtSparql/QSparqlConnection>
#include <QtSparql/QSparqlResult>
#include <QtSparql/QSparqlError>

#include <MTheme>
#include <MSortFilterProxyModel>
#include <MCalendar>
#include <MLocale>
#include "misc.h"
#include "trackerutils.h"

#define WEEK  (7)
#define MONTH (30)
#define YEAR  (360)

DocumentListModel::DocumentListModel()
    : MAbstractItemModel(),
      currentGrouping(NoGroup),
      groups(),
      groupsSize(),
      documentsUpdated(false)
{
    liveQuery = TrackerUtils::Instance().createTrackerLiveQuery();

    connect(liveQuery, SIGNAL(initialQueryFinished()), this, SLOT(liveModelQueryFinished()));
}

void DocumentListModel::liveModelQueryFinished()
{
    qDebug() << __PRETTY_FUNCTION__;

    connect(model(), SIGNAL(modelAboutToBeReset()), this, SIGNAL(modelAboutToBeReset()));
    connect(model(), SIGNAL(modelReset()), this, SIGNAL(modelReset()));

    connect(model(), SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
    connect(model(), SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));

    connect(model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(handleDataChanged(QModelIndex,QModelIndex)));

    connect(model(), SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)),
            this, SLOT(handleRowsAboutToBeInserted(QModelIndex, int, int)));
    connect(model(), SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)),
            this, SLOT(handleRowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)));
    connect(model(), SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)),
            this, SLOT(handleRowsAboutToBeRemoved(QModelIndex, int, int)));
    connect(model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(handleRowsInserted(QModelIndex, int, int)));
    connect(model(), SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
            this, SLOT(handleRowsMoved(QModelIndex, int, int, QModelIndex, int)));
    connect(model(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(handleRowsRemoved(QModelIndex, int, int)));

    recalculateGroups();

    emit liveQueryFinished();
}

DocumentListModel::~DocumentListModel()
{
    if (liveQuery) {
        delete liveQuery;
    }
}

void DocumentListModel::handleDataChanged(const QModelIndex &topLeft,
                                         const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    emit layoutAboutToBeChanged();
    //We get repeated dataChanged signal. We can live with one signal
    //And do recalculate groups after 500msec
    if (!documentsUpdated) {
        QTimer::singleShot(500, this, SLOT(resetDocumentUpdatedFlag()));
        documentsUpdated = true;
        recalculateGroups();
    }
    emit layoutChanged();
    // Do not emit dataChanged signal here. We have mismatch with proxymodel.
}

void DocumentListModel::handleRowsAboutToBeRemoved(const QModelIndex &index,
                                                  int start, int end)
{
    Q_UNUSED(index);
    qDebug() << __PRETTY_FUNCTION__;
    beginRemoveRows(QModelIndex(), start, end, false);
    if(pathsToMonitor.count() != 0)
    {
        for(int i = start; i <= end; i++)
        {
            qDebug() << " PATH DELETED " << liveQuery->model()->index(i, 0).data().toString();
            pathsToMonitor.removeAll(liveQuery->model()->index(i, 0).data().toString());
        }
        if(pathsToMonitor.count() == 0)
        {
            notifyListDeleteCompleted = true;
        }
    }
}

void DocumentListModel::handleRowsRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
    qDebug() << __PRETTY_FUNCTION__;
    recalculateGroups();
    endRemoveRows();
    emit updateListPage();
    if(notifyListDeleteCompleted)
    {
        qDebug() << "*******************************************************************************";
        qDebug() << "EMITTING THE SIGNAL";
        emit listDeleteCompleted();
    }
}

void DocumentListModel::handleRowsAboutToBeMoved(const QModelIndex &index, int start, int end,
                                               const QModelIndex &dest, int destIndex)
{
    Q_UNUSED(index);
    qDebug() << __PRETTY_FUNCTION__;
    beginMoveRows(QModelIndex(), start, end, dest, destIndex);

}

void DocumentListModel::handleRowsMoved(const QModelIndex &startIndex,
                                       int start, int end, const QModelIndex &destIndex, int dest)
{
    Q_UNUSED(startIndex);
    Q_UNUSED(destIndex);
    Q_UNUSED(start);
    Q_UNUSED(dest);
    Q_UNUSED(end);
    qDebug() << __PRETTY_FUNCTION__;
    recalculateGroups();
    endMoveRows();
}

void DocumentListModel::handleRowsAboutToBeInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    qDebug() << __PRETTY_FUNCTION__;
    beginInsertRows(QModelIndex(), start, end, false);
}

void DocumentListModel::handleRowsInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index);
    Q_UNUSED(start);
    Q_UNUSED(end);
    qDebug() << __PRETTY_FUNCTION__;
    recalculateGroups();
    endInsertRows();
    emit updateListPage();
}

int DocumentListModel::groupCount() const
{
    return groups.count();
}

int DocumentListModel::rowCountInGroup(int group) const
{
    if(group < groupsSize.count() && group >= 0)
        return groupsSize[group];
    else if(group == -1)
        return liveQuery->model()->rowCount();

    return 0;
}

QString DocumentListModel::groupTitle(int group) const
{
    if(group < groups.count())
        return groups[group];

    return QString();
}

bool DocumentListModel::groupLessThan(QModelIndex left, QModelIndex right)
{
    QString leftText = left.data().toString();
    QString rightText = right.data().toString();

    if(currentGrouping == GroupByType) {
        if(leftText == qtTrId("qtn_offi_favorites"))
            return true;

        if(rightText == qtTrId("qtn_offi_favorites"))
            return false;

        return (leftText.localeAwareCompare(rightText) < 0);
    } else if(currentGrouping == GroupByTime) {
        QDateTime leftDate, rightDate;
        QMapIterator<QString, TimeGroupLimitsEntry> iter(timeGroupLimits);

        while(iter.hasNext()) {
            iter.next();

            if(iter.key() == leftText) {
                leftDate = iter.value().end;
                break;
            }
        }

        iter.toFront();

        while(iter.hasNext()) {
            iter.next();

            if(iter.key() == rightText) {
                rightDate = iter.value().end;
                break;
            }
        }

        return (leftDate < rightDate);
    } else {
        // if the documentName is used all the time the result is not correct.
        if (leftText == rightText) {
            // compare the document name in case the first letter is the same
            leftText = documentName(left.parent().isValid() ? left.parent().row() : -1 , left.row());
            rightText = documentName(right.parent().isValid() ? right.parent().row() : -1 , right.row());
        }

        return (leftText.localeAwareCompare(rightText) < 0);
    }

    return true;
}

QVariant DocumentListModel::itemData(int row, int group, int role) const
{
    int flatRow = row;

    if (group >= 0 && row >= 0) {
        flatRow = rowToGroupReference.values(group).at(row);
    }

    Q_ASSERT(flatRow >= 0);
    Q_ASSERT(flatRow < liveQuery->model()->rowCount());

    static QFileInfo fileInfo;
    QModelIndex index = liveQuery->model()->index(flatRow, 0);
    fileInfo.setFile(QUrl::fromPercentEncoding((index.data().toString()).toUtf8()));

    if(role == Qt::DisplayRole) {
        DocumentListEntry entry;
        entry.url = index.data().toString();
        entry.isFavorite = !(index.sibling(flatRow, 3).data().toString().isNull());
        entry.documentType = Misc::getFileTypeFromMime(index.sibling(flatRow, 2).data().toString(), fileInfo.suffix());
        entry.documentCat = getDocumentCategory(entry.documentType);
        entry.documentName = fileInfo.completeBaseName();
//        entry.lastAccessed = index.sibling(flatRow, 1).data().toDateTime();
        return QVariant::fromValue(entry);
    }
    else if(role == DocumentListNameRole)
        return QVariant::fromValue(fileInfo.completeBaseName());
    else if(role == DocumentListAccessTimeRole)
        return QVariant::fromValue(index.sibling(flatRow, 1).data().toDateTime());
    else if(role == DocumentListTypeRole)
        return QVariant::fromValue(Misc::getFileTypeFromMime(index.sibling(flatRow, 2).data().toString(), fileInfo.suffix()));
    else if(role == DocumentListLiveFilterRole)
        return QVariant::fromValue(fileInfo.completeBaseName() + "\n" +
                                   qtTrId(Misc::getFileTypeFromMime(index.sibling(flatRow, 2).data().toString(),
                                                                    fileInfo.suffix()).toLatin1().data()));
    return QVariant();
}

void DocumentListModel::updateData(const QModelIndex &first, const QModelIndex &last)
{
    emit dataChanged(first, last);
}
void DocumentListModel::clearGroups()
{
    groups.clear();
    groupsSize.clear();

    rowToGroupReference.clear();
}

QString DocumentListModel::createTimeStampGroups(const QDateTime &laDate)
{
    const int toPresent = laDate.daysTo(QDateTime::currentDateTime());
    static QDateTime currentDate = QDateTime::currentDateTime();
    static MLocale currentLocale;
    static MCalendar calendar(currentLocale);
    calendar.setDateTime(QDateTime::currentDateTime());
    TimeGroupLimitsEntry entry;

    if(toPresent < 1) {
        entry.title = qtTrId("qtn_comm_time_today");

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date(),QTime(0,0));
            entry.end = QDateTime(currentDate.date(),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 2) {
        entry.title = qtTrId("qtn_comm_time_yesterday");

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(-1),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(-1),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 7) {
        entry.title = qtTrId("qtn_comm_time_day_ago", toPresent);

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(-toPresent),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(-toPresent),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 14) {
        entry.title = qtTrId("qtn_comm_time_last_week");

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(-13),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(-7),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 31) {
        int numberOfWeeks = static_cast<int>(toPresent/WEEK + 0.5);
        entry.title = qtTrId("qtn_comm_time_week_ago", numberOfWeeks);

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(numberOfWeeks * (WEEK+1) * -1),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(numberOfWeeks * WEEK * -1),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 61) {
        entry.title = qtTrId("qtn_comm_time_last_month");

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(-60),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(-31),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 181) {
        int numberOfMonths = static_cast<int>(toPresent/MONTH + 0.5);
        entry.title = qtTrId("qtn_comm_time_month_ago", numberOfMonths);

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(numberOfMonths * (MONTH+1) * -1),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(numberOfMonths * MONTH * -1),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else if(toPresent < 365) {
        entry.title = qtTrId("qtn_comm_time_this_year");

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(-360),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(-181),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
    else {
        int numberOfYears = static_cast<int>(toPresent/YEAR + 0.5);
        entry.title = qtTrId("qtn_comm_time_year_ago", numberOfYears);

        if(!timeGroupLimits.contains(entry.title)) {
            entry.start = QDateTime(currentDate.date().addDays(numberOfYears * (YEAR+1) * -1),QTime(0,0));
            entry.end = QDateTime(currentDate.date().addDays(numberOfYears * YEAR * -1),QTime(23,59,59,999));
            timeGroupLimits.insert(entry.title, entry);
        }

        return entry.title;
    }
}

void DocumentListModel::makeNameGroups()
{
    clearGroups();

    QAbstractItemModel *sourceModel = liveQuery->model();
    int row = 0;
    QModelIndex index = sourceModel->index(row, 4);
    while (index.isValid()) {
        QString fileName = index.data().toString();

        QString group(fileName.at(0).toUpper());

        if(!groups.contains(group)) {
            groups.append(group);
            groupsSize.append(1);
        } else {
            groupsSize[groups.indexOf(group)] ++ ;
        }
        rowToGroupReference.insertMulti(groups.indexOf(group), row);
        index = index.sibling(++row, 4);
    }
}
void DocumentListModel::makeTimeGroups()
{
    clearGroups();
    timeGroupLimits.clear();

    QAbstractItemModel *sourceModel = liveQuery->model();
    int row = 0;
    QModelIndex index = sourceModel->index(0, 0);
    while (index.isValid()) {
        QString group(createTimeStampGroups(sourceModel->index(row, 1).data().toDateTime()));

        if (!groups.contains(group)) {
            groups.append(group);
            groupsSize.append(1);
        } else {
            groupsSize[groups.indexOf(group)] ++;
        }
        rowToGroupReference.insertMulti(groups.indexOf(group), row);
        index = index.sibling(++row, 0);
    }
}
void DocumentListModel::makeTypeGroups()
{
    clearGroups();

    QAbstractItemModel *sourceModel = liveQuery->model();
    int row = 0;
    QModelIndex index = sourceModel->index(0, 2);
    while (index.isValid()) {
        QString group(documentCatString(getDocumentCategory(Misc::getFileTypeFromMime(index.data().toString())),
                                        !(index.sibling(row, 3).data().toString().isNull())));

        if(!groups.contains(group)) {
            groups.append(group);
            groupsSize.append(1);
        } else {
            groupsSize[groups.indexOf(group)] ++ ;
        }
        rowToGroupReference.insertMulti(groups.indexOf(group), row);
        index = index.sibling(++row, 2);
    }
}
void DocumentListModel::setCurrentGrouping(DocumentListModel::DocumentListGroups group)
{
    currentGrouping = group;
    recalculateGroups();
}
void DocumentListModel::recalculateGroups()
{
    beginResetModel();

    switch(currentGrouping) {
    case NoGroup:
        clearGroups();
        break;
    case GroupByName:
        makeNameGroups();
        break;
    case GroupByType:
        makeTypeGroups();
        break;
    case GroupByTime:
        makeTimeGroups();
        break;
    }

    endResetModel();
#ifdef DOCUMENTLISTMODEL_DEBUG
    qDebug() << " Now lets check what is there in groups for grouping style " << currentGrouping;

    for(int i = 0 ; i < groups.count(); i++) {
        qDebug() << "Group "<< i<< " Title " << groups[i] <<" size " << groupsSize[i];

    }

#endif
}

QString DocumentListModel::documentCatString(int cat, bool isFavorite) const
{
    if (isFavorite) {
        return qtTrId("qtn_offi_favorites");
    }

    switch(cat) {
    case DOCUMENT:
    case OPEN_DOC_TEXT:
    case TEXT:
        return qtTrId("qtn_offi_text_documents");
    case PRESENTATION:
    case OPEN_DOC_PPT:
        return qtTrId("qtn_offi_presentations");
    case SPREADSHEET:
    case OPEN_DOC_XLS:
        return qtTrId("qtn_offi_spreadsheets");
    case PDF:
        return qtTrId("qtn_offi_pdf_documents");
    }

    return "UNKNOWN";
}

int DocumentListModel::getDocumentCategory(QString const &documentType) const
{
    if( (QString::compare("qtn_comm_filetype_doc", documentType) == 0) ||
        (QString::compare("qtn_comm_filetype_rtf", documentType) == 0) ||
        (QString::compare("qtn_comm_filetype_docx",documentType) == 0)) {
        return DOCUMENT;
    } else if(QString::compare("qtn_comm_filetype_odt", documentType) == 0) {
        return OPEN_DOC_TEXT;
    } else if(QString::compare("qtn_comm_filetype_txt", documentType) == 0) {
        return TEXT;
    } else if(QString::compare("qtn_comm_filetype_pdf", documentType) == 0) {
        return PDF;
    } else if(
        (QString::compare("qtn_comm_filetype_ppt", documentType) == 0) ||
        (QString::compare("qtn_comm_filetype_pps", documentType) == 0) ||
        (QString::compare("qtn_comm_filetype_pptx",documentType) == 0)||
        (QString::compare("qtn_comm_filetype_ppsx",documentType) == 0)) {
        return PRESENTATION;
    } else if(QString::compare("qtn_comm_filetype_odp", documentType) == 0) {
        return OPEN_DOC_PPT;
    } else if(
        (QString::compare("qtn_comm_filetype_xls", documentType) == 0) ||
        (QString::compare("qtn_comm_filetype_xlsx",documentType) == 0)) {
        return SPREADSHEET;
    } else if(QString::compare("qtn_comm_filetype_ods", documentType) == 0) {
        return OPEN_DOC_XLS;
    }

    return UNKNOWNTYPE;
}

QString DocumentListModel::documentUri(QModelIndex &index) const
{
    qDebug() << __PRETTY_FUNCTION__ << " Index is valid = " << index.isValid();

    if (index.isValid()) {
        return index.sibling(index.row(), 5).data().toString();
    }

    return QString();
}
QString DocumentListModel::documentUri(int group, int row) const
{
    int flatRow = rowToGroupReference.values(group)[row];

    if (flatRow >= 0) {
        return liveQuery->model()->index(flatRow, 5).data().toString();
    }

    return QString();
}
QString DocumentListModel::documentName(int group, int row) const
{
    int flatRow = rowToGroupReference.values(group)[row];

    if (flatRow >= 0) {
        static QFileInfo fileInfo;
        fileInfo.setFile(liveQuery->model()->index(flatRow, 0).data().toString());
        return fileInfo.completeBaseName();
    }

    return QString();
}
QString DocumentListModel::documentPath(int group, int row) const
{
    int flatRow = rowToGroupReference.values(group)[row];

    if (flatRow >= 0) {
        return liveQuery->model()->index(flatRow, 0).data().toString();
    }

    return QString();
}

bool DocumentListModel::documentIsFavorite(int group, int row) const
{
    int flatRow = rowToGroupReference.values(group)[row];

    if (flatRow >= 0) {
        return !(liveQuery->model()->index(flatRow, 3).data().toString().isNull());
    }

    return false;
}
bool DocumentListModel::documentIsFavorite(QString uri)
{
    if(uri.isEmpty())
        return false;

    return TrackerUtils::Instance().isItemMarkedAsFavorite(uri);
}

void DocumentListModel::setFavourite(int group, int row) const
{
    setFavourite(documentUri(group, row));
}
void DocumentListModel::setFavourite(QString uri)
{
    if(uri.isEmpty())
        return;
    qDebug() << __PRETTY_FUNCTION__ << uri;

    if(documentIsFavorite(uri)) {
        TrackerUtils::Instance().unmarkItemAsFavorite(uri);
    } else {
        TrackerUtils::Instance().markItemAsFavorite(uri);
    }
}

void DocumentListModel::notifyOnDeleteFinished(QStringList list)
{
    pathsToMonitor = list;
    qDebug() << "PATHS TO MONITOR " << pathsToMonitor;
}
