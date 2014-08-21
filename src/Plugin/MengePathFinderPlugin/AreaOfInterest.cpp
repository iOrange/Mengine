#	include "AreaOfInterest.h"

#	include <algorithm>

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	AOIActor::AOIActor()
		: m_provider(nullptr)
		, m_position(0.f, 0.f)
		, m_radius(0.f)
		, m_userData(nullptr)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	void AOIActor::setProvider( AOIActorProviderInterface * _provider, void * _userData )
	{
		m_provider = _provider;
		m_userData = _userData;
	}
	//////////////////////////////////////////////////////////////////////////
	AOIActorProviderInterface * AOIActor::getProvider()
	{
		return m_provider;
	}
	//////////////////////////////////////////////////////////////////////////
	void AOIActor::update()
	{
		m_position = m_provider->getAOIActorPosition();
		m_radius = m_provider->getAOIActorRadius();
	}
	//////////////////////////////////////////////////////////////////////////
	const mt::vec2f & AOIActor::getPosition() const
	{
		return m_position;
	}	
	//////////////////////////////////////////////////////////////////////////
	float AOIActor::getRadius() const
	{
		return m_radius;
	}
	//////////////////////////////////////////////////////////////////////////
	void * AOIActor::getUserData() const
	{
		return m_userData;
	}
	//////////////////////////////////////////////////////////////////////////	
	void AOIActor::addActorNeighbor( AOIActor * _actor )
	{
		TVectorAOIActors::iterator it_erase = std::find( m_neighbours.begin(), m_neighbours.end(), _actor );

		if( it_erase != m_neighbours.end() )
		{
			return;
		}

		m_neighbours.push_back( _actor );

		m_provider->onAOIActorEnter( _actor );
	}
	//////////////////////////////////////////////////////////////////////////
	void AOIActor::removeActorNeighbor( AOIActor * _actor )
	{
		TVectorAOIActors::iterator it_erase = std::find( m_neighbours.begin(), m_neighbours.end(), _actor );

		if( it_erase == m_neighbours.end() )
		{
			return;
		}

		*it_erase = m_neighbours.back();
		m_neighbours.pop_back();

		m_provider->onAOIActorLeave( _actor );
	}
	//////////////////////////////////////////////////////////////////////////
	bool AOIActor::isActorNeighbor( AOIActor * _actor ) const
	{
		TVectorAOIActors::const_iterator it_erase = std::find( m_neighbours.begin(), m_neighbours.end(), _actor );

		if( it_erase == m_neighbours.end() )
		{
			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void AOIActor::remove()
	{
		for( TVectorAOIActors::iterator
			it = m_neighbours.begin(),
			it_end = m_neighbours.end();
		it != it_end; )
		{
			AOIActor * neignbour = *it;

			neignbour->removeActorNeighbor( this );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	AreaOfInterest::AreaOfInterest()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	static void s_removeFromActors( TVectorAOIActors & _actors, AOIActor * _actor )
	{
		TVectorAOIActors::iterator it_erase = std::find( _actors.begin(), _actors.end(), _actor );

		if( it_erase == _actors.end() )
		{
			return;
		}

		*it_erase = _actors.back();
		_actors.pop_back();
	}
	//////////////////////////////////////////////////////////////////////////
	AOIActor * AreaOfInterest::createActor( AOIActorProviderInterface * _provider, void * _userData )
	{
		AOIActor * actor = m_factoryAOIActor.createObjectT();

		actor->setProvider( _provider, _userData );

		m_actorsAdd.push_back( actor );

		return actor;			 
	}
	//////////////////////////////////////////////////////////////////////////
	void AreaOfInterest::removeActor( AOIActor * _actor )
	{		
		m_actorsRemove.push_back( _actor );

		s_removeFromActors( m_actorsAdd, _actor );
	}
	//////////////////////////////////////////////////////////////////////////
	void AreaOfInterest::update()
	{
		for( TVectorAOIActors::iterator
			it = m_actorsRemove.begin(),
			it_end = m_actorsRemove.end();
		it != it_end;
		++it )
		{
			AOIActor * actor = *it;

			actor->remove();

			s_removeFromActors( m_actors, actor );

			actor->destroy();
		}

		m_actorsRemove.clear();

		m_actors.insert( m_actors.end(), m_actorsAdd.begin(), m_actorsAdd.end() );
		m_actorsAdd.clear();

		for( TVectorAOIActors::iterator
			it = m_actors.begin(),
			it_end = m_actors.end();
		it != it_end;
		++it )
		{
			AOIActor * actor = *it;

			actor->update();

			float radius = actor->getRadius();
			float d_radius = radius * radius;

			const mt::vec2f & position = actor->getPosition();

			for( TVectorAOIActors::iterator
				it2 = m_actors.begin(),
				it2_end = m_actors.end();
			it2 != it2_end;
			++it2 )
			{
				AOIActor * actor2 = *it2;

				if( actor == actor2 )
				{
					continue;
				}
				
				const mt::vec2f & position2 = actor2->getPosition();

				if( mt::sqrlength_v2_v2( position, position2 ) < d_radius )
				{
					actor->addActorNeighbor( actor2 );
				}
				else
				{
					actor->removeActorNeighbor( actor2 );
				}				
			}
		}
	}
}