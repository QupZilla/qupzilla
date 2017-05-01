/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014  S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2013-2014  David Rosca <nowrep@gmail.com>
*
* This is based on a work by Saju Pillai <saju.pillai@gmail.com>
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
#ifndef AESINTERFACE_H
#define AESINTERFACE_H

#include "qzcommon.h"

#include <openssl/evp.h>

#include <QObject>
#include <QHash>
#include <QList>

class QUPZILLA_EXPORT AesInterface : public QObject
{
    Q_OBJECT

public:
    static const int VERSION;

    explicit AesInterface(QObject* parent = 0);
    ~AesInterface();

    bool isOk();

    QByteArray encrypt(const QByteArray &plainData, const QByteArray &password);
    QByteArray decrypt(const QByteArray &cipherData, const QByteArray &password);

    static QByteArray passwordToHash(const QString &masterPassword);
    static QByteArray createRandomData(int length);

private:
    bool init(int evpMode, const QByteArray &password, const QByteArray &iVector = QByteArray());

    EVP_CIPHER_CTX* m_encodeCTX;
    EVP_CIPHER_CTX* m_decodeCTX;

    bool m_ok;
    QByteArray m_iVector;
};
#endif //AESINTERFACE_H
