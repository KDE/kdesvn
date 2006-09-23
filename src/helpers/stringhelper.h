#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <qstring.h>
#include <qtextstream.h>

namespace helpers
{

class ByteToString
{
protected:

public:
    ByteToString(){};

    QString operator()(long value)
    {
        char pre = 0;
        double v = (double)value;
        if (v<0) v=0;
        while (v>=1024.0 && pre != 'T')
        {
            switch (pre)
            {
            case 'k':
                pre = 'M';
                break;
            case 'M':
                pre = 'G';
                break;
            case 'G':
                pre = 'T';
                break;
            default:
                pre = 'k';
                break;
            }
            v /= 1024.0;
        }
        QString res_;
        QTextOStream s(&res_);
        s.precision(3);
        s << v << (pre?" ":"")<<(pre?pre:' ') << "Byte";
        return res_;
    }
};

}

#endif
