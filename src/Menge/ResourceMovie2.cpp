#	include "ResourceMovie2.h"

#	include "Interface/ResourceInterface.h"
#	include "Interface/StringizeInterface.h"

#	include "ResourceImageDefault.h"

#	include "Metacode/Metacode.h"

#	include "Logger/Logger.h"

#	include "Consts.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	static void * stdex_movie_alloc( void * _data, uint32_t _size )
	{
		(void)_data;

		return stdex_malloc( _size );
	}
	//////////////////////////////////////////////////////////////////////////
	static void * stdex_movie_alloc_n( void * _data, uint32_t _size, uint32_t _count )
	{
		(void)_data;

		uint32_t total = _size * _count;

		return stdex_malloc( total );
	}
	//////////////////////////////////////////////////////////////////////////
	static void stdex_movie_free( void * _data, const void * _ptr )
	{
		(void)_data;

		stdex_free( (void *)_ptr );
	}
	//////////////////////////////////////////////////////////////////////////
	static void stdex_movie_free_n( void * _data, const void * _ptr )
	{
		(void)_data;

		stdex_free( (void *)_ptr );
	}
	//////////////////////////////////////////////////////////////////////////
	static void Mengine_read_stream( void * _data, void * _buff, uint32_t _size )
	{
		InputStreamInterface * stream = (InputStreamInterface *)_data;

		stream->read( _buff, _size );
	}
	//////////////////////////////////////////////////////////////////////////
	static void * Mengine_resource_provider( const aeMovieResource * _resource, void * _data )
	{
		ResourceMovie2 * resourceMovie2 = (ResourceMovie2 *)_data;

		uint8_t resource_type = _resource->type;

		switch( resource_type )
		{
		case AE_MOVIE_RESOURCE_IMAGE:
			{
				const aeMovieResourceImage * resource_image = (const aeMovieResourceImage *)_resource;

				ResourceReference * data_resource = resourceMovie2->createImageResource_( resource_image );

				return data_resource;
			}break;
		}		

		return AE_NULL;
	}
	//////////////////////////////////////////////////////////////////////////
	ResourceMovie2::ResourceMovie2()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	ResourceMovie2::~ResourceMovie2()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	aeMovieComposition * ResourceMovie2::createComposition( const ConstString & _name )
	{
		if( this->isCompile() == false )
		{
			return nullptr;
		}

		const aeMovieCompositionData * compositionData = get_movie_composition_data( m_movieData, _name.c_str() );

		if( compositionData == nullptr )
		{
			return nullptr;
		}

		aeMovieComposition * composition = create_movie_composition( &m_instance, m_movieData, compositionData );

		return composition;
	}
	//////////////////////////////////////////////////////////////////////////
	void ResourceMovie2::destroyComposition( aeMovieComposition * _composition )
	{
		destroy_movie_composition( &m_instance, _composition );
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceMovie2::_initialize()
	{		
		make_movie_instance( &m_instance, &stdex_movie_alloc, &stdex_movie_alloc_n, &stdex_movie_free, &stdex_movie_free_n, AE_NULL );

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceMovie2::_loader( const Metabuf::Metadata * _meta )
	{
        const Metacode::Meta_DataBlock::Meta_ResourceMovie2 * metadata
            = static_cast<const Metacode::Meta_DataBlock::Meta_ResourceMovie2 *>(_meta);
               
        metadata->swap_File_Path( m_filePath );

        return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceMovie2::_compile()
	{
		if( ResourceReference::_compile() == false )
		{
			return false;
		}

		if( m_filePath.empty() == true )
		{
			LOGGER_ERROR(m_serviceProvider)("ResourceMovie::_compile: '%s' group '%s' don`t have Key Frames Pack Path"
				, this->getName().c_str()
				, this->getGroup().c_str()
				);

			return false;
		}

		const ConstString & category = this->getCategory();

		InputStreamInterfacePtr stream = FILE_SERVICE( m_serviceProvider )
			->openInputFile( category, m_filePath, false );

		if( stream == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("ResourceMovie2::_compile: '%s' group '%s' can`t open file '%s'"
				, this->getName().c_str()
				, this->getGroup().c_str()
				, m_filePath.c_str()
				);

			return false;
		}

		m_movieData = create_movie_data( &m_instance );

		aeMovieStream movie_stream;
		movie_stream.read = &Mengine_read_stream;
		movie_stream.data = stream.get();

		if( load_movie_data( &m_instance, &movie_stream, m_movieData, &Mengine_resource_provider, this ) == AE_MOVIE_FAILED )
		{
			return 0;
		}

		stream = nullptr;

		for( TVectorResources::const_iterator
			it = m_resources.begin(),
			it_end = m_resources.end();
		it != it_end;
		++it )
		{
			const ResourceReferencePtr & resource = *it;

			if( resource->compile() == false )
			{
				return false;
			}
		}
		
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void ResourceMovie2::_release()
	{
		delete_movie_data( &m_instance, m_movieData );

		ResourceReference::_release();
	}
	//////////////////////////////////////////////////////////////////////////
	ResourceReference * ResourceMovie2::createImageResource_( const aeMovieResourceImage * _resource )
	{
		const ConstString & locale = this->getLocale();
		const ConstString & category = this->getCategory();
		const ConstString & group = this->getGroup();

		ResourceImageDefaultPtr image = RESOURCE_SERVICE( m_serviceProvider )
			->generateResourceT<ResourceImageDefaultPtr>( locale, category, group, ConstString::none(), CONST_STRING( m_serviceProvider, ResourceImageDefault ) );

		PathString full_path;

		ConstString folder = Helper::getPathFolder( m_serviceProvider, m_filePath );

		full_path += folder.c_str();
		full_path += _resource->path;

		FilePath c_path = Helper::stringizeString( m_serviceProvider, full_path );

		mt::uv4f uv_image;
		mt::uv4f uv_alpha;

		mt::vec2f size( _resource->width, _resource->height );

		image->setup( c_path, ConstString::none(), uv_image, uv_alpha, size );

		m_resources.push_back( image );

		return image.get();
	}

}