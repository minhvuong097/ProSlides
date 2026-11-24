#include <QtTest>
#include <QSignalSpy>
#include <QDir>
#include "Types.h"
#include "ShowFile.h"
#include "core/ShowDocument.h"
#include "persistence/Database.h"
#include "persistence/ShowRepository.h"
#include "crdt/Document.h"
#include "timeline/TimelineEngine.h"
#include "scripture/Bible.h"
#include "timers/TimerManager.h"
#include "templates/TemplateLibrary.h"
#include "overlays/OverlayManager.h"

using namespace ps;

class ProSlidesTest : public QObject
{
    Q_OBJECT

private slots:
    void showFileRoundTrip();
    void lyricSlideParsing();
    void crdtConverges();
    void ftsVietnameseSearch();
    void timelineTriggers();
    void scriptureReferenceParsing();
    void timerLifecycle();
    void templateRoundTrip();
    void overlaySceneSwitch();
};

void ProSlidesTest::showFileRoundTrip()
{
    Show show;
    show.id = makeId();
    show.name = u"Sunday Service";

    Slide slide;
    slide.id = makeId();
    slide.group = u"Worship Set";
    slide.notes = u"Pray before song";
    Item item;
    item.type = ItemType::Text;
    Line line;
    line.align = u"center";
    TextPart part;
    part.value = u"Amazing grace";
    part.style = u"color:#fff;";
    line.text.push_back(part);
    item.lines.push_back(line);
    slide.items.push_back(item);
    show.slides.insert(slide.id, slide);

    Layout layout;
    layout.id = makeId();
    layout.name = u"Main";
    SlideData ref;
    ref.id = slide.id;
    layout.slides.push_back(ref);
    show.layouts.insert(layout.id, layout);
    show.settings.activeLayout = layout.id;

    const auto bytes = encodeShowFile(show);
    const auto result = decodeShowFile(bytes);
    QVERIFY(result.ok);
    QCOMPARE(result.show.id, show.id);
    QCOMPARE(result.show.name, show.name);
    QCOMPARE(result.show.slides.size(), size_t(1));
    QCOMPARE(result.show.slides.begin()->second.items.at(0).plainText(), u"Amazing grace");

    // FreeShow files are [id, Show] tuples.
    const auto doc = QJsonDocument::fromJson(bytes);
    QVERIFY(doc.isArray());
    QCOMPARE(doc.array().size(), 2);
}

void ProSlidesTest::lyricSlideParsing()
{
    ShowDocument doc;
    const Id id = doc.addLyricSlide(u"Worship", u"[C]When all [G]see is the battle\nYou see my victory", u"C");
    QVERIFY(!id.isEmpty());
    const auto& slide = doc.show().slides.at(id);
    QCOMPARE(slide.items.size(), size_t(2));
    QCOMPARE(slide.items.at(0).lines.at(0).chords.size(), size_t(2));
    QCOMPARE(slide.items.at(0).lines.at(0).chords.at(0).key, u"C");
    QCOMPARE(slide.items.at(0).lines.at(0).plainText(), u"When all see is the battle");
    QVERIFY(doc.canUndo());
    QVERIFY(doc.undo());
    QVERIFY(!doc.show().slides.contains(id));
}

void ProSlidesTest::crdtConverges()
{
    crdt::Document a(u"actor-a");
    crdt::Document b(u"actor-b");

    a.set({ u"slides", u"s1", u"notes" }, u"Hello from A");
    b.set({ u"slides", u"s1", u"notes" }, u"Hello from B");

    const auto aOps = a.operationsSince({});
    const auto bOps = b.operationsSince({});
    QVERIFY(b.applyBatch(aOps));
    QVERIFY(a.applyBatch(bOps));

    QCOMPARE(a.get({ u"slides", u"s1", u"notes" }).toString(),
             b.get({ u"slides", u"s1", u"notes" }).toString());

    // Edge insertion also converges.
    a.insert({ u"order" }, 0, u"first");
    b.applyBatch(a.operationsSince(b.vectorClock()));
    QCOMPARE(a.sequence({ u"order" }).size(), b.sequence({ u"order" }).size());
}

