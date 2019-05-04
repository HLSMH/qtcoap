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

#include <QtCoap/qcoapmessage.h>

class tst_QCoapMessage : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void copyAndDetach();
    void setMessageType_data();
    void setMessageType();
    void addOption();
    void addOption_string_data();
    void addOption_string();
    void addOption_uint_data();
    void addOption_uint();
    void removeOption_data();
    void removeOption();
    void removeOptionByName_data();
    void removeOptionByName();
    void removeAll();
};

void tst_QCoapMessage::copyAndDetach()
{
    QCoapMessage a;
    a.setMessageId(3);
    a.setPayload("payload");
    a.setToken("token");
    a.setType(QCoapMessage::MessageType::Acknowledgment);
    a.setVersion(5);

    // Test the copy
    QCoapMessage b(a);
    QVERIFY2(b.messageId() == 3, "Message not copied correctly");
    QVERIFY2(b.payload() == "payload", "Message not copied correctly");
    QVERIFY2(b.token() == "token", "Message not copied correctly");
    QVERIFY2(b.type() == QCoapMessage::MessageType::Acknowledgment, "Message not copied correctly");
    QVERIFY2(b.version() == 5, "Message not copied correctly");

    // Detach
    b.setMessageId(9);
    QCOMPARE(b.messageId(), 9);
    QCOMPARE(a.messageId(), 3);
}

void tst_QCoapMessage::setMessageType_data()
{
    QTest::addColumn<QCoapMessage::MessageType>("type");

    QTest::newRow("acknowledgment") << QCoapMessage::MessageType::Acknowledgment;
    QTest::newRow("confirmable") << QCoapMessage::MessageType::Confirmable;
    QTest::newRow("non-confirmable") << QCoapMessage::MessageType::NonConfirmable;
    QTest::newRow("reset") << QCoapMessage::MessageType::Reset;
}

void tst_QCoapMessage::setMessageType()
{
    QFETCH(QCoapMessage::MessageType, type);
    QCoapMessage message;
    message.setType(type);
    QCOMPARE(message.type(), type);
}

void tst_QCoapMessage::addOption()
{
    QCoapMessage message;

    QList<QCoapOption::OptionName> optionNames = {
        QCoapOption::ProxyUri,
        QCoapOption::UriHost,
        QCoapOption::LocationQuery,
        QCoapOption::ProxyUri,
        QCoapOption::UriHost,
        QCoapOption::UriQuery
    };
    QVERIFY(!std::is_sorted(optionNames.cbegin(), optionNames.cend()));

    const QByteArray value("\xAF\x01\xC2");
    for (const auto& name : optionNames)
        message.addOption(name, value);

    QCOMPARE(message.optionCount(), optionNames.size());
    QVERIFY(std::is_sorted(message.options().cbegin(), message.options().cend(),
                           [](const QCoapOption &a, const QCoapOption &b) -> bool {
                               return a.name() < b.name();
           }));

    for (const auto& name : optionNames)
        QVERIFY2(message.hasOption(name), qPrintable(QString("Missing option %1").arg(name)));

    QVERIFY(std::all_of(message.options().cbegin(), message.options().cend(),
                        [value](const QCoapOption opt) -> bool {
                            return opt.value() == value;
           }));
}

void tst_QCoapMessage::addOption_string_data()
{
    QTest::addColumn<QVector<QCoapOption>>("options");

    QVector<QCoapOption> single_char_option = { { QCoapOption::LocationPath, "path1" } };
    QVector<QCoapOption> single_ba_option = {
        { QCoapOption::LocationPath, QByteArray("\xAF\x01\xC2") }
    };
    QVector<QCoapOption> multiple_string_options = {
        { QCoapOption::LocationPath, QString("str_path2") },
        { QCoapOption::LocationPath, QString("str_path3") }
    };

    QTest::newRow("single_char_option") << single_char_option;
    QTest::newRow("single_ba_option") << single_ba_option;
    QTest::newRow("multiple_string_options") << multiple_string_options;
}

