/****************************************************************************
**
** Copyright (C) 2017 Witekio.
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCoap module.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QCOAPPROTOCOL_P_H
#define QCOAPPROTOCOL_P_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapreply.h>
#include <QtCoap/qcoapresource.h>
#include <QtCore/qvector.h>
#include <QtCore/qqueue.h>
#include <QtCore/qpointer.h>
#include <QtCore/qobject.h>
#include <private/qobject_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QCoapInternalRequest;
class QCoapInternalReply;
class QCoapProtocolPrivate;
class QCoapConnection;
class Q_AUTOTEST_EXPORT QCoapProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QCoapProtocol(QObject *parent = nullptr);
    ~QCoapProtocol();

    uint ackTimeout() const;
    double ackRandomFactor() const;
    uint maximumRetransmitCount() const;
    quint16 blockSize() const;
    uint maximumTransmitSpan() const;
    uint maximumTransmitWait() const;
    uint maximumLatency() const;

    uint minimumTimeout() const;
    uint maximumTimeout() const;

    uint nonConfirmLifetime() const;
    uint maximumServerResponseDelay() const;

Q_SIGNALS:
    void finished(QCoapReply *reply);
    void responseToMulticastReceived(QCoapReply *reply, const QCoapMessage &message,
                                     const QHostAddress &sender);
    void error(QCoapReply *reply, QtCoap::Error error);

public:
    Q_INVOKABLE void setAckTimeout(uint ackTimeout);
    Q_INVOKABLE void setAckRandomFactor(double ackRandomFactor);
    Q_INVOKABLE void setMaximumRetransmitCount(uint maximumRetransmitCount);
    Q_INVOKABLE void setBlockSize(quint16 blockSize);
    Q_INVOKABLE void setMaximumServerResponseDelay(uint responseDelay);
    Q_INVOKABLE void setMinimumTokenSize(int tokenSize);

private:
    Q_INVOKABLE void sendRequest(QPointer<QCoapReply> reply, QCoapConnection *connection);
    Q_INVOKABLE void cancelObserve(QPointer<QCoapReply> reply) const;
    Q_INVOKABLE void cancelObserve(const QUrl &url) const;

private:
    Q_DECLARE_PRIVATE(QCoapProtocol)

    friend class QCoapClient;
    friend class QCoapClientPrivate;
};

struct CoapExchangeData {
    QPointer<QCoapReply> userReply;
    QSharedPointer<QCoapInternalRequest> request;
    QVector<QSharedPointer<QCoapInternalReply> > replies;
};

typedef QMap<QByteArray, CoapExchangeData> CoapExchangeMap;

class Q_AUTOTEST_EXPORT QCoapProtocolPrivate : public QObjectPrivate
{
public:
    QCoapProtocolPrivate() = default;

    quint16 generateUniqueMessageId() const;
    QCoapToken generateUniqueToken() const;

    QCoapInternalReply *decode(const QByteArray &data, const QHostAddress &sender);

    void sendAcknowledgment(QCoapInternalRequest *request) const;
    void sendReset(QCoapInternalRequest *request) const;
    void sendRequest(QCoapInternalRequest *request, const QString& host = QString()) const;

    void onLastMessageReceived(QCoapInternalRequest *request, const QHostAddress &sender);
    void onRequestError(QCoapInternalRequest *request, QCoapInternalReply *reply);
    void onRequestError(QCoapInternalRequest *request, QtCoap::Error error,
                        QCoapInternalReply *reply = nullptr);

    void onRequestTimeout(QCoapInternalRequest *request);
    void onRequestMaxTransmissionSpanReached(QCoapInternalRequest *request);
    void onMulticastRequestExpired(QCoapInternalRequest *request);
    void onFrameReceived(const QByteArray &data, const QHostAddress &sender);
    void onConnectionError(QAbstractSocket::SocketError error);
    void onRequestAborted(const QCoapToken &token);

    bool isMessageIdRegistered(quint16 id) const;
    bool isTokenRegistered(const QCoapToken &token) const;
    bool isRequestRegistered(const QCoapInternalRequest *request) const;

    QCoapInternalRequest *requestForToken(const QCoapToken &token) const;
    QPointer<QCoapReply> userReplyForToken(const QCoapToken &token) const;
    QVector<QSharedPointer<QCoapInternalReply>> repliesForToken(const QCoapToken &token) const;
    QCoapInternalReply *lastReplyForToken(const QCoapToken &token) const;
    QCoapInternalRequest *findRequestByMessageId(quint16 messageId) const;
    QCoapInternalRequest *findRequestByUserReply(const QCoapReply *reply) const;

    void registerExchange(const QCoapToken &token, QCoapReply *reply,
                          QSharedPointer<QCoapInternalRequest> request);
    bool addReply(const QCoapToken &token, QSharedPointer<QCoapInternalReply> reply);
    bool forgetExchange(const QCoapToken &token);
    bool forgetExchange(const QCoapInternalRequest *request);
    bool forgetExchangeReplies(const QCoapToken &token);

    CoapExchangeMap exchangeMap;
    quint16 blockSize = 0;

    uint maximumRetransmitCount = 4;
    uint ackTimeout = 2000;
    uint maximumServerResponseDelay = 250 * 1000;
    int minimumTokenSize = 4;
    double ackRandomFactor = 1.5;

    Q_DECLARE_PUBLIC(QCoapProtocol)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QHostAddress)

#endif // QCOAPPROTOCOL_P_H
