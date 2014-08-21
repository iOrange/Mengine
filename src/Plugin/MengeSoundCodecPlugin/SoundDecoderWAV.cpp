#	include "SoundDecoderWAV.h"

#	include "Interface/FileSystemInterface.h"

#	include "Core/Magic.h"
#	include "Logger/Logger.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	struct WAVE_HEADER
	{
		char ChunkID[4];
		uint32_t ChunkSize;
		char Format[4];
		char Subchunk1ID[4];
		uint32_t Subchunk1Size;
		uint16_t AudioFormat;
		uint16_t NumChannels;
		uint32_t SampleRate;
		uint32_t ByteRate;
		uint16_t BlockAlign;
		uint16_t BitsPerSample;
	};
	//////////////////////////////////////////////////////////////////////////
	SoundDecoderWAV::SoundDecoderWAV()		
		: m_chunkDataPos(0)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	SoundDecoderWAV::~SoundDecoderWAV()
	{
	}
    //////////////////////////////////////////////////////////////////////////
    bool SoundDecoderWAV::_prepareData()
    {
		WAVE_HEADER header;
		
		m_stream->read( header.ChunkID, 4 );

		if( magicTest4( header.ChunkID, "RIFF" ) == false )
		{
			return false;
		}

		m_stream->read( &header.ChunkSize, sizeof(header.ChunkSize) );

		m_stream->read( &header.Format, 4 );

		if( magicTest4( header.Format, "WAVE" ) == false )
		{
			return false;
		}
		
		m_stream->read( &header.Subchunk1ID, 4 );

		if( magicTest4( header.Subchunk1ID, "fmt " ) == false )
		{
			return false;
		}

		m_stream->read( &header.Subchunk1Size, sizeof(header.Subchunk1Size) );

		m_stream->read( &header.AudioFormat, sizeof(header.AudioFormat) );
        
		if( header.AudioFormat != 1 )
		{
			return false;
		}

		m_stream->read( &header.NumChannels, sizeof(header.NumChannels) );
		m_stream->read( &header.SampleRate, sizeof(header.SampleRate) );
		m_stream->read( &header.ByteRate, sizeof(header.ByteRate) );
		m_stream->read( &header.BlockAlign, sizeof(header.BlockAlign) );
		m_stream->read( &header.BitsPerSample, sizeof(header.BitsPerSample) );

		if( ( header.BitsPerSample != 16 ) && ( header.BitsPerSample != 8 ) )
		{			
			return false;
		}

		m_stream->skip( header.Subchunk1Size - 16 );
				
		size_t chunkDataSize;
		if( this->findChunkData_( chunkDataSize ) == false )
		{
			return false;
		}

		m_chunkDataPos = m_stream->tell();
				
        m_dataInfo.size = chunkDataSize;
        m_dataInfo.channels = header.NumChannels;
        m_dataInfo.frequency = header.SampleRate;

		m_dataInfo.length = float(m_dataInfo.size) * 1000.f / float(m_dataInfo.frequency);

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	bool SoundDecoderWAV::findChunkData_( size_t & _size )
	{
		while( m_stream->eof() == false )
		{
			char Subchunk2ID[4];
			m_stream->read( &Subchunk2ID, 4 );

			uint32_t Subchunk2Size;
			m_stream->read( &Subchunk2Size, sizeof(Subchunk2Size) );

			if( magicTest4( Subchunk2ID, "data" ) == true )
			{
				_size = Subchunk2Size;

				return true;
			}

			m_stream->skip( Subchunk2Size );
		}

		return false;
	}
	//////////////////////////////////////////////////////////////////////////
	size_t SoundDecoderWAV::decode( void * _buffer, size_t _bufferSize )
	{
		size_t bytesDone = m_stream->read( _buffer, _bufferSize );
		
		return bytesDone;
	}
	//////////////////////////////////////////////////////////////////////////
	bool SoundDecoderWAV::seek( float _timing )
	{         
        if( _timing > m_dataInfo.length )
        {
            LOGGER_ERROR(m_serviceProvider)("SoundDecoderOGGVorbis::seek timing %f > total %f"
                , _timing
                , m_dataInfo.length
                );

			_timing = m_dataInfo.length;
        }
              
        size_t wav_pos = (size_t)((double)(_timing) * 0.001 * (double)(m_dataInfo.frequency));

		bool result = m_stream->seek( wav_pos + m_chunkDataPos );
        
		return result;
	}
	//////////////////////////////////////////////////////////////////////////
	float SoundDecoderWAV::timeTell()
	{
		size_t wav_pos = m_stream->tell();
		
		float length = float(wav_pos - m_chunkDataPos) * 1000.f / float(m_dataInfo.frequency);
		       
		return length;
	}
	//////////////////////////////////////////////////////////////////////////
}	// namespace Menge