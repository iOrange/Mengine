#include "OpenALSoundBufferStream.h"
#include "OpenALSoundSystem.h"
#include "OpenALSoundError.h"

#include "Interface/SoundCodecInterface.h"
#include "Interface/ThreadServiceInterface.h"

#include "Kernel/ThreadMutexScope.h"
#include "Kernel/Logger.h"
#include "Kernel/Document.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    OpenALSoundBufferStream::OpenALSoundBufferStream()
        : m_sourceId( 0 )
        , m_looped( false )
        , m_updating( false )
    {
        for( uint32_t i = 0; i != MENGINE_OPENAL_STREAM_BUFFER_COUNT; ++i )
        {
            m_alBuffersId[i] = 0;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    OpenALSoundBufferStream::~OpenALSoundBufferStream()
    {
        this->removeBuffers_();
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::_initialize()
    {
        ThreadMutexInterfacePtr mutexUpdating = THREAD_SERVICE()
            ->createMutex( MENGINE_DOCUMENT_FUNCTION );

        if( mutexUpdating == nullptr )
        {
            return false;
        }

        m_mutexUpdating = mutexUpdating;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void OpenALSoundBufferStream::removeBuffers_()
    {
        this->setUpdating_( false );

        for( uint32_t i = 0; i != MENGINE_OPENAL_STREAM_BUFFER_COUNT; ++i )
        {
            ALuint id = m_alBuffersId[i];

            if( id != 0 )
            {
                m_soundSystem->releaseBufferId( id );
            }

            m_alBuffersId[i] = 0;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::load( const SoundDecoderInterfacePtr & _soundDecoder )
    {
        for( uint32_t i = 0; i != MENGINE_OPENAL_STREAM_BUFFER_COUNT; ++i )
        {
            ALuint id = m_soundSystem->genBufferId();

            if( id == 0 )
            {
                // TODO: report in case of error
                LOGGER_ERROR( "invalid gen %d buffer ID"
                    , i
                );

                this->removeBuffers_();

                return false;
            }

            m_alBuffersId[i] = id;
        }

        m_soundDecoder = _soundDecoder;

        const SoundCodecDataInfo* dataInfo = m_soundDecoder->getCodecDataInfo();
        m_frequency = dataInfo->frequency;

        if( dataInfo->channels != 2 )
        {
            LOGGER_ERROR( "invalid channels %d must be %d"
                , dataInfo->channels
                , 2
            );

            return false;
        }

        m_channels = dataInfo->channels;
        m_length = dataInfo->length;

        if( m_channels == 1 )
        {
            m_format = AL_FORMAT_MONO16;
            // Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
            //m_bufferSize = m_frequency >> 1;
            // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
            //m_bufferSize -= (m_bufferSize % 2);
            m_isStereo = false;
        }
        else if( m_channels == 2 )
        {
            m_format = AL_FORMAT_STEREO16;
            // Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
            //m_bufferSize = m_frequency;
            // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
            //m_bufferSize -= (m_bufferSize % 4);
            m_isStereo = true;
        }
        else if( m_channels == 4 )
        {
            m_format = alGetEnumValue( "AL_FORMAT_QUAD16" );
            // Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
            //m_bufferSize = m_frequency * 2;
            // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
            //m_bufferSize -= (m_bufferSize % 8);
            m_isStereo = true;
        }
        else if( m_channels == 6 )
        {
            m_format = alGetEnumValue( "AL_FORMAT_51CHN16" );
            // Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
            //m_bufferSize = m_frequency * 3;
            // IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
            //m_bufferSize -= (m_bufferSize % 12);
            m_isStereo = true;
        }
        else
        {
            LOGGER_ERROR( "invalid channels %d"
                , m_channels
            );

            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::playSource( ALuint _source, bool _looped, float _pos )
    {
        m_sourceId = _source;
        m_looped = _looped;

        if( _pos > m_length )
        {
            LOGGER_ERROR( "pos %f > length %f"
                , _pos
                , m_length
            );

            return false;
        }

        ALint state = 0;
        OPENAL_CALL( alGetSourcei, (m_sourceId, AL_SOURCE_STATE, &state) );
        
        if( state != AL_STOPPED && state != AL_INITIAL )
        {
            this->stopSource( m_sourceId );
            //alSourceRewind( _source );
        }

        OPENAL_CALL( alSourcei, (m_sourceId, AL_BUFFER, 0) ); // clear source buffering
        OPENAL_CALL( alSourcei, (m_sourceId, AL_LOOPING, AL_FALSE) );

        if( m_soundDecoder->seek( _pos ) == false )
        {
            LOGGER_ERROR( "invalid seek '%f'"
                , _pos
            );

            return false;
        }

        for( uint32_t i = 0; i != MENGINE_OPENAL_STREAM_BUFFER_COUNT; ++i )
        {
            ALuint bufferId = m_alBuffersId[i];

            size_t bytesWritten;
            if( this->bufferData_( bufferId, bytesWritten ) == false )
            {
                return false;
            }

            if( bytesWritten == 0 )
            {
                break;
            }

            OPENAL_CALL( alSourceQueueBuffers, (m_sourceId, 1, &bufferId) );

            if( bytesWritten != MENGINE_OPENAL_STREAM_BUFFER_SIZE )
            {
                break;
            }
        }

        OPENAL_CALL( alSourcePlay, (m_sourceId) );

        this->setUpdating_( true );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::resumeSource( ALuint _source )
    {
        (void)_source;

        OPENAL_CALL( alSourcePlay, (m_sourceId) );

        this->setUpdating_( true );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void OpenALSoundBufferStream::pauseSource( ALuint _source )
    {
        this->setUpdating_( false );

        OPENAL_CALL( alSourcePause, (_source) );
        //m_soundSystem->clearSourceId( _source );
    }
    //////////////////////////////////////////////////////////////////////////
    void OpenALSoundBufferStream::stopSource( ALuint _source )
    {
        this->setUpdating_( false );

        ALint process_count = 0;
        // �������� ���������� ������������ �������
        OPENAL_CALL( alGetSourcei, (_source, AL_BUFFERS_PROCESSED, &process_count) );

        // ���� ������� ���������� ��
        while( process_count-- > 0 )
        {            
            // ��������� �� �� �������
            ALuint buffer = 0;

            OPENAL_CALL( alSourceUnqueueBuffers, (_source, 1, &buffer) );
        }

        OPENAL_CALL( alSourceStop, (_source) );

        ALint queued_count = 0;
        // unqueue remaining buffers
        OPENAL_CALL( alGetSourcei, (_source, AL_BUFFERS_QUEUED, &queued_count) );

        while( queued_count-- > 0 )
        {
            // ��������� �� �� �������
            ALuint buffer = 0;

            OPENAL_CALL( alSourceUnqueueBuffers, (_source, 1, &buffer) );
        }

        OPENAL_CALL( alSourcei, (m_sourceId, AL_BUFFER, 0) );

        OPENAL_CALL( alSourceRewind, (_source) );
    }
    //////////////////////////////////////////////////////////////////////////
    void OpenALSoundBufferStream::setUpdating_( bool _updating )
    {
        m_mutexUpdating->lock();
        m_updating = _updating;
        m_mutexUpdating->unlock();
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::getUpdating_() const
    {
        return m_updating;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::setTimePos( ALuint _source, float _pos ) const
    {
        (void)_source;

        bool result = m_soundDecoder->seek( _pos );

        return result;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::getTimePos( ALuint _source, float & _pos ) const
    {
        (void)_source;

        float timeTell = m_soundDecoder->tell();

        if( timeTell > m_length )
        {
            LOGGER_ERROR( "get tell %f > length %f"
                , timeTell
                , m_length
            );

            return false;
        }

        _pos = timeTell;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::update()
    {
        MENGINE_THREAD_MUTEX_SCOPE( m_mutexUpdating );

        if( m_updating == false )
        {
            return true;
        }

        ALint state;
        OPENAL_CALL( alGetSourcei, (m_sourceId, AL_SOURCE_STATE, &state) );

        ALint processed_count = 0;
        OPENAL_CALL( alGetSourcei, (m_sourceId, AL_BUFFERS_PROCESSED, &processed_count) );

        ALuint unqueueBuffersId[MENGINE_OPENAL_STREAM_BUFFER_COUNT];
        OPENAL_CALL( alSourceUnqueueBuffers, (m_sourceId, processed_count, unqueueBuffersId) );

        for( ALint curr_processed = 0; curr_processed != processed_count; ++curr_processed )
        {
            // ��������� �� �� �������
            ALuint bufferId = unqueueBuffersId[curr_processed];

            // ������ ��������� ������ ������
            size_t bytesWritten;
            this->bufferData_( bufferId, bytesWritten );

            if( bytesWritten == 0 )
            {
                continue;
            }

            OPENAL_CALL( alSourceQueueBuffers, (m_sourceId, 1, &bufferId) );
        }

        if( state != AL_PLAYING && state != AL_PAUSED )
        {
            ALint queued_count = 0;
            OPENAL_CALL( alGetSourcei, (m_sourceId, AL_BUFFERS_QUEUED, &queued_count) );

            if( queued_count == 0 )
            {
                return false;
            }

            OPENAL_CALL( alSourcePlay, (m_sourceId) );
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool OpenALSoundBufferStream::bufferData_( ALuint _alBufferId, size_t & _bytes )
    {
        char dataBuffer[MENGINE_OPENAL_STREAM_BUFFER_SIZE];
        size_t bytesWritten = m_soundDecoder->decode( dataBuffer, MENGINE_OPENAL_STREAM_BUFFER_SIZE );

        if( bytesWritten == 0 )
        {
            _bytes = 0;

            return true;
        }

        if( bytesWritten != MENGINE_OPENAL_STREAM_BUFFER_SIZE && m_looped == true )
        {
            m_soundDecoder->rewind();

            size_t bytesWritten2 = m_soundDecoder->decode( dataBuffer + bytesWritten, MENGINE_OPENAL_STREAM_BUFFER_SIZE - bytesWritten );

            bytesWritten += bytesWritten2;
        }

        ALsizei al_bytesWritten = (ALsizei)bytesWritten;
        IF_OPENAL_CALL( alBufferData, (_alBufferId, m_format, dataBuffer, al_bytesWritten, m_frequency) )
        {
            LOGGER_ERROR( "buffer=%d id=%d format=%d bytes=%d frequency=%d"
                , _alBufferId
                , m_format
                , bytesWritten
                , m_frequency
            );

            return false;
        }

        _bytes = bytesWritten;

        return true;
    }
}