void tst_QCoapMessage::addOption_string()
{
    QFETCH(QVector<QCoapOption>, options);

    QCoapMessage message;
    for (const auto& option : options)
        message.addOption(option);

    QCOMPARE(message.optionCount(), options.size());
    for (const auto& option : options)
    {
        const auto it = std::find(message.options().cbegin(), message.options().cend(), option);
        QVERIFY(it != message.options().cend());
    }
}

void tst_QCoapMessage::addOption_uint_data()
{
    QTest::addColumn<quint32>("value");
    QTest::addColumn<int>("size");

    QTest::newRow("4 bytes") << static_cast<quint32>(0xF0aF0010) << 4;
    QTest::newRow("3 bytes") << static_cast<quint32>(0x300010) << 3;
    QTest::newRow("2 bytes") << static_cast<quint32>(0x5010) << 2;
    QTest::newRow("1 byte")  << static_cast<quint32>(0x80) << 1;
}

void tst_QCoapMessage::addOption_uint()
{
    QFETCH(quint32, value);
    QFETCH(int, size);

    const auto name = QCoapOption::Block1;
    const QCoapOption option(name, value);

    QCoapMessage message;
    message.addOption(option);

    QCOMPARE(message.options(name).size(), 1);
    QCOMPARE(message.option(name).valueToInt(), value);
    QCOMPARE(option.value().size(), size);
}

void tst_QCoapMessage::removeOption_data()
{
    QTest::addColumn<QVector<QCoapOption>>("options");

    QVector<QCoapOption> single_option = { { QCoapOption::LocationPath, "path1" } };
    QVector<QCoapOption> multiple_options = {
        { QCoapOption::LocationPath, "path2" },
        { QCoapOption::LocationPath, "path3" }
    };

    QTest::newRow("single_option") << single_option;
    QTest::newRow("multiple_options") << multiple_options;
}

void tst_QCoapMessage::removeOption()
{
    QFETCH(QVector<QCoapOption>, options);

    QCoapMessage message;
    for (const auto& option : options)
        message.addOption(option);

    for (const auto& option : options)
    {
        // Make sure option is present before removal
        auto it = std::find(message.options().cbegin(), message.options().cend(), option);
        QVERIFY(it != message.options().end());

        message.removeOption(option);
        it = std::find(message.options().cbegin(), message.options().cend(), option);
        QVERIFY(it == message.options().end());
    }
}

void tst_QCoapMessage::removeOptionByName_data()
{
    QTest::addColumn<QVector<QCoapOption>>("options");
    QTest::addColumn<QCoapOption::OptionName>("name");

    QVector<QCoapOption> single_option = { { QCoapOption::LocationPath, "path1" } };
    QVector<QCoapOption> multiple_options = {
        { QCoapOption::LocationPath, "path2" },
        { QCoapOption::LocationPath, "path3" }
    };

    QTest::newRow("remove_single_option") << single_option << single_option.back().name();
    QTest::newRow("remove_multiple_options") << multiple_options << multiple_options.back().name();
}

void tst_QCoapMessage::removeOptionByName()
{
    QFETCH(QVector<QCoapOption>, options);
    QFETCH(QCoapOption::OptionName, name);

    QCoapMessage message;
    for (const auto& option : options)
        message.addOption(option);

    message.removeOption(name);
    QVERIFY(!message.hasOption(name));
}

void tst_QCoapMessage::removeAll()
{
    QCoapMessage message;
    message.addOption(QCoapOption::LocationPath, "path");
    message.addOption(QCoapOption::ProxyUri, "proxy1");
    message.addOption(QCoapOption::ProxyUri, "proxy2");
    message.removeAllOptions();

    QVERIFY(message.options().isEmpty());
}

QTEST_APPLESS_MAIN(tst_QCoapMessage)

#include "tst_qcoapmessage.moc"
