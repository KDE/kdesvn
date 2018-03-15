/***************************************************************************
 *   Copyright (C) 2006-2009 by Rajko Albrecht                             *
 *   ral@alwins-world.de                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "blamedisplay.h"
#include "ui_blamedisplay.h"

#include "simple_logcb.h"
#include "settings/kdesvnsettings.h"
#include "svnqt/log_entry.h"
#include "fronthelpers/cursorstack.h"
#include "ksvnwidgets/encodingselector_impl.h"

#include <KColorScheme>
#include <KTextEdit>
#include <KTreeWidgetSearchLine>

#include <QBrush>
#include <QFontDatabase>
#include <QInputDialog>
#include <QMap>
#include <QPushButton>
#include <QTextCodec>
#include <QTime>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

#define COL_LINENR 0
#define COL_REV 1
#define COL_DATE 2
#define COL_AUT 3
#define COL_LINE 4

#define TREE_ITEM_TYPE QTreeWidgetItem::UserType+1

#define BORDER 4

class LocalizedAnnotatedLine: public svn::AnnotateLine
{
public:
    explicit LocalizedAnnotatedLine(const svn::AnnotateLine &al)
        : svn::AnnotateLine(al)
    {}

    void localeChanged()
    {
        if (!codec_searched) {
            cc = QTextCodec::codecForName(Kdesvnsettings::locale_for_blame().toLocal8Bit());
            codec_searched = true;
        }
        if (cc) {
            m_tLine = cc->toUnicode(line().data(), line().size());
            m_tAuthor = cc->toUnicode(author().data(), author().size());
        } else {
            m_tLine = QString::fromUtf8(line().data(), line().size());
            m_tAuthor = QString::fromUtf8(author().data(), author().size());
        }
    }

    const QString &tAuthor()const
    {
        return m_tAuthor;
    }
    const QString &tLine()const
    {
        return m_tLine;
    }

    static void reset_codec()
    {
        codec_searched = false;
        cc = nullptr;
    }

protected:
    QString m_tAuthor, m_tLine;

    static bool codec_searched;
    static QTextCodec *cc;
};

QTextCodec *LocalizedAnnotatedLine::cc = nullptr;
bool LocalizedAnnotatedLine::codec_searched = false;

class BlameTreeItem: public QTreeWidgetItem
{
public:
    BlameTreeItem(const svn::AnnotateLine &al, bool disp)
      : QTreeWidgetItem(TREE_ITEM_TYPE)
      , m_Content(al)
      , m_disp(disp)
    {
        for (int i = 0; i <= COL_LINE; ++i) {
            setTextAlignment(i, Qt::AlignLeft | Qt::AlignVCenter);
            setFont(i, QFontDatabase::systemFont(QFontDatabase::FixedFont));
        }
        display();
    }

    qlonglong lineNumber() const
    {
        return m_Content.lineNumber();
    }
    svn_revnum_t rev() const
    {
        return m_Content.revision();
    }
    void localeChanged()
    {
        m_Content.localeChanged();
        if (m_disp) {
            setText(COL_AUT, m_Content.tAuthor());
        }
        QString _line = m_Content.tLine();
        _line.replace(QLatin1Char('\t'), QLatin1String("    "));
        setText(COL_LINE, _line);
    }

protected:
    LocalizedAnnotatedLine m_Content;

    bool m_disp;
    void display();
};


void BlameTreeItem::display()
{
    setTextAlignment(COL_LINENR, Qt::AlignRight | Qt::AlignVCenter);

    if (m_disp) {
        setTextAlignment(COL_REV, Qt::AlignRight | Qt::AlignVCenter);
        setText(COL_REV, QString::number(m_Content.revision()));
        if (m_Content.date().isValid()) {
            setText(COL_DATE, m_Content.date().toString(Qt::SystemLocaleShortDate));
        }
    }
    setText(COL_LINENR, QString::number(m_Content.lineNumber() + 1));
    localeChanged();
}

class BlameDisplayData
{
public:
    BlameDisplayData()
        : max(-1)
        , min(INT_MAX - 1)
        , rev_count(0)
        , up(false)
        , m_cb(nullptr)
        , m_pbGoToLine(nullptr)
        , m_pbShowLog(nullptr)
    {}

    svn_revnum_t max, min;
    QMap<svn_revnum_t, QColor> m_shadingMap;
    QMap<svn_revnum_t, svn::LogEntry> m_logCache;

    QColor m_lastCalcColor;
    unsigned int rev_count;
    bool up;
    SimpleLogCb *m_cb;
    QString m_File;

    QString reposRoot;
    QPushButton *m_pbGoToLine;
    QPushButton *m_pbShowLog;
};

BlameDisplay::BlameDisplay(const QString &what, const svn::AnnotatedFile &blame, SimpleLogCb *cb, QWidget *parent)
    : KSvnDialog(QLatin1String("blame_display_dlg"), parent)
    , m_ui(new Ui::BlameDisplay)
    , m_Data(new BlameDisplayData)
{
    m_ui->setupUi(this);
    m_Data->m_cb = cb;

    m_Data->m_pbShowLog = new QPushButton(QIcon::fromTheme(QStringLiteral("kdesvnlog")), i18n("Log message for revision"), this);
    connect(m_Data->m_pbShowLog, SIGNAL(clicked(bool)),
            this, SLOT(slotShowCurrentCommit()));
    m_ui->buttonBox->addButton(m_Data->m_pbShowLog, QDialogButtonBox::ActionRole);

    m_Data->m_pbGoToLine = new QPushButton(i18n("Go to line"), this);
    connect(m_Data->m_pbGoToLine, SIGNAL(clicked(bool)),
            this, SLOT(slotGoLine()));
    m_ui->buttonBox->addButton(m_Data->m_pbGoToLine, QDialogButtonBox::ActionRole);

    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(accept()));

    QAction *ac = new QAction(QIcon::fromTheme(QStringLiteral("kdesvnlog")), i18n("Log message for revision"), this);
    connect(ac, SIGNAL(triggered()), this, SLOT(slotShowCurrentCommit()));
    m_ui->m_BlameTree->addAction(ac);

    KTreeWidgetSearchLine *searchLine = m_ui->m_TreeSearch->searchLine();
    searchLine->addTreeWidget(m_ui->m_BlameTree);

    connect(m_ui->m_BlameTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(m_ui->m_BlameTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(m_ui->m_encodingSel, SIGNAL(TextCodecChanged(QString)),
            this, SLOT(slotTextCodecChanged(QString)));

    setContent(what, blame);
}

BlameDisplay::~BlameDisplay()
{
    delete m_Data;
    delete m_ui;
}

void BlameDisplay::setContent(const QString &what, const svn::AnnotatedFile &blame)
{
    m_Data->m_File = what;
    m_Data->m_pbShowLog->setEnabled(false);

    svn::AnnotatedFile::const_iterator bit;
    //m_BlameList->setSorting(COL_LINENR,false);
    m_Data->max = -1;
    svn_revnum_t lastRev(-1);
    QColor a(160, 160, 160);
    int offset = 10;
    int r = 0; int g = 0; int b = 0;
    uint colinc = 0;

    QTime t;
    t.start();
    QList<QTreeWidgetItem *> _list;
    _list.reserve(blame.size());
    QBrush _b, _bt, _bb;

    bool _b_init = false, _bt_init = false;

    for (bit = blame.begin(); bit != blame.end(); ++bit) {
        bool disp = (*bit).revision() != lastRev || bit == blame.begin() ;

        if ((*bit).revision() > m_Data->max) {
            m_Data->max = (*bit).revision();
            ++(m_Data->rev_count);
        }
        if ((*bit).revision() < m_Data->min) {
            m_Data->min = (*bit).revision();
        }
        BlameTreeItem *item = new BlameTreeItem((*bit), disp);
        _list.append(item);

        if (disp) {
            lastRev = (*bit).revision();
        }
        if (Kdesvnsettings::self()->colored_blame()) {
            if (m_Data->m_shadingMap.find((*bit).revision()) == m_Data->m_shadingMap.end()) {
                a.setRgb(a.red() + offset, a.green() + offset, a.blue() + offset);
                m_Data->m_shadingMap[(*bit).revision()] = a;
                if (a.red() > 245 || a.green() > 245 || a.blue() > 245) {
                    if (colinc == 0) {
                        ++colinc;
                    } else if (r >= 50 || g >= 50 || b >= 50) {
                        if (++colinc > 6) {
                            colinc = 0;
                            r = g = b = 0;
                        } else {
                            r = g = b = -10;
                        }
                    }
                    if (colinc & 0x1) {
                        r += 10;
                    }
                    if (colinc & 0x2) {
                        g += 10;
                    }
                    if (colinc & 0x4) {
                        b += 10;
                    }
                    a.setRgb(160 + r, 160 + g, 160 + b);
                }
            }
            if (!_b_init) {
                _b_init = true;
                _b = item->foreground(COL_LINENR);
                _b.setColor(KColorScheme(QPalette::Active, KColorScheme::Selection).foreground().color());
                _bb = item->background(COL_LINENR);
                _b.setStyle(Qt::SolidPattern);
                _bb.setStyle(Qt::SolidPattern);
                _bb.setColor(KColorScheme(QPalette::Active, KColorScheme::Selection).background().color());
            }
            item->setForeground(COL_LINENR, _b);
            item->setBackground(COL_LINENR, _bb);

            if (!_bt_init) {
                _bt_init = true;
                _bt = item->background(COL_REV);
                _bt.setStyle(Qt::SolidPattern);
            }
            _bt.setColor(m_Data->m_shadingMap.value((*bit).revision()));
            item->setBackground(COL_REV, _bt);
            item->setBackground(COL_DATE, _bt);
            item->setBackground(COL_AUT, _bt);
            item->setBackground(COL_LINE, _bt);
        } else {
            m_Data->m_shadingMap[(*bit).revision()] = QColor();
        }
    }
    m_ui->m_BlameTree->addTopLevelItems(_list);
    qDebug("Time elapsed: %d ms", t.elapsed());
    m_ui->m_BlameTree->resizeColumnToContents(COL_REV);
    m_ui->m_BlameTree->resizeColumnToContents(COL_DATE);
    m_ui->m_BlameTree->resizeColumnToContents(COL_AUT);
    m_ui->m_BlameTree->resizeColumnToContents(COL_LINENR);
    m_ui->m_BlameTree->resizeColumnToContents(COL_LINE);
}

void BlameDisplay::slotGoLine()
{
    bool ok = true;
    int line = QInputDialog::getInt(this, i18n("Show line"), i18n("Show line number"),
                                    1, 1, m_ui->m_BlameTree->topLevelItemCount(), 1, &ok);
    if (!ok) {
        return;
    }
    QTreeWidgetItemIterator it(m_ui->m_BlameTree);
    --line;
    while (*it) {
        BlameTreeItem *_it = static_cast<BlameTreeItem *>((*it));
        if (_it->lineNumber() == line) {
            m_ui->m_BlameTree->scrollToItem(*it);
            m_ui->m_BlameTree->setCurrentItem(*it);
            return;
        }
        ++it;
    }
}

void BlameDisplay::showCommit(BlameTreeItem *bti)
{
    if (!bti) {
        return;
    }
    QString text;
    const QMap<svn_revnum_t, svn::LogEntry>::const_iterator it = m_Data->m_logCache.constFind(bti->rev());
    if (it != m_Data->m_logCache.constEnd()) {
        text = it.value().message;
    } else {
        CursorStack a(Qt::BusyCursor);
        svn::LogEntry t;
        if (m_Data->m_cb && m_Data->m_cb->getSingleLog(t, bti->rev(), m_Data->m_File, m_Data->max, m_Data->reposRoot)) {
            m_Data->m_logCache[bti->rev()] = t;
            text = t.message;
        }
    }

    QPointer<QDialog> dlg(new KSvnDialog(QLatin1String("simplelog_display"), this));
    dlg->setWindowTitle(i18nc("@title:window", "Log Message for Revision %1", bti->rev()));
    QVBoxLayout *vbox = new QVBoxLayout(dlg);

    KTextEdit *textEdit = new KTextEdit(dlg);
    vbox->addWidget(textEdit);
    textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    textEdit->setReadOnly(true);
    textEdit->setWordWrapMode(QTextOption::NoWrap);
    textEdit->setPlainText(text);

    QDialogButtonBox *bbox = new QDialogButtonBox(dlg);
    bbox->setStandardButtons(QDialogButtonBox::Close);
    vbox->addWidget(bbox);
    // QDialogButtonBox::Close is a reject role
    connect(bbox, SIGNAL(rejected()), dlg, SLOT(accept()));

    dlg->exec();
    delete dlg;
}

void BlameDisplay::slotShowCurrentCommit()
{
    QTreeWidgetItem *item = m_ui->m_BlameTree->currentItem();
    if (item == nullptr || item->type() != TREE_ITEM_TYPE) {
        return;
    }
    BlameTreeItem *bit = static_cast<BlameTreeItem *>(item);
    showCommit(bit);
}

void BlameDisplay::slotCurrentItemChanged(QTreeWidgetItem *item, QTreeWidgetItem *)
{
    const bool enabled = item && item->type() == TREE_ITEM_TYPE;
    m_Data->m_pbShowLog->setEnabled(enabled);
}

void BlameDisplay::displayBlame(SimpleLogCb *_cb, const QString &item, const svn::AnnotatedFile &blame, QWidget *parent)
{
    QPointer<BlameDisplay> dlg(new BlameDisplay(item, blame, _cb, parent ? parent : QApplication::activeModalWidget()));
    dlg->exec();
    delete dlg;
}

void BlameDisplay::slotItemDoubleClicked(QTreeWidgetItem *item, int)
{
    if (item == nullptr || item->type() != TREE_ITEM_TYPE) {
        return;
    }
    BlameTreeItem *bit = static_cast<BlameTreeItem *>(item);
    showCommit(bit);
}

void BlameDisplay::slotTextCodecChanged(const QString &what)
{
    if (Kdesvnsettings::locale_for_blame() != what) {
        Kdesvnsettings::setLocale_for_blame(what);
        Kdesvnsettings::self()->save();
        LocalizedAnnotatedLine::reset_codec();

        QTreeWidgetItemIterator it(m_ui->m_BlameTree);

        while (*it) {
            BlameTreeItem *_it = static_cast<BlameTreeItem *>((*it));
            _it->localeChanged();
            ++it;
        }
    }
}
