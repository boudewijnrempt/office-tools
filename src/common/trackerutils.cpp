#include "trackerutils.h"
#include "misc.h"

#include <QtSparql/QSparqlConnection>
#include <QtSparql/QSparqlResult>
#include <QtSparql/QSparqlQuery>
#include <QtSparql/QSparqlError>

#include <TrackerLiveQuery>

#include <QLatin1String>
#include <QDateTime>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>

QSharedPointer<TrackerUtils> TrackerUtils::m_instance;

TrackerUtils & TrackerUtils::Instance()
{
    if (m_instance.data() == 0) {
        // TODO should we lock here?
        m_instance = QSharedPointer<TrackerUtils>(new TrackerUtils());
    }
    return *m_instance;
}

void TrackerUtils::Shutdown()
{
     m_instance.clear();
}

TrackerUtils::TrackerUtils()
{
    m_connection = new QSparqlConnection("QTRACKER_DIRECT");
}

TrackerUtils::~TrackerUtils()
{
    delete m_connection;
}

void TrackerUtils::markItemAsFavorite(const QString& itemUrn)
{
    if (itemUrn.isEmpty()) {
        return;
    }
    qDebug() << __PRETTY_FUNCTION__;

    static QSparqlQuery query("INSERT { ?:urn nao:hasTag nao:predefined-tag-favorite }",
                              QSparqlQuery::InsertStatement);
    query.bindValue("urn", QUrl(itemUrn));

    QSparqlResult *result = m_connection->exec(query);
    connect(result, SIGNAL(finished()),
            m_instance.data(), SLOT(deleteResult()));
}

void TrackerUtils::unmarkItemAsFavorite(const QString& itemUrn)
{
    if (itemUrn.isEmpty()) {
        return;
    }
    qDebug() << __PRETTY_FUNCTION__;

    static QSparqlQuery query("DELETE { ?:urn nao:hasTag nao:predefined-tag-favorite }",
                              QSparqlQuery::DeleteStatement);
    query.bindValue("urn", QUrl(itemUrn));

    QSparqlResult *result = m_connection->exec(query);
    connect(result, SIGNAL(finished()),
            m_instance.data(), SLOT(deleteResult()));
}

bool TrackerUtils::isItemMarkedAsFavorite(const QString& itemUrn)
{
    static QSparqlQuery query("ASK { ?:urn nao:hasTag nao:predefined-tag-favorite }",
                              QSparqlQuery::AskStatement);
    query.bindValue("urn", QUrl(itemUrn));

    QSparqlResult *result = m_connection->syncExec(query);
    if (result == 0) {
        return false;
    }
    result->next();
    bool value(false);
    if (result->hasError()) {
        qWarning("Could not check URN %s for favorite status: %s",
                 itemUrn.toAscii().data(),
                 result->lastError().message().toAscii().data());
    } else {
        value = result->value(0).toBool();
    }

    delete result;
    return value;
}

void TrackerUtils::updateContentAccessedProperty(const QString& itemUrn)
{
    if (itemUrn.isEmpty()) {
        return;
    }

    // Updating is performed with combined delete+insert
    static QSparqlQuery query("DELETE { ?:urn nfo:fileLastAccessed ?date }\n"
                              "WHERE  { ?:urn nfo:fileLastAccessed ?date }\n"
                              "INSERT { ?:urn nfo:fileLastAccessed ?:now }",
                              QSparqlQuery::DeleteStatement);
    query.bindValue("now", QDateTime::currentDateTime());
    query.bindValue("urn", QUrl(itemUrn));

    QSparqlResult *result = m_connection->exec(query);
    connect(result, SIGNAL(finished()),
            m_instance.data(), SLOT(deleteResult()));
}

