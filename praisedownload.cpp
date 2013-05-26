#include "praisedownload.h"
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include <QRegExp>
#include <QApplication>
#include <QWebFrame>
#include <QFile>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkReply>

PraiseDownload::PraiseDownload(EBrowser &browser) : EAction(browser), letter('A'-1)
{
    result += "\\documentclass[12pt]{book}\n"
            "\\usepackage{latexsym,fancyhdr}\n"
            "\\usepackage[chordbk]{songbook}\n"
            "\\newcommand{\\RelDate}{23~October,~2012}\n"
            "\\newcommand{\\RevDate}{\\today}"
            "\\begin{document}\n"
            "\\makeTitleIndex         %% Title and First Line Index.\n"
            "\\makeTitleContents      %% Table of Contents.\n"
            "\\makeKeyIndex           %% Index of song by key.\n"
            "\\makeArtistIndex        %% Index of song by artist.\n"
            "\\font\\myTinySF=cmss8    at  8pt\n"
            "\\font\\myHugeSF=cmssbx10 at 25pt\n"
            "\\newcommand{\\myTitleFont}{\\Huge\\myHugeSF}\n"
            "\\newcommand{\\mySubTitleFont}{\\large\\sf}\n";
}

void PraiseDownload::continueAction()
{
    disconnect(&s_browser,SIGNAL(ready()),this,0);

    if (letter == 'Z') {
        urls.removeDuplicates();
        c3();
        return;
    }
    s_browser.loadPage(QString("http://www.worshiparchive.com/") + QChar(++letter));
    connect(&s_browser,SIGNAL(ready()),this,SLOT(c2()));
}

void PraiseDownload::c2()
{
    disconnect(&s_browser,SIGNAL(ready()),this,0);

    QString a = s_browser.html();
    while(a.contains("/song/")) {
        a.remove(0, a.indexOf("/song/"));
        QString b = a;
        b.truncate(b.indexOf("\""));
        urls.push_back("http://www.worshiparchive.com/"+b);
        a.remove(0,a.indexOf("\""));
    }
    continueAction();
}

void PraiseDownload::c3()
{
    disconnect(&s_browser,SIGNAL(ready()),this,0);

    if (!urls.size()) {
        c6();
        qApp->quit();
        return;
    }
    qDebug() << "Requesting download of" << urls.front();

    s_browser.loadPage(urls.takeFirst());
    connect(&s_browser,SIGNAL(ready()),this,SLOT(c4()));
}

bool isChord(QString s) {
    bool ok = true;
    int c1 = 0;
    for(char cc = 'A'; cc <= 'G'; ++cc) {
        if (s.contains(QChar(cc))) ++c1;
    }
    if (c1) {
        int c2 = 0;
        for(char cc = 'H'; cc <= 'Z'; ++cc) {
            if (s.contains(QChar(cc))) ++c2;
        }
        for(char cc = 'a'; cc <= 'z'; ++cc) {
            if (cc == 'm') continue;
            if (cc == 'a') continue;
            if (cc == 'j') continue;
            if (cc == 's') continue;
            if (cc == 'u') continue;
            if (s.contains(QChar(cc))) ++c2;
        }
        if (c1>c2) ok = false;
    }
    return !ok;
}

