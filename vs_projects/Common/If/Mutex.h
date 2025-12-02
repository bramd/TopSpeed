/**
* Common library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#ifndef __COMMON_MUTEX_H__
#define __COMMON_MUTEX_H__

#include <Common/If/Common.h>

class CMutex;
class CSingleLock;

class Mutex
{
public:
    ///@name Constructor and destructor
    //@{
    _common_ Mutex( );
    _common_ virtual ~Mutex( );
    //@}
public:
    // Note: renamed from lock/unlock to enter/leave to match implementation
    _common_ void       enter( );
    _common_ void       leave( );
    // Aliases for compatibility
    void lock() { enter(); }
    void unlock() { leave(); }

public:
    class Guard;
private:
#ifdef __EMSCRIPTEN__
    // No mutex needed - single threaded in WASM
    int m_dummy;
#else
    CRITICAL_SECTION    m_criticalSection;
#endif
};


class Mutex::Guard
{
public:
    Guard(Mutex& mutex) : m_mutex(mutex)    { m_mutex.lock( );      }
    ~Guard( )                               { m_mutex.unlock( );    }
    Mutex::Guard& operator= (const Mutex::Guard&) { return *this; }
private:
    Mutex&  m_mutex;
};

#endif /* __COMMON_MUTEX_H__ */
