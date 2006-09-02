/***************************************************************************
 *   Copyright (C) 2005 by Rajko Albrecht                                  *
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
#include "urldlg.h"
#include <kcombobox.h>
#include <kurlrequester.h>
#include <qlayout.h>
#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <klineedit.h>
#include <kurl.h>
#include <kdebug.h>

#include <qlabel.h>

UrlDlg::UrlDlg(QWidget *parent, const char *name)
 : KDialogBase(Plain, QString::null, Ok|Cancel|User1, Ok, parent, name,
                true,true, KStdGuiItem::clear())
{
    init_dlg();
}


UrlDlg::~UrlDlg()
{
}


/*!
    \fn UrlDlg::init_dlg
 */
void UrlDlg::init_dlg()
{
    QVBoxLayout * topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());
    QLabel * label = new QLabel(i18n("Open repository or working copy") , plainPage());
    topLayout->addWidget(label);

    KHistoryCombo * combo = new KHistoryCombo(0,"history_combo");
    combo->setDuplicatesEnabled(false);
    KConfig *kc = KGlobal::config();
    KConfigGroupSaver ks( kc, QString::fromLatin1("Open-repository settings") );
    int max = kc->readNumEntry( QString::fromLatin1("Maximum history"), 15 );
    combo->setMaxCount( max );
    QStringList list = kc->readListEntry( QString::fromLatin1("History") );
    combo->setHistoryItems(list);
    combo->setMinimumWidth(100);
    combo->adjustSize();
    if (combo->width()>300) {
        combo->resize(300,combo->height());
    }

    urlRequester_ = new KURLRequester(combo, plainPage(), "urlRequester");
    topLayout->addWidget( urlRequester_ );
    urlRequester_->setFocus();
    KFile::Mode mode = static_cast<KFile::Mode>(KFile::ExistingOnly|KFile::Directory);
    urlRequester_->setMode(mode);
    connect(urlRequester_->comboBox(),SIGNAL(textChanged(const QString&)),SLOT(slotTextChanged(const QString&)));
    enableButtonOK( false );
    enableButton( KDialogBase::User1, false );
    connect( this, SIGNAL(user1Clicked()), SLOT(slotClear()));
    urlRequester_->adjustSize();
    resize(QSize(400,sizeHint().height()));
}

/*!
    \fn UrlDlg::accept()
 */
void UrlDlg::accept()
{
    KHistoryCombo *combo = static_cast<KHistoryCombo*>(urlRequester_->comboBox());
    if (combo) {
        combo->addToHistory(urlRequester_->url());
        KConfig *kc = KGlobal::config();
        KConfigGroupSaver ks(kc, QString::fromLatin1("Open-repository settings"));
        kc->writeEntry(QString::fromLatin1("History"), combo->historyItems());
        kc->sync();
    }
    KDialogBase::accept();
}


/*!
    \fn UrlDlg::slotTextChanged(const QString&)
 */
void UrlDlg::slotTextChanged(const QString&text)
{
    bool state = !text.stripWhiteSpace().isEmpty();
    enableButtonOK( state );
    enableButton( KDialogBase::User1, state );
}


/*!
    \fn UrlDlg::slotClear()
 */
void UrlDlg::slotClear()
{
    urlRequester_->clear();
}


/*!
    \fn UrlDlg::selectedURL()
 */
KURL UrlDlg::selectedURL()
{
    if ( result() == QDialog::Accepted ) {
        KURL uri = urlRequester_->url();
        return uri;
        //return KURL::fromPathOrURL( urlRequester_->url() );
    } else {
        return KURL();
    }
}


/*!
    \fn UrlDlg::getURL(QWidget*parent)
 */
KURL UrlDlg::getURL(QWidget*parent)
{
    UrlDlg dlg(parent);
    dlg.setCaption(i18n("Open"));
    dlg.exec();
    const KURL& url = dlg.selectedURL();
    return url;
}

#include "urldlg.moc"