QUrl TrackerUtils::urlFromUrn(const QString& urn)
{
    if ( urn.isEmpty() ) {
        return QUrl();
    }

    static QSparqlQuery query("SELECT ?url WHERE { ?:urn nie:url ?url . }");
    query.bindValue("urn", QUrl(urn));

    QSparqlResult *result = m_connection->syncExec(query);

    QUrl value;
    if (result->hasError()) {
        qWarning("Could not map URN %s to an URL: %s",
                 urn.toAscii().data(),
                 result->lastError().message().toAscii().data());
    } else if (!result->first()) {
        qWarning("URN %s does not have an associated URL "
                 "(in general this should not happen)",
                 urn.toAscii().data());
    } else {
        value = result->value(0).toUrl();
    }

    delete result;
    return value;
}

QString TrackerUtils::urnFromUrl(const QUrl& url)
{
    if ( url.isEmpty() ) {
        return QString();
    }

    QString resolvedUrl = url.toEncoded();
    if (resolvedUrl.startsWith('/')) {
        resolvedUrl = resolvedUrl.insert(0, "file://");
    }

    static QSparqlQuery query("SELECT ?urn WHERE { ?urn nie:url ?:url . }");
    query.bindValue("url", resolvedUrl);
    QSparqlResult *result = m_connection->syncExec(query);

    QString value;
    if (result->hasError()) {
        qWarning("Could not map URL %s to an URN: %s",
                 url.toString().toAscii().data(),
                 result->lastError().message().toAscii().data());
    } else if (!result->first()) {
        qWarning("URL %s does not have an associated URN "
                 "(in general this should not happen)",
                 url.toString().toAscii().data());
    } else {
        value = result->value(0).toString();
    }

    delete result;
    return value;
}

QDateTime TrackerUtils::contentCreatedForUrl(const QUrl& url)
{
    QDateTime dateTime;

    if ( url.isEmpty() ) {
        return dateTime;
    }

    QString resolvedUrl = url.toEncoded();
    if ( 0 == url.scheme().length() ) {
        resolvedUrl = resolvedUrl.insert(0, "file://");
    }

    static QSparqlQuery query("SELECT ?created { ?urn nie:url ?:url .\n"
                              "?urn nfo:fileLastAccessed ?created }");
    query.bindValue("url", resolvedUrl);

    QSparqlResult *result = m_connection->syncExec(query);

    if (result->hasError()) {
        qWarning("Could not query URL %s for nfo:fileLastAccessed: %s",
                 url.toString().toAscii().data(),
                 result->lastError().message().toAscii().data());
    } else if (!result->first()) {
        qWarning("URL %s does not have an associated "
                 "nfo:fileLastAccessed property "
                 "(in general this should not happen)",
                 url.toString().toAscii().data());
    } else {
        dateTime = result->value(0).toDateTime();
    }

    delete result;
    return dateTime;
}

QSparqlResult * TrackerUtils::doInitialTrackerQuery(bool waitForFinish)
{
    static QSparqlQuery query("SELECT DISTINCT ?e nie:url(?e) nfo:fileLastAccessed(?e)" \
                       "nie:mimeType(?e) nao:hasTag(?e) WHERE { { ?e a nfo:Presentation }" \
                       "UNION { ?e a nfo:Spreadsheet } UNION { ?e a nfo:TextDocument} }");

    //Wait for finished Since this function will be executed in ASync thread
    QSparqlResult *result;
    if (waitForFinish) {
        result = m_connection->syncExec(query);
    } else {
        result = m_connection->exec(query);
    }

    return result;
}

void TrackerUtils::deleteUrl(const QString &url)
{
    deleteUrn(TrackerUtils::urnFromUrl(QUrl(url)));
}

void TrackerUtils::deleteUrn(const QString &urn)
{
    if (urn.isEmpty()) {
        return;
    }

    static QSparqlQuery query("DELETE { ?:urn a rdfs:Resource . }");
    query.bindValue("urn", QUrl(urn));

    QSparqlResult *result = m_connection->exec(query);
    connect(result, SIGNAL(finished()),
            m_instance.data(), SLOT(deleteResult()));
}

