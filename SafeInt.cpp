#ifdef _WIN32
#include <windows.h>
#endif

#include "SafeInt.hpp"

SAFE_INT SAFE_INT::_uniqueIdentifier;

SAFE_INT::SAFE_INT(LONG iInitialValue)
{
   lock = new Lock("LOCK"); 
}

LONG SAFE_INT::operator++()

{
   lock->LockIt();
   LONG l = _iInt++;
   lock->UnLockIt();
   return l;
}

LONG SAFE_INT::operator++(int dummy)
{
   lock->LockIt();
   LONG l =  _iInt;
   _iInt++;
   lock->UnLockIt();  
   return l;
}

LONG SAFE_INT::operator--()

{
   lock->LockIt();
   LONG l = _iInt--;
   lock->UnLockIt();
   return l;
}

LONG SAFE_INT::operator--(int dummy)

{
   lock->LockIt();
   LONG l = _iInt;
   _iInt--;
   lock->UnLockIt();  
   return l;
}

LONG SAFE_INT::getValue()

{
   return _iInt;
}

void SAFE_INT::operator=(LONG iNewValue)

{
   lock->LockIt();
   _iInt = iNewValue;
   lock->UnLockIt();  
}

bool SAFE_INT::operator==(LONG iNewValue)

{
   return _iInt == iNewValue;
}

bool SAFE_INT::operator!=(LONG iNewValue)

{
   return _iInt != iNewValue;
}

bool SAFE_INT::operator<(LONG iNewValue)

{
   return _iInt < iNewValue;
}

bool SAFE_INT::operator>(LONG iNewValue)

{
   return _iInt > iNewValue;
}

bool SAFE_INT::operator<=(LONG iNewValue)

{
   return _iInt <= iNewValue;
}

bool SAFE_INT::operator>=(LONG iNewValue)

{
   return _iInt >= iNewValue;
}

bool operator==(const LONG iNewValue, const SAFE_INT& si)

{
   return iNewValue == si._iInt;
}

bool operator==(const SAFE_INT& si, const LONG iNewValue)

{
   return si._iInt == iNewValue;
}

bool operator==(const SAFE_INT &si1, const SAFE_INT &si2)

{
   return si1._iInt == si2._iInt;
}

LONG SAFE_INT::operator+=(LONG iAddValue)

{
   lock->LockIt();
   _iInt += iAddValue;
   LONG l = _iInt;
   lock->UnLockIt();
   return _iInt;
}


LONG SAFE_INT::getUniqueIdentifier()
{
    LONG l = getUniqueSAFE_INT().getValue();
    return l;
}

SAFE_INT SAFE_INT::getUniqueSAFE_INT()
{
    return _uniqueIdentifier++;
}


