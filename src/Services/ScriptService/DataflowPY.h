#pragma once

#include "Interface/DataflowInterface.h"

#include "Kernel/Factorable.h"
#include "Kernel/Factory.h"

#include "pybind/pybind.hpp"

namespace Mengine
{
    class DataflowPY
        : public DataflowInterface
        , public Factorable
    {
    public:
        DataflowPY();
        ~DataflowPY() override;

    public:
        void setKernel( pybind::kernel_interface * _kernel );
        pybind::kernel_interface * getKernel() const;

    public:
        bool initialize() override;
        void finalize() override;

    public:
        DataInterfacePtr create( const Char * _doc ) override;

    public:
        bool load( const DataInterfacePtr & _data, const InputStreamInterfacePtr & _stream, const Char * _doc ) override;

    protected:
        pybind::kernel_interface * m_kernel;

        FactoryPtr m_factoryScriptCodeData;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<DataflowPY> DataflowPYPtr;
    //////////////////////////////////////////////////////////////////////////
}