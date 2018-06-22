#pragma once

#include "Interface/GlobalHandleSystemInterface.h"

#include "Core/ServiceBase.h"

#include "Config/Vector.h"

namespace Mengine
{
	//////////////////////////////////////////////////////////////////////////
	struct GlobalHandlerDesc
	{
		uint32_t id;

		InputHandlerInterfacePtr handler;
        String doc;
		bool enable;
		bool dead;
	};
	//////////////////////////////////////////////////////////////////////////
	class GlobalHandleSystem
		: public ServiceBase<GlobalHandleSystemInterface>
	{
    public:
        GlobalHandleSystem();
        ~GlobalHandleSystem() override;

    public:
        bool _initializeService() override;
        void _finalizeService() override;

	public:
        uint32_t addGlobalHandler( const InputHandlerInterfacePtr & _handler, const String & _doc ) override;
		InputHandlerInterfacePtr removeGlobalHandler( uint32_t _id ) override;

	public:
		bool enableGlobalHandler( uint32_t _id, bool _value ) override;

	public:
		void update() override;
        void clear() override;

	public:
		bool handleKeyEvent( const InputKeyEvent & _event ) override;
		bool handleTextEvent( const InputTextEvent & _event ) override;

	public:
		bool handleMouseButtonEvent( const InputMouseButtonEvent & _event ) override;
		bool handleMouseButtonEventBegin( const InputMouseButtonEvent & _event ) override;
		bool handleMouseButtonEventEnd( const InputMouseButtonEvent & _event ) override;
		bool handleMouseMove( const InputMouseMoveEvent & _event ) override;
		bool handleMouseWheel( const InputMouseWheelEvent & _event ) override;

	protected:      		
		typedef Vector<GlobalHandlerDesc> TVectorGlobalHandler;
		TVectorGlobalHandler m_handlers;
		TVectorGlobalHandler m_handlersAdd;

        uint32_t m_handlersEnumerator;
	};
}