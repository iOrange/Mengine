#	include "HotSpotBubbles.h"

#	include "Interface/ResourceInterface.h"
#   include "Interface/StringizeInterface.h"

#   include "ResourceHIT.h"

#	include "Logger/Logger.h"

#	include "Core/String.h"
#	include "Consts.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	HotSpotBubbles::HotSpotBubbles()
		: m_enumerator(0)
		, m_invalidateBubbleWM(true)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	HotSpotBubbles::~HotSpotBubbles()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	uint32_t HotSpotBubbles::addBubble( const mt::vec2f & _pos, float _radius, bool _outward )
	{		
		Bubble b;

		uint32_t new_id = ++m_enumerator;
		b.id = new_id;
		b.pos = _pos;
		b.pos_wm = _pos;
		b.radius = _radius;
		b.outward = _outward;

		m_bubbles.push_back( b );

		return new_id;
	}
	//////////////////////////////////////////////////////////////////////////
	bool HotSpotBubbles::testPoint( const mt::vec2f & _point ) const
	{
		this->updateBubbleWM_();

		for( TVectorBubbles::const_iterator
			it = m_bubbles.begin(),
			it_end = m_bubbles.end();
		it != it_end;
		++it )
		{
			const Bubble & b = *it;

			if( mt::sqrlength_v2_v2( b.pos_wm, _point) < b.radius * b.radius )
			{
				return !m_outward;
			}
		}

		return m_outward;
	}
	//////////////////////////////////////////////////////////////////////////
	bool HotSpotBubbles::testRadius( const mt::vec2f & _point, float _radius ) const
	{
		this->updateBubbleWM_();

		for( TVectorBubbles::const_iterator
			it = m_bubbles.begin(),
			it_end = m_bubbles.end();
		it != it_end;
		++it )
		{
			const Bubble & b = *it;

			if( mt::sqrlength_v2_v2( b.pos_wm, _point) < (b.radius + _radius) * (b.radius + _radius) )
			{
				return !m_outward;
			}
		}

		return m_outward;
	}
	//////////////////////////////////////////////////////////////////////////
	bool HotSpotBubbles::testPolygon( const mt::vec2f & _point, const Polygon & _polygon ) const
	{
		//TODO:
		(void)_polygon;

		bool test = this->testPoint( _point );

		return test;
	}
	//////////////////////////////////////////////////////////////////////////
	void HotSpotBubbles::_invalidateWorldMatrix()
	{
		HotSpot::_invalidateWorldMatrix();

		this->invalidatemBubbleWM_();
	}
	//////////////////////////////////////////////////////////////////////////
	void HotSpotBubbles::invalidatemBubbleWM_()
	{
		m_invalidateBubbleWM = true;
	}
	//////////////////////////////////////////////////////////////////////////
	void HotSpotBubbles::updateBubbleWM_() const
	{
		if( m_invalidateBubbleWM == false )
		{
			return;
		}

		m_invalidateBubbleWM = false;

		const mt::mat4f & wm = this->getWorldMatrix();

		for( TVectorBubbles::const_iterator
			it = m_bubbles.begin(),
			it_end = m_bubbles.end();
		it != it_end;
		++it )
		{
			const Bubble & b = *it;

			mt::mul_v2_m4( b.pos_wm, b.pos, wm );
		}
	}

}	// namespace Menge