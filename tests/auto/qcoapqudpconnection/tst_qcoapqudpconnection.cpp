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

#include <QtTest>
#include <QCoreApplication>

#include <QtCore/qglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCore/qbuffer.h>
#include <QtNetwork/qudpsocket.h>
#include <QtNetwork/qnetworkdatagram.h>
#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapqudpconnection.h>
#include <QtCoap/qcoaprequest.h>
#include <private/qcoapqudpconnection_p.h>
#include <private/qcoapinternalrequest_p.h>
#include "../coapnetworksettings.h"

using namespace QtCoapNetworkSettings;

struct QCoapRequestForTest : public QCoapRequest
{
    QCoapRequestForTest(const QUrl &url) : QCoapRequest(url) {}
    using QCoapRequest::setMethod;
};

class tst_QCoapQUdpConnection : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void ctor();
    void connectToHost();
    void sendRequest_data();
    void sendRequest();
};

class QCoapQUdpConnectionForTest : public QCoapQUdpConnection
{
    Q_OBJECT
public:
    QCoapQUdpConnectionForTest(QObject *parent = nullptr) :
        QCoapQUdpConnection(QtCoap::NoSec, parent)
    {}

    void bindSocketForTest() { d_func()->bindSocket(); }
};

void tst_QCoapQUdpConnection::ctor()
{
    QCoapQUdpConnection connection;
    QVERIFY(connection.socket());
}

void tst_QCoapQUdpConnection::connectToHost()
{
#ifdef QT_BUILD_INTERNAL
    QCoapQUdpConnectionForTest connection;

    QUdpSocket *socket = qobject_cast<QUdpSocket*>(connection.socket());
    QSignalSpy spyConnectionBound(&connection, SIGNAL(bound()));
    QSignalSpy spySocketStateChanged(socket , SIGNAL(stateChanged(QAbstractSocket::SocketState)));

    QCOMPARE(connection.state(), QCoapQUdpConnection::Unconnected);

    // This will trigger connection.bind()
    connection.sendRequest(QByteArray(), QString(), 0);

    QTRY_COMPARE(spySocketStateChanged.count(), 1);
    QTRY_COMPARE(spyConnectionBound.count(), 1);
    QCOMPARE(connection.state(), QCoapQUdpConnection::Bound);
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

void tst_QCoapQUdpConnection::sendRequest_data()
{
    QTest::addColumn<QString>("protocol");
    QTest::addColumn<QString>("host");
    QTest::addColumn<QString>("path");
    QTest::addColumn<quint16>("port");
    QTest::addColumn<QtCoap::Method>("method");
    QTest::addColumn<QString>("dataHexaHeader");
    QTest::addColumn<QString>("dataHexaPayload");

    QTest::newRow("simple_get_request")
        << "coap://"
        << testServerHost()
        << "/test"
        << quint16(QtCoap::DefaultPort)
        << QtCoap::Get
        << "5445"
        << "61626364c0211eff547970653a203120284e4f4e290a436f64653a2031202847"
           "4554290a4d49443a2032343830360a546f6b656e3a203631363236333634";

    QTest::newRow("simple_put_request")
        << "coap://"
        << testServerHost()
        << "/test"
        << quint16(QtCoap::DefaultPort)
        << QtCoap::Put
        << "5444"
        << "61626364";

    QTest::newRow("simple_post_request")
        << "coap://"
        << testServerHost()
        << "/test"
        << quint16(QtCoap::DefaultPort)
        << QtCoap::Post
        << "5441"
        << "61626364896c6f636174696f6e31096c6f636174696f6e32096c6f636174696f"
           "6e33";

    QTest::newRow("simple_delete_request")
        << "coap://"
        << testServerHost()
        << "/test"
        << quint16(QtCoap::DefaultPort)
        << QtCoap::Delete
        << "5442"
        << "61626364";
}

void tst_QCoapQUdpConnection::sendRequest()
{
#ifdef QT_BUILD_INTERNAL
    QFETCH(QString, protocol);
    QFETCH(QString, host);
    QFETCH(QString, path);
    QFETCH(quint16, port);
    QFETCH(QtCoap::Method, method);
    QFETCH(QString, dataHexaHeader);
    QFETCH(QString, dataHexaPayload);

    QCoapQUdpConnectionForTest connection;

    QSignalSpy spySocketReadyRead(connection.socket(), &QUdpSocket::readyRead);
    QSignalSpy spyConnectionReadyRead(&connection, &QCoapQUdpConnection::readyRead);

    QCoapRequestForTest request(protocol + host + path);
    request.setMessageId(24806);
    request.setToken(QByteArray("abcd"));
    request.setMethod(method);
    QVERIFY(connection.socket() != nullptr);
    QCoapInternalRequest internalRequest(request);
    connection.sendRequest(internalRequest.toQByteArray(), host, port);

    QTRY_COMPARE(spySocketReadyRead.count(), 1);
    QTRY_COMPARE(spyConnectionReadyRead.count(), 1);

    QByteArray data = spyConnectionReadyRead.first().first().value<QByteArray>();
    QVERIFY(QString(data.toHex()).startsWith(dataHexaHeader));
    QVERIFY(QString(data.toHex()).endsWith(dataHexaPayload));
#else
    QSKIP("Not an internal build, skipping this test");
#endif
}

QTEST_MAIN(tst_QCoapQUdpConnection)

#include "tst_qcoapqudpconnection.moc"
