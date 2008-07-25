#include "leftpane_impl.h"

#include "leftpane_impl.moc"


leftpane_impl::leftpane_impl(QWidget*parent, Qt::WFlags fl)
//     : leftpane(parent,0,fl)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName("leftpane");
}

leftpane_impl::~leftpane_impl()
{
}

/*!
    \fn leftpane_impl::folderSelected(const QString&)
 */
void leftpane_impl::folderSelected(const QString&)
{
    /// @todo implement me
}
