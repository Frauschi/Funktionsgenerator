#ifndef IDMANAGER_H
#define IDMANAGER_H

#include <cstdint>

namespace Util
{

class IdManager
{
public:

	typedef std::uint16_t IdType;

    IdManager() : next(0)
    {
    }

    inline IdType getID() const
    {
        return next = (next == MAX_ID) ? 1 : next + 1;
    }

private:

    static const IdType MAX_ID = 65535;
    volatile mutable IdType next;
};


} /* end namespace Util */

#endif
