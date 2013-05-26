#ifndef ELEARNDOWNLOAD_H
#define ELEARNDOWNLOAD_H

#include "eaction.h"
#include <QStringList>

class   PraiseDownload : public EAction
{
    Q_OBJECT
public:
    PraiseDownload(EBrowser& browser);
    char letter;
    QStringList urls;
    QString result;
public slots:
    void init() { continueAction(); }
    void continueAction();
    void c2();
    void c3();
    void c4();
    void c6();
};


#endif // ELEARNDOWNLOAD_H
