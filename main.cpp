#include "mainwindow.h"

#include <QApplication>
#include <private/qguiapplication_p.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.raise();
    w.activateWindow();
    // https://forum.qt.io/topic/133694/using-alwaysactivatewindow-to-gain-foreground-in-win10-using-qt6-2/2
    if(auto inf = qApp->nativeInterface<QNativeInterface::Private::QWindowsApplication>()) {
        inf->setWindowActivationBehavior(QNativeInterface::Private::QWindowsApplication::AlwaysActivateWindow);
    }
    return a.exec();
}
