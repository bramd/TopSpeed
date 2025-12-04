/**
* Common library
* Copyright 2003-2013 Playing in the Dark (http://playinginthedark.net)
* Code contributors: Davy Kager, Davy Loots and Leonard de Ruijter
* This program is distributed under the terms of the GNU General Public License version 3.
*/
#include <Common/If/Common.h>
#include <time.h>
#include <cstdlib>

Int
random(Int max)
{
    static Boolean firstrun = true;
    if (firstrun)
    {
        ::srand( (unsigned)time( NULL ) );
        firstrun = false;
    }
    if (max <= 0)
        return 0;
    return (rand( ) % max);
}
