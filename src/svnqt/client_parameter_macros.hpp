#ifndef CLIENT_PARAMETER_MACROS_HPP
#define CLIENT_PARAMETER_MACROS_HPP

#define GETSET(cl,ty,var,name) \
    cl&cl::name(const ty&x)\
    {\
        _data->var = x; return *this;\
    }\
    const ty&cl::name()const\
    {\
        return _data->var;\
    }

#define GETSETSI(cl,ty,var,name) \
    cl&cl::name(ty x)\
    {\
        _data->var = x; return *this;\
    }\
    ty cl::name()const\
    {\
        return _data->var;\
    }

#endif
