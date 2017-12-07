#   include "PybindSource.h"

#   include "pybind/kernel.hpp"

#   include "PybindFunctionProvider.h"
#   include "PybindCallbackProvider.h"
#   include "PybindScopeProvider.h"
#   include "PybindIfProvider.h"
#   include "PybindSwitchProvider.h"
#   include "PybindScopeProvider.h"
#   include "PybindGuardProvider.h"

namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
    PybindSource::PybindSource()
        : m_kernel(nullptr)
    {
    }
    //////////////////////////////////////////////////////////////////////////
    PybindSource::~PybindSource()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindSource::addFunction( const pybind::object & _obj, const pybind::detail::args_operator_t & _args )
    {
        GOAP::FunctionProviderPtr provider = GOAP_NEW PybindFunctionProvider( _obj, _args );

        this->addFunctionProvider( provider );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindSource::addCallback( const pybind::object & _obj, const pybind::detail::args_operator_t & _args )
    {
        GOAP::CallbackProviderPtr provider = GOAP_NEW PybindCallbackProvider( _obj, _args );

        this->addCallbackProvider( provider );
    }
    //////////////////////////////////////////////////////////////////////////
    void PybindSource::addScope( const pybind::object & _obj, const pybind::detail::args_operator_t & _args )
    {
        GOAP::ScopeProviderPtr provider = GOAP_NEW PybindScopeProvider( _obj, _args );

        this->addScopeProvider( provider );
    }
    //////////////////////////////////////////////////////////////////////////
    pybind::tuple PybindSource::addIf( const pybind::object & _obj, const pybind::detail::args_operator_t & _args )
    {
        GOAP::IfProviderPtr provider = GOAP_NEW PybindIfProvider( _obj, _args );

        GOAP::IfSource desc = this->addIfProvider( provider );

        return pybind::make_tuple_t( m_kernel, desc.source_true, desc.source_false );
    }
    //////////////////////////////////////////////////////////////////////////
    pybind::tuple PybindSource::addSwitch( size_t _count, const pybind::object & _obj, const pybind::detail::args_operator_t & _args )
    {
        GOAP::SwitchProviderPtr provider = GOAP_NEW PybindSwitchProvider( _obj, _args );

        const GOAP::VectorSources & sources = this->addSwitchProvider( provider, _count );

        return pybind::make_tuple_container_t( m_kernel, sources );
    }
    //////////////////////////////////////////////////////////////////////////
    GOAP::SourcePtr PybindSource::addRepeat( const pybind::object & _obj, const pybind::detail::args_operator_t & _args )
    {
        GOAP::ScopeProviderPtr provider = GOAP_NEW PybindScopeProvider( _obj, _args );

        GOAP::SourcePtr source = this->addRepeatProvider( provider );

        return source;
    }
    //////////////////////////////////////////////////////////////////////////
    GOAP::SourcePtr PybindSource::addGuard( const pybind::object & _begin, const pybind::object & _end, const pybind::detail::args_operator_t & _args )
    {
        GOAP::GuardProviderPtr begin_provider = GOAP_NEW PybindGuardProvider( _begin, _args );
        GOAP::GuardProviderPtr end_provider = GOAP_NEW PybindGuardProvider( _end, _args );

        GOAP::SourcePtr source = this->addGuardProvider( begin_provider, end_provider );

        return source;
    }
    //////////////////////////////////////////////////////////////////////////
    PyObject * PybindSource::_embedded()
    {
        pybind::kernel_interface * kernel = pybind::get_kernel();

        PyObject * py_obj = kernel->scope_create_holder_t( this );

        return py_obj;
    }
    //////////////////////////////////////////////////////////////////////////
    GOAP::SourcePtr PybindSource::_provideSource()
    {
        return GOAP_NEW PybindSource;
    }
}