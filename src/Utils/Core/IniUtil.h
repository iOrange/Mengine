#   pragma once

#   include "tinyini/tinyini.h"

#   include "Interface/ServiceInterface.h"
#   include "Interface/StreamInterface.h"

#	include "Core/Params.h"
#	include "Core/String.h"
#	include "Core/ConstString.h"
#	include "Core/ConstStringTypes.h"
#	include "Core/Tags.h"
#   include "Core/FilePath.h"
#	include "Core/Resolution.h"
#	include "Core/Viewport.h"
#	include "Core/AspectRatioViewports.h"
#	include "Core/ColourValue.h"

#	ifndef MENGINE_INI_BUFFER_SIZE 
#	define MENGINE_INI_BUFFER_SIZE 16384U
#	endif

namespace Menge
{
    namespace IniUtil
    {
		//////////////////////////////////////////////////////////////////////////
		struct IniStore
			: public tinyini::tinyini
		{			
			ConstString category;
			FilePath path;

			char buff[MENGINE_INI_BUFFER_SIZE];
		};
		//////////////////////////////////////////////////////////////////////////
		bool loadIni( IniStore & _ini, const ConstString & _category, const FilePath & _path, ServiceProviderInterface * _serviceProvider );
		bool loadIni( IniStore & _ini, const InputStreamInterfacePtr & _input, ServiceProviderInterface * _serviceProvider );
		//////////////////////////////////////////////////////////////////////////
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, WString & _value, ServiceProviderInterface * _serviceProvider ); 
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, String & _value, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, ConstString & _value, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, Tags & _value, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, Resolution & _value, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, ColourValue & _value, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, FilePath & _value, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, bool & _value, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, int32_t & _value, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, uint32_t & _value, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, uint64_t & _value, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, float & _value, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, TVectorString & _values, ServiceProviderInterface * _serviceProvider );
		bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, TVectorConstString & _values, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, TVectorWString & _values, ServiceProviderInterface * _serviceProvider );
        bool getIniValue( const IniStore & _ini, const Char * _section, const Char * _key, TVectorAspectRatioViewports & _values, ServiceProviderInterface * _serviceProvider );
		//////////////////////////////////////////////////////////////////////////
        bool getIniAllSettings( const IniStore & _ini, const Char * _section, TMapWParams & _values, ServiceProviderInterface * _serviceProvider );
		//////////////////////////////////////////////////////////////////////////
        bool writeIniSetting( ServiceProviderInterface * _serviceProvider, const OutputStreamInterfacePtr & _file, const char * _key, const WString & _value );
        bool writeIniSection( ServiceProviderInterface * _serviceProvider, const OutputStreamInterfacePtr & _file, const char * _section, uint32_t _sectionSize );
		//////////////////////////////////////////////////////////////////////////
        template<class T, size_t I_Size>
        static bool writeIniSection( ServiceProviderInterface * _serviceProvider, const OutputStreamInterfacePtr & _file, T (& _section) [I_Size] )
        {        
            (void)_serviceProvider;

            bool result = writeIniSection( _serviceProvider, _file, _section, sizeof(_section) - 1 );

            return result;
        }
    }
}