#pragma once

#include "AstralaxInterface.h"

#include "Kernel/Resource.h"
#include "Kernel/ResourceImage.h"

#include "Kernel/Polygon.h"

#include "Config/Vector.h"

namespace Mengine
{
    class ResourceParticle
        : public Resource
    {
        DECLARE_VISITABLE( Resource );

    public:
        ResourceParticle();
        ~ResourceParticle() override;

    public:
        void setFilePath( const FilePath& _filePath );
        const FilePath& getFilePath() const;

    public:
        void setConverterType( const ConstString & _converterType );
        const ConstString & getConverterType() const;

    public:
        void addResourceImage( const ResourceImagePtr & _resourceImage );
        uint32_t getResourceImageCount() const;
        const ResourceImagePtr & getResourceImage( uint32_t _index ) const;

    public:
        ParticleEmitterInterfacePtr createEmitter();

    public:
        bool _loader( const Metabuf::Metadata * _parser ) override;
        bool _convert() override;

    protected:
        bool _compile() override;
        void _release() override;

    protected:
        FilePath m_filePath;

        ConstString m_converterType;

        typedef Vector<ResourceImagePtr> VectorResourceImages;
        VectorResourceImages m_resourceImages;

        ParticleEmitterContainerInterface2Ptr m_container;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<ResourceParticle, Resource> ResourceParticlePtr;
}
