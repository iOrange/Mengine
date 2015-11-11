#	include "Config/Config.h"

#	include "MarmaladeApplication.h"

#	include "Menge/Application.h"

#	include "Interface/LoggerInterface.h"
#	include "Interface/FileSystemInterface.h"
#	include "Interface/UnicodeInterface.h"
#	include "Interface/InputSystemInterface.h"

#	include <cstdio>
#	include <clocale>

#	include "Factory/FactorableUnique.h"
#	include "Factory/FactoryDefault.h"

#	include "Core/FileLogger.h"
#	include "Core/IniUtil.h"

//#	include "resource.h"

#	include <ctime>
#	include <algorithm>

#	include <sstream>
#	include <iomanip>

//#	include <mhook.h>
#	include <s3eDevice.h>
#	include <s3eWindow.h>
#	include <s3e.h>

#ifdef _MSC_VER
#	define snprintf _snprintf
#endif

//////////////////////////////////////////////////////////////////////////
SERVICE_PROVIDER_EXTERN( ServiceProvider );
//////////////////////////////////////////////////////////////////////////
SERVICE_EXTERN( Platform );
SERVICE_EXTERN( Application );
SERVICE_EXTERN( StringizeService );
SERVICE_EXTERN( LoggerService );
SERVICE_EXTERN( MarmaladeLayer );

SERVICE_EXTERN( ArchiveService );

SERVICE_EXTERN( ThreadSystem );
SERVICE_EXTERN( ThreadService );

SERVICE_EXTERN( ParticleSystem );
SERVICE_EXTERN( ParticleService );

SERVICE_EXTERN( RenderSystem );
SERVICE_EXTERN( RenderSystemES1 );
SERVICE_EXTERN( RenderService );
SERVICE_EXTERN( RenderTextureService );
SERVICE_EXTERN( RenderMaterialService );

SERVICE_EXTERN( PhysicSystem );

SERVICE_EXTERN( UnicodeSystem );
SERVICE_EXTERN( UnicodeService );

SERVICE_EXTERN( FileService );

SERVICE_EXTERN( NotificationService );
SERVICE_EXTERN( ScriptService );

SERVICE_EXTERN( SoundSystem );
SERVICE_EXTERN( SilentSoundSystem );
SERVICE_EXTERN( SoundService );

SERVICE_EXTERN( InputService );
SERVICE_EXTERN( CodecService );
SERVICE_EXTERN( PluginSystem );
SERVICE_EXTERN( PluginService );

