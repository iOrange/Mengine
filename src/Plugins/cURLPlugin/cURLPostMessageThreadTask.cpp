#include "cURLPostMessageThreadTask.h"

#include "Interface/ConfigServiceInterface.h"

#include "Kernel/Logger.h"

#include "Config/Stringstream.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    cURLPostMessageThreadTask::cURLPostMessageThreadTask()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    cURLPostMessageThreadTask::~cURLPostMessageThreadTask()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool cURLPostMessageThreadTask::initialize( const String & _url, const MapParams & _params )
    {
        m_url = _url;
        m_params = _params;

        return false;
    }
    //////////////////////////////////////////////////////////////////////////
    void cURLPostMessageThreadTask::_onCURL( CURL * _curl )
    {
        /* specify URL to get */
        curl_easy_setopt( _curl, CURLOPT_URL, m_url.c_str() );

        curl_easy_setopt( _curl, CURLOPT_POST, 1 );

        struct curl_httppost * lastptr = nullptr;
        curl_httppost * formpost = nullptr;

        for( MapParams::const_iterator
            it = m_params.begin(),
            it_end = m_params.end();
            it != it_end;
            ++it )
        {
            const ConstString & key = it->first;
            const String & value = it->second;

            /* Fill in the  field */
            curl_formadd( &formpost,
                &lastptr,
                CURLFORM_COPYNAME, key.c_str(),
                CURLFORM_COPYCONTENTS, value.c_str(),
                CURLFORM_END );
        }

        curl_easy_setopt( _curl, CURLOPT_HTTPPOST, formpost );

        /* send all data to this function  */
        this->setupWriteResponse( _curl );

        if( CONFIG_VALUE( "HTTP", "Log", false ) == true )
        {
            Stringstream ss;

            for( MapParams::const_iterator
                it = m_params.begin(),
                it_end = m_params.end();
                it != it_end;
                ++it )
            {
                ss << it->first.c_str() << ": " << it->second;
                ss << std::endl;
            }

            String params_str = ss.str();

            LOGGER_STATISTIC( "HTTP: post message url '%s' params:\n %s"
                , m_url.c_str()
                , params_str.c_str()
            );
        }
    }
}
