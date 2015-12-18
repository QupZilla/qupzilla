/*
 * Copyright 2009 Jakub Wieczorek <faw217@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */
/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */

#include "opensearchengine.h"
#include "qzregexp.h"
#include "opensearchenginedelegate.h"

#include <qbuffer.h>
#include <qcoreapplication.h>
#include <qlocale.h>
#include <qnetworkrequest.h>
#include <qnetworkreply.h>
#include <qregexp.h>
#include <qstringlist.h>

#include <QUrlQuery>
#include <QJsonDocument>


/*!
    \class OpenSearchEngine
    \brief A class representing a single search engine described in OpenSearch format

    OpenSearchEngine is a class that represents a single search engine based on
    the OpenSearch format.
    For more information about the format, see http://www.opensearch.org/.

    Instances of the class hold all the data associated with the corresponding search
    engines, such as name(), description() and also URL templates that are used
    to construct URLs, which can be used later to perform search queries. Search engine
    can also have an image, even an external one, in this case it will be downloaded
    automatically from the network.

    OpenSearchEngine instances can be constructed from scratch but also read from
    external sources and written back to them. OpenSearchReader and OpenSearchWriter
    are the classes provided for reading and writing OpenSearch descriptions.

    Default constructed engines need to be filled with the necessary information before
    they can be used to peform search requests. First of all, a search engine should have
    the metadata including the name and the description.
    However, the most important are URL templates, which are the construction of URLs
    but can also contain template parameters, that are replaced with corresponding values
    at the time of constructing URLs.

    There are two types of URL templates: search URL template and suggestions URL template.
    Search URL template is needed for constructing search URLs, which point directly to
    search results. Suggestions URL template is necessary to construct suggestion queries
    URLs, which are then used for requesting contextual suggestions, a popular service
    offered along with search results that provides search terms related to what has been
    supplied by the user.

    Both types of URLs are constructed by the class, by searchUrl() and suggestionsUrl()
    functions respectively. However, search requests are supposed to be performed outside
    the class, while suggestion queries can be executed using the requestSuggestions()
    method. The class will take care of peforming the network request and parsing the
    JSON response.

    Both the image request and suggestion queries need network access. The class can
    perform network requests on its own, though the client application needs to provide
    a network access manager, which then will to be used for network operations.
    Without that, both images delivered from remote locations and contextual suggestions
    will be disabled.

    \sa OpenSearchReader, OpenSearchWriter
*/

/*!
    Constructs an engine with a given \a parent.
*/
OpenSearchEngine::OpenSearchEngine(QObject* parent)
    : QObject(parent)
    , m_searchMethod(QLatin1String("get"))
    , m_suggestionsMethod(QLatin1String("get"))
    , m_networkAccessManager(0)
    , m_suggestionsReply(0)
    , m_delegate(0)
{
    m_requestMethods.insert(QLatin1String("get"), QNetworkAccessManager::GetOperation);
    m_requestMethods.insert(QLatin1String("post"), QNetworkAccessManager::PostOperation);
}

/*!
    A destructor.
*/
OpenSearchEngine::~OpenSearchEngine()
{
}

QString OpenSearchEngine::parseTemplate(const QString &searchTerm, const QString &searchTemplate)
{
    QString language = QLocale().name();
    // Simple conversion to RFC 3066.
    language = language.replace(QLatin1Char('_'), QLatin1Char('-'));

    QString result = searchTemplate;
    result.replace(QLatin1String("{count}"), QLatin1String("20"));
    result.replace(QLatin1String("{startIndex}"), QLatin1String("0"));
    result.replace(QLatin1String("{startPage}"), QLatin1String("0"));
    result.replace(QLatin1String("{language}"), language);
    result.replace(QLatin1String("{inputEncoding}"), QLatin1String("UTF-8"));
    result.replace(QLatin1String("{outputEncoding}"), QLatin1String("UTF-8"));
    result.replace(QzRegExp(QLatin1String("\\{([^\\}]*:|)source\\??\\}")), QCoreApplication::applicationName());
    result.replace(QLatin1String("{searchTerms}"), QLatin1String(QUrl::toPercentEncoding(searchTerm)));

    return result;
}

