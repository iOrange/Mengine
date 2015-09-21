#	include "VideoDecoderTheora.h"

#	include "Logger/Logger.h"

#	define OGG_BUFFER_SIZE 4096

//Defines
#define MAX( a, b ) ((a > b) ? a : b)
#define MIN( a, b ) ((a < b) ? a : b)

#	ifdef MENGE_RENDER_TEXTURE_RGBA
#		define COLOR_R 0
#		define COLOR_G 1
#		define COLOR_B 2
#	else
#		define COLOR_R 2
#		define COLOR_G 1
#		define COLOR_B 0
#	endif

#define COLOR_A 3

#define CLIP_RGB_COLOR( rgb_color_test, rgb_char_buffer ) \
	rgb_char_buffer = (unsigned char)(MAX( MIN(rgb_color_test, 255), 0 ))

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	static int s_YTable[256];
	static int s_BUTable[256];
	static int s_GUTable[256];
	static int s_GVTable[256];
	static int s_RVTable[256];
	//////////////////////////////////////////////////////////////////////////
	static void s_createCoefTables()
	{
		//used to bring the table into the high side (scale up) so we
		//can maintain high precision and not use floats (FIXED POINT)
		int scale = 1L << 13;

		for( int i = 0; i < 256; i++ )
		{
			int temp = i - 128;

			s_YTable[i] = (int)((1.164 * scale + 0.5) * (i - 16));	//Calc Y component

			s_RVTable[i] = (int)((1.596 * scale + 0.5) * temp);		//Calc R component

			s_GUTable[i] = (int)((0.391 * scale + 0.5) * temp);		//Calc G u & v components
			s_GVTable[i] = (int)((0.813 * scale + 0.5) * temp);

			s_BUTable[i] = (int)((2.018 * scale + 0.5) * temp);		//Calc B component
		}
	}
	//////////////////////////////////////////////////////////////////////////
	VideoDecoderTheora::VideoDecoderTheora()
		: VideoDecoder()
		, m_eof( true )
		, m_currentFrame( 0 )
		, m_lastReadBytes( 0 )
		, m_pitch( 0 )
	{
	}
	//////////////////////////////////////////////////////////////////////////
	VideoDecoderTheora::~VideoDecoderTheora()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	bool VideoDecoderTheora::_initialize()
	{
		// start up Ogg stream synchronization layer
		ogg_sync_init( &m_oggSyncState );

		// init supporting Theora structures needed in header parsing
		theora_comment_init( &m_theoraComment );
		theora_info_init( &m_theoraInfo );

		static bool s_initializeCoefTables = false;
		if( s_initializeCoefTables == false )
		{
			s_initializeCoefTables = true;
			s_createCoefTables();
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool VideoDecoderTheora::_prepareData()
	{
		// Ogg file open; parse the headers
		// Only interested in Vorbis/Theora streams 
		bool stateFlag = false;
		int theoraHeaderPackets = 0;  // ����� ������������ ������� ���������� theora
		while( !stateFlag )
		{
			// � �������� �� � ����� ������ ogg
			if( this->buffer_data_() == 0 )
			{
				// �������� ����, �� ������ ����� ��� ������
				LOGGER_ERROR( m_serviceProvider )("Theora Codec Error: bad file");

				return false;
			}

			ogg_page page;
			// ogg_sync_pageout - ��������� ��������
			while( ogg_sync_pageout( &m_oggSyncState, &page ) > 0 )
				// 1-������ ������ �� ���������
				// 0-��������� ������ ������ ��� �������� ��������
			{

				// ��� �������� ������������ �������

				// ��� �������� ����������? ���� ���, ������� ������ ���������
				if( ogg_page_bos( &page ) == false )
				{
					// ���, ��� �� �������� ����������
					// ������, �������� ���������� ���� ���������� ������� ���������
					// � �������� ������ ���� �������
					// ����� ������� ���� ���������� � ������ ������� ������

					// ���������� ��� ��������� � ���������� ����������
					ogg_stream_pagein( &m_oggStreamState, &page );
					// ���������� ���������
					// � ���������� ����� theora, ����
					// ��������� ������������� ����������� ������
					// � ��������� ������ �������� ������������

					// ������� �� ������
					stateFlag = true;
					break;
				}

				// ��, ��� �������� ����������

				// �������� ���������� �����
				// �������������� �������� ����� �� ��� �� ����� � ����� ��
				// ��������������� ������, ��� � � ������� ���������
				if( ogg_stream_init( &m_oggStreamState, ogg_page_serialno( &page ) ) != 0 )
				{
					LOGGER_ERROR( m_serviceProvider )("TheoraCodec Error: error during ogg_stream_init");

					return false;
				}

				// ��������� �������� � �������� �����
				if( ogg_stream_pagein( &m_oggStreamState, &page ) != 0 )
				{
					LOGGER_ERROR( m_serviceProvider )("TheoraCodec Error: error during ogg_stream_pagein");

					return false;
				}
				// ���������� ������ �� ����� ��������� ������ � �����
				if( ogg_stream_packetout( &m_oggStreamState, &m_oggPacket ) == -1 )
				{
					LOGGER_ERROR( m_serviceProvider )("TheoraCodec Error: error during ogg_stream_packetout");

					return false;
				}

				// theoraHeaderPackets - ����� �����������
				// ������������ ������� theora (�� �������)
				// �� ������������ theora ����� ������� ������ ���� ���
				if( theoraHeaderPackets == 0 )
				{
					int dhr = theora_decode_header( &m_theoraInfo, &m_theoraComment, &m_oggPacket );
					// ���������� ��������� theora
					if( dhr < 0 )
					{
						// ��� �� ��������� theora

						// ������� ��������� ��������� ������
						ogg_stream_clear( &m_oggStreamState );
						//� ���������� ���� � ������� ���������� theora
					}
					else
					{
						// ��� ��������� theora!

						// ��� ����� ������� "��������������" ���������� ����� theora:
						// ������ �� ����� ������ ����� ������ �������� ������ theora

						theoraHeaderPackets++;

						// ����� ����, ��� �� ����� ������������ �������� ����������� ������ theora,
						// ��� ���������� ��������� ��� ��������� ������������ ��������
						// ������ ������� � ��������� �� (���� �������, �������, �������)
					}
				}
			}
		}

		// ������ ���� �������� ��� ��� ������ ���������� theora (��. � ������������)
		// � ����� ���������� � ���������� ���������������

		while( theoraHeaderPackets < 3 )
		{
			int result = ogg_stream_packetout( &m_oggStreamState, &m_oggPacket );
			// ���� ������� ���������� ����, ������ �� ������� ������ ��� �������������

			if( result < 0 )
			{
				// ������ �������������, ������������ �����
				LOGGER_ERROR( m_serviceProvider )("TheoraCodec Error: error during ogg_stream_packetout");

				return false;
			}

			if( result > 0 )
			{
				// ������� ������� ������� ����� ���������� theora
				int result2 = theora_decode_header( &m_theoraInfo, &m_theoraComment, &m_oggPacket );

				if( result2 < 0 )
				{
					// ������ �������������, ������������ �����
					LOGGER_ERROR( m_serviceProvider )("TheoraCodec Error: error during theora_decode_header (corrupt stream)");

					return false;
				}

				++theoraHeaderPackets;
			}

			ogg_page page;

			// ��� ��������� ����������, ���� ������� �����
			// ��� ����� ��������� ����� ������, ����� ��� �������� ���-���� �������
			// �� ���������. ���� �� ��������, ����� ������ ������ ��� ������ �� �����:			
			if( ogg_sync_pageout( &m_oggSyncState, &page ) >0 )
				// ogg_sync_pageout - �������, ����� ������ �� ������ ������ ogg
				// � ���������� �� � ogg_page
			{
				//�� ����� ��������� � ������ �...
				ogg_stream_pagein( &m_oggStreamState, &page );
				// ...������ ��� ������ � ���������� �����
			}
			else
			{
				// ������ �� � ������ �� �����
				int ret = this->buffer_data_();
				// ���� ������ ������! ������ �� �� �����
				if( ret == 0 )
				{
					// ����� ���� ��������!
					LOGGER_ERROR( m_serviceProvider )("TheoraCodec Error: eof searched. terminate...");

					return false;
				}
			}
		}

		// ������� �� ��������, ���, ��� ������. �������������� ��������
		theora_decode_init( &m_theoraState, &m_theoraInfo );

		if( m_options.alpha == true )
		{
			m_dataInfo.frameWidth = m_theoraInfo.width;
			m_dataInfo.frameHeight = m_theoraInfo.height / 2;
			m_dataInfo.channel = 4;
		}
		else
		{
			m_dataInfo.frameWidth = m_theoraInfo.width;
			m_dataInfo.frameHeight = m_theoraInfo.height;
			m_dataInfo.channel = 3;
		}

		m_dataInfo.duration = m_options.duration;
		m_dataInfo.fps = m_options.fps;

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void VideoDecoderTheora::_finalize()
	{
		ogg_stream_clear( &m_oggStreamState );
		ogg_sync_clear( &m_oggSyncState );
		//ogg_packet_clear( &m_oggPacket );

		theora_clear( &m_theoraState );
		theora_comment_clear( &m_theoraComment );
		theora_info_clear( &m_theoraInfo );
	}
	//////////////////////////////////////////////////////////////////////////
	size_t VideoDecoderTheora::decode( void * _buffer, size_t _bufferSize )
	{
		yuv_buffer yuvBuffer;
		// ���������� ��������� � YUV-���� � ����. ��������� yuv_buffer
		if( theora_decode_YUVout( &m_theoraState, &yuvBuffer ) != 0 )
		{
			// ������ �������������
			LOGGER_ERROR( m_serviceProvider )("error during theora_decode_YUVout...");

			return 0;
		}

		unsigned char * byte_buffer = static_cast<unsigned char *>(_buffer);

		if( this->decodeBuffer_( yuvBuffer, byte_buffer, _bufferSize ) == false )
		{
			return 0;
		}

		return 1;
	}
	//////////////////////////////////////////////////////////////////////////
	bool VideoDecoderTheora::decodeBuffer_( const yuv_buffer & _yuvBuffer, unsigned char * _buffer, size_t _size )
	{
		(void)_size;

		if( m_options.alpha == false && m_options.pixelFormat == PF_X8R8G8B8 )
		{
			unsigned char * dstBitmap = _buffer;
			unsigned char * dstBitmapOffset = _buffer + m_pitch;

			unsigned int dstOff = m_pitch * 2 - m_theoraInfo.width * 4;
			int yOff = (_yuvBuffer.y_stride * 2) - _yuvBuffer.y_width;

			int y_height = _yuvBuffer.y_height >> 1;
			int y_width = _yuvBuffer.y_width >> 1;

			unsigned char * ySrc = (unsigned char*)_yuvBuffer.y;
			unsigned char * uSrc = (unsigned char*)_yuvBuffer.u;
			unsigned char * vSrc = (unsigned char*)_yuvBuffer.v;
			unsigned char * ySrc2 = ySrc + _yuvBuffer.y_stride;

			//Loop does four blocks per iteration (2 rows, 2 pixels at a time)
			for( int y = y_height; y > 0; --y )
			{
				for( int x = 0; x < y_width; ++x )
				{
					//Get uv pointers for row
					int u = uSrc[x];
					int v = vSrc[x];

					//get corresponding lookup values
					int rgbY = s_YTable[*ySrc];
					int rV = s_RVTable[v];
					int gUV = s_GUTable[u] + s_GVTable[v];
					int bU = s_BUTable[u];
					++ySrc;

					//scale down - brings are values back into the 8 bits of a byte
					int r = (rgbY + rV) >> 13;
					int g = (rgbY - gUV) >> 13;
					int b = (rgbY + bU) >> 13;

					//Clip to RGB values (255 0)
					CLIP_RGB_COLOR( r, dstBitmap[COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmap[COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmap[COLOR_B] );
					dstBitmap[COLOR_A] = 255;

					//And repeat for other pixels (note, y is unique for each
					//pixel, while uv are not)
					rgbY = s_YTable[*ySrc];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmap[4 + COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmap[4 + COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmap[4 + COLOR_B] );
					dstBitmap[4 + COLOR_A] = 255;
					++ySrc;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmapOffset[COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmapOffset[COLOR_B] );
					dstBitmapOffset[COLOR_A] = 255;
					++ySrc2;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[4 + COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmapOffset[4 + COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmapOffset[4 + COLOR_B] );
					dstBitmapOffset[4 + COLOR_A] = 255;
					++ySrc2;

					/*dstBitmap[COLOR_G] = 0;
					dstBitmap[COLOR_G+4] = 0;
					dstBitmapOffset[COLOR_G] = 0;
					dstBitmapOffset[COLOR_G+4] = 0;*/

					//Advance inner loop offsets
					dstBitmap += 4 << 1;
					dstBitmapOffset += 4 << 1;
				} // end for x

				//Advance destination pointers by offsets
				dstBitmap += dstOff;
				dstBitmapOffset += dstOff;
				ySrc += yOff;
				ySrc2 += yOff;
				uSrc += _yuvBuffer.uv_stride;
				vSrc += _yuvBuffer.uv_stride;
			} //end for y
		}
		else if( m_options.alpha == false && m_options.pixelFormat == PF_R8G8B8 )
		{
			unsigned char * dstBitmap = _buffer;
			unsigned char * dstBitmapOffset = _buffer + m_pitch;

			unsigned int dstOff = m_pitch * 2 - m_theoraInfo.width * 3;
			int yOff = (_yuvBuffer.y_stride * 2) - _yuvBuffer.y_width;

			int y_height = _yuvBuffer.y_height >> 1;
			int y_width = _yuvBuffer.y_width >> 1;

			unsigned char * ySrc = (unsigned char*)_yuvBuffer.y;
			unsigned char * uSrc = (unsigned char*)_yuvBuffer.u;
			unsigned char * vSrc = (unsigned char*)_yuvBuffer.v;
			unsigned char * ySrc2 = ySrc + _yuvBuffer.y_stride;

			//Loop does four blocks per iteration (2 rows, 2 pixels at a time)
			for( int y = y_height; y > 0; --y )
			{
				for( int x = 0; x < y_width; ++x )
				{
					//Get uv pointers for row
					int u = uSrc[x];
					int v = vSrc[x];

					//get corresponding lookup values
					int rgbY = s_YTable[*ySrc];
					int rV = s_RVTable[v];
					int gUV = s_GUTable[u] + s_GVTable[v];
					int bU = s_BUTable[u];
					++ySrc;

					//scale down - brings are values back into the 8 bits of a byte
					int r = (rgbY + rV) >> 13;
					int g = (rgbY - gUV) >> 13;
					int b = (rgbY + bU) >> 13;

					//Clip to RGB values (255 0)
					CLIP_RGB_COLOR( r, dstBitmap[COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmap[COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmap[COLOR_B] );

					//And repeat for other pixels (note, y is unique for each
					//pixel, while uv are not)
					rgbY = s_YTable[*ySrc];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmap[3 + COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmap[3 + COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmap[3 + COLOR_B] );
					++ySrc;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmapOffset[COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmapOffset[COLOR_B] );
					++ySrc2;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[3 + COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmapOffset[3 + COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmapOffset[3 + COLOR_B] );
					++ySrc2;

					/*dstBitmap[COLOR_G] = 0;
					dstBitmap[COLOR_G+4] = 0;
					dstBitmapOffset[COLOR_G] = 0;
					dstBitmapOffset[COLOR_G+4] = 0;*/

					//Advance inner loop offsets
					dstBitmap += 3 << 1;
					dstBitmapOffset += 3 << 1;
				} // end for x

				//Advance destination pointers by offsets
				dstBitmap += dstOff;
				dstBitmapOffset += dstOff;
				ySrc += yOff;
				ySrc2 += yOff;
				uSrc += _yuvBuffer.uv_stride;
				vSrc += _yuvBuffer.uv_stride;
			} //end for y
		}
		else if( m_options.alpha == true )
		{
			unsigned char * dstBitmap = _buffer;
			unsigned char * dstBitmapOffset = _buffer + m_pitch;

			unsigned int dstOff = m_pitch * 2 - m_theoraInfo.width * 4;//m_theoraInfo.width * 4;//( m_Width*6 ) - ( yuv->y_width*3 );
			int yOff = (_yuvBuffer.y_stride * 2) - _yuvBuffer.y_width;

			int y_height = _yuvBuffer.y_height >> 1;
			int y_width = _yuvBuffer.y_width >> 1;

			unsigned char * ySrc = (unsigned char*)_yuvBuffer.y;
			unsigned char * uSrc = (unsigned char*)_yuvBuffer.u;
			unsigned char * vSrc = (unsigned char*)_yuvBuffer.v;
			unsigned char * ySrc2 = ySrc + _yuvBuffer.y_stride;

			//Loop does four blocks per iteration (2 rows, 2 pixels at a time)
			int y_rgb_height_begin = y_height;
			int y_rgb_height_end = y_height / 2;

			for( int y = y_rgb_height_begin; y > y_rgb_height_end; --y )
			{
				for( int x = 0; x < y_width; ++x )
				{
					//Get uv pointers for row
					int u = uSrc[x];
					int v = vSrc[x];

					//get corresponding lookup values
					int rgbY = s_YTable[*ySrc];
					int rV = s_RVTable[v];
					int gUV = s_GUTable[u] + s_GVTable[v];
					int bU = s_BUTable[u];
					++ySrc;

					//scale down - brings are values back into the 8 bits of a byte
					int r = (rgbY + rV) >> 13;
					int g = (rgbY - gUV) >> 13;
					int b = (rgbY + bU) >> 13;

					//Clip to RGB values (255 0)
					CLIP_RGB_COLOR( r, dstBitmap[COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmap[COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmap[COLOR_B] );

					//And repeat for other pixels (note, y is unique for each
					//pixel, while uv are not)
					rgbY = s_YTable[*ySrc];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmap[4 + COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmap[4 + COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmap[4 + COLOR_B] );
					++ySrc;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmapOffset[COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmapOffset[COLOR_B] );
					++ySrc2;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					g = (rgbY - gUV) >> 13;
					b = (rgbY + bU) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[4 + COLOR_R] );
					CLIP_RGB_COLOR( g, dstBitmapOffset[4 + COLOR_G] );
					CLIP_RGB_COLOR( b, dstBitmapOffset[4 + COLOR_B] );
					++ySrc2;

					/*dstBitmap[COLOR_G] = 0;
					dstBitmap[COLOR_G+4] = 0;
					dstBitmapOffset[COLOR_G] = 0;
					dstBitmapOffset[COLOR_G+4] = 0;*/

					//Advance inner loop offsets
					dstBitmap += 4 << 1;
					dstBitmapOffset += 4 << 1;
				} // end for x

				//Advance destination pointers by offsets
				dstBitmap += dstOff;
				dstBitmapOffset += dstOff;
				ySrc += yOff;
				ySrc2 += yOff;
				uSrc += _yuvBuffer.uv_stride;
				vSrc += _yuvBuffer.uv_stride;
			} //end for y

			dstBitmap = _buffer;
			dstBitmapOffset = _buffer + m_pitch;

			ySrc = (unsigned char*)_yuvBuffer.y;
			uSrc = (unsigned char*)_yuvBuffer.u;
			vSrc = (unsigned char*)_yuvBuffer.v;
			ySrc2 = ySrc + _yuvBuffer.y_stride;

			//Loop does four blocks per iteration (2 rows, 2 pixels at a time)
			int y_alpha_height_begin = y_height / 2;
			int y_alpha_height_end = 0;
			for( int y = y_alpha_height_begin; y > y_alpha_height_end; --y )
			{
				for( int x = 0; x < y_width; ++x )
				{
					//Get uv pointers for row
					int v = vSrc[x];

					//get corresponding lookup values
					int rgbY = s_YTable[*ySrc];
					int rV = s_RVTable[v];
					++ySrc;

					//scale down - brings are values back into the 8 bits of a byte
					int r = (rgbY + rV) >> 13;

					//Clip to RGB values (255 0)
					CLIP_RGB_COLOR( r, dstBitmap[COLOR_A] );

					//And repeat for other pixels (note, y is unique for each
					//pixel, while uv are not)
					rgbY = s_YTable[*ySrc];
					r = (rgbY + rV) >> 13;
					CLIP_RGB_COLOR( r, dstBitmap[4 + COLOR_A] );
					++ySrc;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[COLOR_A] );
					++ySrc2;

					rgbY = s_YTable[*ySrc2];
					r = (rgbY + rV) >> 13;
					CLIP_RGB_COLOR( r, dstBitmapOffset[4 + COLOR_A] );
					++ySrc2;

					/*dstBitmap[COLOR_G] = 0;
					dstBitmap[COLOR_G+4] = 0;
					dstBitmapOffset[COLOR_G] = 0;
					dstBitmapOffset[COLOR_G+4] = 0;*/

					//Advance inner loop offsets
					dstBitmap += 4 << 1;
					dstBitmapOffset += 4 << 1;
				} // end for x

				//Advance destination pointers by offsets
				dstBitmap += dstOff;
				dstBitmapOffset += dstOff;
				ySrc += yOff;
				ySrc2 += yOff;
				uSrc += _yuvBuffer.uv_stride;
				vSrc += _yuvBuffer.uv_stride;
			} //end for y
		}
		else
		{
			LOGGER_ERROR( m_serviceProvider )("VideoDecoderTheora::decodeBuffer_ unsupport format");

			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	size_t VideoDecoderTheora::buffer_data_()
	{
		char* buffer = ogg_sync_buffer( &m_oggSyncState, OGG_BUFFER_SIZE );
		size_t bytes = m_stream->read( buffer, OGG_BUFFER_SIZE );
		ogg_sync_wrote( &m_oggSyncState, bytes );

		return bytes;
	}
	//////////////////////////////////////////////////////////////////////////
	EVideoDecoderReadState VideoDecoderTheora::readNextFrame( float & _pts )
	{
		(void)_pts;

		m_lastReadBytes = 0;

		// theora processing...
		while( ogg_stream_packetout( &m_oggStreamState, &m_oggPacket ) <= 0 )
		{
			// �� ������� ������ � ���������� ������ theora
			// ���� ��������� ������ �� ����������� ������ � ��������� �� � ���������� �����

			// ������ ������ �� �����
			size_t bytes = this->buffer_data_();

			if( bytes == 0 )
			{
				return VDRS_END_STREAM;
			}

			m_lastReadBytes += bytes;

			if( m_lastReadBytes == 0 )
			{
				// ���� ��������, ���������� ��������� ����������� ��������
				// � ����� �� ����������
				//TheoraClose();

				//LOG_NUMBER(LOG_NOTE, "frames: ", current_frame);
				//FINISHED=true;
				return VDRS_END_STREAM;
			}

			ogg_page page;
			while( ogg_sync_pageout( &m_oggSyncState, &page ) > 0 )
				// ���������� ������ �� ������ � �������� (ogg_page)
				// ���� ��� �� �������� � ������
			{
				// ������ ��� �������� � �����. ���������� ������
				ogg_stream_pagein( &m_oggStreamState, &page );
			}
		}

		// ������ ������������. � ������ ���������� �������������� ogg-����������
		// (�� ���� �������������� theora-����������)

		// ��������� ����� � ������� theora
		if( theora_decode_packetin( &m_theoraState, &m_oggPacket ) == OC_BADPACKET )
		{
			return VDRS_FAILURE;
		}

		return VDRS_SUCCESS;
	}
	//////////////////////////////////////////////////////////////////////////
	bool VideoDecoderTheora::seek( float _timing )
	{
		(void)_timing;

		if( ogg_sync_reset( &m_oggSyncState ) == -1 )
		{
			return false;
		}

		if( this->_rewind() == false )
		{
			return false;
		}

		//for( ;; )
		//{
		//	ogg_page page;
		//	long seek = ogg_sync_pageseek( &m_oggSyncState, &page );

		//	if( seek != 0 )
		//	{
		//		ogg_int64_t granulepos = ogg_page_granulepos( &page );

		//		double time = theora_granule_time( &m_theoraState, granulepos );

		//		printf( "%f", (float)time );
		//	}
		//	else
		//	{
		//		char* buffer = ogg_sync_buffer( &m_oggSyncState, OGG_BUFFER_SIZE );
		//		size_t bytes = m_stream->read( buffer, OGG_BUFFER_SIZE );
		//		ogg_sync_wrote( &m_oggSyncState, bytes );
		//	}
		//}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	float VideoDecoderTheora::getTiming() const
	{
		double granule_time = theora_granule_time( &m_theoraState, m_theoraState.granulepos );

		float timing = (float)(granule_time * 1000.0);

		return timing;
	}
	//////////////////////////////////////////////////////////////////////////
	void VideoDecoderTheora::setPitch( size_t _pitch )
	{
		m_pitch = _pitch;
	}
	//////////////////////////////////////////////////////////////////////////
}	// namespace Menge
