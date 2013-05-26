#include <QApplication>
#include "ebrowser.h"
#include "praisedownload.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    EBrowser b;

    PraiseDownload w(b);
    w.init();

    return a.exec();
}
