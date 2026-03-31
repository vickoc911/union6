// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Arjen Hiemstra <ahiemstra@heimr.nl>

#include <QtTest>

#include "../../../src/output/qtquick/plugin/positioner/Layout.h"

using namespace Union::Quick;

class TestPositionerLayout : public QObject
{
    Q_OBJECT

    struct ItemData {
        QPointF expectedPosition;
        QSizeF expectedSize;

        QSizeF implicitSize;
        QMarginsF margins;
        Union::Properties::Alignment verticalAlignment;
    };

    struct BucketData {
        QPointF expectedPosition;
        QSizeF expectedSize;

        QList<ItemData> items;
    };

    struct ContainerData {
        QPointF expectedPosition;
        QSizeF expectedSize;

        BucketData start;
        BucketData center;
        BucketData end;
        BucketData fill;
    };

    struct LayoutData {
        QSizeF size;
        qreal spacing;

        ContainerData item;
        ContainerData background;
        ContainerData content;
    };

    struct TestData {
        std::string name;
        LayoutData layout;
        LayoutData expected;
    };

    std::optional<ContainerData> containerDataFromJson(const QJsonObject &json)
    {
        if (json.isEmpty()) {
            return std::nullopt;
        }

        ContainerData data;
        if (json.contains(u"x") && json.contains(u"y")) {
            data.expectedPosition = QPointF{json[u"x"].toDouble(), json[u"y"].toDouble()};
        }
    }

    std::optional<LayoutData> layoutDataFromJson(const QJsonObject &json)
    {
        if (json.isEmpty()) {
            return std::nullopt;
        }

        LayoutData data;
        data.size = QSizeF{json[u"width"].toDouble(), json[u"height"].toDouble()};
        data.spacing = json[u"spacing"].toDouble();

        auto container = containerDataFromJson(json[u"item"].toObject());
        if (!container) {
            return std::nullopt;
        }
        data.item = container.value();

        container = containerDataFromJson(json[u"background"].toObject());
        if (!container) {
            return std::nullopt;
        }
        data.background = container.value();

        container = containerDataFromJson(json[u"content"].toObject());
        if (!container) {
            return std::nullopt;
        }
        data.content = container.value();

        return data;
    }

    QList<TestData> testDataFromJson()
    {
        QList<TestData> result;

        auto data = QFINDTESTDATA("TestPositionerLayoutData");
        for (const auto &entry : std::filesystem::directory_iterator(data.toStdString())) {
            QFile file(entry.path());
            if (!file.open(QFile::ReadOnly)) {
                return result;
            }

            QJsonParseError error;
            auto json = QJsonDocument::fromJson(file.readAll(), &error);
            file.close();

            if (error.error != QJsonParseError::NoError) {
                qWarning() << "JSON file" << entry.path().filename().string() << "failed to parse:" << qPrintable(error.errorString());
                QTest::Internal::maybeThrowOnFail();
                continue;
            }

            TestData data;
            data.name = entry.path().filename();

            auto layout = layoutDataFromJson(json.object()[u"layout"].toObject());
            if (!layout) {
                qWarning() << "Invalid layout in test file" << entry.path().filename().string();
                QTest::Internal::maybeThrowOnFail();
                continue;
            }
            data.layout = layout.value();

            auto expected = layoutDataFromJson(json.object()[u"expected"].toObject());
            if (!expected) {
                qWarning() << "Invalid expected data in test file" << entry.path().filename().string();
                QTest::Internal::maybeThrowOnFail();
                continue;
            }
            data.expected = expected.value();

            result.append(data);
        }

        return result;
    }

private Q_SLOTS:
    void testLayout_data()
    {
        auto data = testDataFromJson();
    }

    void testLayout()
    {

    }


    void testItemContainer()
    {
        Layout layout;
        // layout.spacing = 5.0;

        // auto container = &layout.itemContainer;
        //
        // container->start.items.append(makeItem(10, 10));
        // container->center.items.append(makeItem(20, 10));
        // container->end.items.append(makeItem(10, 20));
        //
        // layout.layout();
        //
        // QCOMPARE(container->implicitSize, QSizeF(50.0, 20.0));
        // QCOMPARE(container->size, QSizeF(50.0, 20.0));
        // QCOMPARE(container->start.position, QPointF(0.0, 0.0));
        // QCOMPARE(container->start.size, QSizeF(10.0, 10.0));
        // QCOMPARE(container->center.position, QPointF(15.0, 0.0));
        // QCOMPARE(container->center.size, QSizeF(20.0, 10.0));
        // QCOMPARE(container->end.position, QPointF(40.0, 0.0));
        // QCOMPARE(container->end.size, QSizeF(10.0, 20.0));
        //
        // layout.size = QSizeF{100.0, 30.0};
        // layout.layout();
        //
        // QCOMPARE(container->implicitSize, QSizeF(50.0, 20.0));
        // QCOMPARE(container->size, QSizeF(100.0, 30.0));
        // QCOMPARE(container->start.position, QPointF(0.0, 0.0));
        // QCOMPARE(container->start.size, QSizeF(10.0, 30.0));
        // QCOMPARE(container->center.position, QPointF(40.0, 0.0));
        // QCOMPARE(container->center.size, QSizeF(20.0, 30.0));
        // QCOMPARE(container->end.position, QPointF(90.0, 0.0));
        // QCOMPARE(container->end.size, QSizeF(10.0, 30.0));

        // layout.size = container->implicitSize;
        // layout.layout();




    }
};

QTEST_MAIN(TestPositionerLayout)

#include "TestPositionerLayout.moc"