/*!
    \property OpenSearchEngine::name
    \brief the name of the engine

    \sa description()
*/
QString OpenSearchEngine::name() const
{
    return m_name;
}

void OpenSearchEngine::setName(const QString &name)
{
    m_name = name;
}

/*!
    \property OpenSearchEngine::description
    \brief the description of the engine

    \sa name()
*/
QString OpenSearchEngine::description() const
{
    return m_description;
}

void OpenSearchEngine::setDescription(const QString &description)
{
    m_description = description;
}

/*!
    \property OpenSearchEngine::searchUrlTemplate
    \brief the template of the search URL

    \sa searchUrl(), searchParameters(), suggestionsUrlTemplate()
*/
QString OpenSearchEngine::searchUrlTemplate() const
{
    return m_searchUrlTemplate;
}

void OpenSearchEngine::setSearchUrlTemplate(const QString &searchUrlTemplate)
{
    m_searchUrlTemplate = searchUrlTemplate;
}

/*!
    Constructs and returns a search URL with a given \a searchTerm.

    The URL template is processed according to the specification:
    http://www.opensearch.org/Specifications/OpenSearch/1.1#OpenSearch_URL_template_syntax

    A list of template parameters currently supported and what they are replaced with:
    \table
    \header \o parameter
            \o value
    \row    \o "{count}"
            \o "20"
    \row    \o "{startIndex}"
            \o "0"
    \row    \o "{startPage}"
            \o "0"
    \row    \o "{language}"
            \o "the default language code (RFC 3066)"
    \row    \o "{inputEncoding}"
            \o "UTF-8"
    \row    \o "{outputEncoding}"
            \o "UTF-8"
    \row    \o "{*:source}"
            \o "application name, QCoreApplication::applicationName()"
    \row    \o "{searchTerms}"
            \o "the string supplied by the user"
    \endtable

    \sa searchUrlTemplate(), searchParameters(), suggestionsUrl()
*/
QUrl OpenSearchEngine::searchUrl(const QString &searchTerm) const
{
    if (m_searchUrlTemplate.isEmpty()) {
        return QUrl();
    }

    QUrl retVal = QUrl::fromEncoded(parseTemplate(searchTerm, m_searchUrlTemplate).toUtf8());

    QUrlQuery query(retVal);
    if (m_searchMethod != QLatin1String("post")) {
        Parameters::const_iterator end = m_searchParameters.constEnd();
        Parameters::const_iterator i = m_searchParameters.constBegin();
        for (; i != end; ++i) {
            query.addQueryItem(i->first, parseTemplate(searchTerm, i->second));
        }
        retVal.setQuery(query);
    }

    return retVal;
}

QByteArray OpenSearchEngine::getPostData(const QString &searchTerm) const
{
    if (m_searchMethod != QLatin1String("post")) {
        return QByteArray();
    }

    QUrl retVal = QUrl("http://foo.bar");

    QUrlQuery query(retVal);
    Parameters::const_iterator end = m_searchParameters.constEnd();
    Parameters::const_iterator i = m_searchParameters.constBegin();
    for (; i != end; ++i) {
        query.addQueryItem(i->first, parseTemplate(searchTerm, i->second));
    }
    retVal.setQuery(query);

    QByteArray data = retVal.toEncoded(QUrl::RemoveScheme);
    return data.contains('?') ? data.mid(data.lastIndexOf('?') + 1) : QByteArray();
}

/*!
    \property providesSuggestions
    \brief indicates whether the engine supports contextual suggestions
*/
bool OpenSearchEngine::providesSuggestions() const
{
    return (!m_suggestionsUrlTemplate.isEmpty() || !m_preparedSuggestionsUrl.isEmpty());
}

/*!
    \property OpenSearchEngine::suggestionsUrlTemplate
    \brief the template of the suggestions URL

    \sa suggestionsUrl(), suggestionsParameters(), searchUrlTemplate()
*/
QString OpenSearchEngine::suggestionsUrlTemplate() const
{
    return m_suggestionsUrlTemplate;
}

