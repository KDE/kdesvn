#include "annotate_line.h"
#include "svnqt/datetime.h"


namespace svn {
      AnnotateLine::AnnotateLine (QLONG line_no,
                    QLONG revision,
                    const char *author,
                    const char *date,
                    const char *line)
    : m_line_no (line_no), m_revision (revision),
      m_date( (date&&strlen(date))?QDateTime::fromString(QString::FROMUTF8(date),Qt::ISODate):QDateTime()),
      m_line(line?line:""),m_author(author?author:""),
      m_merge_revision(-1),
      m_merge_date(QDateTime()),
      m_merge_author(""),m_merge_path("")

    {
    }

    AnnotateLine::AnnotateLine (QLONG line_no,
                  QLONG revision,
                  const char *author,
                  const char *date,
                  const char *line,
                  QLONG merge_revision,
                  const char *merge_author,
                  const char *merge_date,
                  const char *merge_path
                 )
          : m_line_no (line_no), m_revision (revision),
            m_date( (date&&strlen(date))?QDateTime::fromString(QString::FROMUTF8(date),Qt::ISODate):QDateTime()),
            m_line(line?line:""),m_author(author?author:""),
            m_merge_revision(merge_revision),
            m_merge_date( (merge_date&&strlen(merge_date))?QDateTime::fromString(QString::FROMUTF8(merge_date),Qt::ISODate):QDateTime()),
            m_merge_author(merge_author?merge_author:""),m_merge_path(merge_path?merge_path:"")
        {
        }

    AnnotateLine::AnnotateLine(
        QLONG line_no,
        QLONG revision,
        PropertiesMap revisionproperties,
        const char *line,
        QLONG merge_revision,
        PropertiesMap mergeproperties,
        const char *merge_path,
        QLONG revstart,
        QLONG revend,
        bool local
    )
    : m_line_no (line_no), m_revision (revision),m_date(QDateTime()),m_line(line?line:""),m_merge_revision(merge_revision),m_merge_path(merge_path?merge_path:"")
    {
        QString _s = revisionproperties["svn:author"];
        m_author = _s.toUtf8();
        _s = revisionproperties["svn:date"];
        if (_s.length()>0)
        {
            m_date = QDateTime::fromString(_s,Qt::ISODate);
        }
        _s = mergeproperties["svn:author"];
        m_merge_author = _s.toUtf8();
        _s = mergeproperties["svn:date"];
        if (_s.length()>0)
        {
            m_merge_date = QDateTime::fromString(_s,Qt::ISODate);
        }
    }

    AnnotateLine::AnnotateLine ( const AnnotateLine &other)
          : m_line_no (other.m_line_no), m_revision (other.m_revision), m_date (other.m_date),
            m_line (other.m_line), m_author (other.m_author)
    {
    }
    AnnotateLine::AnnotateLine()
          : m_line_no(0),m_revision(-1),m_date(),
            m_line(), m_author()
    {
    }
}