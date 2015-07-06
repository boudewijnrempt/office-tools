/// Lifted from Gallery Core sources.

#ifndef TRACKERUTILS_H
#define TRACKERUTILS_H

#include <QString>
#include <QObject>

#include "documentlistmodel.h"
#include <common_export.h>

class QSparqlConnection;
class QSparqlResult;
class QDateTime;
class QUrl;
class TrackerLiveQuery;

struct DocumentDetails {
    QString   mimeType;
    QDateTime lastAccessed;
    double    size;
    QDateTime created;
    QString   subject;
    QString   publisher;
    QString   author;
};

//! \class TrackerUtils
//! \brief TrackerUtils contains static utility functions for interacting with tracker
//!
class COMMON_EXPORT TrackerUtils: public QObject
{
    Q_OBJECT

public:
    static TrackerUtils &Instance();

    static void Shutdown();

    ~TrackerUtils();

    //! Adds favorite tag to item
    //! \param itemUrn Urn to the item that is marked as favorite.
    void markItemAsFavorite(const QString& itemUrn);

    //! Removes favorite tag from item
    //! \param itemUrn Urn to the item that is unmarked as favorite
    void unmarkItemAsFavorite(const QString& itemUrn);

    //! Checks whether item is marked as favorite
    //! \param itemUrn Urn to the item
    //! \return True if item is marked as favorite, otherwise false
    bool isItemMarkedAsFavorite(const QString& itemUrn);

    //! Updates nie::contentAccessed property in tracker with current date and time
    //! \param itemUrn Urn to the item that is to be updated
    void updateContentAccessedProperty(const QString& itemUrn);

    //! Returns url from urn
    //! \param urn Urn of item
    //! \return Url of the item
    QUrl urlFromUrn(const QString& urn);

    //! Returns urn from url
    //! \param url Url of item
    //! \return Urn of the item
    QString urnFromUrl(const QUrl& url);

    //! Returns the nie:contentCreated property for a given URL
    //! \param url URL of the item
    //! \return Content Created date of the item. QDateTime::isNull is true,
    //!    if the contentCreated property was unavailable.
    QDateTime contentCreatedForUrl(const QUrl& url);

    QSparqlResult * doInitialTrackerQuery(bool waitForFinish = false);

    void deleteUrn(const QString& urn);

    void deleteUrl(const QString& url);

    DocumentDetails * documentDetailsFromUrl(const QString &url);

    bool isDocumentEncrypted(const QString& url);

    TrackerLiveQuery * createTrackerLiveQuery();

    TrackerLiveQuery * createDocumentLiveUpdate(const QUrl &url);

private Q_SLOTS:
    //! Deletes a QSparqlResult that was left running asynchronously.
    void deleteResult();

private:
    static QSharedPointer<TrackerUtils> m_instance;

    //! Private defauult constructor, to disallow instantiation.
    TrackerUtils();

    //! Instance, needed to get signals delivered to the slot above.
    QSparqlConnection *m_connection;

};

#endif // TRACKERUTILS_H