void OpenSearchEngine::setSuggestionsUrlTemplate(const QString &suggestionsUrlTemplate)
{
    m_suggestionsUrlTemplate = suggestionsUrlTemplate;
}

/*!
    Constructs a suggestions URL with a given \a searchTerm.

    The URL template is processed according to the specification:
    http://www.opensearch.org/Specifications/OpenSearch/1.1#OpenSearch_URL_template_syntax

    See searchUrl() for more information about processing template parameters.

    \sa suggestionsUrlTemplate(), suggestionsParameters(), searchUrl()
*/
QUrl OpenSearchEngine::suggestionsUrl(const QString &searchTerm) const
{
    if (!m_preparedSuggestionsUrl.isEmpty()) {
        QString s = m_preparedSuggestionsUrl;
        s.replace(QLatin1String("%s"), searchTerm);
        return QUrl(s);
    }

    if (m_suggestionsUrlTemplate.isEmpty()) {
        return QUrl();
    }

    QUrl retVal = QUrl::fromEncoded(parseTemplate(searchTerm, m_suggestionsUrlTemplate).toUtf8());

    QUrlQuery query(retVal);
    if (m_suggestionsMethod != QLatin1String("post")) {
        Parameters::const_iterator end = m_suggestionsParameters.constEnd();
        Parameters::const_iterator i = m_suggestionsParameters.constBegin();
        for (; i != end; ++i) {
            query.addQueryItem(i->first, parseTemplate(searchTerm, i->second));
        }
        retVal.setQuery(query);
    }

    return retVal;
}

/*!
    \property searchParameters
    \brief additional parameters that will be included in the search URL

    For more information see:
    http://www.opensearch.org/Specifications/OpenSearch/Extensions/Parameter/1.0
*/
OpenSearchEngine::Parameters OpenSearchEngine::searchParameters() const
{
    return m_searchParameters;
}

void OpenSearchEngine::setSearchParameters(const Parameters &searchParameters)
{
    m_searchParameters = searchParameters;
}

/*!
    \property suggestionsParameters
    \brief additional parameters that will be included in the suggestions URL

    For more information see:
    http://www.opensearch.org/Specifications/OpenSearch/Extensions/Parameter/1.0
*/
OpenSearchEngine::Parameters OpenSearchEngine::suggestionsParameters() const
{
    return m_suggestionsParameters;
}

void OpenSearchEngine::setSuggestionsParameters(const Parameters &suggestionsParameters)
{
    m_suggestionsParameters = suggestionsParameters;
}

/*!
    \property searchMethod
    \brief HTTP request method that will be used to perform search requests
*/
QString OpenSearchEngine::searchMethod() const
{
    return m_searchMethod;
}

void OpenSearchEngine::setSearchMethod(const QString &method)
{
    QString requestMethod = method.toLower();
    if (!m_requestMethods.contains(requestMethod)) {
        return;
    }

    m_searchMethod = requestMethod;
}

/*!
    \property suggestionsMethod
    \brief HTTP request method that will be used to perform suggestions requests
*/
QString OpenSearchEngine::suggestionsMethod() const
{
    return m_suggestionsMethod;
}

void OpenSearchEngine::setSuggestionsMethod(const QString &method)
{
    QString requestMethod = method.toLower();
    if (!m_requestMethods.contains(requestMethod)) {
        return;
    }

    m_suggestionsMethod = requestMethod;
}

/*!
    \property imageUrl
    \brief the image URL of the engine

    When setting a new image URL, it won't be loaded immediately. The first request will be
    deferred until image() is called for the first time.

    \note To be able to request external images, you need to provide a network access manager,
          which will be used for network operations.

    \sa image(), networkAccessManager()
*/
QString OpenSearchEngine::imageUrl() const
{
    return m_imageUrl;
}

void OpenSearchEngine::setImageUrl(const QString &imageUrl)
{
    m_imageUrl = imageUrl;
}

