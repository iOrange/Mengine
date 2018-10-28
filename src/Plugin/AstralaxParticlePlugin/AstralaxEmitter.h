#pragma once

#include "AstralaxInterface.h"
#include "UnknownParticleEmitterInterface.h"

#include "ResourceAstralax.h"

#include "Kernel/Node.h"
#include "Kernel/BaseEventation.h"
#include "Kernel/BaseAnimation.h"
#include "Kernel/BaseUpdation.h"
#include "Kernel/BaseRender.h"
#include "Kernel/AnimationEventReceiver.h"
#include "Kernel/ResourceHolder.h"
#include "Kernel/Polygon.h"

#include "Config/Vector.h"

#include "math/mat3.h"

#define MENGINE_PARTICLE_MAX_ATLAS_TEXTURE 8

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    enum ParticleEmitter2EventFlag
    {
    };
    //////////////////////////////////////////////////////////////////////////
    class ParticleEmitter2EventReceiver
        : public AnimationEventReceiver
    {
    public:
    };
    //////////////////////////////////////////////////////////////////////////
    class AstralaxEmitter
        : public Node
        , public BaseEventation
        , public BaseUpdation
        , public BaseRender
        , public BaseAnimation
        , public AstralaxPositionProviderInterface
        , public AstralaxCameraProviderInterface
        , public UnknownParticleEmitterInterface
    {
        DECLARE_UNKNOWABLE();
        DECLARE_ANIMATABLE();
        DECLARE_UPDATABLE();
        DECLARE_RENDERABLE();
        DECLARE_EVENTABLE( ParticleEmitter2EventReceiver );

    public:
        AstralaxEmitter();
        ~AstralaxEmitter() override;

    public:
        bool _play( uint32_t _enumerator, float _time ) override;
        bool _restart( uint32_t _enumerator, float _time ) override;
        void _pause( uint32_t _enumerator ) override;
        void _resume( uint32_t _enumerator, float _time ) override;
        void _stop( uint32_t _enumerator ) override;
        void _end( uint32_t _enumerator ) override;
        bool _interrupt( uint32_t _enumerator ) override;

    public:
        void setEmitterTranslateWithParticle( bool _translateWithParticle ) override;
        void setEmitterPositionRelative( bool _positionRelative ) override;
        void setEmitterCameraRelative( bool _cameraRelative ) override;

        void setEmitterPositionProviderOriginOffset( const mt::vec3f & _originOffset ) override;

        void changeEmitterImage( const ConstString & _emitterImageName );
        void removeEmitterImage();

        bool changeEmitterPolygon( const Polygon & _polygon );
        void removeEmitterPolygon();

        void setResourceParticle( const ResourcePtr & _resource ) override;
        const ResourcePtr & getResourceParticle() const override;

        float getDuration() const;

        void setEmitterRandomMode( bool _randomMode );
        bool getEmitterRandomMode() const;

    protected:
        void _setLoop( bool _value ) override;

    protected:
        bool _activate() override;
        void _deactivate() override;

        bool _compile() override;
        void _release() override;

        void _update( const UpdateContext * _context ) override;
        void _render( const RenderContext * _state ) override;

        void _updateBoundingBox( mt::box2f& _boundingBox ) const override;

    private:
        bool compileEmitterImage_( const AstralaxEmitterInterfacePtr & _emitter );
        bool compilePolygon_( const AstralaxEmitterInterfacePtr & _emitter );

    protected:
        void updateVertexColor_( RenderVertex2D * _vertices, uint32_t _verticesCount );
        void updateVertexWM_( RenderVertex2D * _vertices, uint32_t _verticesCount );

    protected:
        void onProviderEmitterPosition( mt::vec3f & _position ) override;
        void onProviderEmitterCamera( bool & _orthogonality, mt::vec3f & _position, mt::vec3f & _direction ) override;

    protected:
        ResourceHolder<ResourceAstralax> m_resourceParticle;

        ConstString m_emitterImageName;

        AstralaxEmitterInterfacePtr m_emitter;

        Polygon m_polygon;

        mt::vec3f m_positionProviderOriginOffset;

        bool m_emitterPositionRelative;
        bool m_emitterCameraRelative;

        bool m_randomMode;

        RenderVertex2D * m_vertices;
        uint32_t m_vertexCount;

        RenderIndex * m_indicies;
        uint32_t m_indexCount;

        bool m_emitterTranslateWithParticle;
    };
    //////////////////////////////////////////////////////////////////////////
    typedef IntrusivePtr<AstralaxEmitter> ParticleEmitter2Ptr;
    //////////////////////////////////////////////////////////////////////////
}
