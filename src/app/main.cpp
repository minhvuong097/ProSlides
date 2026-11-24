#include "presentation/PresentationController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(u"ProSlides");
    app.setApplicationDisplayName(u"ProSlides");
    app.setOrganizationName(u"ProSlides");
    app.setOrganizationDomain(u"proslides.app");
    app.setApplicationVersion(u"0.9.0");
    QGuiApplication::setWindowIcon(QIcon(u":/resources/icons/app.svg"));

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Professional worship presentation software");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(u"file", u"Optional .show file to open", u"[file]");
    parser.process(app);

    ps::presentation::PresentationController controller;

    QQmlApplicationEngine engine;
    controller.attachEngine(&engine);
    engine.rootContext()->setContextProperty(u"ProSlides", &controller);
    engine.rootContext()->setContextProperty(u"AppVersion", u"0.9.0");

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { qCritical() << "Failed to load main.qml"; QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(QUrl(u"qrc:/qml/main.qml"));
    if (engine.rootObjects().isEmpty()) return -1;

    const auto args = parser.positionalArguments();
    if (!args.isEmpty()) controller.importShowFile(args.first());

    return app.exec();
}
