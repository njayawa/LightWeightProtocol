#ifndef __SAFE_INT__
#define __SAFE_INT__


#include "Lock.hpp"


#ifndef _WIN32
typedef long LONG;
#endif

class SAFE_INT
{
public:
    SAFE_INT(LONG iInitialValue = 0);
    ~SAFE_INT() { if (lock) delete lock; }
    LONG operator ++();     // pre-increment
    LONG operator ++(int);  // post-increment
    LONG operator --();     // pre-decrement
    LONG operator --(int);  // post-decrement
    LONG operator +=(LONG);
    LONG getValue();
    void operator=(LONG);
    bool operator==(LONG iNewValue);
    bool operator!=(LONG iNewValue);
    bool operator<(LONG iNewValue);
    bool operator>(LONG iNewValue);
    bool operator<=(LONG iNewValue);
    bool operator>=(LONG iNewValue);
    friend bool operator==(const LONG iNewValue, const SAFE_INT& si);
    friend bool operator==(const SAFE_INT& si, const LONG iNewValue);
    friend bool operator==(const SAFE_INT &si1, const SAFE_INT &si2);
    static LONG getUniqueIdentifier();
    static SAFE_INT getUniqueSAFE_INT();

private:
    LONG               _iInt;
	Lock * lock;
    static SAFE_INT _uniqueIdentifier;
};

#endif
