#include <QTest>

#include "zen/core/Version.h"

class VersionTest : public QObject {
    Q_OBJECT
private slots:
    void versionIsPopulated() {
        const auto v = zen::core::version();
        QVERIFY(v.major >= 0);
        QVERIFY(!zen::core::versionString().empty());
    }
};

QTEST_MAIN(VersionTest)
#include "VersionTest.moc"
