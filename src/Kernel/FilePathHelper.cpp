#include "FilePathHelper.h"

#include "Interface/StringizeServiceInterface.h"
#include "Interface/UnicodeSystemInterface.h"
#include "Interface/FileServiceInterface.h"

#include "Kernel/Logger.h"

namespace Mengine
{
    namespace Helper
    {
        //////////////////////////////////////////////////////////////////////////
        FilePath emptyPath()
        {
            return FilePath( ConstString::none() );
        }
        //////////////////////////////////////////////////////////////////////////
        FilePath concatenationFilePath( const FilePath & _left, const FilePath & _right )
        {
            PathString path;

            path += _left;
            path += _right;

            ConstString c_path = Helper::stringizeStringSize( path.c_str(), path.size() );

            return FilePath( c_path );
        }
        //////////////////////////////////////////////////////////////////////////
        bool makeFullPath( const ConstString & _fileGroupName, const FilePath & _fileName, FilePath & _fullPath )
        {
            FileGroupInterfacePtr group;

            if( FILE_SERVICE()
                ->hasFileGroup( _fileGroupName, &group ) == false )
            {
                LOGGER_ERROR( "makeFullPath: not found file group '%s'"
                    , _fileGroupName.c_str()
                );

                return false;
            }

            const FilePath & groupPath = group->getFolderPath();

            FilePath fullPath = Helper::concatenationFilePath( groupPath, _fileName );

            _fullPath = fullPath;

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        FilePath getPathFolder( const FilePath & _fullpath )
        {
            const Char * str_fullpath = _fullpath.c_str();

            const Char * folder_delimiter_1 = strrchr( str_fullpath, '\\' );
            const Char * folder_delimiter_2 = strrchr( str_fullpath, '/' );

            const Char * folder_delimiter = (folder_delimiter_1 > folder_delimiter_2) ? folder_delimiter_1 : folder_delimiter_2;

            if( folder_delimiter == nullptr )
            {
                return FilePath( ConstString::none() );
            }

            FilePath c_folder = Helper::stringizeFilePathSize( str_fullpath, folder_delimiter - str_fullpath + 1 );

            return c_folder;
        }
        //////////////////////////////////////////////////////////////////////////
        FilePath unicodeToFilePath( const WString & _unicode )
        {
            String utf8;
            if( Helper::unicodeToUtf8Size( _unicode.c_str(), _unicode.size(), utf8 ) == false )
            {
                return FilePath( ConstString::none() );
            }

            FilePath fp = Helper::stringizeFilePath( utf8 );

            return fp;
        }
        //////////////////////////////////////////////////////////////////////////
        void pathCorrectBackslash( WChar * _path )
        {
            WChar * pch = wcschr( _path, L'\\' );
            while( pch != NULL )
            {
                *pch = L'/';

                pch = wcschr( pch, L'\\' );
            }
        }
        //////////////////////////////////////////////////////////////////////////
        void pathCorrectBackslashTo( WChar * _out, const WChar * _in )
        {
            wcscpy( _out, _in );

            pathCorrectBackslash( _out );
        }
        //////////////////////////////////////////////////////////////////////////
        void pathCorrectForwardslash( WChar * _path )
        {
            WChar * pch = wcschr( _path, L'/' );
            while( pch != NULL )
            {
                *pch = L'\\';

                pch = wcschr( pch, L'/' );
            }
        }
        //////////////////////////////////////////////////////////////////////////
        void pathRemoveBackslash( WChar * _path )
        {
            size_t len = wcslen( _path );

            WChar * pch = _path + len - 1;

            if( *pch == L'/' )
            {
                *pch = L'\0';
            }
        }
        //////////////////////////////////////////////////////////////////////////
        bool pathRemoveFileSpec( WChar * _path )
        {
            size_t len = wcslen( _path );

            if( len == 0 )
            {
                return true;
            }

            WChar * pch = _path + len - 1;

            if( *pch == L'/' )
            {
                return false;
            }

            pch--;

            while( *pch != L'/' )
            {
                pch--;
            }

            pch++;

            *pch = L'\0';

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        void pathCorrectBackslashA( Char * _path )
        {
            Char * pch = strchr( _path, '\\' );
            while( pch != NULL )
            {
                *pch = '/';

                pch = strchr( pch, '\\' );
            }
        }
        //////////////////////////////////////////////////////////////////////////
        void pathCorrectBackslashToA( Char * _out, const Char * _in )
        {
            strcpy( _out, _in );

            pathCorrectBackslashA( _out );
        }
        //////////////////////////////////////////////////////////////////////////
        void pathRemoveBackslashA( Char * _path )
        {
            size_t len = strlen( _path );

            Char * pch = _path + len - 1;

            if( *pch == '/' )
            {
                *pch = '\0';
            }
        }
        //////////////////////////////////////////////////////////////////////////
        bool pathRemoveFileSpecA( Char * _path )
        {
            size_t len = strlen( _path );

            if( len == 0 )
            {
                return true;
            }

            Char * pch = _path + len - 1;

            if( *pch == '/' )
            {
                return false;
            }

            pch--;

            while( *pch != '/' )
            {
                pch--;
            }

            pch++;

            *pch = '\0';

            return true;
        }

    }
}