void PraiseDownload::c4()
{
    QString a = s_browser.html();

    QString key = a;
    key.remove(0,key.indexOf("$(\"#song\").transpose({ key: '")+29);
    key.truncate(key.indexOf("'"));

    a.remove(0,a.indexOf("<h1 id=\"song-title\">")+20);

    QString title = a;
    title.truncate(title.indexOf("</h1>"));

    a.remove(0,a.indexOf("\">", a.indexOf("<h2 class=\"credits\">")+21)+2);
    QString author = a;
    author.truncate(author.indexOf("</a>"));

    a.remove(0,a.indexOf("<pre id=\"song")+15);
    a.truncate(a.indexOf("</pre>"));
    a.replace("<br>", "\n");
    while(a.contains("<span")) {
        a.remove(a.indexOf("<span"), a.indexOf(">",a.indexOf("<span")) - a.indexOf("<span")+1);
        a.replace("</span>", "");
    }


    result += "\\begin{song}{" + title + "}{"+key+"}\n";
    result += "{"+author+"}\n";
    result += "{"+author+"}\n";
    result += "{}\n";
    result += "{}\n\n";
    result += "\\renewcommand{\\RevDate}{October~23,~2012}\n";

    QStringList s = a.split("\n");
    QList<QStringList> verses;
    verses.push_back(QStringList());
    for(int i = 0; i< s.size(); ++i) {
        if (s[i].size() < 4 && !isChord(s[i])) {
            if (verses.back().size()) verses.push_back(QStringList());
        } else {
            bool ok = true;
            if (s[i].contains("verse", Qt::CaseInsensitive)) ok = false;
            if (s[i].contains("chorus", Qt::CaseInsensitive)) ok = false;
            if (s[i].contains("bridge", Qt::CaseInsensitive)) ok = false;
            if (s[i].contains("intro", Qt::CaseInsensitive)) ok = false;
            if (s[i].contains("coda", Qt::CaseInsensitive)) ok = false;
            if (!ok) {
                continue;
            }

            verses.back().push_back(s[i]);
        }
    }
    for (int i = 0; i < verses.size(); ++i) {
        for (int j = 1; j < verses[i].size(); j += 2) {
            if (isChord(verses[i][j])) {
                --j;
                continue;
            }

            bool ok = true;

            if (verses[i][j].contains("#")) ok = false;
            if (verses[i][j].contains("verse", Qt::CaseInsensitive)) ok = false;
            if (verses[i][j].contains("chorus", Qt::CaseInsensitive)) ok = false;
            if (verses[i][j].contains("bridge", Qt::CaseInsensitive)) ok = false;
            if (verses[i][j].contains("intro", Qt::CaseInsensitive)) ok = false;
            if (verses[i][j].contains("coda", Qt::CaseInsensitive)) ok = false;
            if (!ok && s[i].size() < 10) {
                --j;
                continue;
            } else ok=true;
            qDebug() << "Line:" << verses[i][j];
            if ( j >= verses[i].size()) break;
            int offset = 0;
            qDebug() << verses[i][j-1] << isChord(verses[i][j-1]);
            if (isChord(verses[i][j-1])) {
                for (int k = 0; k < verses[i][j-1].size(); ++k) {
                    if (verses[i][j-1][k] == ' ') continue;
                    int kl = k;
                    QString c;
                    do c += verses[i][j-1][k]; while (++k < verses[i][j-1].size() && verses[i][j-1][k] != ' ');
                    verses[i][j].insert(offset + kl, "\\Ch{"+c+"}{");
                    verses[i][j].insert(offset + kl+9+c.size(), "}");
                    offset += 6+1+c.size();
                }
            }
        }
        for (int j = 0; j < verses[i].size(); ++j) {
            if (isChord(verses[i][j])) {
                verses[i].removeAt(j);
                --j;
            }
        }
        if (verses[i].size()) {
            result += "\\begin{SBVerse}\n";
            result += verses[i].join("\n\n");
            result += "\n\\end{SBVerse}\n";
        }
    }
    result += "\\end{song}";

    QTimer::singleShot(800, this, SLOT(c3()));
}


void PraiseDownload::c6()
{
    result += "\\begin{center}\n"
"{\\myTitleFont Title Index}\\end{center}\n"
            "\\vskip 20pt\n"
             "\n"
            "\\begin{itemize}\n"
            "\\input{abba.adx}\n"
            "\\end{itemize}\n"
            "\\end{document}  \n";
    QFile v("sb.tex");
    v.open(QFile::WriteOnly | QFile::Text);
    v.write(result.toAscii());
    v.close();
}
