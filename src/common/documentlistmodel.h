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

#ifndef DOCUMENTLISTMODEL_H
#define DOCUMENTLISTMODEL_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QDateTime>
#include <QVector>

#include <TrackerLiveQuery>

#include <MAbstractItemModel>
#include <common_export.h>

// Structure which contain data for each row
struct COMMON_EXPORT DocumentListEntry {
    QString uri;
    QString url;
    QString documentName;
    QString documentType;
    QDateTime lastAccessed;
    bool isFavorite;
    int documentCat;
    QPixmap *icon;
    QVariant cell;
};

Q_DECLARE_METATYPE(DocumentListEntry)

class COMMON_EXPORT DocumentListModel: public MAbstractItemModel
{
    Q_OBJECT

public:
    // Defining roles here which will be used for sorting and filtering in DocumentListSortedModel
    enum DocumentListRoles {
        DocumentListNameRole = Qt::UserRole + 1,
        DocumentListAccessTimeRole,
        DocumentListTypeRole,
        DocumentListFavRole,
        DocumentListLiveFilterRole,
    };

    enum DocumentListGroups {
        NoGroup,
        GroupByName,
        GroupByTime,
        GroupByType
    };
    typedef enum {
        UNKNOWNTYPE  = 0,
        DOCUMENT     = 1,
        PRESENTATION = 2,
        SPREADSHEET  = 3,
        PDF          = 4,
        OPEN_DOC_PPT = 5,
        OPEN_DOC_TEXT= 6,
        OPEN_DOC_XLS = 7,
        TEXT         = 8,
        FAVORITE     = 9
    } DocumentCategory;

    DocumentListModel();
    virtual ~DocumentListModel();

    int groupCount() const;
    int rowCountInGroup(int group) const;
    QString groupTitle(int group) const;
    bool groupLessThan(QModelIndex left, QModelIndex right);
    QVariant itemData(int row, int group, int role) const;

    QString documentUri(int group, int row) const;
    QString documentPath(int group, int row) const;
    QString documentName(int group, int row) const;
    void    setFavourite(int group, int row) const;
    bool    documentIsFavorite(int group, int row) const;
    QString documentUri(QModelIndex &index) const;

    void updateData(const QModelIndex &first, const QModelIndex &last);
    void makeTimeGroupLimits();
    void clearGroups();
    void makeNameGroups();
    void makeTypeGroups();
    void makeTimeGroups();
    void setCurrentGrouping(DocumentListModel::DocumentListGroups group);
    void recalculateGroups();
    void notifyOnDeleteFinished(QStringList list);

    QAbstractItemModel *model() const
    {
        return liveQuery->model();
    }

    static bool documentIsFavorite(QString uri);
    static void setFavourite(QString uri);

protected slots:
    virtual void handleRowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    virtual void handleRowsAboutToBeInserted(const QModelIndex &index, int start, int end);
    virtual void handleRowsAboutToBeMoved(const QModelIndex &index, int start, int end,
                                  const QModelIndex &dest, int destIndex);

    virtual void handleRowsRemoved(const QModelIndex &index, int start, int end);
    virtual void handleRowsMoved(const QModelIndex &startIndex, int start, int end,
                         const QModelIndex &destIndex, int dest);
    virtual void handleRowsInserted(const QModelIndex &index, int start, int end);

    // Methods to translate signals to current model
    virtual void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);


private:
    struct TimeGroupLimitsEntry {
        QString title;
        QDateTime start;
        QDateTime end;
    };
    QHash<int, int> rowToGroupReference;
    QMap <QString, TimeGroupLimitsEntry> timeGroupLimits;

    DocumentListGroups currentGrouping;
    QList<QString> groups;
    QList<int> groupsSize;
    TrackerLiveQuery *liveQuery;
    bool documentsUpdated;
    QStringList pathsToMonitor;
    bool notifyListDeleteCompleted;

    int getDocumentCategory(const QString &documentType) const;
    QString documentCatString(int cat, bool isFavorite) const;

    QString createTimeStampGroups(const QDateTime &laDate);

signals:
    void liveQueryFinished();
    void updateListPage();
    void listDeleteCompleted();

private slots:
    void liveModelQueryFinished();
    void resetDocumentUpdatedFlag() {
        documentsUpdated = false;
        recalculateGroups();
    }
};

#endif
