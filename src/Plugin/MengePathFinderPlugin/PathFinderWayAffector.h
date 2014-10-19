#	pragma once

#	include "Kernel/Affector.h"
#	include "Kernel/Node.h"

#	include "Math/vec2.h"

#	include "fastpathfinder/pathfinder.h"
#	include "fastpathfinder/map.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	class PathFinderWayAffector
		: public Affector
	{
	public:
		PathFinderWayAffector();
		~PathFinderWayAffector();
		
	public:
		void setServiceProvider( ServiceProviderInterface * _serviceProvider );

	public:
		bool initialize( Node * _node, PyObject * _way, float _speed, PyObject * _cb );
		
	public:
		bool prepare() override;
		bool affect( float _timing ) override;
		void complete() override;
		void stop() override;

	protected:
		bool invalidateTarget_();

	protected:
		void destroy() override;

	protected:
		ServiceProviderInterface * m_serviceProvider;

		Node * m_node;
		PyObject * m_way;

		float m_speed;
		PyObject * m_cb;

		uint32_t m_iterator;
		uint32_t m_wayCount;	
	};
}