DocumentDetails * TrackerUtils::documentDetailsFromUrl(const QString &url)
{
    QString urn = urnFromUrl(QUrl(url));
    if (urn.isEmpty()) {
        return 0;
    }

    DocumentDetails *details = 0;

    static QSparqlQuery query("SELECT ?mt ?la ?bs ?add ?sb ?cr ?pub WHERE  { ?:urn a nfo:Document; nie:mimeType ?mt ; "\
                       "nfo:fileLastAccessed ?la ; nie:byteSize ?bs ; tracker:added ?add ." \
                       "OPTIONAL {  ?:urn nie:subject ?sb }"   \
                       "OPTIONAL {  ?:urn nco:creator ?aut ."  \
                                    "?aut nco:fullname ?cr }"  \
                       "OPTIONAL {  ?:urn nco:publisher ?nm ." \
                                    "?nm nco:fullname ?pub }"  \
                        "}");
    query.bindValue("urn", QUrl(urn));
    QSparqlResult *result = m_connection->syncExec(query);

    if (result->hasError()) {
        qWarning("Could not query %s for Document details - %s",
                 url.toAscii().data(),
                 result->lastError().message().toAscii().data());
    } else if (!result->first()) {
        qWarning (" %s -- Did not return any data",
                  query.query().toAscii().data());
    } else {
        details = new DocumentDetails();
        details->mimeType = result->binding(0).value().toString();
        details->lastAccessed = result->binding(1).value().toDateTime();
        details->size = result->binding(2).value().toDouble();
        details->created = result->binding(3).value().toDateTime();
        details->subject = result->binding(4).value().toString();
        details->author = result->binding(5).value().toString();
        details->publisher = result->binding(6).value().toString();
    }

    delete result;
    return details;
}

bool TrackerUtils::isDocumentEncrypted(const QString &url)
{
    bool value = false;
    QString resolvedUrl = url;
    if ( !url.startsWith("file://", Qt::CaseInsensitive) ) {
        resolvedUrl = resolvedUrl.insert(0, "file://");
    }

    static QSparqlQuery query("SELECT ?enc { ?urn nie:url ?:url .\n"
                              "?urn nfo:isContentEncrypted ?enc }");
    query.bindValue("url", resolvedUrl);

    QSparqlResult *result = m_connection->syncExec(query);

    if (result->hasError()) {
        qWarning("Could not query Encryption details for %s -- Error Occured %s ",
                 url.toAscii().data(),
                 result->lastError().message().toAscii().data());
    } else if (!result->first()) {
        qWarning("Did not find %s in tracker database - %s ",
                 url.toAscii().data(),
                 query.preparedQueryText().toAscii().data());
    } else {
        value = (result->binding(0).value().isNull())?false:true;
    }

    delete result;
    return value;
}

void TrackerUtils::deleteResult()
{
    QSparqlResult *result(qobject_cast<QSparqlResult*>(sender()));
    if (result == 0)
        return;

    if (result->hasError()) {
        qWarning("Error executing query %s - %s",
                 result->query().toAscii().data(),
                 result->lastError().message().toAscii().data());
    }
    result->deleteLater();
}

