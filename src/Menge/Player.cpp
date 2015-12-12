#	include "Player.h"

#	include "Interface/RenderSystemInterface.h"
#   include "Interface/ScriptSystemInterface.h"
#   include "Interface/ParticleSystemInterface.h"
#   include "Interface/ResourceInterface.h"
#	include "Interface/GraveyardInterface.h"
#	include "Interface/TimelineInterface.h"

#	include "Interface/UnicodeInterface.h"

#	include "Interface/NotificationServiceInterface.h"
#	include "Interface/StringizeInterface.h"
#	include "Interface/PrefetcherInterface.h"

#   include "Config/Stringstream.h"

#	include "NodeManager.h"

#	include "Game.h"
#	include "Application.h"

#	include "Kernel/Scene.h"

#	include "Kernel/RenderViewport.h"
#	include "Kernel/RenderClipplane.h"

#	include "Kernel/Camera2D.h"
#	include "Consts.h"

#	include "Arrow.h"

#	include "Logger/Logger.h"

#	include "TextField.h"

#   include "Kernel/ResourceVisitor.h"
#   include "Kernel/ResourceReference.h"

#	include "Math/mat3.h"

#   include "pybind/pybind.hpp"

#   include <iomanip>

//////////////////////////////////////////////////////////////////////////
SERVICE_FACTORY( PlayerService, Menge::Player );
//////////////////////////////////////////////////////////////////////////
namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	Player::Player()
        : m_scene(nullptr)
		, m_arrow(nullptr)
		, m_scheduleManager(nullptr)
		, m_scheduleManagerGlobal(nullptr)
		, m_arrowCamera2D(nullptr)
		, m_renderCamera(nullptr)
		, m_renderViewport(nullptr)
		, m_renderClipplane(nullptr)
		, m_renderTarget(nullptr)
		, m_switchSceneTo(nullptr)
		, m_mousePickerSystem(nullptr)
		, m_switchScene(false)
		, m_removeScene(false)
		, m_destroyOldScene(false)
		, m_restartScene(false)
		, m_arrowHided(false)
		, m_fps(0)	
		, m_affectorable(nullptr)
		, m_affectorableGlobal(nullptr)
		, m_showDebugText(0)
		, m_debugText(nullptr)
		, m_camera2D(nullptr)
		, m_viewport2D(nullptr)
		, m_debugCamera2D(nullptr)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	Player::~Player()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::setCurrentScene( Scene * _scene, bool _destroyOld, const SceneChangeCallbackInterfacePtr & _cb )
	{
		if( _scene == nullptr )
		{
			return false;
		}

		if( this->isChangedScene() == true )
		{
			return false;
		}

		m_switchSceneTo = _scene;
		
		m_restartScene = false;
		m_switchScene = true;
        m_removeScene = false;

		m_destroyOldScene = _destroyOld;

		m_changeSceneCb = _cb;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::restartCurrentScene( const SceneChangeCallbackInterfacePtr & _cb )
	{
		if( this->isChangedScene() == true )
		{
			return false;
		}

		m_restartScene = true;
		m_switchScene = false;
		m_removeScene = true;

		m_destroyOldScene = false;

		m_changeSceneCb = _cb;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::removeCurrentScene( const SceneChangeCallbackInterfacePtr & _cb )
	{
		if( this->isChangedScene() == true )
		{
			return false;
		}

		m_restartScene = false;
        m_switchScene = false;
		m_removeScene = true;
		
		m_removeSceneCb = _cb;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	Scene * Player::getCurrentScene()
	{
		return m_scene;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::updateChangeScene()
	{
		if( m_switchScene == true )
		{
			this->updateSwitchScene_();
		}
		else if( m_removeScene == true )
		{
			this->updateRemoveScene_();
		}
		else if( m_restartScene == true )
		{
			this->updateRestartScene_();
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::updateRemoveScene_()
	{
		m_removeScene = false;

		if( m_scene != nullptr )
		{
			if( m_arrow != nullptr )
			{
				m_arrow->removeFromParent();
				m_arrow->disable();
			}

			m_scheduleManager->removeAll();
			m_timingManager->removeAll();
			m_affectorable->stopAllAffectors();

			m_globalHandleSystem->clear();

            m_scene->release();

			NODE_SERVICE(m_serviceProvider)
				->clearHomeless();

			GRAVEYARD_SERVICE(m_serviceProvider)
				->clearTextures();

			m_scene = nullptr;
		}

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->setScene( nullptr );
		}

		if( m_removeSceneCb != nullptr )
		{
			SceneChangeCallbackInterfacePtr cb = m_removeSceneCb;
			m_removeSceneCb = nullptr;

			cb->onSceneChange( nullptr, false, true );
		}
	}
	//////////////////////////////////////////////////////////////////////////
    namespace
    {
        class PlayerResourceUselessCompile
        {
        public:
            PlayerResourceUselessCompile( ServiceProviderInterface * _serviceProvider )
                : m_serviceProvider(_serviceProvider)
            {
            }

            ~PlayerResourceUselessCompile()
            {
            }

        public:
            void begin()
            {
                m_observerResourceCompile = NOTIFICATION_SERVICE(m_serviceProvider)
                    ->addObserverMethod( NOTIFICATOR_RESOURCE_COMPILE, this, &PlayerResourceUselessCompile::resourceCompile );

                m_observerResourceRelease = NOTIFICATION_SERVICE(m_serviceProvider)
                    ->addObserverMethod( NOTIFICATOR_RESOURCE_RELEASE, this, &PlayerResourceUselessCompile::resourceRelease );
            }

            void end()
            {
                NOTIFICATION_SERVICE(m_serviceProvider)
                    ->removeObserver( NOTIFICATOR_RESOURCE_COMPILE, m_observerResourceCompile );

                NOTIFICATION_SERVICE(m_serviceProvider)
                    ->removeObserver( NOTIFICATOR_RESOURCE_RELEASE, m_observerResourceRelease );
            }

        protected:
            void resourceCompile( ResourceReference * _resource )
            {
                m_resources.push_back( _resource );
            }

            void resourceRelease( ResourceReference * _resource )
            {
                TVectorResourceDesc::iterator it_remove =
                    std::find( m_resources.begin(), m_resources.end(), _resource );

                if( it_remove == m_resources.end() )
                {
                    return;
                }
                
                LOGGER_PERFORMANCE(m_serviceProvider)("Useless Compile %s %s:%s"
					, _resource->getType().c_str()
                    , _resource->getGroup().c_str()
                    , _resource->getName().c_str()
                    );

                m_resources.erase( it_remove );
            }

        protected:
            ServiceProviderInterface * m_serviceProvider;

            ObserverInterface * m_observerResourceCompile;
			ObserverInterface * m_observerResourceRelease;

            typedef stdex::vector<ResourceReference *> TVectorResourceDesc;
            TVectorResourceDesc m_resources;
        };
    }
	//////////////////////////////////////////////////////////////////////////
	void Player::updateSwitchScene_()
	{
		m_switchScene = false;

		Scene * oldScene = m_scene;
		m_scene = nullptr;

		bool destroyOldScene = m_destroyOldScene;
		m_destroyOldScene = false;

		SceneChangeCallbackInterfacePtr cb = m_changeSceneCb;
		m_changeSceneCb = nullptr;

		if( m_arrow != nullptr )
		{
			m_arrow->removeFromParent();
			m_arrow->disable();
		}

		m_scheduleManager->removeAll();
		m_timingManager->removeAll();
		m_affectorable->stopAllAffectors();


		if( oldScene != nullptr && destroyOldScene == true )
		{
			oldScene->destroy();

			NODE_SERVICE( m_serviceProvider )
				->clearHomeless();

			GRAVEYARD_SERVICE( m_serviceProvider )
				->clearTextures();
		}
		
		//m_globalHandleSystem->clear();
        
        if( cb != nullptr )
        {
			cb->onSceneChange( nullptr, false, false );
        }
        
		m_scene = m_switchSceneTo;

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->setScene( m_scene );
		}

		if( cb != nullptr )
		{			
			cb->onSceneChange( m_scene, false, false );
		}

		//Holder<ResourceManager>::get()->_dumpResources( "before compile next scene " + m_scene->getName() );

#   ifndef MENGINE_MASTER_RELEASE
        PlayerResourceUselessCompile unlessCompile(m_serviceProvider);
        unlessCompile.begin();
#   endif

		m_scene->enable();

#   ifndef MENGINE_MASTER_RELEASE
        unlessCompile.end();
#   endif

        if( m_arrow != nullptr )
        {
			m_arrow->enable();
        }

		if( cb != nullptr )
		{
			cb->onSceneChange( m_scene, true, false );
		}

		return;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::updateRestartScene_()
	{
		m_restartScene = false;

		if( m_arrow != nullptr )
		{
			m_arrow->removeFromParent();
			m_arrow->disable();
		}

		m_scheduleManager->removeAll();
		m_timingManager->removeAll();
		m_affectorable->stopAllAffectors();

		m_scene->disable();

		NODE_SERVICE( m_serviceProvider )
			->clearHomeless();

		GRAVEYARD_SERVICE( m_serviceProvider )
			->clearTextures();

		SceneChangeCallbackInterfacePtr cb = m_changeSceneCb;
		m_changeSceneCb = nullptr;

		if( cb != nullptr )
		{
			cb->onSceneChange( nullptr, false, false );
		}

		if( cb != nullptr )
		{
			cb->onSceneChange( m_scene, false, false );
		}

		//Holder<ResourceManager>::get()->_dumpResources( "before compile next scene " + m_scene->getName() );

#   ifndef MENGINE_MASTER_RELEASE
		PlayerResourceUselessCompile unlessCompile( m_serviceProvider );
		unlessCompile.begin();
#   endif

		m_scene->enable();

#   ifndef MENGINE_MASTER_RELEASE
		unlessCompile.end();
#   endif

		if( m_arrow != nullptr )
		{
			m_arrow->enable();
		}

		if( cb != nullptr )
		{
			cb->onSceneChange( m_scene, true, false );
		}

		return;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::isChangedScene() const
	{
		return m_switchScene == true || m_restartScene == true || m_removeScene == true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::setArrow(Arrow * _arrow)
	{
		if( m_arrow != nullptr )
		{
			m_arrow->disable();
		}

		m_arrow = _arrow;

		if( m_arrow != nullptr)
		{
			m_arrow->setRenderCamera( m_arrowCamera2D );
			m_arrow->setRenderViewport( m_renderViewport );
			m_arrow->setRenderClipplane( m_renderClipplane );

			if( m_scene != nullptr )
			{
				m_arrow->enable();
			}
		}

        if( m_mousePickerSystem != nullptr )
        {
		    m_mousePickerSystem->setArrow( m_arrow );
        }
	}
	//////////////////////////////////////////////////////////////////////////
	Arrow * Player::getArrow() const
	{
		return m_arrow;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::calcGlobalMouseWorldPosition( const mt::vec2f & _screenPoint, mt::vec2f & _worldPoint )
	{
		const mt::mat4f & pm = m_renderCamera->getCameraProjectionMatrix();

		mt::mat4f pm_inv;
		mt::inv_m4( pm_inv, pm );

		mt::vec2f p1 = _screenPoint * 2.f - mt::vec2f(1.f, 1.f);
		p1.y = -p1.y;

		mt::vec2f p_pm;
		mt::mul_v2_m4( p_pm, p1, pm_inv );

		const mt::mat4f & vm = m_renderCamera->getCameraViewMatrix();

		mt::mat4f vm_inv;
		mt::inv_m4( vm_inv, vm );

		mt::vec2f p = p_pm;

		mt::vec2f p_vm;
		mt::mul_v2_m4( p_vm, p, vm_inv );

		_worldPoint = p_vm;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::calcGlobalMouseWorldDelta( const mt::vec2f & _screenPoint, const mt::vec2f & _screenDeltha, mt::vec2f & _worldDeltha )
	{
		const mt::mat4f & pm = m_renderCamera->getCameraProjectionMatrix();

		mt::mat4f pm_inv;
		mt::inv_m4( pm_inv, pm );

		mt::vec2f p1 = (_screenPoint + _screenDeltha) * 2.f - mt::vec2f(1.f, 1.f);
		p1.y = -p1.y;

		mt::vec2f p_pm;
		mt::mul_v2_m4( p_pm, p1, pm_inv );

		mt::vec2f p2 = (_screenPoint) * 2.f - mt::vec2f(1.f, 1.f);
		p2.y = -p2.y;

		mt::vec2f p_pm_base;
		mt::mul_v2_m4( p_pm_base, p2, pm_inv );

		mt::vec2f deltha = p_pm - p_pm_base;

		_worldDeltha = deltha;
	}
	//////////////////////////////////////////////////////////////////////////
	ScheduleManagerInterface * Player::createSchedulerManager()
	{
		ScheduleManagerInterface * sm = m_factoryScheduleManager.createObject();

		sm->setServiceProvider( m_serviceProvider );

		m_schedulers.push_back( sm );

		return sm;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::destroySchedulerManager( ScheduleManagerInterface * _scheduler )
	{	
		TVectorUserScheduler::iterator it_found = std::find( m_schedulers.begin(), m_schedulers.end(), _scheduler );

		if( it_found == m_schedulers.end() )
		{
			LOGGER_ERROR(m_serviceProvider)("Player::destroySchedulerManager scheduler not found!"
				);

			return false;
		}
		
		m_schedulers.erase( it_found );

		_scheduler->destroy();

		return true;
	}
    //////////////////////////////////////////////////////////////////////////
    MousePickerSystemInterface * Player::getMousePickerSystem() const
    {
        return m_mousePickerSystem;
    }
    //////////////////////////////////////////////////////////////////////////
    GlobalHandleSystemInterface * Player::getGlobalHandleSystem() const
    {
        return m_globalHandleSystem;
    }
	//////////////////////////////////////////////////////////////////////////
	ScheduleManagerInterface * Player::getScheduleManager() const
	{
		return m_scheduleManager;
	}
	//////////////////////////////////////////////////////////////////////////
	ScheduleManagerInterface * Player::getScheduleManagerGlobal() const
	{
		return m_scheduleManagerGlobal;
	}	
	//////////////////////////////////////////////////////////////////////////
	ScheduleManagerInterface * Player::createTimingManager()
	{
		ScheduleManagerInterface * tm = m_factoryTimingManager.createObject();

		tm->setServiceProvider( m_serviceProvider );

		m_timingers.push_back( tm );

		return tm;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::destroyTimingManager( ScheduleManagerInterface * _timing )
	{	
		TVectorUserTiming::iterator it_found = std::find( m_timingers.begin(), m_timingers.end(), _timing );

		if( it_found == m_timingers.end() )
		{
			LOGGER_ERROR(m_serviceProvider)("Player::destroyTimingManager timing not found!"
				);

			return false;
		}

		m_timingers.erase( it_found );

		_timing->destroy();

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	ScheduleManagerInterface * Player::getTimingManager() const
	{
		return m_timingManager;
	}
    //////////////////////////////////////////////////////////////////////////
	ScheduleManagerInterface * Player::getTimingManagerGlobal() const
    {
        return m_timingManagerGlobal;
    }
	//////////////////////////////////////////////////////////////////////////
	Affectorable * Player::getAffectorable() const
	{
		return m_affectorable;
	}
	//////////////////////////////////////////////////////////////////////////
	Affectorable * Player::getAffectorableGlobal() const
	{
		return m_affectorableGlobal;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::_initialize()
	{
		m_mousePickerSystem = new MousePickerSystem(m_serviceProvider);
		m_globalHandleSystem = new GlobalHandleSystem(m_serviceProvider);
		
        m_scheduleManager = m_factoryScheduleManager.createObject();
        m_scheduleManager->setServiceProvider( m_serviceProvider );

		m_scheduleManagerGlobal = m_factoryScheduleManager.createObject();
        m_scheduleManagerGlobal->setServiceProvider( m_serviceProvider );

		m_timingManager = m_factoryTimingManager.createObject();
        m_timingManager->setServiceProvider( m_serviceProvider );

        m_timingManagerGlobal = m_factoryTimingManager.createObject();
        m_timingManagerGlobal->setServiceProvider( m_serviceProvider );

		m_affectorable = new Affectorable;
		m_affectorableGlobal = new Affectorable;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::_finalize()
	{
        if( m_scene != nullptr )
        {
            m_scene->destroy();
            m_scene = nullptr;
        }

		m_removeSceneCb = nullptr;
		m_changeSceneCb = nullptr;
        
        if( m_camera2D != nullptr )
        {
            m_camera2D->destroy();
			m_camera2D = nullptr;
		}

		if( m_debugCamera2D != nullptr )
		{
			m_debugCamera2D->destroy();
			m_debugCamera2D = nullptr;
		}

		if( m_arrowCamera2D != nullptr )
		{
			m_arrowCamera2D->destroy();
			m_arrowCamera2D = nullptr;
		}

		if( m_viewport2D != nullptr )
		{
			m_viewport2D->destroy();
			m_viewport2D = nullptr;
		}

		if( m_mousePickerSystem != nullptr )
		{
			delete m_mousePickerSystem;
			m_mousePickerSystem = nullptr;
		}

		if( m_globalHandleSystem != nullptr )
		{
			delete m_globalHandleSystem;
			m_globalHandleSystem = nullptr;
		}

		if( m_scheduleManager != nullptr )
		{
			m_scheduleManager->destroy();
			m_scheduleManager = nullptr;
		}
		
		if( m_scheduleManagerGlobal != nullptr )
		{
			m_scheduleManagerGlobal->destroy();
			m_scheduleManagerGlobal = nullptr;
		}

		for( TVectorUserScheduler::iterator
			it = m_schedulers.begin(),
			it_end = m_schedulers.end();
		it != it_end;
		++it )
		{
			ScheduleManagerInterface * sm = *it;

			sm->destroy();
		}

		m_schedulers.clear();

		for( TVectorUserTiming::iterator
			it = m_timingers.begin(),
			it_end = m_timingers.end();
		it != it_end;
		++it )
		{
			ScheduleManagerInterface * tm = *it;

			tm->destroy();
		}

		if( m_timingManager != nullptr )
		{
			m_timingManager->destroy();
			m_timingManager = nullptr;
		}

        if( m_timingManagerGlobal != nullptr )
        {
            m_timingManagerGlobal->destroy();
            m_timingManagerGlobal = nullptr;
        }

		if( m_affectorable != nullptr )
		{
			delete m_affectorable;
			m_affectorable = nullptr;
		}

		if( m_affectorableGlobal != nullptr )
		{
			delete m_affectorableGlobal;
			m_affectorableGlobal = nullptr;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::initializeRenderResources()
	{
		m_debugText = NODE_SERVICE(m_serviceProvider)->
			createNodeT<TextField>( CONST_STRING(m_serviceProvider, TextField) );

		m_debugText->setFontName( STRINGIZE_STRING_LOCAL(m_serviceProvider, "__CONSOLE_FONT__") );
		m_debugText->setTextID( STRINGIZE_STRING_LOCAL(m_serviceProvider, "__ID_TEXT_CONSOLE") );
		m_debugText->setLocalColor( ColourValue( 1.0, 0.0, 0.0, 1.0 ) );
		m_debugText->enable();

		const Resolution & contentResolution = APPLICATION_SERVICE( m_serviceProvider )
			->getContentResolution();

		mt::vec2f cr;
		contentResolution.calcSize( cr );
		Viewport vp( 0.f, 0.f, cr.x, cr.y );

		m_camera2D = NODE_SERVICE( m_serviceProvider )
			->createNodeT<Camera2D>( CONST_STRING( m_serviceProvider, Camera2D ) );

		m_camera2D->setRenderport( vp );

		//mt::vec2f vp_pos(crx * 0.5f, cry * 0.5f);
		//camera->setLocalPosition(vp_pos);

		m_camera2D->enable();
		this->setRenderCamera( m_camera2D );


		m_viewport2D = NODE_SERVICE( m_serviceProvider )
			->createNodeT<RenderViewport>( CONST_STRING( m_serviceProvider, RenderViewport ) );

		m_viewport2D->setViewport( vp );
		m_viewport2D->enable();

		this->setRenderViewport( m_viewport2D );

		m_arrowCamera2D = NODE_SERVICE( m_serviceProvider )
			->createNodeT<Camera2D>( CONST_STRING( m_serviceProvider, Camera2D ) );

		m_arrowCamera2D->setRenderport( vp );
		m_arrowCamera2D->enable();

		if( m_arrow != nullptr )
		{
			m_arrow->setRenderCamera( m_arrowCamera2D );
			m_arrow->setRenderViewport( m_renderViewport );
			m_arrow->setRenderClipplane( m_renderClipplane );
		}


		m_debugCamera2D = NODE_SERVICE( m_serviceProvider )
			->createNodeT<Camera2D>( CONST_STRING( m_serviceProvider, Camera2D ) );

		m_debugCamera2D->setRenderport( vp );

		m_debugCamera2D->enable();
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::finalizeRenderResources()
	{
		if( m_debugText != nullptr	 )
		{
			m_debugText->destroy();
			m_debugText = nullptr;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::handleKeyEvent( const InputKeyEvent & _event )
	{
		bool handler = false;

		if( m_globalHandleSystem != nullptr )
		{
			m_globalHandleSystem->handleKeyEvent( _event );
		}

		if( m_mousePickerSystem != nullptr )
		{
			if( handler == false )
			{
				handler = m_mousePickerSystem->handleKeyEvent( _event );
			}
		}

		return handler;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::handleMouseButtonEvent( const InputMouseButtonEvent & _event )
	{
		if( m_globalHandleSystem != nullptr )
		{
			m_globalHandleSystem->handleMouseButtonEvent( _event );
		}

        bool handler = false;

		if( m_mousePickerSystem != nullptr )
		{
			if( handler == false )
			{
				handler = m_mousePickerSystem->handleMouseButtonEvent( _event );
			}
		}

		return handler;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::handleMouseButtonEventBegin( const InputMouseButtonEvent & _event )
	{
		if( m_globalHandleSystem != nullptr )
		{
			m_globalHandleSystem->handleMouseButtonEventBegin( _event );
		}

        bool handler = false;

        if( m_mousePickerSystem != nullptr )
        {
            if( handler == false )
            {
				handler = m_mousePickerSystem->handleMouseButtonEventBegin( _event );
            }
		}

        return handler;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::handleMouseButtonEventEnd( const InputMouseButtonEvent & _event )
	{
		if( m_globalHandleSystem != nullptr )
		{
			m_globalHandleSystem->handleMouseButtonEventEnd( _event );
		}

        bool handler = false;

        if( m_mousePickerSystem != nullptr )
        {
            if( handler == false )
            {
				handler = m_mousePickerSystem->handleMouseButtonEventEnd( _event );
            }
		}

        return handler;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::handleMouseMove( const InputMouseMoveEvent & _event )
	{
		if( m_globalHandleSystem != nullptr )
		{
			mt::vec2f point( _event.x, _event.y );
			mt::vec2f delta( _event.dx, _event.dy );

			mt::vec2f wp;
			this->calcGlobalMouseWorldPosition( point, wp );

			mt::vec2f wd;
			this->calcGlobalMouseWorldDelta( point, delta, wd );

			InputMouseMoveEvent ne;
			ne.type = _event.type;
			ne.touchId = _event.touchId;
			ne.x = wp.x;
			ne.y = wp.y;
			ne.dx = wd.x;
			ne.dy = wd.y;
			ne.pressure = _event.pressure;

			m_globalHandleSystem->handleMouseMove( ne );
		}

        bool handler = false;

		if( m_mousePickerSystem != nullptr )
		{
			if( handler == false )
			{
				handler = m_mousePickerSystem->handleMouseMove( _event );
			}
		}

		return handler;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::handleMouseWheel( const InputMouseWheelEvent & _event )
	{
		if( m_globalHandleSystem != nullptr )
		{
			m_globalHandleSystem->handleMouseWheel( _event );
		}

		bool handler = false;

		if( m_mousePickerSystem != nullptr )
		{
			if( handler == false )
			{
				handler = m_mousePickerSystem->handleMouseWheel( _event );
			}
		}

		return handler;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::tick( float _timing )
	{
		static float fpsTiming = 0.0f;
		fpsTiming += _timing;
		if( fpsTiming >= 1000.0f )
		{
			m_fps = RENDER_SERVICE(m_serviceProvider)->getDebugInfo().frameCount;
			RENDER_SERVICE(m_serviceProvider)->resetFrameCount();
			while( fpsTiming >= 1000.0f ) 
			{
				fpsTiming -= 1000.0f;
			}
		}

		float time = TIMELINE_SERVICE( m_serviceProvider )
			->getTime();

		//if( PhysicEngine2D::get()->isWorldCreate() )
		//{
		//	const mt::vec2f & arrowPos = 
		//		m_arrow->getLocalPosition() + m_renderCamera2D->getViewport().begin;

		//	PhysicEngine2D::get()
		//		->onMouseMove( arrowPos );
		//}

		if( m_camera2D != nullptr )
		{
			m_camera2D->update( time, _timing );
		}

		if( m_arrowCamera2D != nullptr )
		{
			m_arrowCamera2D->update( time, _timing );
		}

		if( m_arrow != nullptr )
		{
			m_arrow->update( time, _timing );
		}

		if( m_scene != nullptr )
		{
			m_scene->update( time, _timing );
		}

		if( m_scheduleManager != nullptr )
		{
			m_scheduleManager->update( time, _timing );
		}

		if( m_scheduleManagerGlobal != nullptr )
		{
			m_scheduleManagerGlobal->update( time, _timing );
		}

		for( TVectorUserScheduler::iterator
			it = m_schedulers.begin(),
			it_end = m_schedulers.end();
		it != it_end;
		++it )
		{
			ScheduleManagerInterface * sm = *it;

			sm->update( time, _timing );
		}

		if( m_affectorable != nullptr )
		{
			m_affectorable->updateAffectors( time, _timing );
		}

		if( m_affectorableGlobal != nullptr )
		{
			m_affectorableGlobal->updateAffectors( time, _timing );
		}

		m_timingManager->update( time, _timing );
		m_timingManagerGlobal->update( time, _timing );

		for( TVectorUserTiming::iterator
			it = m_timingers.begin(),
			it_end = m_timingers.end();
		it != it_end;
		++it )
		{
			ScheduleManagerInterface * tm = *it;

			tm->update( time, _timing );
		}

		MODULE_SERVICE(m_serviceProvider)
			->update( time, _timing );

		TIMELINE_SERVICE( m_serviceProvider )
			->update( _timing );
	}
	//////////////////////////////////////////////////////////////////////////
	bool Player::update()
	{
		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->update();
		}

		if( m_globalHandleSystem != nullptr )
		{
			m_globalHandleSystem->update();
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::setRenderCamera( RenderCameraInterface * _camera)
	{
		m_renderCamera = _camera;

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->setRenderCamera( m_renderCamera );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	const RenderCameraInterface * Player::getRenderCamera() const
	{
		return m_renderCamera;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::setRenderViewport( RenderViewportInterface * _viewport )
	{
		m_renderViewport = _viewport;

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->setRenderViewport( m_renderViewport );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	const RenderViewportInterface * Player::getRenderViewport() const 
	{
		return m_renderViewport;
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::setRenderClipplane( RenderClipplaneInterface * _clipplane )
	{
		m_renderClipplane = _clipplane;

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->setRenderClipplane( _clipplane );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	const RenderClipplaneInterface * Player::getRenderClipplane() const
	{
		return m_renderClipplane;
	}
	//////////////////////////////////////////////////////////////////////////
	class VisitorPlayerFactoryManager
		: public VisitorPrototypeGenerator
	{
	public:
		VisitorPlayerFactoryManager( ServiceProviderInterface * _serviceProvider, const ConstString & _category, Stringstream & _ss )
			: m_serviceProvider(_serviceProvider)
            , m_category(_category)
            , m_ss(_ss)
		{
		}

    private:
        VisitorPlayerFactoryManager & operator = ( const VisitorPlayerFactoryManager & _vpfm )
        {
            (void)_vpfm;

            return *this;
        }

	protected:
		void visit( const ConstString & _category, const ConstString & _type, const PrototypeGeneratorInterfacePtr & _generator ) override
		{
            if( m_category != _category )
            {
                return;
            }

			uint32_t count = _generator->count();

			if( count == 0 )
			{
				return;
			}

			m_ss << "Factory Object " << _type.c_str() << ": " << count << "\n";
		}

	protected:
        ServiceProviderInterface * m_serviceProvider;
        ConstString m_category;
		Stringstream & m_ss;
	};
	//////////////////////////////////////////////////////////////////////////
	void Player::render()
	{
		if( this->isChangedScene() == true )
		{
			return;
		}

		//if( m_scene == 0 )
		//{
		//	return;
		//}

		//const Viewport & vp = m_renderCamera2D->getViewport();
		//const mt::mat4f & wm = m_renderCamera2D->getWorldMatrix();

		//mt::mat4f inv_wm;

		//mt::inv_m4(inv_wm, wm);

		//const mt::mat4f & camera_vm = m_renderCamera2D->getViewMatrix();
		//const mt::mat4f & camera_pm = m_renderCamera2D->getProjectionMatrix();


		//renderEngine->newRenderPass( m_renderCamera2D );

        unsigned int debugMask = APPLICATION_SERVICE(m_serviceProvider)
            ->getDebugMask();

		RenderObjectState state;
		state.viewport = m_renderViewport;
		state.camera = m_renderCamera;
		state.clipplane = m_renderClipplane;
		state.target = m_renderTarget;

		if( m_scene != nullptr )
		{
			m_scene->render( &state, debugMask );
		}

		//renderEngine->setRenderArea( mt::vec4f( 0.0f, 0.0f, 0.0f, 0.0f ) );

		//mt::mat4f arrow_wm;
		//mt::ident_m4(arrow_wm);

		//const mt::mat4f & arrow_vm = m_renderCamera2D->getViewMatrix();
		//const mt::mat4f & arrow_pm = m_renderCamera2D->getProjectionMatrix();

		//renderEngine->newRenderPass( m_renderCamera2D );

		MODULE_SERVICE(m_serviceProvider)
			->render( &state, debugMask );

		RENDER_SERVICE(m_serviceProvider)
			->endLimitRenderObjects();

		if( m_arrow != nullptr )
		{
			m_arrow->render( &state, debugMask );
		}

//#	ifndef MENGINE_MASTER_RELEASE

		if( m_showDebugText != 0 )
		{			
			const RenderDebugInfo & rdi = 
				RENDER_SERVICE(m_serviceProvider)->getDebugInfo();

			const RenderTextureDebugInfo & rtdi =
				RENDERTEXTURE_SERVICE(m_serviceProvider)->getDebugInfo();

			//size_t particlesCount = 
			//	Holder<ParticleEngine>::get()->getFrameParticlesCount();

			uint32_t particlesCount = 0;

			Stringstream ss;

			ss << "FPS: " << m_fps << std::endl;

			if( m_showDebugText > 1 )
			{
				const Resolution & contentResolution = APPLICATION_SERVICE( m_serviceProvider )
					->getContentResolution();

				double sreenfillrate = rdi.fillrate / double( contentResolution.getWidth() * contentResolution.getHeight() );

				ss << "Fillrate " << std::setiosflags(std::ios::fixed) << std::setprecision(2) << sreenfillrate << " (Object " << rdi.object << " Triangle " << rdi.triangle << ")" << std::endl;
				ss << "DIP: " << rdi.dips << std::endl;

				ERenderBatchMode mode = RENDER_SERVICE(m_serviceProvider)
					->getBatchMode();

				ss << "Smart Batch: " << mode << " " << rdi.batch <<  std::endl;
				ss << "Texture Memory Usage: " << (float)rtdi.textureMemory / (1024.f*1024.f) << std::endl;
				ss << "Texture Count: " << rtdi.textureCount << std::endl;

				ss << "Particles: " << particlesCount << std::endl;
			}

			if( m_showDebugText == 2 )
			{
				class CompileResourceVisitor
					: public ResourceVisitor
				{
				public:
					CompileResourceVisitor()
						: m_count(0)
					{
					}

				public:
					uint32_t getCount() const
					{
						return m_count;
					}

				protected:
					void visit( ResourceReference * _resource )
					{
						if( _resource->isCompile() == false )
						{
							return;
						}

						++m_count;
					}

				protected:
					uint32_t m_count;
				};

				CompileResourceVisitor crv;

				RESOURCE_SERVICE(m_serviceProvider)
					->visitResources( &crv );

				ss << "Resources: " << crv.getCount() << std::endl;

				PrefetcherDebugInfo pdInfo = PREFETCHER_SERVICE(m_serviceProvider)
					->getDebugInfo();

				ss << "Prefetcher " << pdInfo.receiverCount << std::endl;

				MousePickerSystemInterface * mousePickerSystem = 
					PLAYER_SERVICE(m_serviceProvider)->getMousePickerSystem();

				ss << "PickerTrapCount:" << mousePickerSystem->getPickerTrapCount() << std::endl;
			}
			else if( m_showDebugText == 3 )
            {
			    VisitorPlayerFactoryManager pfmv(m_serviceProvider, CONST_STRING(m_serviceProvider, Node), ss);

			    PROTOTYPE_SERVICE(m_serviceProvider)
				    ->visitGenerators( &pfmv );
            }
			else if( m_showDebugText == 4 )
            {
                class MyVisitorClassTypeScope
                    : public pybind::visitor_class_type_scope
                {
                public:
                    MyVisitorClassTypeScope( Stringstream & _ss )
                        : m_ss(_ss)
                        , m_count(0)
                    {
                    }

                protected:
                    void operator = ( const MyVisitorClassTypeScope & )
                    {
                    }

                protected:
                    void visit_scope( pybind::class_type_scope * _scope ) override
                    {   
                        uint32_t count = _scope->getObjectCount();

						if( count == 0 )
						{
							return;
						}

						if( m_count % 3 == 0 )
						{
							m_ss << "Py: ";
						}
                        
						m_ss << _scope->get_name() << " " << count;
                        
						if( m_count % 3 != 2 )
						{
							m_ss << "        ";
						}
						else
						{
							m_ss << std::endl;
						}

                        ++m_count;
                    }

                protected:                    
                    Stringstream & m_ss;
                    uint32_t m_count;
                };

                MyVisitorClassTypeScope mvcts(ss);

				pybind::detail::visit_types_scope( &mvcts );
            }
            
			String text = ss.str();

			TVectorString args;
			args.push_back( text );
			m_debugText->setTextFormatArgs( args );

            float gameViewportAspect;
            Viewport gameViewport;

            APPLICATION_SERVICE(m_serviceProvider)
                ->getGameViewport( gameViewportAspect, gameViewport );

            m_debugText->setLocalPosition( mt::vec3f(gameViewport.begin, 0.f) );

			m_debugText->render( &state, debugMask );
		}
//#	endif
		//m_renderCamera2D->setLocalPosition( pos );
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::onAppMouseLeave( const InputMousePositionEvent & _event )
	{
		if( m_arrow != nullptr )
		{
			m_arrow->onAppMouseLeave();
		}

		if( m_scene != nullptr && m_scene->isActivate() == true )
		{
			m_scene->onAppMouseLeave();
		}

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->handleMouseLeave( _event );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::onAppMouseEnter( const InputMousePositionEvent & _event )
	{
		if( m_arrow != nullptr )
		{
			m_arrow->onAppMouseEnter();
		}

		if( m_scene != nullptr && m_scene->isActivate() == true )
		{
			m_scene->onAppMouseEnter();
		}

		if( m_mousePickerSystem != nullptr )
		{
			m_mousePickerSystem->handleMouseEnter( _event );
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::onAppMousePosition( const InputMousePositionEvent & _event )
	{
		(void)_event;
		//if( m_mousePickerSystem != nullptr )
		//{
		//	m_mousePickerSystem->handleMousePosition( _event );
		//}
	}
	//////////////////////////////////////////////////////////////////////////
	void Player::onFullscreen( const Resolution & _resolution, bool _fullscreen )
	{
		(void)_resolution;
        (void)_fullscreen;
	}
    //////////////////////////////////////////////////////////////////////////
    void Player::onFixedContentResolution( const Resolution & _resolution, bool _fixed )
    {
		(void)_resolution;
        (void)_fixed;
    }
	//////////////////////////////////////////////////////////////////////////
	void Player::onFocus( bool _focus )
	{
		if( m_scene != nullptr && m_scene->isActivate() == true )
		{
			m_scene->onFocus( _focus );
		}
	}
	//////////////////////////////////////////////////////////////////////////
//#	ifndef MENGINE_MASTER_RELEASE
	//////////////////////////////////////////////////////////////////////////
	void Player::toggleDebugText()
	{
		++m_showDebugText;
        m_showDebugText %= 5;

		if( m_showDebugText == 2 )
		{
			RENDER_SERVICE(m_serviceProvider)
				->enableDebugMode( true );
		}
		else if( m_showDebugText == 0 )
		{
			RENDER_SERVICE(m_serviceProvider)
				->enableDebugMode( false );
		}
	}
	//////////////////////////////////////////////////////////////////////////
//#	endif
}
