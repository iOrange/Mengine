#	pragma once

#	include "Interface/ServiceInterface.h"
#	include "Interface/StreamInterface.h"

#	include "Factory/FactorablePtr.h"

#	include "Core/MemoryCacheInput.h"
#	include "Core/MemoryProxyInput.h"
#	include "Core/MemoryInput.h"

#	include "Config/Blobject.h"

#   include "stdex/intrusive_ptr.h"

namespace Menge
{
	class CacheServiceInterface
		: public ServiceInterface
	{
		SERVICE_DECLARE( "CacheService" )

	public:
		virtual bool initialize() = 0;
		virtual void finalize() = 0;

	public:
		virtual size_t lockBuffer( size_t _size, void ** _memory, const char * _doc ) = 0;
		virtual void unlockBuffer( size_t _bufferId ) = 0;

	public:
		virtual MemoryCacheInputPtr createMemoryCacheInput() = 0;
		virtual MemoryProxyInputPtr createMemoryProxyInput() = 0;
		virtual MemoryInputPtr createMemoryInput() = 0;
	};

#   define CACHE_SERVICE( serviceProvider )\
	SERVICE_GET(serviceProvider, Menge::CacheServiceInterface)
}