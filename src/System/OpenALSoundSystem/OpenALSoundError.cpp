#include "OpenALSoundError.h"

#include "AL/al.h"

#include "Kernel/Logger.h"

namespace Mengine
{
    bool s_OpenALErrorCheck( const char * _file, int _line )
    {
        ALenum error = alGetError();

        if( error == AL_NO_ERROR )
        {
            return true;
        }

        const char * message = alGetString( error );

        LOGGER_ERROR( "OpenAL Error: (%s %d) %d:%s"
            , _file
            , _line
            , error
            , message
        );

        return false;
    }
}
