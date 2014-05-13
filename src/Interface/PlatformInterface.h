#   pragma once

#	include "Config/Typedef.h"
#   include "Config/String.h"

#	include "Core/ConstString.h"
#   include "Core/FilePath.h"
#	include "Core/Viewport.h"
#	include "Core/Resolution.h"

#	include "Interface/ServiceInterface.h"

#   include <map>

namespace Menge
{
    class TimerInterface
    {
    public:
        virtual void initialize() = 0;
        virtual void reset() = 0;
        virtual float getDeltaTime() const = 0;

    public:
        virtual unsigned long getMilliseconds() = 0;
        virtual unsigned long getMicroseconds() = 0;
        virtual unsigned long getMillisecondsCPU() = 0;
        virtual unsigned long getMicrosecondsCPU() = 0;
    };

    class ConsoleInterface 
    {
    public:
        virtual void render() = 0;
        virtual void proccessInput( unsigned int _key, unsigned int _char, bool _isDown ) = 0;
    };

    class DynamicLibraryInterface;

    typedef std::map<ConstString, WString> TMapParams;

    class PlatformInterface
        : public ServiceInterface
    {
        SERVICE_DECLARE("PlatformService");

    public:
        virtual void stop() = 0;

        virtual void getDesktopResolution( Resolution & _resolution ) const = 0;

        virtual const String & getCurrentPath() const = 0;

        virtual void minimizeWindow() = 0;

        virtual void setHandleMouse( bool _handle ) = 0;
        virtual void setCursorPosition( const mt::vec2f & _pos ) = 0;

        virtual TimerInterface * getTimer() const = 0;

        virtual void showKeyboard() = 0;
        virtual void hideKeyboard() = 0;

        virtual void notifyWindowModeChanged( const Resolution & _resolution, bool _fullscreen ) = 0;
        virtual void notifyVsyncChanged( bool _vsync ) = 0;
        virtual void notifyCursorModeChanged( bool _mode ) = 0;
        virtual void notifyCursorIconSetup( const FilePath & _name, void * _buffer, size_t _size ) = 0;

        virtual void notifyCursorClipping( const Viewport & _viewport ) = 0;
        virtual void notifyCursorUnClipping() = 0;

    public:
        virtual void onEvent( const ConstString & _event, const TMapParams & _params ) = 0;

    public:
        virtual size_t getShortPathName( const String & _name, char * _shortpath, size_t _shortpathlen ) = 0;

    public:
        virtual void getMaxClientResolution( Resolution & _resolution ) const = 0;

    public:
        virtual bool isDevelopmentMode() const = 0;
        //virtual void notifySoundInitialize() = 0;

        //virtual void setAsScreensaver( bool _set ) = 0;

    public:
        virtual bool openUrlInDefaultBrowser( const WString & _url ) = 0;       

    public:
        virtual size_t getMemoryUsage() const = 0;
        virtual void * checkpointMemory() const = 0;
        virtual size_t diffMemory( void * _checkpoint ) const = 0;
    };

#   define PLATFORM_SERVICE( serviceProvider )\
    SERVICE_GET(serviceProvider, Menge::PlatformInterface)
}