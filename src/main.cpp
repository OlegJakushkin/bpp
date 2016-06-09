#define APP_VERSION QString("v0.0.3")
#define APP_NAME QString("bpp")
#define APP_NAME_FULL tr("Bullet Physics Playground")
#define APP_ORGANIZATION QString("bullet-physics-playground.github.io")

#include <QApplication>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <GL/freeglut.h>

#include "gui.h"
#include "viewer.h"

QTextStream& qStdOut() { static QTextStream ts( stdout ); return ts; }
QTextStream& qStdErr() { static QTextStream ts( stderr ); return ts; }

int main(int argc, char **argv) {
    QApplication application(argc, argv);

    QSettings *settings = new QSettings(APP_ORGANIZATION, APP_NAME);

    application.setApplicationVersion(APP_VERSION);

    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption luaOption(QStringList() << "f" << "file",
                                 QObject::tr("Runs the given Lua script without GUI."), "file");
    QCommandLineOption luaExpressionOption(QStringList() << "l" << "lua",
                                           QObject::tr("Runs the given Lua expression without GUI."), "expression");
    QCommandLineOption luaStdinOption(QStringList() << "i" << "stdin",
                                           QObject::tr("Interprets Lua code from stdin without GUI."));
    QCommandLineOption nOption(QStringList() << "n" << "frames",
                               QObject::tr("Number of frames to simulate."), "n", "10");
    QCommandLineOption verboseOption(QStringList() << "V" << "verbose",
                                     QObject::tr("Verbose output."));
    parser.addOption(luaOption);
    parser.addOption(luaExpressionOption);
    parser.addOption(luaStdinOption);
    parser.addOption(nOption);
    parser.addOption(verboseOption);

    parser.process(application);

    if (!parser.isSet(luaOption) && !parser.isSet(luaStdinOption) && !parser.isSet(luaExpressionOption)) {
        Gui *g;

        glutInit(&argc,argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

        if (!QIcon::hasThemeIcon("document-new")) {
            QIcon::setThemeName("humanity");
        }

        g = new Gui(settings);
        g->show();

        return application.exec();
    } else {
        QStringList lua = parser.values(luaOption);
        QStringList luaExpression = parser.values(luaExpressionOption);

        if (lua.isEmpty() && luaExpression.isEmpty() && !parser.isSet(luaStdinOption)) {
            qStdErr() << QObject::tr("Error: Option '--lua' requires a Lua script file as an argument. Exiting.") << endl;
            return EXIT_FAILURE;
        }

        QString txt;

        if (!lua.isEmpty()) {
            QFile file(lua[0]);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString errMsg = file.errorString();
                qStdErr() << QObject::tr("Error: reading '%1': %2. Exiting.").arg(lua[0], errMsg) << endl;
                return EXIT_FAILURE;
            }

            QTextStream in(&file);
            txt = in.readAll();
            file.close();
        }

        if (parser.isSet(luaStdinOption)) {
            QTextStream in(stdin);
            txt += "\n" + in.readAll();
        }

        if (!luaExpression.isEmpty()) {
            txt += "\n" + luaExpression[0];
        }

        int n = parser.value(nOption).toInt();
        if (n < 1) {
            qStdErr() << QObject::tr("Error: -n must be > 1. Exiting.") << endl;
            return EXIT_FAILURE;
        }

        Viewer *v = new Viewer();
        v->setSettings(settings);

        QObject::connect(v, &Viewer::scriptHasOutput, [=](QString o) {
            qStdOut() << o << endl;
        });
        QObject::connect(v, &Viewer::statusEvent, [=](QString e) {
            qStdErr() << e << endl;
        });

        if (parser.isSet("verbose"))  {
            QObject::connect(v, &Viewer::scriptStarts, [=]() {
                qStdErr() << "scriptStarts()" << endl;
            });
            QObject::connect(v, &Viewer::scriptStopped, [=]() {
                qStdErr() << "scriptStoppend()" << endl;
            });
            QObject::connect(v, &Viewer::scriptFinished, [=]() {
                qStdErr() << "scriptFinished()" << endl;
            });
        }

        if (!lua.isEmpty()) {
          v->setScriptName(lua[0]);
        } else {
            v->setScriptName("stdin.lua");
        }

        v->parse(txt);
        v->startSim();

        for (int i = 0; i < n; ++i) {
            v->animate();
        }

        v->close();

        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);

        return application.exec();
    }
}
