#include "DataService.h"

#include "Interface/StringizeServiceInterface.h"
#include "Interface/VocabularyServiceInterface.h"

#include "Kernel/Logger.h"
#include "Kernel/AssertionVocabulary.h"
#include "Kernel/AssertionMemoryPanic.h"

//////////////////////////////////////////////////////////////////////////
SERVICE_FACTORY( DataService, Mengine::DataService );
//////////////////////////////////////////////////////////////////////////
namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    DataService::DataService()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    DataService::~DataService()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool DataService::_initializeService()
    {
        //Empty

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void DataService::_finalizeService()
    {
        MENGINE_ASSERTION_VOCABULARY_EMPTY( STRINGIZE_STRING_LOCAL( "Dataflow" ) );
    }
    //////////////////////////////////////////////////////////////////////////
    DataInterfacePtr DataService::dataflow( const DataflowInterfacePtr & _dataflow, const InputStreamInterfacePtr & _stream, const Char * _doc )
    {
		DataInterfacePtr data = _dataflow->create( _doc );

		MENGINE_ASSERTION_MEMORY_PANIC( data, nullptr )("invalid create data '%s'"
			, _doc);

        if( _dataflow->load( data, _stream, _doc ) == false )
        {
            LOGGER_ERROR( "invalid load data"
            );

            return nullptr;
        }

        return data;
    }
}