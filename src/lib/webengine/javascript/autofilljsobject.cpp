/* ============================================================
* QupZilla - QtWebEngine based browser
* Copyright (C) 2015 David Rosca <nowrep@gmail.com>
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

#include "autofilljsobject.h"
#include "externaljsobject.h"
#include "mainapplication.h"
#include "autofill.h"
#include "webpage.h"

AutoFillJsObject::AutoFillJsObject(ExternalJsObject *parent)
    : QObject(parent)
    , m_jsObject(parent)
{
}

void AutoFillJsObject::formSubmitted(const QString &frameUrl, const QString &username, const QString &password, const QByteArray &data)
{
    PageFormData formData;
    formData.username = username;
    formData.password = password;
    formData.postData = data;

    mApp->autoFill()->saveForm(m_jsObject->page(), QUrl(frameUrl), formData);
}

