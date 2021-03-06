#include "AndroidNativeFacebookPlugin.h"

#include "AndroidNativeFacebookModule.h"

#include "Interface/StringizeServiceInterface.h"

#include "Kernel/ModuleFactory.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PLUGIN_FACTORY( AndroidNativeFacebook, Mengine::AndroidNativeFacebookPlugin )
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    AndroidNativeFacebookPlugin::AndroidNativeFacebookPlugin()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    AndroidNativeFacebookPlugin::~AndroidNativeFacebookPlugin()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool AndroidNativeFacebookPlugin::_initializePlugin()
    {
        this->addModuleFactory( STRINGIZE_STRING_LOCAL( "ModuleAndroidFacebook" )
            , new ModuleFactory<AndroidNativeFacebookModule>() );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void AndroidNativeFacebookPlugin::_finalizePlugin()
    {
    }
}  