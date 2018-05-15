#pragma once

#include "Interface/MemoryInterface.h"
#include "Interface/ThreadSystemInterface.h"

#include "Core/ServiceBase.h"

#include "MemoryBuffer.h"
#include "MemoryProxy.h"
#include "MemoryCacheBuffer.h"
#include "MemoryCacheInput.h"
#include "MemoryProxyInput.h"
#include "MemoryInput.h"

#include "Factory/FactoryWithMutex.h"

#include "stdex/stl_vector.h"

namespace Mengine
{
	typedef uint32_t CacheBufferID;

	static const CacheBufferID INVALID_CACHE_BUFFER_ID = 0;

	class MemoryManager
		: public ServiceBase<MemoryServiceInterface>
	{
	public:
		MemoryManager();
		~MemoryManager() override;

	public:
		bool _initializeService() override;
		void _finalizeService() override;

	public:
		CacheBufferID lockBuffer( size_t _size, void ** _memory, const char * _file, uint32_t _line );
		void unlockBuffer( CacheBufferID _bufferId );

	public:
		void clearCacheBuffers() override;

	public:
        MemoryBufferInterfacePtr createMemoryBuffer() override;
        MemoryProxyInterfacePtr createMemoryProxy() override;
        MemoryBufferInterfacePtr createMemoryCacheBuffer() override;
		MemoryCacheInputInterfacePtr createMemoryCacheInput() override;
		MemoryProxyInputInterfacePtr createMemoryProxyInput() override;
		MemoryInputInterfacePtr createMemoryInput() override;

	protected:
		CacheBufferID lockBufferNoMutex_( size_t _size, void ** _memory, const char * _file, uint32_t _line );
		
	protected:
		struct CacheBufferMemory
		{
			CacheBufferID id;
			void * memory;
			size_t size;
			const char * file;
            uint32_t line;
			bool lock;
		};

		typedef stdex::vector<CacheBufferMemory> TVectorCacheBufferMemory;
		TVectorCacheBufferMemory m_buffers;

		CacheBufferID m_enumeratorId;
			
		ThreadMutexInterfacePtr m_memoryCacheMutex;

        FactoryWithMutexPtr m_factoryMemoryBuffer;
        FactoryWithMutexPtr m_factoryMemoryProxy;
        FactoryWithMutexPtr m_factoryMemoryCacheBuffer;
		FactoryWithMutexPtr m_factoryMemoryCacheInput;
		FactoryWithMutexPtr m_factoryMemoryProxyInput;
		FactoryWithMutexPtr m_factoryMemoryInput;
	};
}