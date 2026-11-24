#include "presentation/PresentationController.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral(u"ProSlides"));
    app.setApplicationDisplayName(QStringLiteral(u"ProSlides"));
    app.setOrganizationName(QStringLiteral(u"ProSlides"));
    app.setOrganizationDomain(QStringLiteral(u"proslides.app"));
    app.setApplicationVersion(QStringLiteral(u"0.9.0"));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(u":/resources/icons/app.svg")));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(u"Professional worship presentation software"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral(u"file"), QStringLiteral(u"Optional .show file to open"), QStringLiteral(u"[file]"));
    parser.process(app);

    ps::presentation::PresentationController controller;

    QQmlApplicationEngine engine;
    controller.attachEngine(&engine);
    engine.rootContext()->setContextProperty(QStringLiteral(u"ProSlides"), &controller);
    engine.rootContext()->setContextProperty(QStringLiteral(u"AppVersion"), QStringLiteral(u"0.9.0"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { qCritical() << "Failed to load main.qml"; QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(QUrl(QStringLiteral(u"qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty()) return -1;

    const auto args = parser.positionalArguments();
    if (!args.isEmpty()) controller.importShowFile(args.first());

    return app.exec();
}
