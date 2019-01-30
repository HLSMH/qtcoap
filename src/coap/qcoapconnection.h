/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef QCOAPCONNECTION_H
#define QCOAPCONNECTION_H

#include <QtCoap/qcoapglobal.h>
#include <QtCoap/qcoapnamespace.h>
#include <QtCoap/qcoapsecurityconfiguration.h>

#include <QtNetwork/QAbstractSocket>

QT_BEGIN_NAMESPACE

class QCoapConnectionPrivate;
class Q_COAP_EXPORT QCoapConnection : public QObject
{
    Q_OBJECT
public:
    enum ConnectionState {
        Unconnected,
        Bound
    };

    explicit QCoapConnection(QtCoap::SecurityMode securityMode = QtCoap::NoSec,
                             QObject *parent = nullptr);
    virtual ~QCoapConnection();

    void sendRequest(const QByteArray &request, const QString &host, quint16 port);

    bool isSecure() const;
    QtCoap::SecurityMode securityMode() const;
    QCoapConnection::ConnectionState state() const;

    QCoapSecurityConfiguration securityConfiguration() const;
public Q_SLOTS:
    void setSecurityConfiguration(const QCoapSecurityConfiguration &configuration);

Q_SIGNALS:
    void error(QAbstractSocket::SocketError error);
    void readyRead(const QByteArray &data, const QHostAddress &sender);
    void bound();
    void securityConfigurationChanged();

private:
    void startToSendRequest();

protected:
    QCoapConnection(QObjectPrivate &dd, QObject *parent = nullptr);

    virtual void bind(const QString &host, quint16 port) = 0;
    virtual void writeData(const QByteArray &data, const QString &host, quint16 port) = 0;

    Q_DECLARE_PRIVATE(QCoapConnection)
};

QT_END_NAMESPACE

#endif // QCOAPCONNECTION_H
