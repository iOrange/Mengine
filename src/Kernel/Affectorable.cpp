#include "Kernel/Affectorable.h"
#include "Kernel/Affector.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    Affectorable::Affectorable()
        : m_angularSpeed( 0.f )
        , m_linearSpeed( 0.f, 0.f, 0.f )
        , m_enumerator( 0 )
    {
    }
    //////////////////////////////////////////////////////////////////////////
    Affectorable::~Affectorable()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    AFFECTOR_ID Affectorable::addAffector( const AffectorPtr & _affector )
    {
        if( _affector == nullptr )
        {
            return INVALID_AFFECTOR_ID;
        }

        AFFECTOR_ID id = ++m_enumerator;

        _affector->setId( id );

        uint32_t updatableMode = this->getAffectorableUpdatableMode();
        uint32_t updatableLeaf = this->getAffectorableUpdatableLeaf();

        if( _affector->prepare( updatableMode, updatableLeaf ) == false )
        {
            return INVALID_AFFECTOR_ID;
        }

        m_affectors.push_back( _affector );

        return id;
    }
    //////////////////////////////////////////////////////////////////////////
    bool Affectorable::stopAffector( uint32_t _id )
    {
        for( TSlugAffector it( m_affectors ); it.eof() == false; )
        {
            AffectorPtr affector = *it;

            it.next_shuffle();

            if( affector->getId() == _id )
            {
                affector->stop();

                return true;
            }
        }

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void Affectorable::stopAffectors( EAffectorType _type )
    {
        for( TSlugAffector it( m_affectors ); it.eof() == false; )
        {
            AffectorPtr affector = *it;

            it.next_shuffle();

            if( affector->getAffectorType() == _type )
            {
                affector->stop();
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Affectorable::stopAllAffectors()
    {
        for( TSlugAffector it( m_affectors ); it.eof() == false; )
        {
            AffectorPtr affector = *it;

            it.next_shuffle();

            affector->stop();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Affectorable::setAngularSpeed( float _angular )
    {
        m_angularSpeed = _angular;
    }
    //////////////////////////////////////////////////////////////////////////
    float Affectorable::getAngularSpeed() const
    {
        return m_angularSpeed;
    }
    //////////////////////////////////////////////////////////////////////////
    void Affectorable::setLinearSpeed( const mt::vec3f & _linearSpeed )
    {
        m_linearSpeed = _linearSpeed;
    }
    //////////////////////////////////////////////////////////////////////////
    const mt::vec3f & Affectorable::getLinearSpeed() const
    {
        return m_linearSpeed;
    }
}
