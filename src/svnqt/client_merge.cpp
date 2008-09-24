#include "svnqt/client_impl.hpp"

// subversion api
#include "svn_client.h"

#include "svnqt/exception.hpp"
#include "svnqt/pool.hpp"
#include "svnqt/targets.hpp"
#include "svnqt/svnqt_defines.hpp"
#include "svnqt/stringarray.hpp"

#include "svnqt/helper.hpp"

namespace svn
{
void Client_impl::merge (const Path & path1, const Revision & revision1,
                 const Path & path2, const Revision & revision2,
                 const Path & localPath,
                 bool force,
                 Depth depth,
                 bool notice_ancestry,
                 bool dry_run,
                 bool record_only,
                 const StringArray&merge_options
                     ) throw (ClientException)
{
    Pool pool;
    svn_error_t * error = 0;
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
    error = svn_client_merge3(path1.cstr (),
                    revision1.revision (),
                    path2.cstr (),
                    revision2.revision (),
                    localPath.cstr (),
                    internal::DepthToSvn(depth),
                    !notice_ancestry,
                    force,
                    record_only,
                    dry_run,
                    merge_options.array(pool),
                    *m_context,
                    pool);
#else
    bool recurse = depth==DepthInfinity;
    Q_UNUSED(record_only);
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4))
    error = svn_client_merge2(path1.cstr (),
                    revision1.revision (),
                    path2.cstr (),
                    revision2.revision (),
                    localPath.cstr (),
                    recurse,
                    !notice_ancestry,
                    force,
                    dry_run,
                    merge_options.array(pool),
                    *m_context,
                    pool);
#else
    Q_UNUSED(merge_options);
    error = svn_client_merge(path1.cstr (),
                        revision1.revision (),
                        path2.cstr (),
                        revision2.revision (),
                        localPath.cstr (),
                        recurse,
                        !notice_ancestry,
                        force,
                        dry_run,
                        *m_context,
                        pool);
#endif
#endif

    if(error != 0) {
        throw ClientException (error);
    }
  }

  void Client_impl::merge_peg(const Path&src,
                              const RevisionRanges&ranges,
                    const Revision&peg,
                    const Path&targetWc,
                    Depth depth,
                    bool notice_ancestry,
                    bool dry_run,
                    bool force,
                    bool record_only,
                    const StringArray&merge_options
                   ) throw (ClientException)
  {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
      Pool pool;
      internal::RevisionRangesToHash _rhash(ranges);

      svn_error_t*error;

      error = svn_client_merge_peg3(
                                    src.cstr(),
                                    _rhash.array(pool),
                                    peg,
                                    targetWc.cstr(),
                                    internal::DepthToSvn(depth),
                                    !notice_ancestry,
                                    force,
                                    record_only,
                                    dry_run,
                                    merge_options.array(pool),
                                    *m_context,
                                    pool
                                   );
      if(error != 0) {
          throw ClientException (error);
      }
#else
      Q_UNUSED(record_only);
      for (RevisionRanges::size_type i=0;i<ranges.count();++i) {
          merge_peg(src,ranges[i],peg,targetWc,depth,notice_ancestry,dry_run,force,merge_options);
      }
#endif
  }

    void Client_impl::merge_peg(const Path&src,
                              const RevisionRange&range,
                              const Revision&peg,
                              const Path&targetWc,
                              Depth depth,
                              bool notice_ancestry,
                              bool dry_run,
                              bool force,
                              const StringArray&merge_options
                             ) throw (ClientException)
    {
#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 5)) || (SVN_VER_MAJOR > 1)
        RevisionRanges ranges;
        ranges.append(range);
        merge_peg(src,ranges,peg,targetWc,depth,notice_ancestry,dry_run,force,false,merge_options);
#else
        Pool pool;
        bool recurse=depth==DepthInfinity;
        svn_error_t*error;

#if ((SVN_VER_MAJOR == 1) && (SVN_VER_MINOR >= 4)) || (SVN_VER_MAJOR > 1)
        error = svn_client_merge_peg2(
                                      src.cstr(),
                                      range.first,
                                      range.second,
                                      peg.revision(),
                                      targetWc.cstr(),
                                      recurse,
                                      !notice_ancestry,
                                      force,
                                      dry_run,
                                      merge_options.array(pool),
                                      *m_context,
                                      pool
                                     );
#else
        Q_UNUSED(merge_options);
        error = svn_client_merge_peg(
                                      src.cstr(),
                                      range.first,
                                      range.second,
                                      peg.revision(),
                                      targetWc.cstr(),
                                      recurse,
                                      !notice_ancestry,
                                      force,
                                      dry_run,
                                      *m_context,
                                      pool
                                     );
#endif
        if(error != 0) {
            throw ClientException (error);
        }
#endif
    }

}