SERVICE_EXTERN( ModuleService );
SERVICE_EXTERN( DataService );
SERVICE_EXTERN( MemoryService );
SERVICE_EXTERN( ConfigService );
SERVICE_EXTERN( PrefetcherService );
//////////////////////////////////////////////////////////////////////////
PLUGIN_EXPORT( MengeImageCodec );
PLUGIN_EXPORT( MengeSoundCodec );
PLUGIN_EXPORT( MengeVideoCodec );
PLUGIN_EXPORT( MengeAmplifier );
PLUGIN_EXPORT( MengeZip );
PLUGIN_EXPORT( MengeLZ4 );
PLUGIN_EXPORT( MengeSpine );
PLUGIN_EXPORT( MengeOggVorbis );
PLUGIN_EXPORT( PathFinder );
PLUGIN_EXPORT( MarmaladeFileGroup );
//////////////////////////////////////////////////////////////////////////
namespace Menge
{
    //////////////////////////////////////////////////////////////////////////
    static const float s_activeFrameTime = 1000.f/60.f;
    static const float s_inactiveFrameTime = 100;
    //////////////////////////////////////////////////////////////////////////
    namespace Helper
    {
        //////////////////////////////////////////////////////////////////////////
        static void s_getOption( const Menge::String& _option
            , const Menge::String& _commandLine
            , Menge::String* _value )
        {
            if( _value == NULL )
            {
                return;
            }

            Menge::String::size_type option_index = 0;
            while( (option_index = _commandLine.find( _option, option_index )) != Menge::String::npos )
            {
                option_index += _option.length();
                if( option_index >= _commandLine.length() )
                {
                    break;
                }
                char next_delim = _commandLine[option_index] == '\"' ? '\"' : ' ';
                Menge::String::size_type next_index = _commandLine.find( next_delim, option_index+1 );
                if( next_delim == '\"' )
                {
                    ++option_index;
                }

                Menge::String::size_type value_length = next_index - option_index;
                (*_value) += _commandLine.substr( option_index, value_length );
                _value->push_back( ' ' );
            }

            if( _value->empty() == false )
            {
                _value->erase( _value->length() - 1 );
            }
        }
        //////////////////////////////////////////////////////////////////////////
        static bool s_hasOption( const char * _option, const Menge::String& _commandLine )
        {
            if( _commandLine.find( _option ) == Menge::String::npos )
            {
                return false;
            }

            return true;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    MarmaladeApplication::MarmaladeApplication()
        : m_serviceProvider(nullptr)
        , m_loggerConsole(nullptr)
        , m_fileLog(nullptr)
		, m_running(false)
		, m_active(false)
		, m_developmentMode(false)
    {
    }
    //////////////////////////////////////////////////////////////////////////
    MarmaladeApplication::~MarmaladeApplication()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeApplicationService_()
    {

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeNotificationService_()
    {
        LOGGER_INFO(m_serviceProvider)( "Inititalizing Notification Service..." );

        if( SERVICE_CREATE( m_serviceProvider, NotificationService ) == false )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeThreadEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Inititalizing Thread Service..." );

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeFileEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Inititalizing File Service..." );

		{
			LOGGER_INFO( m_serviceProvider )("Initialize Marmalade file group...");
			PLUGIN_CREATE( m_serviceProvider, MarmaladeFileGroup );
		}

        // mount root		
        ConstString c_dir = Helper::stringizeString(m_serviceProvider, "dir");
		if( FILE_SERVICE( m_serviceProvider )
			->mountFileGroup( ConstString::none(), m_currentPath, c_dir ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "WinApplication::setupFileService: failed to mount application directory %ls"
                , m_currentPath.c_str()
                );

            return false;
        }

        //m_tempPath.clear();
        m_userPath.clear();

        m_userPath += L"User";
        m_userPath += L"/";

        String utf8_userPath;
        if( Helper::unicodeToUtf8( m_serviceProvider, m_userPath, utf8_userPath ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "WinApplication: failed user directory %ls convert to ut8f"
                , m_userPath.c_str()
                );

            return false;
        }

        FilePath userPath = Helper::stringizeString( m_serviceProvider, utf8_userPath );

        // mount user directory
		if( FILE_SERVICE( m_serviceProvider )
			->mountFileGroup( Helper::stringizeString( m_serviceProvider, "user" ), userPath, Helper::stringizeString( m_serviceProvider, "dir" ) ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)( "WinApplication: failed to mount user directory %ls"
                , m_userPath.c_str()
                );

            return false;
        }

		if( FILE_SERVICE( m_serviceProvider )
			->existDirectory( Helper::stringizeString( m_serviceProvider, "user" ), ConstString::none() ) == false )
		{
			FILE_SERVICE( m_serviceProvider )
				->createDirectory( Helper::stringizeString( m_serviceProvider, "user" ), ConstString::none() );
		}

        FilePath logFilename = Helper::stringizeString( m_serviceProvider, "log.log" );

        OutputStreamInterfacePtr fileLogInterface = FILE_SERVICE( m_serviceProvider )
			->openOutputFile( Helper::stringizeString( m_serviceProvider, "user" ), logFilename );

        if( fileLogInterface != nullptr )
        {
            m_fileLog = new FileLogger();

			LOGGER_SERVICE( m_serviceProvider )
				->registerLogger( m_fileLog );
        }

#	ifndef MENGINE_MASTER_RELEASE
		ConstString c_dev = Helper::stringizeString( m_serviceProvider, "dev" );
		// mount root		
		if( FILE_SERVICE(m_serviceProvider)
			->mountFileGroup( c_dev, ConstString::none(), c_dir ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)( "WinApplication::setupFileService: failed to mount dev directory %ls"
				, m_currentPath.c_str()
				);

			return false;
		}
#   endif

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::getApplicationPath_( const char * _section, const char * _key, ConstString & _path )
	{
		FilePath applicationPath = STRINGIZE_STRING_LOCAL( m_serviceProvider, "application.ini" );

		InputStreamInterfacePtr applicationInputStream = 
			FILE_SERVICE(m_serviceProvider)->openInputFile( ConstString::none(), applicationPath, false );

		if( applicationInputStream == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ Invalid open application settings %s"
				, applicationPath.c_str()
				);

			return false;
		}

		IniUtil::IniStore ini;
		if( IniUtil::loadIni( ini, applicationInputStream, m_serviceProvider ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ Invalid load application settings %s"
				, applicationPath.c_str()
				);

			return false;
		}

		const char * gameIniPath = ini.getSettingValue( _section, _key );

		if( gameIniPath == nullptr )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ Not found Game Path %s"
				, applicationPath.c_str()
				);

			return false;
		}

		_path = Helper::stringizeString( m_serviceProvider, gameIniPath );

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializeConfigEngine_()
	{
		LOGGER_WARNING(m_serviceProvider)("Inititalizing Config Manager..." );

		FilePath gameIniPath;
		if( this->getApplicationPath_( "Game", "Path", gameIniPath ) == false )
		{
			return false;
		}

		if( CONFIG_SERVICE(m_serviceProvider)
			->loadConfig( ConstString::none(), gameIniPath ) == false )
		{
			LOGGER_ERROR(m_serviceProvider)("WinApplication::initializeConfigEngine_ invalid load config %s"				
				, gameIniPath.c_str()
				);

			return false;
		}

		return true;
	}
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeStringizeService_()
    {
        if( SERVICE_CREATE( m_serviceProvider, StringizeService ) == false )
        {
            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeLoggerEngine_()
    {
        if( SERVICE_CREATE( m_serviceProvider, LoggerService ) == false )
        {
            return false;
        }

        m_loggerConsole = new MarmaladeLogger();

        LOGGER_SERVICE(m_serviceProvider)
			->registerLogger( m_loggerConsole );

        EMessageLevel m_logLevel;

        m_logLevel = LM_LOG;

        if( HAS_OPTIONS(m_serviceProvider, "log:0" ) == true )
        {
            m_logLevel = LM_INFO;
        }
		else if( HAS_OPTIONS( m_serviceProvider, "log:1" ) == true )
        {
            m_logLevel = LM_LOG;
        }
		else if( HAS_OPTIONS( m_serviceProvider, "log:2" ) == true )
        {
            m_logLevel = LM_WARNING;
        }
		else if( HAS_OPTIONS( m_serviceProvider, "log:3" ) == true )
        {
            m_logLevel = LM_ERROR;
        }

        LOGGER_SERVICE(m_serviceProvider)
			->setVerboseLevel( m_logLevel );

		if( HAS_OPTIONS( m_serviceProvider, "verbose" ) == true )
        {
            LOGGER_SERVICE(m_serviceProvider)
				->setVerboseLevel( LM_MAX );

            LOGGER_INFO(m_serviceProvider)( "Verbose logging mode enabled" );
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeArchiveService_()
    {
        LOGGER_INFO(m_serviceProvider)( "Inititalizing Archive Service..." );

		{
			LOGGER_INFO(m_serviceProvider)( "initialize Zip..." );
			
			PLUGIN_CREATE( m_serviceProvider, MengeZip );
		}

		{
			LOGGER_INFO(m_serviceProvider)( "initialize LZ4..." );

			PLUGIN_CREATE( m_serviceProvider, MengeLZ4 );
		}

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeUnicodeEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Unicode Service..." );

        if( SERVICE_CREATE( m_serviceProvider, UnicodeSystem ) == false )
        {
            return false;
        }

        if( SERVICE_CREATE( m_serviceProvider, UnicodeService ) == false )
        {
            return false;
        }

        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializeParticleEngine2_()
	{
		LOGGER_INFO( m_serviceProvider )("Initializing Particle Service 2...");



		return true;
	}
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeRenderEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Render Service..." );
		
		uint32_t config_gl_version = 2;

		char config_SysGlesVersion[S3E_CONFIG_STRING_MAX] = {0};
		if( s3eConfigGetString( "S3E", "SysGlesVersion", config_SysGlesVersion ) == S3E_RESULT_SUCCESS )
		{
			if( strcmp( config_SysGlesVersion, "1" ) == 0 )
			{
				config_gl_version = 1;
			}
			else if( strcmp( config_SysGlesVersion, "2" ) == 0 )
			{
				config_gl_version = 2;
			}
			else
			{
				LOGGER_ERROR( m_serviceProvider )("MarmaladeApplication::initializeRenderEngine_ not support OpenGL ES version '%s'"
					, config_SysGlesVersion
					);

				return false;
			}
		}

		RenderSystemInterface * renderSystem = nullptr;

		for( uint32_t i = config_gl_version; i != 0; --i )
		{
			if( i == 2 )
			{
				if( SERVICE_CREATE( m_serviceProvider, RenderSystem ) == false )
				{
					continue;
				}

				break;
			}
			else if( i == 1 )
			{
				if( SERVICE_CREATE( m_serviceProvider, RenderSystemES1 ) == false )
				{
					continue;
				}

				break;
			}
		}

		if( SERVICE_EXIST( m_serviceProvider, Menge::RenderSystemInterface ) == false )
		{
			return false;
		}

        if( SERVICE_CREATE( m_serviceProvider, RenderService ) == false )
        {
            return false;
        }

		if( SERVICE_CREATE( m_serviceProvider, RenderTextureService ) == false )
		{
			return false;
		}

		if( SERVICE_CREATE( m_serviceProvider, RenderMaterialService ) == false )
		{
			return false;
		}

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeSoundEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Sound Service..." );

		SERVICE_CREATE( m_serviceProvider, SoundSystem );

		bool muteMode = HAS_OPTIONS( m_serviceProvider, "mute" );

		if( muteMode == true || SERVICE_EXIST( m_serviceProvider, Menge::SoundSystemInterface ) == false )
		{
			SERVICE_CREATE( m_serviceProvider, SilentSoundSystem );
		}

        if( SERVICE_CREATE( m_serviceProvider, SoundService ) == false )
        {
            LOGGER_ERROR(m_serviceProvider)("MarmaladeApplication::initializeSoundEngine_ Failed to create Sound Engine"
                );

            return false;
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeScriptEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Script Service..." );



        return true;
    }
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializeModuleEngine_()
	{
		LOGGER_INFO(m_serviceProvider)( "Initializing Module Service..." );



		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializeDataManager_()
	{
		LOGGER_INFO(m_serviceProvider)( "Inititalizing Data Manager..." );

		if( SERVICE_CREATE( m_serviceProvider, DataService ) == false )
		{
			return false;
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializeMemoryManager_()
	{
		LOGGER_INFO(m_serviceProvider)( "Inititalizing Memory Manager..." );


		return true;
	}	
	//////////////////////////////////////////////////////////////////////////
	bool MarmaladeApplication::initializePrefetcherService_()
	{
		LOGGER_INFO(m_serviceProvider)( "Inititalizing Prefetcher Service..." );
				
				
		return true;
	}
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeCodecEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Codec Service..." );

		
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeInputEngine_()
    {
        LOGGER_INFO(m_serviceProvider)( "Initializing Input Service..." );


        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializePluginService_()
    {
        //LOGGER_INFO(m_serviceProvider)( "Initializing Plugin Service..." );

        //PluginServiceInterface * pluginService;
        //if( createPluginService( &pluginService ) == false )
        //{
        //    return false;
        //}

        //if( SERVICE_REGISTRY(m_serviceProvider, pluginService) == false )
        //{
        //    return false;
        //}

        //m_pluginService = pluginService;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initializeNodeManager_()
    {
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    bool MarmaladeApplication::initialize()
    {
        setlocale( LC_CTYPE, "" );

        ServiceProviderInterface * serviceProvider;
        if( SERVICE_PROVIDER_CREATE( ServiceProvider, &serviceProvider ) == false )
        {
            return false;
        }

		SERVICE_CREATE( m_serviceProvider, StringizeService );
		SERVICE_CREATE( m_serviceProvider, Platform );
		

		SERVICE_CREATE( m_serviceProvider, LoggerService );

        if( this->initializeLoggerEngine_() == false )
        {
            return false;
        }

		SERVICE_CREATE( m_serviceProvider, NotificationService );
		SERVICE_CREATE( m_serviceProvider, UnicodeSystem );		
		SERVICE_CREATE( m_serviceProvider, UnicodeService );

		SERVICE_CREATE( m_serviceProvider, FileService );

        if( this->initializeFileEngine_() == false )
        {
            return false;
        }

		SERVICE_CREATE( m_serviceProvider, ConfigService );

		if( this->initializeConfigEngine_() == false )
		{
			return false;
		}

		SERVICE_CREATE( m_serviceProvider, ArchiveService );

		if( this->initializeArchiveService_() == false )
		{
			return false;
		}

		SERVICE_CREATE( m_serviceProvider, ThreadSystem );
		SERVICE_CREATE( m_serviceProvider, ThreadService );
		
		SERVICE_CREATE( m_serviceProvider, ParticleSystem );
		SERVICE_CREATE( m_serviceProvider, ParticleService );

        if( this->initializeRenderEngine_() == false )
        {
            return false;
        }
                
        if( this->initializeSoundEngine_() == false )
        {
			return false;            
        }
        
		SERVICE_CREATE( m_serviceProvider, ScriptService );

		SERVICE_CREATE( m_serviceProvider, ModuleService );

		SERVICE_CREATE( m_serviceProvider, DataService );

		SERVICE_CREATE( m_serviceProvider, MemoryService );

		SERVICE_CREATE( m_serviceProvider, CodecService );
		
		SERVICE_CREATE( m_serviceProvider, InputService );
		
		SERVICE_CREATE( m_serviceProvider, Application );

		SERVICE_CREATE( m_serviceProvider, PrefetcherService );

		SERVICE_CREATE( m_serviceProvider, PluginSystem );
		SERVICE_CREATE( m_serviceProvider, PluginService );


		PLUGIN_CREATE( m_serviceProvider, MengeImageCodec );
		PLUGIN_CREATE( m_serviceProvider, MengeSoundCodec );
		PLUGIN_CREATE( m_serviceProvider, MengeOggVorbis );
		PLUGIN_CREATE( m_serviceProvider, MengeVideoCodec );
		PLUGIN_CREATE( m_serviceProvider, MengeAmplifier );
		PLUGIN_CREATE( m_serviceProvider, PathFinder );
		PLUGIN_CREATE( m_serviceProvider, MengeSpine );

		TVectorString modules;
		CONFIG_VALUES(m_serviceProvider, "Modules", "Name", modules);

		for( TVectorString::const_iterator
			it = modules.begin(),
			it_end = modules.end();
		it != it_end;
		++it )
		{
			const String & moduleName = *it;

			if( MODULE_SERVICE(m_serviceProvider)
				->runModule( Helper::stringizeString(m_serviceProvider, moduleName) ) == false )
			{
				LOGGER_ERROR(m_serviceProvider)("Application Failed to run module %s"
					, moduleName.c_str()
					);

				return false;
			}
		}

		ConstString renderMaterialsPath = CONFIG_VALUE( m_serviceProvider, "Engine", "RenderMaterials", ConstString::none() );

		if( renderMaterialsPath.empty() == false )
		{
			if( RENDERMATERIAL_SERVICE( m_serviceProvider )
				->loadMaterials( ConstString::none(), renderMaterialsPath ) == false )
			{
				return false;
			}
		}

		LOGGER_INFO( m_serviceProvider )("Application Create..."
			);

		ConstString resourceIniPath;
		if( this->getApplicationPath_( "Resource", "Path", resourceIniPath ) == false )
		{
			LOGGER_CRITICAL( m_serviceProvider )("Application invalid setup resource path"
				);

			return false;
		}

		if( APPLICATION_SERVICE( m_serviceProvider )
			->initializeGame( ConstString::none(), resourceIniPath ) == false )
		{
			LOGGER_CRITICAL( m_serviceProvider )("Application invalid initialize game"
				);

			return false;
		}

        LOGGER_INFO(m_serviceProvider)( "Creating Render Window..." );

		if( APPLICATION_SERVICE( m_serviceProvider )
			->createRenderWindow() == false )
        {
            return false;
        }

		APPLICATION_SERVICE( m_serviceProvider )
			->turnSound( true );
		
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::loop()
    {
		PLATFORM_SERVICE( m_serviceProvider )
			->update();
    }
    //////////////////////////////////////////////////////////////////////////
    void MarmaladeApplication::finalize()
    {
		SERVICE_FINALIZE( m_serviceProvider, Menge::ApplicationInterface );

		SERVICE_FINALIZE( m_serviceProvider, Menge::DataServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::ModuleServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::InputServiceInterface );

		SERVICE_FINALIZE( m_serviceProvider, Menge::UnicodeServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::UnicodeSystemInterface );

		SERVICE_FINALIZE( m_serviceProvider, Menge::FileServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::CodecServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::ParticleServiceInterface2 );

		SERVICE_FINALIZE( m_serviceProvider, Menge::SoundServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::SoundSystemInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::ScriptServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::RenderServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::RenderMaterialServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::RenderTextureServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::RenderSystemInterface );

		SERVICE_FINALIZE( m_serviceProvider, Menge::PrefetcherServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::ThreadServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::ThreadSystemInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::ScriptServiceInterface );

		SERVICE_FINALIZE( m_serviceProvider, Menge::ConfigServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::StringizeServiceInterface );
		
		if( m_fileLog != nullptr )
		{
			LOGGER_SERVICE( m_serviceProvider )
				->unregisterLogger( m_fileLog );

			delete m_fileLog;
			m_fileLog = nullptr;
		}

		SERVICE_FINALIZE( m_serviceProvider, Menge::ArchiveServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::MemoryServiceInterface );

		if( m_loggerConsole != nullptr )
		{
			LOGGER_SERVICE( m_serviceProvider )
				->unregisterLogger( m_loggerConsole );

			delete m_loggerConsole;
			m_loggerConsole = nullptr;
		}

		SERVICE_FINALIZE( m_serviceProvider, Menge::NotificationServiceInterface );
		SERVICE_FINALIZE( m_serviceProvider, Menge::LoggerServiceInterface );

		SERVICE_PROVIDER_FINALIZE( m_serviceProvider );
    }
}
