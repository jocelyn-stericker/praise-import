/****************************************************
eaction.h

   Part of Data Import for Ontario Tire Stewardship
        Copyright (C) Joshua Netterfield 2012

                 All rights reserved.
*****************************************************/

#ifndef EACTION_H
#define EACTION_H

#include <QObject>
#include "ebrowser.h"

class EAction : public QObject
{
    Q_OBJECT
protected:
    EBrowser& s_browser;
public:
    EAction(EBrowser& browser);
    virtual void init() {}
signals:
    void done();
    void error(QString);
    void info(QString);
};

#endif // EACTION_H
