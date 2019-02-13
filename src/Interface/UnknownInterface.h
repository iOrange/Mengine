#pragma once

#include "Interface/Interface.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    class UnknownInterface
        : public Interface
    {
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<UnknownInterface> UnknownInterfacePtr;
    //////////////////////////////////////////////////////////////////////////
}