
#ifndef _KDESVNPART_H_
#define _KDESVNPART_H_

#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kparts/factory.h>
#include <kparts/statusbarextension.h>
#include <kparts/browserextension.h>

class kdesvnView;
class QPainter;
class KURL;
class KdesvnBrowserExtension;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Rajko Albrecht <rajko.albrecht@tecways.com>
 * @version 0.1
 */
class kdesvnPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kdesvnPart(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name, const QStringList&);

    /**
     * Destructor
     */
    virtual ~kdesvnPart();
    virtual bool closeURL();
    static KAboutData* createAboutData();

signals:
    void refreshTree();

public slots:
    virtual bool openURL (const KURL &url);
    virtual void slotDispPopup(const QString&);
    virtual void slotFileProperties();
protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();
    virtual void setupActions();

protected slots:
    void fileOpen();
    virtual void slotLogFollowNodes(bool);
    virtual void slotDisplayIgnored(bool);
    virtual void slotDisplayUnkown(bool);
    virtual void slotUseKompare(bool);

private:
    kdesvnView *m_view;
    KdesvnBrowserExtension*m_browserExt;
};

typedef KParts::GenericFactory<kdesvnPart> kdesvnPartFactory;

class KdesvnBrowserExtension : public KParts::BrowserExtension
{
    Q_OBJECT
public:
    KdesvnBrowserExtension( kdesvnPart * );
    virtual ~KdesvnBrowserExtension();
    void setPropertiesActionEnabled(bool enabled);

public slots:
    void properties();
};

#endif // _KDESVNPART_H_
