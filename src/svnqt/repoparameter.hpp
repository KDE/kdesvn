#ifndef REPOPARAMETER_H_
#define REPOPARAMETER_H_

#include <QString>

#include "svnqt/shared_pointer.hpp"
#include "svnqt/svnqt_defines.hpp"

namespace svn {

namespace repository {

struct CreateRepoParameterData;

class SVNQT_EXPORT CreateRepoParameter
{
    SharedPointer<CreateRepoParameterData> _data;

public:
    CreateRepoParameter();
    ~CreateRepoParameter();

    /** path to create
     * default is emtpy
     */
    const QString&path()const;
    /** path to create
     * default is emtpy
     */
    CreateRepoParameter&path(const QString&);
    /** fs type of repository
     *
     * default is "fsfs"
     */
    const QString&fstype()const;
    /** fs type of repository
     *
     * default is "fsfs"
     */
    CreateRepoParameter&fstype(const QString&);
    /** switch of syncing of bdb
     *
     * default is false
     */
    bool bdbnosync()const;
    /** switch of syncing of bdb
     *
     * default is false
     */
    CreateRepoParameter&bdbnosync(bool);
    /** bdb automatic remove log
     *
     * default is true
     */
    bool bdbautologremove()const;
    /** bdb automatic remove log
     *
     * default is true
     */
    CreateRepoParameter&bdbautologremove(bool);
    /** default is false */
    bool pre14_compat()const;
    /** default is false */
    CreateRepoParameter&pre14_compat(bool);
    /** default is false */
    bool pre15_compat()const;
    /** default is false */
    CreateRepoParameter&pre15_compat(bool);
    /** default is false */
    bool pre16_compat()const;
    /** default is false */
    CreateRepoParameter&pre16_compat(bool);

};

} // namespace repository
} // namespace svn
#endif
