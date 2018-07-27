#pragma once

#include "Interface/ServiceInterface.h"
#include "Interface/ScheduleManagerInterface.h"
#include "Interface/MousePickerSystemInterface.h"
#include "Interface/GlobalHandleSystemInterface.h"
#include "Interface/NodeInterface.h"

namespace Mengine
{
    typedef IntrusivePtr<class Node> NodePtr;
    typedef IntrusivePtr<class Scene> ScenePtr;
    typedef IntrusivePtr<class Arrow> ArrowPtr;
    typedef IntrusivePtr<class Affectorable> AffectorablePtr;
    typedef IntrusivePtr<class RenderViewportInterface> RenderViewportInterfacePtr;
    typedef IntrusivePtr<class RenderCameraInterface> RenderCameraInterfacePtr;
    typedef IntrusivePtr<class RenderScissorInterface> RenderScissorInterfacePtr;
    //////////////////////////////////////////////////////////////////////////
    class Affectorable;
	//////////////////////////////////////////////////////////////////////////
	class SceneChangeCallbackInterface
		: public Factorable
	{
	public:
		virtual void onSceneChange( const ScenePtr & _scene, bool _enable, bool _remove ) = 0;
	};
	//////////////////////////////////////////////////////////////////////////
	typedef IntrusivePtr<SceneChangeCallbackInterface> SceneChangeCallbackInterfacePtr;
	//////////////////////////////////////////////////////////////////////////
    class PlayerServiceInterface
        : public ServiceInterface
		, public InputHandlerInterface
    {
        SERVICE_DECLARE("PlayerService")

    public:
		virtual bool setCurrentScene( const ScenePtr & _scene, bool _destroyOld, const SceneChangeCallbackInterfacePtr & _cb ) = 0;
		virtual bool restartCurrentScene( const SceneChangeCallbackInterfacePtr & _cb ) = 0;
		virtual bool removeCurrentScene( const SceneChangeCallbackInterfacePtr & _cb ) = 0;
		virtual void destroyCurrentScene() = 0;
        
		virtual const ScenePtr & getCurrentScene() = 0;

    public:
		virtual bool createGlobalScene() = 0;
		virtual void removeGlobalScene() = 0;
		virtual const ScenePtr & getGlobalScene() = 0;

	public:
		virtual void onFocus( bool _focus ) = 0;

		virtual void onAppMouseLeave( const InputMousePositionEvent & _event ) = 0;
		virtual void onAppMouseEnter( const InputMousePositionEvent & _event ) = 0;
		virtual void onAppMousePosition( const InputMousePositionEvent & _event ) = 0;

		virtual void onFullscreen( const Resolution & _resolution, bool _fullscreen ) = 0;
		virtual void onFixedContentResolution( const Resolution & _resolution, bool _fixed ) = 0;
		virtual void onFixedDisplayResolution( const Resolution & _resolution, bool _fixed ) = 0;

	public:
		virtual void tick( float _current, float _time ) = 0;
		virtual bool update() = 0;
		virtual void render() = 0;

	public:
		virtual void initializeRenderResources() = 0;
		virtual void finalizeRenderResources() = 0;


    public:
        virtual void setArrow( const ArrowPtr & _arrow ) = 0;
        virtual const ArrowPtr & getArrow() const = 0;

	public:
		virtual void calcGlobalMouseWorldPosition( const mt::vec2f & _screenPoint, mt::vec2f & _worldPoint ) = 0;
		virtual void calcGlobalMouseWorldDelta( const mt::vec2f & _screenPoint, const mt::vec2f & _screenDeltha, mt::vec2f & _worldDeltha ) = 0;

	public:
		virtual ScheduleManagerInterfacePtr createSchedulerManager() = 0;
		virtual bool destroySchedulerManager( const ScheduleManagerInterfacePtr & _scheduler ) = 0;

    public:
        virtual void setRenderCamera( const RenderCameraInterfacePtr & _camera) = 0;
        virtual const RenderCameraInterfacePtr & getRenderCamera() const = 0;

	public:
		virtual void setRenderViewport( const RenderViewportInterfacePtr & _renderViewport ) = 0;
		virtual const RenderViewportInterfacePtr & getRenderViewport() const = 0;

	public:
        virtual void setRenderScissor( const RenderScissorInterfacePtr & _scissor ) = 0;
		virtual const RenderScissorInterfacePtr & getRenderScissor() const = 0;
        
    public:
        virtual const MousePickerSystemInterfacePtr & getMousePickerSystem() const = 0;
        virtual const GlobalHandleSystemInterfacePtr & getGlobalHandleSystem() const = 0;

    public:
        virtual const ScheduleManagerInterfacePtr & getScheduleManager() const = 0;
        virtual const ScheduleManagerInterfacePtr & getScheduleManagerGlobal() const = 0;

	public:
		virtual const AffectorablePtr & getAffectorable() const = 0;
		virtual const AffectorablePtr & getAffectorableGlobal() const = 0;
		
    public:
        virtual void toggleDebugText() = 0;
    };

#   define PLAYER_SERVICE()\
    ((Mengine::PlayerServiceInterface*)SERVICE_GET(Mengine::PlayerServiceInterface))
}