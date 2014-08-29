#	include "Sprite.h"
#	include "Layer2D.h"

#	include "Interface/RenderSystemInterface.h"
#	include "Interface/ResourceInterface.h"

#	include "Kernel/ResourceImage.h"

#	include "Consts.h"

#	include "Logger/Logger.h"

#	include "math/box2.h"
#	include "math/clamp.h"

namespace	Menge
{
	//////////////////////////////////////////////////////////////////////////
	Sprite::Sprite()
		: m_isCustomSize(false)
		, m_customSize(0.f, 0.f)
		, m_material(nullptr)
		, m_texturesNum(0)
	{
	}
	//////////////////////////////////////////////////////////////////////////
	Sprite::~Sprite()
	{
	}
	//////////////////////////////////////////////////////////////////////////
	bool Sprite::_compile()
	{
		if( this->compileResource_() == false )
		{
			return false;
		}

        this->updateResource_();

        this->invalidateMaterial();
		
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool Sprite::compileResource_()
	{
		if( m_resourceImage == nullptr )
		{
            LOGGER_ERROR(m_serviceProvider)("Sprite::compileResource_ '%s' image resource null"
                , m_name.c_str()
                );

            return false;
        }

        if( m_resourceImage.compile() == false )
        {
            LOGGER_ERROR(m_serviceProvider)("Sprite::compileResource_ '%s' image resource %s not compile"
                , m_name.c_str()
                , m_resourceImage->getName().c_str()
                );

            return false;
        }

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	void Sprite::_release()
	{
        m_resourceImage.release();

        m_textures[0] = nullptr;
        m_textures[1] = nullptr;

		m_material = nullptr;
	}
    //////////////////////////////////////////////////////////////////////////
    void Sprite::setResourceImage( ResourceImage * _resourceImage )
    {
        if( m_resourceImage == _resourceImage )
        {
            return;
        }

        m_resourceImage = _resourceImage;

        this->recompile();
    }
    //////////////////////////////////////////////////////////////////////////
    ResourceImage * Sprite::getResourceImage() const
    {
        return m_resourceImage;
    }
    //////////////////////////////////////////////////////////////////////////
    void Sprite::updateResource_()
    {
        bool wrapX = m_resourceImage->isWrapU();
        this->setTextureWrapX( wrapX );

        bool wrapY = m_resourceImage->isWrapV();
        this->setTextureWrapY( wrapY );

        if( m_isCustomSize == false )
        {
            const mt::vec2f & size = m_resourceImage->getSize();
            this->setSize( size );

            const mt::vec2f & maxSize = m_resourceImage->getMaxSize();
            this->setMaxSize( maxSize );

			const mt::vec2f & offset = m_resourceImage->getOffset();
			this->setOffset( offset );
        }
        else
        {
            this->setSize( m_customSize );
            this->setMaxSize( m_customSize );
        }

        bool uvRotate = m_resourceImage->isUVRotate();
        this->setUVRotate( uvRotate );

        const mt::vec4f & uv = m_resourceImage->getUVImage();
        this->setUV( uv );

        const mt::vec4f & uvAlpha = m_resourceImage->getUVAlpha();
        this->setUV2( uvAlpha );

        m_textures[0] = m_resourceImage->getTexture();
        m_textures[1] = m_resourceImage->getTextureAlpha();
    }
	//////////////////////////////////////////////////////////////////////////
	void Sprite::updateMaterial()
	{
        m_invalidateMaterial = false;

        const RenderTextureInterfacePtr & textureAlpha = m_resourceImage->getTextureAlpha();

		ConstString stageName;

        if( textureAlpha != nullptr )
        {
			if( m_blendAdd == true )
            {
				if( m_disableTextureColor == true )
				{
					m_texturesNum = 2;

					stageName = CONST_STRING(m_serviceProvider, ExternalAlphaIntensive_OnlyColor);
				}
				else if( m_resourceImage->isAlpha() == true || m_solid == false )
				{
					m_texturesNum = 2;

					stageName = CONST_STRING(m_serviceProvider, ExternalAlphaIntensive);
				}
				else
				{
					m_texturesNum = 1;

					stageName = CONST_STRING(m_serviceProvider, SolidSprite);
				}
            }
			else
			{
				if( m_disableTextureColor == true )
				{
					m_texturesNum = 2;

					stageName = CONST_STRING(m_serviceProvider, ExternalAlpha_OnlyColor);
				}
				else if( m_resourceImage->isAlpha() == true || m_solid == false )
				{
					m_texturesNum = 2;

					stageName = CONST_STRING(m_serviceProvider, ExternalAlpha);
				}
				else
				{
					m_texturesNum = 1;

					stageName = CONST_STRING(m_serviceProvider, SolidSprite);
				}
			}
        }
		else
		{
			m_texturesNum = 1;

			if( m_blendAdd == true )
			{
				stageName = CONST_STRING(m_serviceProvider, ParticleIntensive);
			}
			else if( m_disableTextureColor == true )
			{
				stageName = CONST_STRING(m_serviceProvider, OnlyColor);
			}
			else
			{
				if( m_resourceImage->isAlpha() == true || m_solid == false )
				{
					stageName = CONST_STRING(m_serviceProvider, BlendSprite);
				}
				else
				{
					stageName = CONST_STRING(m_serviceProvider, SolidSprite);
				}
			}
		}

		bool wrapU = m_resourceImage->isWrapU();
		bool wrapV = m_resourceImage->isWrapV();

		m_material = RENDERMATERIAL_SERVICE(m_serviceProvider)
			->getMaterial( stageName, wrapU, wrapV, PT_TRIANGLELIST, m_texturesNum, m_textures );

		if( m_material == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("Sprite::updateMaterial_ %s resource %s m_material is NULL"
				, this->getName().c_str()
				, m_resourceImage->getName().c_str()
				);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	void Sprite::_render( const RenderViewportInterface * _viewport, const RenderCameraInterface * _camera )
	{
		Node::_render( _viewport, _camera );

		const RenderVertex2D * vertices = this->getVerticesWM();
        const RenderMaterialInterfacePtr & material = this->getMaterial();

		const mt::box2f & bb = this->getBoundingBox();

		RENDER_SERVICE(m_serviceProvider)
			->addRenderQuad( _viewport, _camera, material, vertices, 4, &bb );
	}
	//////////////////////////////////////////////////////////////////////////
	void Sprite::_updateBoundingBox( mt::box2f & _boundingBox ) const
	{
		const RenderVertex2D * vertices = this->getVerticesWM();

		mt::reset( _boundingBox, vertices[0].pos.x, vertices[0].pos.y );

		mt::add_internal_point( _boundingBox, vertices[1].pos.x, vertices[1].pos.y );
		mt::add_internal_point( _boundingBox, vertices[2].pos.x, vertices[2].pos.y );
		mt::add_internal_point( _boundingBox, vertices[3].pos.x, vertices[3].pos.y );
	}
    //////////////////////////////////////////////////////////////////////////
    void Sprite::_invalidateColor()
    {
        Shape::_invalidateColor();

        bool solid = this->isSolidColor();

        if( m_solid != solid )
        {
			m_solid = solid;

            this->invalidateMaterial();
        }
    }
    //////////////////////////////////////////////////////////////////////////
    void Sprite::setCustomSize( const mt::vec2f & _size )
    {
        m_customSize = _size;
        m_isCustomSize = true;

        this->recompile();
    }
}
