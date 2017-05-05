#	include "ResourceAnimation.h"

#   include "Interface/ResourceInterface.h"
#   include "Interface/ConfigInterface.h"

#	include "Metacode/Metacode.h"

#	include "Kernel/ResourceImage.h"

#	include "Logger/Logger.h"

namespace Menge
{
	//////////////////////////////////////////////////////////////////////////
	ResourceAnimation::ResourceAnimation()
		: m_duration(0.f)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceAnimation::_loader( const Metabuf::Metadata * _meta )
	{
        const Metacode::Meta_DataBlock::Meta_ResourceAnimation * metadata
            = static_cast<const Metacode::Meta_DataBlock::Meta_ResourceAnimation *>(_meta);

        const Metacode::Meta_DataBlock::Meta_ResourceAnimation::TVectorMeta_Sequence & includes_sequence = metadata->get_IncludesSequence();

		m_duration = 0.f;

        for( Metacode::Meta_DataBlock::Meta_ResourceAnimation::TVectorMeta_Sequence::const_iterator
            it = includes_sequence.begin(),
            it_end = includes_sequence.end();
        it != it_end;
        ++it )
        {
            const Metacode::Meta_DataBlock::Meta_ResourceAnimation::Meta_Sequence & meta_sequence = *it;

            AnimationSequence sq;

            meta_sequence.swap_ResourceImageName( sq.resourceName );
            float delay = meta_sequence.get_Delay();

            sq.delay = delay;

            m_duration += delay;

            m_sequence.push_back( sq );
        }

        return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceAnimation::_compile()
	{
		if( m_sequence.empty() )
		{
			LOGGER_ERROR(m_serviceProvider)("ResourceAnimation::_compile: '%s' sequence count is empty"
				, this->getName().c_str()
				);

			return false;
		}
		
		for( TVectorAnimationSequence::iterator
			it = m_sequence.begin(),
			it_end = m_sequence.end();
		it != it_end;
		++it )
		{
			AnimationSequence & sequence = *(it);
			
			ResourceImagePtr resource  = RESOURCE_SERVICE(m_serviceProvider)
				->getResourceT<ResourceImagePtr>( sequence.resourceName );

			if( resource == nullptr )
			{
				LOGGER_ERROR(m_serviceProvider)("ResourceAnimation::_compile: '%s' Image resource not found resource '%s'"
					, this->getName().c_str()
					, sequence.resourceName.c_str() 
					);

				return false;
			}

			sequence.resource = resource;			
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void ResourceAnimation::_release()
	{
		for( TVectorAnimationSequence::iterator
			it = m_sequence.begin(),
			it_end = m_sequence.end();
		it != it_end;
		++it )
		{
			AnimationSequence & sequence = *(it);
            		
			sequence.resource->decrementReference();
			sequence.resource = nullptr;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	bool ResourceAnimation::_isValid() const
	{
        size_t total_memory = 0;

		for( TVectorAnimationSequence::const_iterator
			it = m_sequence.begin(),
			it_end = m_sequence.end();
		it != it_end;
		++it )
		{
			const AnimationSequence & sequence = *(it);

			ResourceImagePtr resourceImage;
			if( RESOURCE_SERVICE(m_serviceProvider)
				->hasResourceT<ResourceImagePtr>( sequence.resourceName, &resourceImage ) == false )
			{
				LOGGER_ERROR(m_serviceProvider)("ResourceAnimation::_isValid: '%s' not found image resource '%s'"
					, this->getName().c_str()
					, sequence.resourceName.c_str() 
					);

				return false;
			}

            if( resourceImage->isValid() == false )
            {
                LOGGER_ERROR( m_serviceProvider )("ResourceAnimation::_isValid %s invalid validate sequence resource '%s'"
                    , this->getName().c_str()
                    , resourceImage->getName().c_str()
                    );

                return false;
            }

            const RenderTextureInterfacePtr & texture = resourceImage->getTexture();

            if( texture == nullptr )
            {
                continue;
            }

            const RenderImageInterfacePtr & image = texture->getImage();

            uint32_t width = texture->getWidth();
            uint32_t height = texture->getHeight();

            uint32_t HWChannels = image->getHWChannels();
            uint32_t HWDepth = image->getHWDepth();
            PixelFormat HWPixelFormat = image->getHWPixelFormat();

            size_t textureSize = Helper::getTextureMemorySize( width, height, HWChannels, HWDepth, HWPixelFormat );

            total_memory += textureSize;
		}

		uint32_t animationMemoryLimit = CONFIG_VALUE(m_serviceProvider, "Limit", "AnimationMemoryLimit", 4194304U ); //4kb
		
		if( total_memory > animationMemoryLimit && animationMemoryLimit != 0U )
		{
			LOGGER_ERROR(m_serviceProvider)("ResourceAnimation::_isValid: '%s' overflow %.2fmb max video memory %.2fmb (coeff %f)"
				, this->getName().c_str()
				, float(total_memory) / (1024.f * 1024.f)
				, float(animationMemoryLimit) / (1024.f * 1024.f)
				, float(total_memory) / float(animationMemoryLimit)
				);

			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	uint32_t ResourceAnimation::getSequenceCount() const
	{
        TVectorAnimationSequence::size_type sequenceCount = m_sequence.size();

		return (uint32_t)sequenceCount;
	}
	//////////////////////////////////////////////////////////////////////////
	float ResourceAnimation::getSequenceDelay( uint32_t _index ) const
	{
#   ifdef _DEBUG
        uint32_t sequenceCount = this->getSequenceCount();

        if( _index >= sequenceCount )
        {
            LOGGER_ERROR( m_serviceProvider )("ResourceAnimation::getSequenceDelay: '%s' sequence '%u' out of range '%u'"
                , this->getName().c_str()
                , _index
                , sequenceCount
                );

            return 0.f;
        }
#   endif

        const AnimationSequence & sequence = m_sequence[_index];

        float delay = sequence.delay;

		return delay;
	}
	//////////////////////////////////////////////////////////////////////////
	const ConstString & ResourceAnimation::getSequenceResourceName( uint32_t _index ) const
	{
#   ifdef _DEBUG
        uint32_t sequenceCount = this->getSequenceCount();

        if( _index >= sequenceCount )
        {
            LOGGER_ERROR( m_serviceProvider )("ResourceAnimation::getSequenceResourceName: '%s' sequence '%u' out of range '%u'"
                , this->getName().c_str()
                , _index
                , sequenceCount
                );

            return ConstString::none();
        }
#   endif

        const AnimationSequence & sequence = m_sequence[_index];

        const ConstString & resourceName = sequence.resourceName;

		return resourceName;
	}	
	//////////////////////////////////////////////////////////////////////////
	uint32_t ResourceAnimation::getLastFrameIndex() const
	{
        uint32_t sequenceCount = this->getSequenceCount();

#   ifdef _DEBUG
        if( sequenceCount == 0 )
        {
            LOGGER_ERROR( m_serviceProvider )("ResourceAnimation::getLastFrameIndex: '%s' invalid get last frame on empty sequences"
                , this->getName().c_str()
                );

            return 0;
        }
#   endif

		uint32_t lastIndex = sequenceCount - 1;

		return lastIndex;	
	}
	//////////////////////////////////////////////////////////////////////////
	void ResourceAnimation::setSequences( const TVectorAnimationSequence & _sequence )
	{
		m_sequence = _sequence;
	}
	//////////////////////////////////////////////////////////////////////////
	const TVectorAnimationSequence & ResourceAnimation::getSequences() const
	{
		return m_sequence;
	}
	//////////////////////////////////////////////////////////////////////////
	const ResourceImagePtr & ResourceAnimation::getSequenceResource( uint32_t _index ) const
	{
#   ifdef _DEBUG
        uint32_t sequenceCount = this->getSequenceCount();

        if( _index >= sequenceCount )
        {
            LOGGER_ERROR( m_serviceProvider )("ResourceAnimation::getSequenceResource: '%s' sequence '%u' out of range '%u'"
                , this->getName().c_str()
                , _index
                , sequenceCount
                );

            return ResourceImagePtr::none();
        }
#   endif

        const AnimationSequence & sequence = m_sequence[_index];

		const ResourceImagePtr & resource = sequence.resource;

		return resource;
	}
	//////////////////////////////////////////////////////////////////////////
	float ResourceAnimation::getSequenceDuration() const
	{
		return m_duration;
	}
}
