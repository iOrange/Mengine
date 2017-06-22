#	include "String.h"

#	include "Config/Typedef.h"
#   include "Config/Stringstream.h"


#	include <sstream>
#	include <clocale>

namespace Menge
{
	namespace Helper
	{
		//////////////////////////////////////////////////////////////////////////
		const ConstString& emptyConstString()
		{
			return ConstString::none();
		}
		//////////////////////////////////////////////////////////////////////////
		void split( TVectorString & _outStrings, const String& _str, bool _trimDelims, const String& _delim )
		{
			uint32_t numSplits = 0;
			String::size_type start = 0;
			String::size_type pos = 0;

			do
			{
				pos = _str.find_first_of( _delim, start );

				if( pos == String::npos )
				{
					_outStrings.push_back( _str.substr( start ) );
					break;
				}
				else
				{
					_outStrings.push_back( _str.substr( start, pos - start ) );
					start = pos + 1;
				}

				if( _trimDelims == true )
				{
					start = _str.find_first_not_of( _delim, start );
				}

				++numSplits;

			} while( pos != String::npos );
		}
		//////////////////////////////////////////////////////////////////////////
		template<class T, class C = stdex::vector<T> >
		static void t_split2( C & _outStrings, const T & _str, bool _trimDelims, const C & _delims )
		{
			typename T::size_type start = 0;
			typename T::size_type pos = 0;

			do
			{
				pos = T::npos;

				for( typename C::const_iterator
					it = _delims.begin(),
					it_end = _delims.end();
					it != it_end;
					++it )
				{
					const T & delim = *it;

					typename T::size_type pos_delim = _str.find_first_of( delim, start );

					if( pos > pos_delim )
					{
						pos = pos_delim;
					}
				}

				if( pos == T::npos )
				{
					_outStrings.push_back( _str.substr( start ) );
					break;
				}
				else
				{
					_outStrings.push_back( _str.substr( start, pos - start ) );
					start = pos + 1;
				}

				if( _trimDelims == true )
				{
					typename T::size_type pos_n = T::npos;

					for( typename C::const_iterator
						it = _delims.begin(),
						it_end = _delims.end();
						it != it_end;
						++it )
					{
						const T & delim = *it;

						typename T::size_type pos_delim = _str.find_first_not_of( delim, start );

						if( pos_n > pos_delim )
						{
							pos_n = pos_delim;
						}
					}

					start = pos_n;
				}
			} while( pos != T::npos );
		}
		//////////////////////////////////////////////////////////////////////////
		void split2( TVectorString & _outStrings, const String& _str, bool _trimDelims, const TVectorString & _delims )
		{
			t_split2( _outStrings, _str, _trimDelims, _delims );
		}
		//////////////////////////////////////////////////////////////////////////
		void u32split2( TVectorU32String & _outStrings, const U32String& _str, bool _trimDelims, const TVectorU32String & _delims )
		{
			t_split2( _outStrings, _str, _trimDelims, _delims );
		}
		//////////////////////////////////////////////////////////////////////////
		void wsplit( TVectorWString & _outStrings, const WString& _str, bool _trimDelims, const WString& _delims )
		{
			uint32_t numSplits = 0;
			WString::size_type start = 0;
			WString::size_type pos = 0;

			do 
			{
				pos = _str.find_first_of(_delims, start);
			
				if (pos == WString::npos )
				{
					_outStrings.push_back( _str.substr(start) );
					break;
				}
				else
				{
					_outStrings.push_back( _str.substr(start, pos - start) );
					start = pos + 1;
				}

				if( _trimDelims == true )
				{
					start = _str.find_first_not_of(_delims, start);
				}

				++numSplits;

			} while (pos != WString::npos);
		}
		//////////////////////////////////////////////////////////////////////////
		void join( const String& _delim, const TVectorString& _stringArray, String & _outString )
		{
			if( _stringArray.empty() == true )
			{
				_outString.clear();
				return;
			}

			TVectorString::const_iterator it = _stringArray.begin();

			_outString = *it;

			++it;

			for( TVectorString::const_iterator it_end = _stringArray.end();
				it != it_end;
				++it )
			{
				_outString += _delim;
				_outString += (*it);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		void trim( String& str, bool left/* = true*/, bool right/* = true */)
		{
			static const String delims = " \t\r";

			if( right )
			{
				str.erase( str.find_last_not_of( delims ) + 1 ); // trim right
			}

			if( left )
			{
				str.erase(0, str.find_first_not_of( delims ) ); // trim left
			}
		}
		//////////////////////////////////////////////////////////////////////////
		bool intToString( int32_t _value, String & _str )
		{
			Stringstream ss;
			ss << _value;

			_str = ss.str();

            return true;
		}
		//////////////////////////////////////////////////////////////////////////
		bool unsignedToString( uint32_t _value, String & _str )
		{
			Stringstream ss;
			ss << _value;

			_str = ss.str();

            return true;
		}
        //////////////////////////////////////////////////////////////////////////
        bool floatToString( float _value, String & _str )
        {
            Stringstream ss;
            ss << _value;

            _str = ss.str();

            return true;
        }
		//////////////////////////////////////////////////////////////////////////
		bool intToWString( int32_t _value, WString & _str )
		{
			WStringstream ss;
			ss << _value;

            _str = ss.str();

			return true;
		}
		//////////////////////////////////////////////////////////////////////////
		bool unsignedToWString( uint32_t _value, WString & _str )
		{
			WStringstream ss;
			ss << _value;

            _str = ss.str();

            return true;
		}
		//////////////////////////////////////////////////////////////////////////
		bool unsigned64ToWString( uint64_t _value, WString & _str )
		{
			WStringstream ss;
			ss << _value;

			_str = ss.str();

			return true;
		}
        //////////////////////////////////////////////////////////////////////////
        bool floatToWString( float _value, WString & _str )
        {
            WStringstream ss;
            ss << _value;

            _str = ss.str();

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool stringToInt( const String & _str, int32_t & _value )
        {
            Stringstream ss;
            ss << _str;

            ss >> _value;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool stringToUnsigned( const String & _str, uint32_t & _value )
        {
            Stringstream ss;
            ss << _str;
            ss >> _value;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool stringToFloat( const String & _str, float & _value )
        {
            Stringstream ss;
            ss << _str;
            ss >> _value;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool wstringToInt( const WString & _str, int32_t & _value )
        {
            WStringstream ss;
            ss << _str;
            ss >> _value;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool wstringToInt2( const WString & _str, int32_t & _value1, int32_t & _value2, bool _separator )
        {
            WStringstream ss;
            ss << _str;
            ss >> _value1;

            if( ss.fail() == true )
            {
                return false;
            }

            if( _separator == true )
            {
                WChar sep;
                ss >> sep;

                if( ss.fail() == true )
                {
                    return false;
                }
            }

            ss >> _value2;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool wstringToInt4( const WString & _str, int32_t & _value1, int32_t & _value2, int32_t & _value3, int32_t & _value4, bool _separator )
        {
            WStringstream ss;
            ss << _str;
            ss >> _value1;

            if( ss.fail() == true )
            {
                return false;
            }

            if( _separator == true )
            {
                WChar sep;
                ss >> sep;

                if( ss.fail() == true )
                {
                    return false;
                }
            }

            ss >> _value2;

            if( ss.fail() == true )
            {
                return false;
            }

            if( _separator == true )
            {
                WChar sep;

                ss >> sep;

                if( ss.fail() == true )
                {
                    return false;
                }
            }

            ss >> _value3;

            if( ss.fail() == true )
            {
                return false;
            }

            if( _separator == true )
            {
                WChar sep;
                ss >> sep;

                if( ss.fail() == true )
                {
                    return false;
                }
            }

            ss >> _value4;

            if( ss.fail() == true )
            {
                return false;
            }

            if( _separator == true )
            {
                WChar sep;
                ss >> sep;

                if( ss.fail() == true )
                {
                    return false;
                }
            }

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        bool wstringToUnsigned( const WString & _str, uint32_t & _value )
        {
            WStringstream ss;
            ss << _str;
            ss >> _value;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
		//////////////////////////////////////////////////////////////////////////
		bool wstringToUnsigned64( const WString & _str, uint64_t & _value )
		{
			WStringstream ss;
			ss << _str;
			ss >> _value;

			if( ss.fail() == true )
			{
				return false;
			}

			return true;
		}
        //////////////////////////////////////////////////////////////////////////
        bool wstringToFloat( const WString & _str, float & _value )
        {
            WStringstream ss;
            ss << _str;
            ss >> _value;

            if( ss.fail() == true )
            {
                return false;
            }

            return true;
        }
	}
}