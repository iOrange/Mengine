#pragma once

#include "Config/Char.h"
#include "Kernel/Typename.h"

#include "stdex/allocator.h"

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    template<class T>
    class MemoryAllocator
    {
    public:
        inline void * operator new (size_t _size)
        {
            return stdex_malloc( _size, Typename<T>::value );
        }

        inline void operator delete (void * _ptr, size_t _size)
        {
            (void)_size;

            stdex_free( _ptr, Typename<T>::value );
        }

        inline void * operator new []( size_t _size )
        {
            return stdex_malloc( _size, Typename<T>::value );
        }

            inline void operator delete []( void * _ptr, size_t _size )
        {
            (void)_size;

            stdex_free( _ptr, Typename<T>::value );
        }
    };
    //////////////////////////////////////////////////////////////////////////
    namespace Helper
    {
        template<class T>
        T * allocateT()
        {
            size_t memory_size = sizeof( T );
            void * memory_buffer = stdex_malloc( memory_size, Typename<T>::value );

            new (memory_buffer)T();

            return reinterpret_cast<T *>(memory_buffer);
        }

        template<class T>
        void freeT( T * _t )
        {
            _t->~T();

            stdex_free( _t, Typename<T>::value );
        }

        template<class T>
        T * allocateArrayT( uint32_t _count )
        {
            size_t memory_size = sizeof( T ) * _count;
            void * memory_buffer = stdex_malloc( memory_size, Typename<T>::value );

            return reinterpret_cast<T *>(memory_buffer);
        }

        template<class T>
        T * reallocateArrayT( T * _buffer, uint32_t _count )
        {
            size_t memory_size = sizeof( T ) * _count;
            void * memory_buffer = stdex_realloc( _buffer, memory_size, Typename<T>::value );

            return reinterpret_cast<T *>(memory_buffer);
        }

        template<class T>
        inline void freeArrayT( T * _memory )
        {
            stdex_free( _memory, Typename<T>::value );
        }

        inline void * allocateMemory( size_t _size, const Char * _doc )
        {
            void * memory_buffer = stdex_malloc( _size, _doc );

            return memory_buffer;
        }

        inline 	void * reallocateMemory( void * _buffer, size_t _size, const Char * _doc )
        {
            (void)_doc;
            void * memory_buffer = stdex_realloc( _buffer, _size, _doc );

            return memory_buffer;
        }

        inline void freeMemory( void * _memory, const Char * _doc )
        {
            stdex_free( _memory, _doc );
        }
    }
}