/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2013-2014 S. Razi Alavizadeh <s.r.alavizadeh@gmail.com>
* Copyright (C) 2013-2017 David Rosca <nowrep@gmail.com>
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

#include "aesinterface.h"

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <QCryptographicHash>
#include <QByteArray>
#include <QMessageBox>

//////////////////////////////////////////////
/// Version 1:
/// init(): n=5, EVP_CIPHER=EVP_aes_256_cbc(), EVP_MD=EVP_sha256(), Random IV
/// Encrypted data structure: Version$InitializationVector_base64$EncryptedData_base64
const int AesInterface::VERSION = 1;

AesInterface::AesInterface(QObject* parent)
    : QObject(parent)
    , m_ok(false)
{
    m_encodeCTX = EVP_CIPHER_CTX_new();
    m_decodeCTX = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(m_encodeCTX);
    EVP_CIPHER_CTX_init(m_decodeCTX);
}

AesInterface::~AesInterface()
{
    EVP_CIPHER_CTX_cleanup(m_encodeCTX);
    EVP_CIPHER_CTX_cleanup(m_decodeCTX);
    EVP_CIPHER_CTX_free(m_encodeCTX);
    EVP_CIPHER_CTX_free(m_decodeCTX);
}

bool AesInterface::isOk()
{
    return m_ok;
}

// Create an 256 bit 'key' using the supplied password, and creates a random 'iv'.
// saltArray is an array of 8 bytes can be added for taste.
// Fills in the encryption and decryption ctx objects and returns true on success
bool AesInterface::init(int evpMode, const QByteArray &password, const QByteArray &iVector)
{
    m_iVector.clear();

    int i;
    const int nrounds = 5;
    uchar key[EVP_MAX_KEY_LENGTH];

    // Gen "key" for AES 256 CBC mode. A SHA1 digest is used to hash the supplied
    // key material. nrounds is the number of times that we hash the material.
    // More rounds are more secure but slower.
    i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), 0, (uchar*)password.data(), password.size(), nrounds, key, 0);

    if (i != 32) {
        qWarning("Key size is %d bits - should be 256 bits", i * 8);
        return false;
    }

    int result = 0;
    if (evpMode == EVP_PKEY_MO_ENCRYPT) {
        m_iVector = createRandomData(EVP_MAX_IV_LENGTH);
        result = EVP_EncryptInit_ex(m_encodeCTX, EVP_aes_256_cbc(), NULL, key, (uchar*)m_iVector.constData());
    }
    else if (evpMode == EVP_PKEY_MO_DECRYPT) {
        result = EVP_DecryptInit_ex(m_decodeCTX, EVP_aes_256_cbc(), NULL, key, (uchar*)iVector.constData());
    }

    if (result == 0) {
        qWarning("EVP is not initialized!");
        return false;
    }

    return true;
}

QByteArray AesInterface::encrypt(const QByteArray &plainData, const QByteArray &password)
{
    if (!init(EVP_PKEY_MO_ENCRYPT, password)) {
        m_ok = false;
        return plainData;
    }

    // max ciphertext len for a n bytes of plaintext is n + AES_BLOCK_SIZE -1 bytes
    int dataLength = plainData.size();
    int cipherlength = dataLength + AES_BLOCK_SIZE;
    int finalLength = 0;
    uchar* ciphertext = (uchar*)malloc(cipherlength);

    // allows reusing of 'm_encodeCTX' for multiple encryption cycles
    EVP_EncryptInit_ex(m_encodeCTX, NULL, NULL, NULL, NULL);

    // update ciphertext, c_len is filled with the length of ciphertext generated,
    // dataLength is the size of plaintext in bytes
    EVP_EncryptUpdate(m_encodeCTX, ciphertext, &cipherlength, (uchar*)plainData.data(), dataLength);

    // update ciphertext with the final remaining bytes
    EVP_EncryptFinal_ex(m_encodeCTX, ciphertext + cipherlength, &finalLength);

    dataLength = cipherlength + finalLength;
    QByteArray out((char*)ciphertext, dataLength);
    out = QByteArray::number(AesInterface::VERSION) + '$' + m_iVector.toBase64() + '$' + out.toBase64();
    free(ciphertext);

    m_ok = true;
    return out;
}

QByteArray AesInterface::decrypt(const QByteArray &cipherData, const QByteArray &password)
{
    m_ok = false;

    if (cipherData.isEmpty()) {
        m_ok = true;
        return QByteArray();
    }

    QList<QByteArray> cipherSections(cipherData.split('$'));
    if (cipherSections.size() != 3) {
        qWarning() << "Decrypt error: It seems data is corrupted";
        return QByteArray();
    }

    if (cipherSections.at(0).toInt() > AesInterface::VERSION) {
        QMessageBox::information(0, tr("Warning!"), tr("Data has been encrypted with a newer version of QupZilla."
                                 "\nPlease install latest version of QupZilla."));
        return QByteArray();
    }

    if (cipherSections.at(0).toInt() != 1) {
        qWarning() << Q_FUNC_INFO << "There is just version 1 of decoder, yet ;-)";
        return QByteArray();
    }

    if (!init(EVP_PKEY_MO_DECRYPT, password, QByteArray::fromBase64(cipherSections.at(1)))) {
        return QByteArray();
    }

    QByteArray cipherArray = QByteArray::fromBase64(cipherSections.at(2));
    int cipherLength = cipherArray.size();
    int plainTextLength = cipherLength;
    int finalLength = 0;

    uchar* cipherText = (uchar*)cipherArray.data();
    // because we have padding ON, we must allocate an extra cipher block size of memory
    uchar* plainText = (uchar*)malloc(plainTextLength + AES_BLOCK_SIZE);

    EVP_DecryptInit_ex(m_decodeCTX, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate(m_decodeCTX, plainText, &plainTextLength, cipherText, cipherLength);
    int success = EVP_DecryptFinal_ex(m_decodeCTX, plainText + plainTextLength, &finalLength);

    cipherLength = plainTextLength + finalLength;

    QByteArray result((char*)plainText, cipherLength);
    free(plainText);

    if (success != 1) {
        return QByteArray();
    }

    m_ok = true;
    return result;
}

QByteArray AesInterface::passwordToHash(const QString &masterPassword)
{
    if (!masterPassword.isEmpty()) {
        QByteArray result = masterPassword.toUtf8();
        result = QCryptographicHash::hash(result, QCryptographicHash::Sha1) + result;
        result = QCryptographicHash::hash(result, QCryptographicHash::Sha1);
        return result.toBase64();
    }
    else {
        return QByteArray();
    }
}

QByteArray AesInterface::createRandomData(int length)
{
    uchar* randomData = (uchar*)malloc(length);

    RAND_bytes(randomData, length);
    QByteArray data((char*)randomData, length);
    free(randomData);

    return data;
}