TrackerLiveQuery * TrackerUtils::createTrackerLiveQuery()
{
#if 0
    //This query includes plain/text file with extensions other than txt.
    QString mainQuery("SELECT DISTINCT nie:url(?urn) AS ?url nfo:fileLastAccessed(?urn) AS ?la nie:mimeType(?urn) AS ?mimetype "
                      " ?fav nfo:fileName(?urn) AS ?filename ?urn  tracker:id(?urn) AS ?trackerid"
                      "WHERE { { ?urn a nfo:FileDataObject } { ?urn a nfo:Presentation }   UNION { ?urn a nfo:Spreadsheet } "
                      "UNION {?urn a nfo:PaginatedTextDocument } UNION { ?urn a nfo:PlainTextDocument ; nie:mimeType ?mt . FILTER ( ?mt = \"text/plain\" ) } "
                      "OPTIONAL { ?urn nao:hasTag ?fav . FILTER(?fav = nao:predefined-tag-favorite) } ");
#endif

    QString mainQuery("SELECT DISTINCT nie:url(?urn) AS ?url nfo:fileLastAccessed(?urn) AS ?la nie:mimeType(?urn) AS ?mimetype "
                      "?fav nfo:fileName(?urn) AS ?filename ?urn  tracker:id(?urn) AS ?trackerid WHERE { "
                      "{ ?urn a nfo:FileDataObject } { ?urn a nfo:PaginatedTextDocument } UNION { ?urn a nfo:PlainTextDocument } UNION {?urn a nfo:Presentation } "
                      "{ ?urn a nfo:Document ; nfo:fileName ?fn . "
                      "FILTER regex(?fn, \"\\\\.txt$|\\\\.ppt$|\\\\.odp$|\\\\.pptx$|\\\\.pps$|\\\\.ppsx$|\\\\.doc$|\\\\.pdf$|\\\\.xls$|\\\\.docx$|\\\\.odt$|\\\\.xlsx$|\\\\.ods$\", \"i\" ) } "
                      "OPTIONAL { ?urn nao:hasTag ?fav . FILTER(?fav = nao:predefined-tag-favorite) } ");

    QString updateQuery(mainQuery);
    updateQuery += "  %FILTER } ORDER BY ?mimetype";

    mainQuery += " } ORDER BY ?mimetype";

    QSparqlQuery mainSparqlQuery(mainQuery);
    TrackerLiveQuery *liveQuery = new TrackerLiveQuery(mainSparqlQuery, 7, (QList<int>() << 6), *m_connection);

    liveQuery->setCollationColumns(QList<TrackerLiveQuery::CollationColumn>()
                                  << TrackerLiveQuery::CollationColumn(2, QVariant::String, Qt::AscendingOrder));

    TrackerPartialUpdater updater(updateQuery);
    QStringList documentPredicates;
    documentPredicates << "nfo:fileLastAccessed" << "nao:hasTag" << "rdf:type";
    updater.watchClass("nfo:Document",
                       documentPredicates,
                       "tracker:id(?urn) in %LIST",
                       TrackerPartialUpdater::Subject,
                       6);

    liveQuery->addUpdater(updater);
    liveQuery->start();

    qDebug("%s -> QUERY\n%s\n", Q_FUNC_INFO,
           qPrintable(mainSparqlQuery.preparedQueryText()));

    return liveQuery;
}

TrackerLiveQuery * TrackerUtils::createDocumentLiveUpdate(const QUrl &url)
{
    if ( url.isEmpty() ) {
        return 0;
    }

    QString resolvedUrl = url.toEncoded();
    if ( 0 == url.scheme().length() ) {
        resolvedUrl = resolvedUrl.insert(0, "file://");
    }


    QString mainQuery("SELECT tracker:id(?urn) ?fav WHERE{ ?urn nie:url ?:url . "
                      " OPTIONAL { ?urn nao:hasTag ?fav . FILTER(?fav = nao:predefined-tag-favorite)} ");
    QString updateQuery(mainQuery);
    updateQuery += "  %FILTER } ";

    mainQuery += " } ";

    QSparqlQuery mainSparqlQuery(mainQuery);
    mainSparqlQuery.bindValue("url", resolvedUrl);
    TrackerLiveQuery *liveQuery = new TrackerLiveQuery(mainSparqlQuery, 2, (QList<int>() << 0),  *m_connection);

    QSparqlQuery updateSparqlQuery(updateQuery);
    updateSparqlQuery.bindValue("url", resolvedUrl);
    TrackerPartialUpdater updater(updateSparqlQuery.preparedQueryText());
    QStringList documentPredicates;
    documentPredicates << "nao:hasTag" << "rdf:type";
    updater.watchClass("nfo:Document",
                       documentPredicates,
                       "tracker:id(?urn) in %LIST",
                       TrackerPartialUpdater::Subject,
                       0);

    liveQuery->addUpdater(updater);
    liveQuery->start();

    qDebug("%s -> QUERY\n%s\n", Q_FUNC_INFO,
           qPrintable(mainSparqlQuery.preparedQueryText()));

    return liveQuery;
}