void OpenSearchEngine::loadImage() const
{
    if (!m_networkAccessManager || m_imageUrl.isEmpty()) {
        return;
    }

    QNetworkReply* reply = m_networkAccessManager->get(QNetworkRequest(QUrl::fromEncoded(m_imageUrl.toUtf8())));
    connect(reply, SIGNAL(finished()), this, SLOT(imageObtained()));
}

void OpenSearchEngine::imageObtained()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (!reply) {
        return;
    }

    QByteArray response = reply->readAll();

    reply->close();
    reply->deleteLater();

    if (response.isEmpty()) {
        return;
    }

    m_image.loadFromData(response);
    emit imageChanged();
}

/*!
    \property image
    \brief the image of the engine

    When no image URL has been set and an image will be set explicitly, a new data URL
    will be constructed, holding the image data encoded with Base64.

    \sa imageUrl()
*/
QImage OpenSearchEngine::image() const
{
    if (m_image.isNull()) {
        loadImage();
    }
    return m_image;
}

void OpenSearchEngine::setImage(const QImage &image)
{
    if (m_imageUrl.isEmpty()) {
        QBuffer imageBuffer;
        imageBuffer.open(QBuffer::ReadWrite);
        if (image.save(&imageBuffer, "PNG")) {
            m_imageUrl = QString(QLatin1String("data:image/png;base64,%1"))
                         .arg(QLatin1String(imageBuffer.buffer().toBase64()));
        }
    }

    m_image = image;
    emit imageChanged();
}

/*!
    \property valid
    \brief indicates whether the engine is valid i.e. the description was properly formed and included all necessary information
*/
bool OpenSearchEngine::isValid() const
{
    return (!m_name.isEmpty() && !m_searchUrlTemplate.isEmpty());
}

bool OpenSearchEngine::operator==(const OpenSearchEngine &other) const
{
    return (m_name == other.m_name
            && m_description == other.m_description
            && m_imageUrl == other.m_imageUrl
            && m_searchUrlTemplate == other.m_searchUrlTemplate
            && m_suggestionsUrlTemplate == other.m_suggestionsUrlTemplate
            && m_searchParameters == other.m_searchParameters
            && m_suggestionsParameters == other.m_suggestionsParameters);
}

bool OpenSearchEngine::operator<(const OpenSearchEngine &other) const
{
    return (m_name < other.m_name);
}

/*!
    Requests contextual suggestions on the search engine, for a given \a searchTerm.

    If succeeded, suggestions() signal will be emitted once the suggestions are received.

    \note To be able to request suggestions, you need to provide a network access manager,
          which will be used for network operations.

    \sa requestSearchResults()
*/

void OpenSearchEngine::setSuggestionsParameters(const QByteArray &parameters)
{
    m_preparedSuggestionsParameters = parameters;
}

void OpenSearchEngine::setSuggestionsUrl(const QString &string)
{
    m_preparedSuggestionsUrl = string;
}

QString OpenSearchEngine::getSuggestionsUrl()
{
    return suggestionsUrl("searchstring").toString().replace(QLatin1String("searchstring"), QLatin1String("%s"));
}

QByteArray OpenSearchEngine::getSuggestionsParameters()
{
    QStringList parameters;
    Parameters::const_iterator end = m_suggestionsParameters.constEnd();
    Parameters::const_iterator i = m_suggestionsParameters.constBegin();
    for (; i != end; ++i) {
        parameters.append(i->first + QLatin1String("=") + i->second);
    }

    QByteArray data = parameters.join(QLatin1String("&")).toUtf8();

    return data;
}

