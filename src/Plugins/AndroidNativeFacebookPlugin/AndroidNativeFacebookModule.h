#pragma once

#include "Kernel/PluginBase.h"

#include "FacebookEventHandler.h"

#include "Environment/Android/AndroidEventation.h"

#include "Kernel/ModuleBase.h"

#include "Config/String.h"
#include "Config/Lambda.h"

#include "Config/VectorString.h"

namespace Mengine
{
    class AndroidNativeFacebookModule
        : public ModuleBase
    {
    public:
        typedef AndroidEventation<FacebookEventHandler> FacebookEventation;
        typedef typename FacebookEventation::LambdaEventHandler LambdaFacebookEventHandler;

    public:
        AndroidNativeFacebookModule();
        ~AndroidNativeFacebookModule() override;
        
    protected:
        bool _initializeModule() override;
        void _finalizeModule() override;

    protected:
        void _update( bool _focus ) override;

    public:
        void addCommand( const LambdaFacebookEventHandler & _command );

    public:
        void setEventHandler( const FacebookEventHandlerPtr & _handler );

    public:
        bool initializeSDK();

    public:
        bool isLoggedIn() const;
        const Char * getAccessToken() const;
        bool performLogin( const VectorString & _permissions );
        bool getUser();
        bool shareLink( const String & _link );
        bool getProfilePictureLink( const String & _typeParameter );

    protected:
        FacebookEventation m_eventation;
    };
}
