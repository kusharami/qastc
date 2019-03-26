#ifndef _MATH_MACROS_H_
#define _MATH_MACROS_H_

template <typename primNumType, typename secNumType>
inline primNumType min( const primNumType& a, const secNumType& b )
{
    return ( a < b ? a : primNumType(b) );
}

template <typename primNumType, typename secNumType>
inline primNumType max( const primNumType& a, const secNumType& b )
{
    return ( a > b ? a : primNumType(b) );
}

#endif //_MATH_MACROS_H_