void OpenSearchEngine::requestSuggestions(const QString &searchTerm)
{
    if (searchTerm.isEmpty() || !providesSuggestions()) {
        return;
    }

    Q_ASSERT(m_networkAccessManager);

    if (!m_networkAccessManager) {
        return;
    }

    if (m_suggestionsReply) {
        m_suggestionsReply->disconnect(this);
        m_suggestionsReply->abort();
        m_suggestionsReply->deleteLater();
        m_suggestionsReply = 0;
    }

    Q_ASSERT(m_requestMethods.contains(m_suggestionsMethod));
    if (m_suggestionsMethod == QLatin1String("get")) {
        m_suggestionsReply = m_networkAccessManager->get(QNetworkRequest(suggestionsUrl(searchTerm)));
    }
    else {
        QStringList parameters;
        Parameters::const_iterator end = m_suggestionsParameters.constEnd();
        Parameters::const_iterator i = m_suggestionsParameters.constBegin();
        for (; i != end; ++i) {
            parameters.append(i->first + QLatin1String("=") + i->second);
        }

        QByteArray data = parameters.join(QLatin1String("&")).toUtf8();
        m_suggestionsReply = m_networkAccessManager->post(QNetworkRequest(suggestionsUrl(searchTerm)), data);
    }

    connect(m_suggestionsReply, SIGNAL(finished()), this, SLOT(suggestionsObtained()));
}

/*!
    Requests search results on the search engine, for a given \a searchTerm.

    The default implementation does nothing, to supply your own you need to create your own
    OpenSearchEngineDelegate subclass and supply it to the engine. Then the function will call
    the performSearchRequest() method of the delegate, which can then handle the request
    in a custom way.

    \sa requestSuggestions(), delegate()
*/
void OpenSearchEngine::requestSearchResults(const QString &searchTerm)
{
    if (!m_delegate || searchTerm.isEmpty()) {
        return;
    }

    Q_ASSERT(m_requestMethods.contains(m_searchMethod));

    QNetworkRequest request(QUrl(searchUrl(searchTerm)));
    QByteArray data;
    QNetworkAccessManager::Operation operation = m_requestMethods.value(m_searchMethod);

    if (operation == QNetworkAccessManager::PostOperation) {
        QStringList parameters;
        Parameters::const_iterator end = m_searchParameters.constEnd();
        Parameters::const_iterator i = m_searchParameters.constBegin();
        for (; i != end; ++i) {
            parameters.append(i->first + QLatin1String("=") + i->second);
        }

        data = parameters.join(QLatin1String("&")).toUtf8();
    }

    m_delegate->performSearchRequest(request, operation, data);
}

void OpenSearchEngine::suggestionsObtained()
{
    const QByteArray response = m_suggestionsReply->readAll();

    m_suggestionsReply->close();
    m_suggestionsReply->deleteLater();
    m_suggestionsReply = 0;

    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(response);
    const QVariant res = json.toVariant();

    if (err.error != QJsonParseError::NoError || res.type() != QVariant::List)
        return;

    const QVariantList list = res.toList();

    if (list.size() < 2)
        return;

    QStringList out;

    foreach (const QVariant &v, list.at(1).toList())
        out.append(v.toString());

    emit suggestions(out);
}

/*!
    \property networkAccessManager
    \brief the network access manager that is used to perform network requests

    It is required for network operations: loading external images and requesting
    contextual suggestions.
*/
QNetworkAccessManager* OpenSearchEngine::networkAccessManager() const
{
    return m_networkAccessManager;
}

void OpenSearchEngine::setNetworkAccessManager(QNetworkAccessManager* networkAccessManager)
{
    m_networkAccessManager = networkAccessManager;
}

/*!
    \property delegate
    \brief the delegate that is used to perform specific tasks.

    It can be currently supplied to provide a custom behaviour ofthe requetSearchResults() method.
    The default implementation does nothing.
*/
OpenSearchEngineDelegate* OpenSearchEngine::delegate() const
{
    return m_delegate;
}

void OpenSearchEngine::setDelegate(OpenSearchEngineDelegate* delegate)
{
    m_delegate = delegate;
}

/*!
    \fn void OpenSearchEngine::imageChanged()

    This signal is emitted whenever the image of the engine changes.

    \sa image(), imageUrl()
*/

/*!
    \fn void OpenSearchEngine::suggestions(const QStringList &suggestions)

    This signal is emitted whenever new contextual suggestions have been provided
    by the search engine. To request suggestions, use requestSuggestions().
    The suggestion set is specified by \a suggestions.

    \sa requestSuggestions()
*/
