/****************************************************
ebrowser.cpp

   Part of Data Import for Ontario Tire Stewardship
        Copyright (C) Joshua Netterfield 2012

                 All rights reserved.
*****************************************************/

#include "ebrowser.h"
#include <QtWebKit/QWebFrame>
#include <QGridLayout>
#include <QDebug>
#include <QWebElement>
#include <QMessageBox>

class EWebPage : public QWebPage {
public:
    EWebPage() : QWebPage() {}
    void javaScriptAlert(QWebFrame *originatingFrame, const QString &msg) {}
    bool javaScriptConfirm(QWebFrame *originatingFrame, const QString &msg) {return 1;}
};

EBrowser::EBrowser(QWidget *parent)
    : QWidget(parent), s_webView(new EWebView(this)), s_webPath(new QLabel(this)), s_question(new QLabel(this))
{
    setWindowTitle("Learn Import");
    s_webView->setPage(new EWebPage);
    setLayout(new QGridLayout);
    layout()->addWidget(s_question);
    QFont f=s_question->font();
    f.setBold(1);
    f.setPointSize(14);
    s_question->setFont(f);
    s_question->setAlignment(Qt::AlignCenter);
    layout()->addWidget(s_webView);
    layout()->addWidget(s_webPath);
    s_webView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    connect(s_webView->page(),SIGNAL(loadFinished(bool)),this,SLOT(loadFinishedLogic(bool)));
    connect(s_webView,SIGNAL(urlChanged(QUrl)),this,SLOT(urlChangedLogic(QUrl)));
}

EBrowser::~EBrowser()
{

}

QString EBrowser::html() const
{
    return s_webView->page()->mainFrame()->toHtml();
}

bool EBrowser::htmlContains(const QString& contains) const
{
    return s_webView->page()->mainFrame()->toHtml().contains(contains);
}

void EBrowser::askQuestion(const QString &question)
{
    s_question->setText(question);
}

QVariant EBrowser::doJS(const QString &js)
{
    return s_webView->page()->mainFrame()->evaluateJavaScript(js);
}

void EBrowser::loadPage(QString page)
{
    s_webView->page()->mainFrame()->setUrl(QUrl(page));
}

void EBrowser::loadFinishedLogic(bool s)
{
    if(!s)
    {
        QWebPage::ErrorPageExtensionReturn ret;
        s_webView->page()->extension(QWebPage::ErrorPageExtension,0,&ret);

        if(ret.content.isEmpty())
        {
            s_webView->setHtml("<center><b>An unknown error occured in loading the webpage.</B></center>");
        }
        else
        {
            s_webView->setUrl(ret.baseUrl);
        }
    }
    else
    {
        QWebFrame *frame = s_webView->page()->mainFrame();
        elements.clear();
        elements.push_back(frame->documentElement().firstChild());
        for(int i=0;i<elements.size();i++)
        {
            if(!elements[i].nextSibling().isNull())
            {
                elements.push_back(elements[i].nextSibling());
            }
            if(!elements[i].firstChild().isNull())
            {
                elements.push_back(elements[i].firstChild());
            }
        }
    }
    emit ready();
}

void EBrowser::urlChangedLogic(QUrl url)
{
    s_webPath->setText(url.toString());
}

void EBrowser::setInput(QString input, QString value)
{
    if(input.startsWith("#"))
    {
        input.remove(0,1);
    }
    for(int i=0;i<elements.size();i++)
    {
        while(value.endsWith(' ')) value.chop(1);
        while(value.startsWith(' ')) value.remove(0,1);
        if(/*elements[i].localName()=="input"&&*/elements[i].attribute("id")==input)
        {
            elements[i].evaluateJavaScript("this.focus()");
            elements[i].evaluateJavaScript("this.value=\""+value+"\"");
            elements[i].evaluateJavaScript("this.blur()");
            return;
        }
    }
    qWarning()<<"Could not find an input named"<<input<<"and give it a value of"<<value;
}

void EBrowser::clickInput(QString input)
{
    if(input.startsWith("#"))
    {
        input.remove(0,1);
    }
    for(int i=0;i<elements.size();i++)
    {
        if(elements[i].localName()=="input"&&elements[i].attribute("id")==input)
        {
            elements[i].evaluateJavaScript("this.click()");
            return;
        }
    }
    qWarning()<<"Could not click"<<input;
}

QStringList EBrowser::getInputs()
{
    QStringList ret;
    for(int i=0;i<elements.size();i++)
    {
        if(elements[i].localName()=="input")
        {
            ret.push_back(elements[i].attribute("id"));
        }
    }
    return ret;
}
