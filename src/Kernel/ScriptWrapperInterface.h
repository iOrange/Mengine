#pragma once

#include "pybind/types.hpp"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    class Scriptable;
    //////////////////////////////////////////////////////////////////////////
    class ScriptWrapperInterface
        : public Mixin
    {
    public:
        virtual bool initialize() = 0;
        virtual void finalize() = 0;

    public:
        virtual PyObject * wrap( Scriptable * _node ) = 0;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<ScriptWrapperInterface> ScriptWrapperInterfacePtr;
    //////////////////////////////////////////////////////////////////////////
}