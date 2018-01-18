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
* Copyright (C) 2010-2018  David Rosca <nowrep@gmail.com>
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
#include "opensearchreader.h"

#include "opensearchengine.h"

#include <qiodevice.h>

/*!
    \class OpenSearchReader
    \brief A class reading a search engine description from an external source

    OpenSearchReader is a class that can be used to read search engine descriptions
    formed using the OpenSearch format.

    It inherits QXmlStreamReader and thus provides additional functions, such as
    QXmlStreamReader::error(), QXmlStreamReader::hasError() that can be used to make sure
    the reading procedure succeeded.

    For more information see:
    http://www.opensearch.org/Specifications/OpenSearch/1.1/Draft_4#OpenSearch_description_document

    \sa OpenSearchEngine, OpenSearchWriter
*/

/*!
    Constructs a new reader.

    \note One instance can be used to read multiple files, one by one.
*/
OpenSearchReader::OpenSearchReader()
    : QXmlStreamReader()
{
}

/*!
    Reads an OpenSearch engine from the \a device and returns an OpenSearchEngine object,
    filled in with all the data that has been retrieved from the document.

    If the \a device is closed, it will be opened.

    To make sure if the procedure succeeded, check QXmlStreamReader::error().

    \return a new constructed OpenSearchEngine object

    \note The function returns an object of the OpenSearchEngine class even if the document
          is bad formed or doesn't conform to the specification. It needs to be manually
          deleted afterwards, if intended.
    \note The lifetime of the returned OpenSearchEngine object is up to the user.
          The object should be deleted once it is not used anymore to avoid memory leaks.
*/
OpenSearchEngine* OpenSearchReader::read(QIODevice* device)
{
    clear();

    if (!device->isOpen()) {
        device->open(QIODevice::ReadOnly);
    }

    setDevice(device);
    return read();
}

OpenSearchEngine* OpenSearchReader::read()
{
    OpenSearchEngine* engine = new OpenSearchEngine();
    m_searchXml = device()->peek(1024 * 5);

    if (!m_searchXml.contains(QLatin1String("http://a9.com/-/spec/opensearch/1.1/")) &&
        !m_searchXml.contains(QLatin1String("http://www.mozilla.org/2006/browser/search/"))
       ) {
        raiseError(QObject::tr("The file is not an OpenSearch 1.1 file."));
        return engine;
    }

    // It just skips the XML declaration
    // The parsing code bellow for some reason doesn't like it -,-

    int index = m_searchXml.indexOf(QLatin1String("<?xml"));
    if (index > 0) {
        int end = m_searchXml.indexOf(QLatin1String("?>"), index);

        if (end > 0) {
            device()->read(end + 2);
        }
    }

    while (!isStartElement() && !atEnd()) {
        readNext();
    }


    while (!atEnd()) {
        readNext();

        if (!isStartElement()) {
            continue;
        }

        if (name() == QLatin1String("ShortName") || name() == QLatin1String("os:ShortName")) {
            engine->setName(readElementText());
        }
        else if (name() == QLatin1String("Description") || name() == QLatin1String("os:Description")) {
            engine->setDescription(readElementText());
        }
        else if (name() == QLatin1String("Url") || name() == QLatin1String("os:Url")) {
            QString type = attributes().value(QLatin1String("type")).toString();
            QString url = attributes().value(QLatin1String("template")).toString();
            QString method = attributes().value(QLatin1String("method")).toString();

            if (type == QLatin1String("application/x-suggestions+json") &&
                !engine->suggestionsUrlTemplate().isEmpty()
               ) {
                continue;
            }

            if ((type.isEmpty() ||
                 type == QLatin1String("text/html") ||
                 type == QLatin1String("application/xhtml+xml")) &&
                !engine->searchUrlTemplate().isEmpty()
               ) {
                continue;
            }

            if (url.isEmpty()) {
                continue;
            }

            QList<OpenSearchEngine::Parameter> parameters;

            readNext();

            while (!isEndElement() || (name() != QLatin1String("Url") && name() != QLatin1String("os:Url"))) {
                if (!isStartElement() || (name() != QLatin1String("Param") && name() != QLatin1String("Parameter") &&
                                          name() != QLatin1String("os:Param") && name() != QLatin1String("os:Parameter"))
                   ) {
                    readNext();
                    continue;
                }

                QString key = attributes().value(QLatin1String("name")).toString();
                QString value = attributes().value(QLatin1String("value")).toString();

                if (!key.isEmpty() && !value.isEmpty()) {
                    parameters.append(OpenSearchEngine::Parameter(key, value));
                }

                while (!isEndElement()) {
                    readNext();
                }
            }

            if (type == QLatin1String("application/x-suggestions+json")) {
                engine->setSuggestionsUrlTemplate(url);
                engine->setSuggestionsParameters(parameters);
                engine->setSuggestionsMethod(method);
            }
            else if (type.isEmpty() || type == QLatin1String("text/html") || type == QLatin1String("application/xhtml+xml")) {
                engine->setSearchUrlTemplate(url);
                engine->setSearchParameters(parameters);
                engine->setSearchMethod(method);
            }

        }
        else if (name() == QLatin1String("Image") || name() == QLatin1String("os:Image")) {
            engine->setImageUrl(readElementText());
        }

        if (!engine->name().isEmpty() &&
            !engine->description().isEmpty() &&
            !engine->suggestionsUrlTemplate().isEmpty() &&
            !engine->searchUrlTemplate().isEmpty() &&
            !engine->imageUrl().isEmpty()
           ) {
            break;
        }
    }

    return engine;
}