void ProSlidesTest::ftsVietnameseSearch()
{
    Database db(u":memory:");
    QString error;
    QVERIFY2(db.open(&error), qPrintable(error));
    ShowRepository repo(db);
    repo.setDataDirectory(QDir::tempPath());

    Show show;
    show.id = makeId();
    show.name = u"Thờ phượng";
    Slide slide;
    slide.id = makeId();
    slide.group = u"Chúa là tình yêu";
    Item item;
    item.type = ItemType::Text;
    Line line;
    TextPart part;
    part.value = u"Yêu Chúa hết lòng";
    line.text.push_back(part);
    item.lines.push_back(line);
    slide.items.push_back(item);
    show.slides.insert(slide.id, slide);

    QVERIFY(repo.saveShow(show, &error));
    const auto hits = repo.search(u"yeu chua");
    QVERIFY(hits.size() >= 1);
    QCOMPARE(hits.first().slideId, slide.id);
}

void ProSlidesTest::timelineTriggers()
{
    ps::timeline::TimelineEngine engine;
    Timeline timeline;
    TimelineAction a;
    a.id = makeId();
    a.name = u"Next song";
    a.type = u"slide";
    a.time = 10;
    timeline.actions.push_back(a);
    engine.setTimeline(timeline);

    QSignalSpy spy(&engine, SIGNAL(actionTriggered(const TimelineAction&)));
    engine.start();
    QTest::qWait(60);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(engine.duration(), qint64(10));
}

void ProSlidesTest::scriptureReferenceParsing()
{
    scripture::Bible bible;
    bible.name = u"Test Bible";
    scripture::Book book;
    book.number = 43;
    book.name = u"John";
    book.abbreviation = u"Jn";
    scripture::Chapter chapter;
    chapter.number = 3;
    scripture::Verse verse;
    verse.number = 16;
    verse.text = u"For God so loved the world";
    chapter.verses.append(verse);
    book.chapters.append(chapter);
    bible.books.append(book);

    const auto ref = scripture::parseReference(u"John 3:16", bible);
    QVERIFY(ref.valid);
    QCOMPARE(ref.book, 43);
    QCOMPARE(ref.chapter, 3);
    QCOMPARE(ref.verse, 16);
    QCOMPARE(scripture::referenceToString(ref, bible), u"John 3:16");

    const auto bytes = scripture::encodeBibleFile(bible);
    const auto loaded = scripture::decodeBibleFile(bytes);
    QVERIFY(loaded.ok);
    QCOMPARE(loaded.bible.books.size(), 1);
}

void ProSlidesTest::timerLifecycle()
{
    ps::timers::TimerManager manager;
    const QString id = manager.startTimer(u"Pre-service", 1, 0);
    QVERIFY(!id.isEmpty());
    QCOMPARE(manager.activeCount(), 1);
    QVERIFY(manager.pauseTimer(id));
    QCOMPARE(manager.activeCount(), 0);
    QVERIFY(manager.resumeTimer(id));
    QCOMPARE(manager.activeCount(), 1);
    QVERIFY(manager.stopTimer(id));
    QCOMPARE(manager.timers().size(), 0);
}

void ProSlidesTest::templateRoundTrip()
{
    ps::templates::Template t;
    t.id = makeId();
    t.name = u"Lyric Dark";
    t.category = u"Text";
    t.color = u"#3ddc84";
    t.settings.insert(u"backgroundColor", u"#000000");

    const auto obj = t.toJson();
    const auto loaded = ps::templates::Template::fromJson(obj);
    QCOMPARE(loaded.name, t.name);
    QCOMPARE(loaded.settings.value(u"backgroundColor").toString(), u"#000000");
}

void ProSlidesTest::overlaySceneSwitch()
{
    overlays::OverlayManager manager;
    overlays::Overlay o1;
    o1.id = u"o1";
    o1.name = u"Lower Third";
    overlays::Overlay o2;
    o2.id = u"o2";
    o2.name = u"QR Code";

    overlays::Scene scene;
    scene.id = u"s1";
    scene.name = u"Welcome";
    scene.overlays = { u"o1", u"o2" };
    scene.hiddenOverlays = { u"o2" };

    // exercise JSON round trip
    QCOMPARE(overlays::Overlay::fromJson(o1.toJson()).name, u"Lower Third");
    QCOMPARE(overlays::Scene::fromJson(scene.toJson()).overlays.size(), size_t(2));
}

QTEST_MAIN(ProSlidesTest)
#include "tst_proslides.moc"
