#include "HelperScriptEmbedding.h"

#include "Interface/UnicodeSystemInterface.h"
#include "Interface/ResourceServiceInterface.h"
#include "Interface/RenderSystemInterface.h"
#include "Interface/StringizeServiceInterface.h"
#include "Interface/TextServiceInterface.h"
#include "Interface/TimeServiceInterface.h"
#include "Interface/TimeSystemInterface.h"
#include "Interface/ApplicationInterface.h"
#include "Interface/UserdataServiceInterface.h"
#include "Interface/ConfigServiceInterface.h"
#include "Interface/FileServiceInterface.h"
#include "Interface/AccountServiceInterface.h"
#include "Interface/WatchdogInterface.h"
#include "Interface/InputServiceInterface.h"
#include "Interface/EnumeratorServiceInterface.h"
#include "Interface/ChronometerServiceInterface.h"

#include "Config/Typedef.h"
#include "Config/Stringstream.h"

#include "Kernel/Node.h"
#include "Kernel/NodeRenderHelper.h"
#include "Kernel/Assertion.h"

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/rand.h"

#include "Kernel/Polygon.h"
#include "Kernel/PolygonHelper.h"
#include "Kernel/Color.h"
#include "Kernel/MemoryHelper.h"
#include "Kernel/UID.h"

#include "Engine/HotSpotShape.h"

#include "Kernel/Logger.h"

#include "Engine/ResourceImageSequence.h"

#include <ctime>
#include <iomanip>

#include "Kernel/ResourceImage.h"
#include "Engine/ResourceCursorICO.h"

#include "Kernel/Affector.h"

#include "Kernel/String.h"
#include "Kernel/CRC32.h"

#include "math/angle.h"

#include "Interface/ScriptServiceInterface.h"

#include "Config/Blobject.h"

#include "pybind/pybind.hpp"
#include "pybind/pickle.hpp"
#include "pybind/stl/stl_type_cast.hpp"

#include <math.h>

namespace Mengine
{
    //////////////////////////////////////////////////////////////////////////
    typedef Vector<ResourceImage *> VectorResourceImage;
    typedef Vector<HotSpotPolygon *> VectorHotSpotPolygons;
    //////////////////////////////////////////////////////////////////////////
    class HelperScriptMethod
        : public Factorable
    {
    public:
        HelperScriptMethod()
        {
        }

        ~HelperScriptMethod() override
        {
        }

    public:
        uint32_t s_refcount( pybind::kernel_interface * _kernel, PyObject * _obj )
        {
            return _kernel->refcount( _obj );
        }

        bool s_is_class( pybind::kernel_interface * _kernel, PyObject * _obj )
        {
            return _kernel->is_class( _obj );
        }

        bool s_is_type_class( pybind::kernel_interface * _kernel, PyTypeObject * _type )
        {
            return _kernel->is_type_class( _type );
        }

        bool s_is_wrap( pybind::kernel_interface * _kernel, PyObject * _obj )
        {
            return _kernel->is_wrap( _obj );
        }

        const ConstString & s_getLanguagePack()
        {
            const ConstString & locale = APPLICATION_SERVICE()
                ->getLocale();

            return locale;
        }

        float s_isometric_length_v3_v3( const mt::vec3f & _v0, const mt::vec3f & _v1 )
        {
            mt::vec3f iso_v0 = _v0;
            iso_v0.y *= 2.f;

            mt::vec3f iso_v1 = _v1;
            iso_v1.y *= 2.f;

            float iso_length = mt::length_v3_v3( iso_v0, iso_v1 );

            return iso_length;
        }

        float s_isometric_sqrlength_v3_v3( const mt::vec3f & _v0, const mt::vec3f & _v1 )
        {
            mt::vec3f iso_v0 = _v0;
            iso_v0.y *= 2.f;

            mt::vec3f iso_v1 = _v1;
            iso_v1.y *= 2.f;

            float iso_length = mt::sqrlength_v3_v3( iso_v0, iso_v1 );

            return iso_length;
        }

        WString s_utf8ToUnicode( const String & _utf8 )
        {
            WString unicode;
            Helper::utf8ToUnicode( _utf8, unicode );

            return unicode;
        }

        String s_unicodeToUtf8( const WString & _unicode )
        {
            String utf8;
            Helper::unicodeToUtf8( _unicode, utf8 );

            return utf8;
        }

        void s_debug()
        {
            printf( "debug!\n" );

#if defined(_MSC_VER) && !defined(I3D_ARCH_ARM) && !defined(_WIN64)
            _asm int 3;
#endif			
        }

        void s_debugNode( Node * _node )
        {
            (void)_node;

            printf( "debug node!\n" );

#if defined(_MSC_VER) && !defined(I3D_ARCH_ARM) && !defined(_WIN64)
            _asm int 3;
#endif	
        }

        bool s_hasGameData( const ConstString & _name )
        {
            bool result = USERDATA_SERVICE()
                ->hasUserdata( _name );

            return result;
        }

        bool s_writeGameData( pybind::kernel_interface * _kernel, const ConstString & _name, PyObject * _data, PyObject * _pickleTypes )
        {
            size_t size;
            pybind::pickle( _kernel, _data, _pickleTypes, nullptr, 0, size );

            MemoryInterfacePtr buffer = Helper::createMemoryCacheBuffer( size, "Python::writeGameData", __FILE__, __LINE__ );

            if( buffer == nullptr )
            {
                return false;
            }

            void * memory_buffer = buffer->getBuffer();
            size_t memory_size = buffer->getSize();

            if( pybind::pickle( _kernel, _data, _pickleTypes, memory_buffer, memory_size, size ) == false )
            {
                LOGGER_ERROR( "s_writeGameData: data %s invalid pickle"
                    , _name.c_str()
                );

                return false;
            }

            if( USERDATA_SERVICE()->writeUserdata( _name, memory_buffer, memory_size ) == false )
            {
                LOGGER_ERROR( "s_writeGameData: data %s invalid write"
                    , _name.c_str()
                );

                return false;
            }

            return true;
        }

        PyObject * s_loadGameData( pybind::kernel_interface * _kernel, const ConstString & _name, PyObject * _pickleTypes )
        {
            MemoryInterfacePtr binaryBuffer = USERDATA_SERVICE()
                ->loadUserdata( _name );

            if( binaryBuffer == nullptr )
            {
                LOGGER_ERROR( "s_readGameData: data %s invalid load"
                    , _name.c_str()
                );

                return _kernel->ret_none();
            }

            void * binaryBuffer_memory = binaryBuffer->getBuffer();
            size_t binaryBuffer_size = binaryBuffer->getSize();

            PyObject * py_data = pybind::unpickle( _kernel, binaryBuffer_memory, binaryBuffer_size, _pickleTypes );

            if( py_data == nullptr )
            {
                LOGGER_ERROR( "s_readGameData: data %s invalid unpickle"
                    , _name.c_str()
                );

                return _kernel->ret_none();
            }

            return py_data;
        }

        bool s_isAltDown() const
        {
            return INPUT_SERVICE()
                ->isAltDown();
        }

        bool s_isShiftDown() const
        {
            return INPUT_SERVICE()
                ->isShiftDown();
        }

        bool s_isCtrlDown() const
        {
            return INPUT_SERVICE()
                ->isControlDown();
        }

        bool s_isKeyDown( uint32_t _keyCode ) const
        {
            return INPUT_SERVICE()
                ->isKeyDown( _keyCode );
        }

        bool s_isExclusiveKeyDown( uint32_t _keyCode ) const
        {
            return INPUT_SERVICE()
                ->isExclusiveKeyDown( _keyCode );
        }

        bool s_isAnyKeyDown() const
        {
            return INPUT_SERVICE()
                ->isAnyKeyDown();
        }

        bool s_isAnyMouseButtonDown() const
        {
            return INPUT_SERVICE()
                ->isAnyMouseButtonDown();
        }

        bool s_isMouseButtonDown( uint32_t _button ) const
        {
            return INPUT_SERVICE()
                ->isMouseButtonDown( _button );
        }

        void s_printChildren2( const NodePtr & _node, uint32_t _tab )
        {
            IntrusiveSlugListNodeChild & children = _node->getChildren();

            for( IntrusiveSlugChild it( children ); it.eof() == false; )
            {
                const NodePtr & child = *it;

                it.next_shuffle();

                Color color;
                if( RenderInterface * render = Helper::getNodeRenderInheritance( child.get() ) )
                {
                    render->calcTotalColor( color );
                }

                printf( "%.*s%s%s [%s] (%.0f, %.0f) %.2f\n"
                    , _tab
                    , "                                         "
                    , child->isActivate() ? child->isEnable() ? "+" : "-" : "#"
                    , child->getName().c_str()
                    , child->getType().c_str()
                    , child->getWorldPosition().x
                    , child->getWorldPosition().y
                    , color.getA()
                );

                s_printChildren2( child, _tab + 1 );
            }
        }

        void s_printChildren( const NodePtr & _node )
        {
            s_printChildren2( _node, 0 );
        }

        uint32_t s_getGroupResourcesMemoryUse( const ConstString & _category, const ConstString & _group )
        {
            (void)_category;
            (void)_group;

            return 0U;
        }

        bool s_intersectPathVsCircle( const pybind::list & _path, const mt::vec3f & _position, float _radius )
        {
            pybind::list::size_type path_count = _path.size();

            if( path_count < 2 )
            {
                return false;
            }

            float dradius = _radius * _radius;

            for( pybind::list::size_type i = 1; i != path_count; ++i )
            {
                mt::vec3f v0 = _path[i - 1];
                mt::vec3f v1 = _path[i - 0];

                mt::vec3f vt;
                float d = mt::project_to_line_v3_v3( vt, v0, v1, _position );

                if( d < 0.f )
                {
                    if( mt::sqrlength_v3_v3( v0, _position ) < dradius )
                    {
                        return true;
                    }
                }
                else if( d > 1.f )
                {
                    if( mt::sqrlength_v3_v3( v1, _position ) < dradius )
                    {
                        return true;
                    }
                }
                else
                {
                    if( mt::sqrlength_v3_v3( vt, _position ) < dradius )
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        pybind::object s_selectRandomPointFromPathRadius( pybind::kernel_interface * _kernel, const pybind::list & _path, const mt::vec3f & _position, float _radius )
        {
            pybind::list::size_type path_count = _path.size();

            float dradius = _radius * _radius;

            Vector<mt::vec3f> points;

            for( pybind::list::size_type i = 0; i != path_count; ++i )
            {
                mt::vec3f v = _path[i];

                if( mt::sqrlength_v3_v3( v, _position ) > dradius )
                {
                    continue;
                }

                points.emplace_back( v );
            }

            if( points.empty() == true )
            {
                return pybind::make_none_t( _kernel );
            }

            uint32_t size = (uint32_t)points.size();

            uint32_t index = mt::rand( size );

            const mt::vec3f & vr = points[index];

            return pybind::make_object_t( _kernel, vr );
        }

        PyObject * s_reloadModule( pybind::kernel_interface * _kernel, PyObject * _module )
        {
            return _kernel->module_reload( _module );
        }

        String s_makeUID( uint32_t _length )
        {
            MENGINE_ASSERTION( _length < 1024, ("max UID length equal 1024") );

            Char uid[1024];
            Helper::makeUID( _length, uid );

            return String( uid, _length );
        }

        String s_getTextFromID( const ConstString & _textId )
        {
            const TextEntryInterfacePtr & textEntry = TEXT_SERVICE()
                ->getTextEntry( _textId );

            if( textEntry == nullptr )
            {
                return String();
            }

            const String & value = textEntry->getValue();

            return value;
        }

        void s_setTextAlias( const ConstString & _aliasEnvironment, const ConstString & _aliasId, const ConstString & _textId )
        {
            TEXT_SERVICE()
                ->setTextAlias( _aliasEnvironment, _aliasId, _textId );
        }

        void s_removeTextAlias( const ConstString & _aliasEnvironment, const ConstString & _aliasId )
        {
            TEXT_SERVICE()
                ->removeTextAlias( _aliasEnvironment, _aliasId );
        }

        bool s_hasTextAlias( const ConstString & _aliasEnvironment, const ConstString & _aliasId )
        {
            bool exist = TEXT_SERVICE()
                ->hasTextAlias( _aliasEnvironment, _aliasId );

            return exist;
        }

        const ConstString & s_getTextAlias( const ConstString & _aliasEnvironment, const ConstString & _aliasId )
        {
            const ConstString & textId = TEXT_SERVICE()
                ->getTextAlias( _aliasEnvironment, _aliasId );

            return textId;
        }

        bool s_setTextAliasArguments( const ConstString & _aliasEnvironment, const ConstString & _aliasId, const pybind::args & _args )
        {
            size_t args_count = _args.size();

            VectorString arguments;
            arguments.reserve( args_count );

            for( const pybind::object & py_obj : _args )
            {
                //PyObject * py_string = pybind::tuple_getitem( _args, it );

                if( py_obj.is_string() == true )
                {
                    String key = py_obj.extract();

                    arguments.emplace_back( key );
                }
                else if( py_obj.is_unicode() == true )
                {
                    WString key = py_obj.extract();

                    String utf8_arg;
                    Helper::unicodeToUtf8( key, utf8_arg );

                    arguments.emplace_back( utf8_arg );
                }
                else
                {
                    const Char * value = py_obj.str();

                    if( value == nullptr )
                    {
                        LOGGER_ERROR( "textfield_setTextFormatArgs %s not suport arg %s"
                            , py_obj.repr()
                        );

                        return false;
                    }

                    arguments.emplace_back( String( value ) );
                }
            }

            TEXT_SERVICE()
                ->setTextAliasArguments( _aliasEnvironment, _aliasId, arguments );

            return true;
        }

        void s_removeTextAliasArguments( const ConstString & _aliasEnvironment, const ConstString & _aliasId )
        {
            TEXT_SERVICE()
                ->removeTextAliasArguments( _aliasEnvironment, _aliasId );
        }

        float s_getJoystickAxis( uint32_t _index )
        {
            float axis = PLATFORM_SERVICE()
                ->getJoystickAxis( _index );

            return axis;
        }

        uint32_t s_addChronometer( pybind::kernel_interface * _kernel, const pybind::object & _cb, const pybind::args & _args )
        {
            uint32_t id = CHRONOMETER_SERVICE()
                ->addChronometer( [_cb, _args]( uint32_t _id, uint64_t _time )
            {
                _cb.call_args( _id, _time, _args );
            }, _kernel->object_repr( _cb.ptr() ) );

            return id;
        }

        bool s_removeChronometer( uint32_t _id )
        {
            bool successful = CHRONOMETER_SERVICE()
                ->removeChronometer( _id );

            return successful;
        }

        mt::box2f s_getHotSpotPolygonBoundingBox( const HotSpotPolygonPtr & _hotspotPolygon )
        {
            mt::box2f bb;
            mt::insideout_box( bb );

            const Polygon & polygon = _hotspotPolygon->getPolygon();

            const VectorPoints & points = polygon.getPoints();

            if( points.empty() == true )
            {
                return bb;
            }

            const mt::mat4f & wm = _hotspotPolygon->getWorldMatrix();

            for( const mt::vec2f & v : points )
            {
                mt::vec2f wmp_it;
                mt::mul_v2_v2_m4( wmp_it, v, wm );

                mt::add_internal_point( bb, wmp_it );
            }

            return bb;
        }

        void s_setCursorPosition( const mt::vec2f & _pos )
        {
            const Resolution & contentResolution = APPLICATION_SERVICE()
                ->getContentResolution();

            const Resolution & currentResolution = APPLICATION_SERVICE()
                ->getCurrentResolution();

            mt::vec2f adapt_pos;
            adapt_pos.x = _pos.x / contentResolution.getWidth() * currentResolution.getWidth();
            adapt_pos.y = _pos.y / contentResolution.getHeight() * currentResolution.getHeight();

            PLATFORM_SERVICE()
                ->setCursorPosition( adapt_pos );
        }

        bool s_isValidWindowMode()
        {
            //bool check = APPLICATION_SERVICE()
                //->isValidWindowMode();

            //return check;

            return true;
        }

        void s_addGlobalModule( const Char * _name, PyObject * _module )
        {
            SCRIPT_SERVICE()
                ->addGlobalModule( _name, _module );
        }

    public:
        float filterpowf( const pybind::list & l, float _pow )
        {
            if( l.empty() == true )
            {
                return 0.f;
            }

            if( _pow == 0.f )
            {
                return 0.f;
            }

            float inv_pow = 1.f / _pow;

            float s = 0.f;

            for( float v : l )
            {
                s += ::powf( v, inv_pow );
            }

            s /= (float)l.size();

            s = ::powf( s, _pow );

            return s;
        }

    public:
        uint32_t mt_enumerator()
        {
            uint32_t new_id = GENERATE_UNIQUE_IDENTITY();

            return new_id;
        }

        int32_t mt_rand( int32_t a )
        {
            return mt::rand( a );
        }

        float mt_randf( float a )
        {
            return mt::randf( a );
        }

        int32_t mt_range_rand( int32_t a, int32_t b )
        {
            return mt::range_rand( a, b );
        }

        float mt_range_randf( float a, float b )
        {
            return mt::range_randf( a, b );
        }

        float mt_around_randf( float a, float b )
        {
            float v_low = a - a * b;
            float v_hight = a + a * b;

            float v = mt_range_randf( v_low, v_hight );

            return v;
        }

        mt::vec2f mt_radius_randf( float _radius )
        {
            float rp = mt::randf( mt::constant::two_pi );
            float rr = mt::randf( 1.f );

            float rr2 = sqrtf( rr ) * _radius;

            float x = ::cosf( rp );
            float y = ::sinf( rp );

            float xr = x * rr2;
            float yr = y * rr2;

            return mt::vec2f( xr, yr );
        }

        uint32_t mt_rounding( uint32_t _value, uint32_t _round )
        {
            uint32_t floor_value = _value / _round;
            uint32_t round_value = floor_value * _round;

            return round_value;
        }

        float mt_roundingf( float _value, float _round )
        {
            float floor_value = floorf( _value / _round );
            float round_value = floor_value * _round;

            return round_value;
        }

        float mt_pi()
        {
            return mt::constant::pi;
        }

        float mt_deg2rad( float _deg )
        {
            return _deg * mt::constant::deg2rad;
        }

        float mt_rad2deg( float _rad )
        {
            return _rad * mt::constant::rad2deg;
        }

        float mt_sqrtf( float a )
        {
            return ::sqrtf( a );
        }

        float mt_powf( float a, float b )
        {
            return ::powf( a, b );
        }

        float mt_absf( float a )
        {
            return ::fabsf( a );
        }

        float mt_cosf( float a )
        {
            return ::cosf( a );
        }

        float mt_sinf( float a )
        {
            return ::sinf( a );
        }

        float mt_cosf_deg( float a )
        {
            return ::cosf( a * mt::constant::deg2rad );
        }

        float mt_sinf_deg( float a )
        {
            return ::sinf( a * mt::constant::deg2rad );
        }

        float mt_tanf( float a )
        {
            return ::tanf( a );
        }

        float mt_acosf( float a )
        {
            return ::acosf( a );
        }

        float mt_atanf( float _x )
        {
            return ::atanf( _x );
        }

        float mt_logf( float _x )
        {
            return ::logf( _x );
        }

        float mt_log10f( float _x )
        {
            return ::log10f( _x );
        }

        double mt_log10( double _x )
        {
            return ::log10( _x );
        }

        uint32_t mt_fibo( uint32_t _n )
        {
            if( _n == 0 )
            {
                return 0;
            }
            else if( _n == 1 )
            {
                return 1;
            }

            uint32_t fn = mt_fibo( _n - 1 ) + mt_fibo( _n - 2 );

            return fn;
        }

        uint64_t mt_fibo_bine( uint64_t _n )
        {
            const double nf = (double)_n;
            const double fplus = (1.0 + 2.2360679774997896964091736687313) * 0.5;
            const double fplusn = pow( fplus, nf );

            const double fminus = (1.0 - 2.2360679774997896964091736687313) * 0.5;
            const double fminusn = pow( fminus, nf );

            double fbine = (fplusn - fminusn) * 0.44721359549995793928183473374626;

            uint64_t fbine64 = (uint64_t)fbine;

            return fbine64;
        }

        float mt_dot_v2_v2( const mt::vec2f & _a, const mt::vec2f & _b )
        {
            float d = mt::dot_v2_v2( _a, _b );

            return d;
        }

        mt::vec2f mt_direction_v2_v2( const mt::vec2f & _from, const mt::vec2f & _to )
        {
            mt::vec2f direction;
            mt::dir_v2_v2( direction, _to, _from );

            return direction;
        }

        mt::vec3f mt_direction_v3_v3( const mt::vec3f & _from, const mt::vec3f & _to )
        {
            mt::vec3f direction;
            mt::dir_v3_v3( direction, _from, _to );

            return direction;
        }

        float mt_angle_from_v2_v2( const mt::vec2f & _from, const mt::vec2f & _to )
        {
            mt::vec2f direction;
            mt::sub_v2_v2( direction, _from, _to );

            mt::vec2f direction_n;
            mt::norm_v2_v2( direction_n, direction );

            float angle = mt::signed_angle( direction_n );

            return angle;
        }

        mt::vec2f mt_norm_v2( const mt::vec2f & _vec )
        {
            mt::vec2f v_n;
            mt::norm_v2_v2( v_n, _vec );

            return v_n;
        }

        mt::vec3f mt_norm_v3( const mt::vec3f & _vec )
        {
            mt::vec3f v_n;
            mt::norm_v3_v3( v_n, _vec );

            return v_n;
        }


        bool s_intersectsEllipseVsPoint( const mt::vec2f & _pos, float _radius, float _ellipse, const mt::vec2f & _point )
        {
            mt::vec2f v = _point - _pos;

            v.y /= _ellipse;

            float v_sqrdist = v.sqrlength();

            if( v_sqrdist > _radius * _radius )
            {
                return false;
            }

            return true;
        }

        bool s_intersectsBoxes( const mt::vec2f & _minFirst, const mt::vec2f & _maxFirst, const mt::vec2f & _minSecond, const mt::vec2f & _maxSecond )
        {
            mt::box2f box_first( _minFirst, _maxFirst );
            mt::box2f box_second( _minSecond, _maxSecond );

            bool result = mt::is_intersect( box_first, box_second );

            return result;
        }

        mt::vec2f projectionPointToLine( const mt::vec2f & _point, const mt::vec2f & _v0, const mt::vec2f & _v1 )
        {
            mt::vec2f dir = _v1 - _v0;

            if( mt::dot_v2_v2( dir, dir ) < 0.0001f )
            {
                return _v0;
            }

            mt::vec2f dir_norm;
            float dir_length = norm_v2_f( dir_norm, dir );

            mt::vec2f dir_point = _point - _v0;

            float dist = dot_v2_v2( dir_point, dir_norm );

            if( dist < 0.f )
            {
                dist = 0.f;
            }
            else if( dist > dir_length )
            {
                dist = dir_length;
            }

            mt::vec2f line_point = _v0 + dir_norm * dist;

            return line_point;
        }

        pybind::tuple s_angle_correct_interpolate_from_to( pybind::kernel_interface * _kernel, float _from, float _to )
        {
            float correct_angle_from;
            float correct_angle_to;

            mt::angle_correct_interpolate_from_to( _from, _to, correct_angle_from, correct_angle_to );

            pybind::tuple py_result = pybind::make_tuple_t( _kernel, correct_angle_from, correct_angle_to );

            return py_result;
        }

        float s_angle_between_two_vectors( const mt::vec2f & _v1, const mt::vec2f & _v2 )
        {
            mt::vec2f v1_norm;
            mt::norm_v2_v2( v1_norm, _v1 );

            mt::vec2f v2_norm;
            mt::norm_v2_v2( v2_norm, _v2 );

            float cos_v = mt::dot_v2_v2( v1_norm, v2_norm );

            float angle = acosf( cos_v );

            return angle;
        }

        pybind::list s_getPolygonPoints( pybind::kernel_interface * _kernel, const Polygon & _polygon )
        {
            const VectorPoints & points = _polygon.getPoints();

            pybind::list py_list = pybind::make_list_container_t( _kernel, points );

            return py_list;
        }

        const Polygon & s_intersectionPolygons( const Polygon & _p1, const Polygon & _p2 )
        {
            VectorGeolygon output;
            Helper::difference( _p1, _p2, output );

            if( output.empty() == true )
            {
                return _p1;
            }

            const Polygon & outer = output[0].getOuter();

            return outer;
        }

        bool s_intersectsPolygons( const Polygon & _p1, const Polygon & _p2 )
        {
            bool intersect = Helper::intersects( _p1, _p2 );

            return intersect;
        }

        bool s_intersectsPolygonsWM( const mt::mat4f & _wm1, Polygon _p1, const mt::mat4f & _wm2, Polygon _p2 )
        {
            Polygon p1wm;
            _p1.mul_wm( p1wm, _wm1 );

            Polygon p2wm;
            _p2.mul_wm( p2wm, _wm2 );

            bool intersect = Helper::intersects( p1wm, p2wm );

            return intersect;
        }

        bool s_intersectsPolygonsWMP( const mt::vec3f & _wm1, Polygon _p1, const mt::vec3f & _wm2, Polygon _p2 )
        {
            Polygon p1wm;
            _p1.transpose( p1wm, _wm1.to_vec2f() );

            Polygon p2wm;
            _p2.transpose( p2wm, _wm2.to_vec2f() );

            bool intersect = Helper::intersects( p1wm, p2wm );

            return intersect;
        }

#ifdef PYBIND_VISIT_OBJECTS
        class MyObjectVisits
            : public pybind::pybind_visit_objects
        {
        public:
            MyObjectVisits( const pybind::list & _py_list )
                : m_py_list( _py_list )
            {
            }

        protected:
            void visit( PyObject * _obj ) override
            {
                m_py_list.append( _obj );
            }

        protected:
            pybind::list m_py_list;
        };
#endif

        pybind::list s_objects( pybind::kernel_interface * _kernel )
        {
            pybind::list py_list( _kernel );

#ifdef PYBIND_VISIT_OBJECTS
            MyObjectVisits mov( py_list );
            pybind::visit_objects( &mov );
#endif

            return py_list;
        }

        pybind::list s_textures( pybind::kernel_interface * _kernel )
        {
            pybind::list py_list( _kernel );

            RENDERTEXTURE_SERVICE()
                ->visitTexture( [&py_list]( const RenderTextureInterfacePtr & _texture )
            {
                const FilePath & filePath = _texture->getFileName();

                py_list.append( filePath );
            } );

            return py_list;
        }

        double s_watchdog( const ConstString & _tag )
        {
            double watch = WATCHDOG( _tag );

            return watch;
        }

        bool s_isPointInsidePolygon( const mt::vec2f & _point, const Polygon & _polygon )
        {
            bool result = Helper::intersects( _polygon, _point );

            return result;
        }

        uint64_t s_getTimeMs()
        {
            uint64_t ms = TIME_SYSTEM()
                ->getTimeMilliseconds();

            return ms;
        }

        uint64_t s_getTime()
        {
            uint64_t ms = s_getTimeMs();

            uint64_t s = ms / 1000;

            return s;
        }

        uint32_t s_getDate()
        {
            std::time_t ctTime;
            std::time( &ctTime );

            return static_cast<uint32_t>(ctTime);
        }

        String s_getTimeString()
        {
            std::time_t ctTime;
            std::time( &ctTime );

            std::tm* sTime = std::localtime( &ctTime );

            Stringstream str;
            str << std::setw( 2 ) << std::setfill( '0' ) << sTime->tm_hour
                << ":" << std::setw( 2 ) << std::setfill( '0' ) << sTime->tm_min
                << ":" << std::setw( 2 ) << std::setfill( '0' ) << sTime->tm_sec;

            return str.str();
        }

        class MyAccountVisitor
            : public AccountVisitorInterface
            , public Factorable
        {
        public:
            MyAccountVisitor( VectorConstString * _accounts )
                : m_accounts( _accounts )
            {
            }

            ~MyAccountVisitor() override
            {
            }

        protected:
            void onAccount( const AccountInterfacePtr & _account ) override
            {
                const ConstString & name = _account->getID();

                m_accounts->emplace_back( name );
            }

        protected:
            VectorConstString * m_accounts;

        private:
            MyAccountVisitor( const MyAccountVisitor & );
            void operator = ( const MyAccountVisitor & );
        };

        VectorConstString s_getAccounts()
        {
            VectorConstString v_accounts;

            AccountVisitorInterfacePtr mav = new FactorableUnique<MyAccountVisitor>( &v_accounts );

            ACCOUNT_SERVICE()
                ->visitAccounts( mav );

            return v_accounts;
        }

        bool s_addSetting( pybind::kernel_interface * _kernel, const ConstString & _setting, PyObject * _defaultValue, const pybind::object & _cb, const pybind::args & _args )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "Error addSetting: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_addAccountSetting( _kernel, accountID, _setting, _defaultValue, _cb, _args );
        }

        bool s_changeSetting( pybind::kernel_interface * _kernel, const ConstString & _setting, PyObject * _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "Error changeSetting: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSetting( _kernel, accountID, _setting, _value );
        }

        bool s_changeSettingBool( const ConstString & _setting, bool _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "Error changeSettingBool: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSettingBool( accountID, _setting, _value );
        }

        bool s_changeSettingInt( const ConstString & _setting, int32_t _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "changeSettingInt: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSettingInt( accountID, _setting, _value );
        }

        bool s_changeSettingUInt( const ConstString & _setting, uint32_t _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "changeSettingUInt: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSettingUInt( accountID, _setting, _value );
        }

        bool s_changeSettingUInt64( const ConstString & _setting, uint64_t _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "changeSettingUInt64: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSettingUInt64( accountID, _setting, _value );
        }

        bool s_changeSettingFloat( const ConstString & _setting, float _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "changeSettingFloat: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSettingFloat( accountID, _setting, _value );
        }

        bool s_changeSettingStrings( const ConstString & _setting, const VectorWString & _values )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "Error changeSettingStrings: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_changeAccountSettingStrings( accountID, _setting, _values );
        }

        class PyAccountSettingProvider
            : public Factorable
            , public AccountSettingProviderInterface
        {
        public:
            PyAccountSettingProvider( pybind::kernel_interface * _kernel, const pybind::object & _cb, const pybind::args & _args )
                : m_kernel( _kernel )
                , m_cb( _cb )
                , m_args( _args )
            {
            }

        protected:
            void onChangeSetting( const Char * _value ) override
            {
                PyObject * pyunicode_value = m_kernel->unicode_from_utf8( _value );

                m_cb.call_args( pyunicode_value, m_args );

                m_kernel->decref( pyunicode_value );
            }

        protected:
            pybind::kernel_interface * m_kernel;

            pybind::object m_cb;
            pybind::args m_args;
        };

        typedef IntrusivePtr<PyAccountSettingProvider> PyAccountSettingProviderPtr;

        bool s_addAccountSetting( pybind::kernel_interface * _kernel, const ConstString & _accountID, const ConstString & _setting, PyObject * _defaultValue, const pybind::object & _cb, const pybind::args & _args )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "addAccountSetting: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            PyAccountSettingProviderPtr provider = nullptr;

            if( _cb.is_none() == false )
            {
                provider = new FactorableUnique<PyAccountSettingProvider>( _kernel, _cb, _args );
            }

            const Char * utf8_defaultValue = _kernel->unicode_to_utf8( _defaultValue );

            bool result = account->addSetting( _setting, utf8_defaultValue, provider );

            return result;
        }

        bool s_hasAccountSetting( const ConstString & _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "addAccountSetting: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            bool result = account->hasSetting( _setting );

            return result;
        }

        bool s_changeAccountSetting( pybind::kernel_interface * _kernel, const ConstString & _accountID, const ConstString & _setting, PyObject * _value )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "s_changeAccountSetting: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "s_changeAccountSetting: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            const Char * utf8_defaultValue = _kernel->unicode_to_utf8( _value );

            bool result = account->changeSetting( _setting, utf8_defaultValue );

            return result;
        }

        bool s_changeAccountSettingBool( const ConstString & _accountID, const ConstString & _setting, bool _value )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            const Char * value_str = _value == true ? "True" : "False";

            bool result = account->changeSetting( _setting, value_str );

            return result;
        }

        bool s_changeAccountSettingInt( const ConstString & _accountID, const ConstString & _setting, int32_t _value )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            String setting_value;
            Helper::intToString( _value, setting_value );

            bool result = account->changeSetting( _setting, setting_value.c_str() );

            return result;
        }

        bool s_changeAccountSettingUInt( const ConstString & _accountID, const ConstString & _setting, uint32_t _value )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            String setting_value;
            Helper::unsignedToString( _value, setting_value );

            bool result = account->changeSetting( _setting, setting_value.c_str() );

            return result;
        }

        bool s_changeAccountSettingUInt64( const ConstString & _accountID, const ConstString & _setting, uint64_t _value )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            String setting_value;
            Helper::unsigned64ToString( _value, setting_value );

            bool result = account->changeSetting( _setting, setting_value.c_str() );

            return result;
        }

        bool s_changeAccountSettingFloat( const ConstString & _accountID, const ConstString & _setting, float _value )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "changeAccountSettingBool: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            String setting_value;
            Helper::floatToString( _value, setting_value );

            bool result = account->changeSetting( _setting, setting_value.c_str() );

            return result;
        }

        bool s_changeAccountSettingStrings( const ConstString & _accountID, const ConstString & _setting, const VectorWString & _values )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "changeAccountSettingStrings: account not found '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->hasSetting( _setting ) == false )
            {
                LOGGER_ERROR( "changeAccountSettingStrings: account '%s' not found setting '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return false;
            }

            String setting_value;

            for( const WString & value : _values )
            {
                if( setting_value.empty() == false )
                {
                    setting_value += " ,,, ";
                }

                String utf8_value;
                Helper::unicodeToUtf8( value, utf8_value );

                setting_value += utf8_value;
            }

            bool result = account->changeSetting( _setting, setting_value.c_str() );

            return result;
        }

        bool s_addGlobalSetting( pybind::kernel_interface * _kernel, const ConstString & _setting, PyObject * _defaultValue, const pybind::object & _cb, const pybind::args & _args )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_addAccountSetting( _kernel, accountID, _setting, _defaultValue, _cb, _args );
        }

        bool s_hasGlobalSetting( const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_hasAccountSetting( accountID, _setting );
        }

        bool s_changeGlobalSetting( pybind::kernel_interface * _kernel, const ConstString & _setting, PyObject * _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSetting( _kernel, accountID, _setting, _value );
        }

        bool s_changeGlobalSettingBool( const ConstString & _setting, bool _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSettingBool( accountID, _setting, _value );
        }

        bool s_changeGlobalSettingInt( const ConstString & _setting, int32_t _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSettingInt( accountID, _setting, _value );
        }

        bool s_changeGlobalSettingUInt( const ConstString & _setting, uint32_t _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSettingUInt( accountID, _setting, _value );
        }

        bool s_changeGlobalSettingUInt64( const ConstString & _setting, uint64_t _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSettingUInt64( accountID, _setting, _value );
        }

        bool s_changeGlobalSettingFloat( const ConstString & _setting, float _value )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSettingFloat( accountID, _setting, _value );
        }

        bool s_changeGlobalSettingStrings( const ConstString & _setting, const VectorWString & _values )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "addGlobalSetting: account not found"
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_changeAccountSettingStrings( accountID, _setting, _values );
        }

        bool s_hasSetting( const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "Error hasSetting: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return false;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_hasAccountSetting( accountID, _setting );
        }

        PyObject * s_getCurrentAccountUID( pybind::kernel_interface * _kernel )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getCurrentAccountUID: currentAccount is none [%s]"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountUID( _kernel, accountID );
        }

        PyObject * s_getSetting( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSetting: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSetting( _kernel, accountID, _setting );
        }

        PyObject * s_getSettingBool( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingBool: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingBool( _kernel, accountID, _setting );
        }

        PyObject * s_getSettingInt( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingInt: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingInt( _kernel, accountID, _setting );
        }

        PyObject * s_getSettingUInt( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingUInt: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingUInt( _kernel, accountID, _setting );
        }

        PyObject * s_getSettingUInt64( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingUInt: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingUInt64( _kernel, accountID, _setting );
        }

        PyObject * s_getSettingFloat( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingFloat: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingFloat( _kernel, accountID, _setting );
        }

        PyObject * s_getSettingStrings( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingFloat: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingStrings( _kernel, accountID, _setting );
        }

        float s_getSettingFloatDefault( const ConstString & _setting, float _default )
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "getSettingFloat: currentAccount is none [%s]"
                    , _setting.c_str()
                );

                return _default;
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return s_getAccountSettingFloatDefault( accountID, _setting, _default );
        }

        bool s_getConfigBool( const Char * _section, const Char * _key, bool _default )
        {
            bool result = CONFIG_SERVICE()
                ->getValue( _section, _key, _default );

            return result;
        }

        int32_t s_getConfigInt( const Char * _section, const Char * _key, int32_t _default )
        {
            int32_t result = CONFIG_SERVICE()
                ->getValue( _section, _key, _default );

            return result;
        }

        uint32_t s_getConfigUInt( const Char * _section, const Char * _key, uint32_t _default )
        {
            uint32_t result = CONFIG_SERVICE()
                ->getValue( _section, _key, _default );

            return result;
        }

        uint64_t s_getConfigUInt64( const Char * _section, const Char * _key, uint64_t _default )
        {
            uint64_t result = CONFIG_SERVICE()
                ->getValue( _section, _key, _default );

            return result;
        }

        float s_getConfigFloat( const Char * _section, const Char * _key, float _default )
        {
            float result = CONFIG_SERVICE()
                ->getValue( _section, _key, _default );

            return result;
        }

        const Char * s_getConfigString( const Char * _section, const Char * _key, const Char * _default )
        {

            const Char * result = CONFIG_SERVICE()
                ->getValue( _section, _key, _default );

            return result;
        }


        PyObject * s_getAccountUID( pybind::kernel_interface * _kernel, const ConstString & _accountID )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSetting account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            const AccountUID & value = account->getUID();

            PyObject * py_value = _kernel->string_from_char_size( value.data, AccountUID::size_data );

            return py_value;
        }

        PyObject * s_getAccountSetting( pybind::kernel_interface * _kernel, const ConstString & _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSetting account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, value ) == false )
            {
                return _kernel->ret_none();
            }

            PyObject * py_value = _kernel->unicode_from_utf8( value );

            return py_value;
        }

        PyObject * s_getAccountSettingBool( pybind::kernel_interface * _kernel, const ConstString& _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSettingBool account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            if( strcmp( value, "True" ) == 0 || strcmp( value, "true" ) == 0 || strcmp( value, "TRUE" ) == 0 )
            {
                return _kernel->ret_true();
            }
            else if( strcmp( value, "False" ) == 0 || strcmp( value, "false" ) == 0 || strcmp( value, "FALSE" ) == 0 )
            {
                return _kernel->ret_false();
            }

            LOGGER_ERROR( "getAccountSettingBool account '%s' setting '%s' value '%s' is not bool [True|False]"
                , _accountID.c_str()
                , _setting.c_str()
                , value
            );

            return _kernel->ret_none();
        }

        PyObject * s_getAccountSettingInt( pybind::kernel_interface * _kernel, const ConstString& _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char setting_value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, setting_value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            int32_t value;
            if( Helper::charsToInt( setting_value, value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' can't scanf from '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                    , setting_value
                );

                return _kernel->ret_none();
            }

            PyObject * py_value = pybind::ptr( _kernel, value );

            return py_value;
        }

        PyObject * s_getAccountSettingUInt( pybind::kernel_interface * _kernel, const ConstString& _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char setting_value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, setting_value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            uint32_t value;
            if( Helper::charsToUnsigned( setting_value, value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' can't scanf from '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                    , setting_value
                );

                return _kernel->ret_none();
            }

            PyObject * py_value = pybind::ptr( _kernel, value );

            return py_value;
        }

        PyObject * s_getAccountSettingUInt64( pybind::kernel_interface * _kernel, const ConstString& _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSettingUInt account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char setting_value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, setting_value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            uint64_t value;
            if( Helper::charsToUnsigned64( setting_value, value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' can't scanf from '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                    , setting_value
                );

                return _kernel->ret_none();
            }

            PyObject * py_value = pybind::ptr( _kernel, value );

            return py_value;
        }

        PyObject * s_getAccountSettingStrings( pybind::kernel_interface * _kernel, const ConstString& _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSettingUInt account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char setting_value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, setting_value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            size_t setting_value_len = strlen( setting_value );

            if( setting_value_len == 0 )
            {
                pybind::list l( _kernel );

                return l.ret();
            }

            WChar setting_valueW[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            Helper::utf8ToUnicode( setting_value, setting_valueW, MENGINE_ACCOUNT_SETTING_MAXVALUE );

            VectorWString strings;
            Helper::wsplit( strings, setting_valueW, true, L" ,,, " );

            pybind::list l = pybind::make_list_container_t( _kernel, strings );

            return l.ret();
        }

        PyObject * s_getAccountSettingFloat( pybind::kernel_interface * _kernel, const ConstString& _accountID, const ConstString & _setting )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSettingFloat account '%s' is none"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            Char setting_value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, setting_value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _kernel->ret_none();
            }

            float value;
            if( Helper::charsToFloat( setting_value, value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' can't scanf from '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                    , setting_value
                );

                return _kernel->ret_none();
            }

            PyObject * py_value = pybind::ptr( _kernel, value );

            return py_value;
        }

        float s_getAccountSettingFloatDefault( const ConstString& _accountID, const ConstString & _setting, float _default )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "getAccountSettingFloatDefault account '%s' is none"
                    , _accountID.c_str()
                );

                return _default;
            }

            if( account->hasSetting( _setting ) == false )
            {
                return _default;
            }

            Char setting_value[MENGINE_ACCOUNT_SETTING_MAXVALUE];
            if( account->getSetting( _setting, setting_value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' not found"
                    , _accountID.c_str()
                    , _setting.c_str()
                );

                return _default;
            }

            float value;
            if( Helper::charsToFloat( setting_value, value ) == false )
            {
                LOGGER_ERROR( "account '%s' setting '%s' can't scanf from '%s'"
                    , _accountID.c_str()
                    , _setting.c_str()
                    , setting_value
                );

                return _default;
            }

            return value;
        }

        PyObject * s_getGlobalAccountUID( pybind::kernel_interface * _kernel )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountUID( _kernel, accountID );
        }

        PyObject * s_getGlobalSetting( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountSetting( _kernel, accountID, _setting );
        }

        PyObject * s_getGlobalSettingBool( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            (void)_kernel;

            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountSettingBool( _kernel, accountID, _setting );
        }

        PyObject * s_getGlobalSettingInt( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountSettingInt( _kernel, accountID, _setting );
        }

        PyObject * s_getGlobalSettingUInt( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountSettingUInt( _kernel, accountID, _setting );
        }

        PyObject * s_getGlobalSettingUInt64( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountSettingUInt64( _kernel, accountID, _setting );
        }

        PyObject * s_getGlobalSettingFloat( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            return s_getAccountSettingFloat( _kernel, accountID, _setting );
        }

        PyObject * s_getGlobalSettingStrings( pybind::kernel_interface * _kernel, const ConstString & _setting )
        {
            if( ACCOUNT_SERVICE()
                ->hasGlobalAccount() == false )
            {
                LOGGER_ERROR( "getGlobalSetting account is none"
                );

                return _kernel->ret_none();
            }

            const ConstString & accountID = ACCOUNT_SERVICE()
                ->getGlobalAccountID();

            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( accountID );

            return s_getAccountSettingStrings( _kernel, accountID, _setting );
        }

        PyObject * s_createAccount( pybind::kernel_interface * _kernel )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->createAccount();

            if( account == nullptr )
            {
                return _kernel->ret_none();
            }

            const ConstString & accountId = account->getID();

            PyObject * py_value = pybind::ptr( _kernel, accountId );

            return py_value;
        }

        PyObject * s_createGlobalAccount( pybind::kernel_interface * _kernel )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->createGlobalAccount();

            if( account == nullptr )
            {
                return _kernel->ret_none();
            }

            const ConstString & accountId = account->getID();

            PyObject * py_value = pybind::ptr( _kernel, accountId );

            return py_value;
        }

        void s_selectAccount( const ConstString& _accountID )
        {
            ACCOUNT_SERVICE()
                ->selectAccount( _accountID );
        }

        bool s_hasCurrentAccount()
        {
            return ACCOUNT_SERVICE()
                ->hasCurrentAccount();
        }

        void s_setDefaultAccount( const ConstString & _accountID )
        {
            ACCOUNT_SERVICE()
                ->setDefaultAccount( _accountID );
        }

        void s_setGlobalAccount( const ConstString & _accountID )
        {
            ACCOUNT_SERVICE()
                ->setGlobalAccount( _accountID );
        }

        bool s_hasGlobalAccount()
        {
            return ACCOUNT_SERVICE()
                ->hasGlobalAccount();
        }

        const ConstString & s_getGlobalAccountName()
        {
            return ACCOUNT_SERVICE()
                ->getGlobalAccountID();
        }

        const ConstString & s_getDefaultAccount()
        {
            return ACCOUNT_SERVICE()
                ->getDefaultAccountID();
        }

        bool s_hasDefaultAccount()
        {
            return ACCOUNT_SERVICE()
                ->hasDefaultAccount();
        }

        bool s_isCurrentDefaultAccount()
        {
            return ACCOUNT_SERVICE()
                ->isCurrentDefaultAccount();
        }

        bool s_selectDefaultAccount()
        {
            return ACCOUNT_SERVICE()
                ->selectDefaultAccount();
        }

        void s_saveAccount()
        {
            //AccountInterface * currentAccount = ACCOUNT_SERVICE()
            //	->getCurrentAccount();

            //if( currentAccount == NULL )
            //{
            //	LOGGER_ERROR("Error saveCurrentAccount: currentAccount is none"
            //		);

            //	return;
            //}

            //currentAccount->save();
        }

        void s_saveAccounts()
        {
            ACCOUNT_SERVICE()
                ->saveAccounts();
        }

        void s_saveAccountsInfo()
        {
            //ACCOUNT_SERVICE()
            //	->saveAccountsInfo();
        }

        void s_deleteAccount( const ConstString& _accountName )
        {
            ACCOUNT_SERVICE()
                ->deleteAccount( _accountName );
        }

        const ConstString & s_getCurrentAccountName()
        {
            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "Error getCurrentAccountName: currentAccount is none"
                );

                return ConstString::none();
            }

            const ConstString & name = ACCOUNT_SERVICE()
                ->getCurrentAccountID();

            return name;
        }

        bool s_writeAccountPickleFile( pybind::kernel_interface * _kernel, const ConstString & _accountID, const WString & _fileName, PyObject * _data, PyObject * _pickleTypes )
        {
            String utf8_fileName;
            if( Helper::unicodeToUtf8( _fileName, utf8_fileName ) == false )
            {
                LOGGER_ERROR( "s_writeAccountBinaryFile: invalid file '%s' convert to utf8"
                    , _fileName.c_str()
                );

                return false;
            }

            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "writeAccountPickleFile: invalid account '%s'"
                    , _accountID.c_str()
                );

                return false;
            }

            FilePath filepath = Helper::stringizeFilePathSize( utf8_fileName.c_str(), utf8_fileName.size() );

            size_t size;
            if( pybind::pickle( _kernel, _data, _pickleTypes, nullptr, 0, size ) == false )
            {
                LOGGER_ERROR( "writeAccountPickleFile: account '%s' invalid get pickle size"
                    , _accountID.c_str()
                );

                return false;
            }

            MemoryInterfacePtr buffer = Helper::createMemoryCacheBuffer( size, "Python::writeAccountPickleFile", __FILE__, __LINE__ );

            if( buffer == nullptr )
            {
                LOGGER_ERROR( "writeAccountPickleFile: account '%s' invalid get memory for '%d' size"
                    , _accountID.c_str()
                    , size
                );

                return false;
            }

            void * memory_buffer = buffer->getBuffer();
            size_t memory_size = buffer->getSize();

            if( pybind::pickle( _kernel, _data, _pickleTypes, memory_buffer, memory_size, size ) == false )
            {
                LOGGER_ERROR( "writeAccountPickleFile: account '%s' invalid pickle"
                    , _accountID.c_str()
                );

                return false;
            }

            if( account->writeBinaryFile( filepath, memory_buffer, memory_size ) == false )
            {
                LOGGER_ERROR( "writeAccountPickleFile: account '%s' invalid write file '%s'"
                    , _accountID.c_str()
                    , _fileName.c_str()
                );

                return false;
            }

            return true;
        }

        PyObject * s_loadAccountPickleFile( pybind::kernel_interface * _kernel, const ConstString & _accountID, const WString & _fileName, PyObject * _pickleTypes )
        {
            String utf8_fileName;
            if( Helper::unicodeToUtf8( _fileName, utf8_fileName ) == false )
            {
                LOGGER_ERROR( "loadAccountPickleFile: invalid convert filename %s to utf8"
                    , _fileName.c_str()
                );

                return _kernel->ret_none();
            }

            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( account == nullptr )
            {
                LOGGER_ERROR( "loadAccountPickleFile: invalid get account '%s'"
                    , _accountID.c_str()
                );

                return _kernel->ret_none();
            }

            FilePath filename = Helper::stringizeFilePathSize( utf8_fileName.c_str(), utf8_fileName.size() );

            MemoryInterfacePtr binaryBuffer = account->loadBinaryFile( filename );

            if( binaryBuffer == nullptr )
            {
                LOGGER_ERROR( "loadAccountPickleFile: account %s invalid load file %s"
                    , _accountID.c_str()
                    , _fileName.c_str()
                );

                return _kernel->ret_none();
            }

            void * binaryBuffer_memory = binaryBuffer->getBuffer();
            size_t binaryBuffer_size = binaryBuffer->getSize();

            PyObject * py_data = pybind::unpickle( _kernel, binaryBuffer_memory, binaryBuffer_size, _pickleTypes );

            if( py_data == nullptr )
            {
                LOGGER_ERROR( "loadAccountPickleFile: account %s invalid unpickle file %s"
                    , _accountID.c_str()
                    , _fileName.c_str()
                );

                return _kernel->ret_none();
            }

            return py_data;
        }

        const FileGroupInterfacePtr & s_getDefaultFileGroup()
        {
            const FileGroupInterfacePtr & fileGroup = FILE_SERVICE()
                ->getDefaultFileGroup();

            return fileGroup;
        }

        bool s_hasFileGroup( const ConstString & _fileGroup )
        {
            bool exist = FILE_SERVICE()
                ->hasFileGroup( _fileGroup, nullptr );

            return exist;
        }

        const FileGroupInterfacePtr & s_getFileGroup( const ConstString & _fileGroup )
        {
            const FileGroupInterfacePtr & fileGroup = FILE_SERVICE()
                ->getFileGroup( _fileGroup );

            return fileGroup;
        }

        bool s_hasAccountPickleFile( const ConstString & _accountID, const WString & _fileName )
        {
            AccountInterfacePtr account = ACCOUNT_SERVICE()
                ->getAccount( _accountID );

            if( ACCOUNT_SERVICE()
                ->hasCurrentAccount() == false )
            {
                LOGGER_ERROR( "hasAccountPickleFile: invalid get account %s"
                    , _accountID.c_str()
                );

                return false;
            }

            String utf8_fileName;
            if( Helper::unicodeToUtf8( _fileName, utf8_fileName ) == false )
            {
                LOGGER_ERROR( "hasAccountPickleFile: invalid convert filename %s to utf8"
                    , _fileName.c_str()
                );

                return false;
            }

            FilePath filename = Helper::stringizeFilePath( utf8_fileName );

            bool exist = account->hasBinaryFile( filename );

            return exist;
        }

        void s_setParticlesEnabled( bool _enable )
        {
            APPLICATION_SERVICE()
                ->setParticleEnable( _enable );
        }

        //////////////////////////////////////////////////////////////////////////
        mt::vec2f s_getNodeScreenPosition( Node * _node )
        {
            if( _node == nullptr )
            {
                LOGGER_ERROR( "Menge.getNodeScreenPosition node is null" );

                return mt::vec2f( 0.f, 0.f );
            }

            RenderCameraInterfacePtr camera = Helper::getNodeRenderCameraInheritance( _node );

            if( camera == nullptr )
            {
                camera = PLAYER_SERVICE()
                    ->getRenderCamera();
            }

            const mt::mat4f & wm = _node->getWorldMatrix();

            mt::vec2f screen;
            camera->fromWorldToScreenPosition( wm, screen );

            return screen;
        }

        bool s_hasTextByKey( const ConstString& _key )
        {
            bool value = TEXT_SERVICE()
                ->existText( _key, nullptr );

            return value;
        }

        WString s_getTextByKey( const ConstString& _key )
        {
            TextEntryInterfacePtr entry;
            if( TEXT_SERVICE()
                ->existText( _key, &entry ) == false )
            {
                pybind::throw_exception( "Mengine.getTextByKey invalid get key %s"
                    , _key.c_str()
                );
            }

            const String & text = entry->getValue();

            WString unicode;
            if( Helper::utf8ToUnicode( text, unicode ) == false )
            {
                pybind::throw_exception( "Mengine.getTextByKey invalid text key '%s' convert '%s' to unicode"
                    , _key.c_str()
                    , text.c_str()
                );
            }

            return unicode;
        }

        uint32_t s_getTextCharCountByKey( const ConstString& _key )
        {
            TextEntryInterfacePtr entry;
            if( TEXT_SERVICE()
                ->existText( _key, &entry ) == false )
            {
                pybind::throw_exception( "Mengine.getTextCharCountByKey invalid get key %s"
                    , _key.c_str()
                );
            }

            const String & text = entry->getValue();

            uint32_t count = (uint32_t)text.size();

            return count;
        }

        void s_setVSync( bool _vSync )
        {
            APPLICATION_SERVICE()
                ->setVSync( _vSync );
        }

        bool s_getVSync()
        {
            bool vsync = APPLICATION_SERVICE()
                ->getVSync();

            return vsync;
        }

        void s_setCursorMode( bool _mode )
        {
            APPLICATION_SERVICE()
                ->setCursorMode( _mode );
        }

        bool s_getCursorMode()
        {
            bool cursorMode = APPLICATION_SERVICE()
                ->getCursorMode();

            return cursorMode;
        }

        bool s_setCursorIcon( const ConstString & _resourceName )
        {
            APPLICATION_SERVICE()
                ->setCursorIcon( _resourceName );

            return true;
        }
    };
    //////////////////////////////////////////////////////////////////////////
    struct extract_Blobject_type
        : public pybind::type_cast_result<Blobject>
    {
        bool apply( pybind::kernel_interface * _kernel, PyObject * _obj, value_type & _value, bool _nothrow ) override
        {
            (void)_kernel;
            (void)_nothrow;

            if( _kernel->string_check( _obj ) == true )
            {
                uint32_t size;
                const Char * value_char = _kernel->string_to_char_and_size( _obj, size );

                if( value_char == 0 )
                {
                    return false;
                }

                _value.assign( value_char, value_char + size );
            }
            else
            {
                return false;
            }

            return true;
        }

        PyObject * wrap( pybind::kernel_interface * _kernel, pybind::type_cast_result<value_type>::TCastRef _value ) override
        {
            (void)_kernel;

            const char * value_str = reinterpret_cast<const char *>(&_value[0]);
            value_type::size_type value_size = _value.size();

            PyObject * py_value = _kernel->string_from_char_size( value_str, (uint32_t)value_size );

            return py_value;
        }
    };
    //////////////////////////////////////////////////////////////////////////
    struct extract_Tags_type
        : public pybind::type_cast_result<Tags>
    {
        bool apply( pybind::kernel_interface * _kernel, PyObject * _obj, Tags & _tags, bool _nothrow ) override
        {
            (void)_kernel;
            (void)_nothrow;

            if( _kernel->list_check( _obj ) == true )
            {
                pybind::list l( _kernel, _obj, pybind::borrowed );

                uint32_t tags_size = l.size();

                for( uint32_t i = 0; i != tags_size; ++i )
                {
                    ConstString tag = l[i];

                    _tags.addTag( tag );
                }
            }
            else
            {
                return false;
            }

            return true;
        }

        PyObject * wrap( pybind::kernel_interface * _kernel, pybind::type_cast_result<Tags>::TCastRef _value ) override
        {
            (void)_kernel;

            const VectorConstString & tags = _value.getTags();

            PyObject * py_tags = _kernel->list_new( 0 );

            for( const ConstString & tag : tags )
            {
                PyObject * py_tag = pybind::ptr( _kernel, tag );

                _kernel->list_appenditem( py_tags, py_tag );

                _kernel->decref( py_tag );
            }

            return py_tags;
        }
    };
    //////////////////////////////////////////////////////////////////////////
    struct extract_String_type
        : public pybind::type_cast_result<String>
    {
    public:
        bool apply( pybind::kernel_interface * _kernel, PyObject * _obj, String & _value, bool _nothrow ) override
        {
            (void)_kernel;
            (void)_nothrow;

            if( _kernel->string_check( _obj ) == true )
            {
                uint32_t size;
                const String::value_type * string_char = _kernel->string_to_char_and_size( _obj, size );

                if( string_char == 0 )
                {
                    return false;
                }

                _value.assign( string_char, size );
            }
            else
            {
                return false;
            }

            return true;
        }

    public:
        PyObject * wrap( pybind::kernel_interface * _kernel, pybind::type_cast_result<String>::TCastRef _value ) override
        {
            (void)_kernel;

            const String::value_type * value_str = _value.c_str();
            String::size_type value_size = _value.size();

            PyObject * py_value = _kernel->string_from_char_size( value_str, (uint32_t)value_size );

            return py_value;
        }
    };
    //////////////////////////////////////////////////////////////////////////
    struct extract_WString_type
        : public pybind::type_cast_result<WString>
    {
    public:
        bool apply( pybind::kernel_interface * _kernel, PyObject * _obj, WString & _value, bool _nothrow ) override
        {
            (void)_kernel;
            (void)_nothrow;

            if( _kernel->unicode_check( _obj ) == true )
            {
                uint32_t size = 0;
                const WString::value_type * value_char = _kernel->unicode_to_wchar_and_size( _obj, size );

                if( value_char == nullptr )
                {
                    return false;
                }

                _value.assign( value_char, size );
            }
            else
            {
                return false;
            }

            return true;
        }

    public:
        PyObject * wrap( pybind::kernel_interface * _kernel, pybind::type_cast_result<WString>::TCastRef _value ) override
        {
            (void)_kernel;

            const WString::value_type * value_str = _value.c_str();
            WString::size_type value_size = _value.size();

            PyObject * py_value = _kernel->unicode_from_wchar_size( value_str, (uint32_t)value_size );

            return py_value;
        }
    };
    //////////////////////////////////////////////////////////////////////////
    HelperScriptEmbedding::HelperScriptEmbedding()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    HelperScriptEmbedding::~HelperScriptEmbedding()
    {
    }
    //////////////////////////////////////////////////////////////////////////
    bool HelperScriptEmbedding::embedding( pybind::kernel_interface * _kernel )
    {
        pybind::registration_type_cast<Blobject>(_kernel, new extract_Blobject_type);
        pybind::registration_type_cast<Tags>(_kernel, new extract_Tags_type);

        pybind::registration_stl_vector_type_cast<ResourceImage *, VectorResourceImage>(_kernel);
        pybind::registration_stl_vector_type_cast<HotSpotPolygon *, VectorHotSpotPolygons>(_kernel);

        pybind::registration_stl_map_type_cast<ConstString, WString, MapWParams>(_kernel);
        pybind::registration_stl_map_type_cast<ConstString, String, MapParams>(_kernel);

        pybind::registration_type_cast<String>(_kernel, new extract_String_type);
        pybind::registration_type_cast<WString>(_kernel, new extract_WString_type);

        pybind::registration_stl_vector_type_cast<String, Vector<String>>(_kernel);
        pybind::registration_stl_vector_type_cast<WString, Vector<WString>>(_kernel);

        HelperScriptMethod * helperScriptMethod = new FactorableUnique<HelperScriptMethod>();

        pybind::def_functor( _kernel, "filterpowf", helperScriptMethod, &HelperScriptMethod::filterpowf );

        pybind::def_functor( _kernel, "enumerator", helperScriptMethod, &HelperScriptMethod::mt_enumerator );

        pybind::def_functor( _kernel, "rand", helperScriptMethod, &HelperScriptMethod::mt_rand );
        pybind::def_functor( _kernel, "randf", helperScriptMethod, &HelperScriptMethod::mt_randf );
        pybind::def_functor( _kernel, "range_rand", helperScriptMethod, &HelperScriptMethod::mt_range_rand );
        pybind::def_functor( _kernel, "range_randf", helperScriptMethod, &HelperScriptMethod::mt_range_randf );
        pybind::def_functor( _kernel, "around_randf", helperScriptMethod, &HelperScriptMethod::mt_around_randf );
        pybind::def_functor( _kernel, "radius_randf", helperScriptMethod, &HelperScriptMethod::mt_radius_randf );

        pybind::def_functor( _kernel, "rounding", helperScriptMethod, &HelperScriptMethod::mt_rounding );
        pybind::def_functor( _kernel, "roundingf", helperScriptMethod, &HelperScriptMethod::mt_roundingf );

        pybind::def_functor( _kernel, "pi", helperScriptMethod, &HelperScriptMethod::mt_pi );
        pybind::def_functor( _kernel, "deg2rad", helperScriptMethod, &HelperScriptMethod::mt_deg2rad );
        pybind::def_functor( _kernel, "rad2deg", helperScriptMethod, &HelperScriptMethod::mt_rad2deg );

        pybind::def_functor( _kernel, "sqrtf", helperScriptMethod, &HelperScriptMethod::mt_sqrtf );
        pybind::def_functor( _kernel, "powf", helperScriptMethod, &HelperScriptMethod::mt_powf );
        pybind::def_functor( _kernel, "absf", helperScriptMethod, &HelperScriptMethod::mt_absf );
        pybind::def_functor( _kernel, "cosf", helperScriptMethod, &HelperScriptMethod::mt_cosf );
        pybind::def_functor( _kernel, "sinf", helperScriptMethod, &HelperScriptMethod::mt_sinf );
        pybind::def_functor( _kernel, "tanf", helperScriptMethod, &HelperScriptMethod::mt_tanf );
        pybind::def_functor( _kernel, "cosf_deg", helperScriptMethod, &HelperScriptMethod::mt_cosf_deg );
        pybind::def_functor( _kernel, "sinf_deg", helperScriptMethod, &HelperScriptMethod::mt_sinf_deg );
        pybind::def_functor( _kernel, "acosf", helperScriptMethod, &HelperScriptMethod::mt_acosf );
        pybind::def_functor( _kernel, "atanf", helperScriptMethod, &HelperScriptMethod::mt_atanf );
        pybind::def_functor( _kernel, "logf", helperScriptMethod, &HelperScriptMethod::mt_logf );
        pybind::def_functor( _kernel, "log10f", helperScriptMethod, &HelperScriptMethod::mt_log10f );
        pybind::def_functor( _kernel, "log10", helperScriptMethod, &HelperScriptMethod::mt_log10 );
        pybind::def_functor( _kernel, "fibo", helperScriptMethod, &HelperScriptMethod::mt_fibo );
        pybind::def_functor( _kernel, "fibo_bine", helperScriptMethod, &HelperScriptMethod::mt_fibo_bine );
        pybind::def_functor( _kernel, "dot_v2_v2", helperScriptMethod, &HelperScriptMethod::mt_dot_v2_v2 );
        pybind::def_functor( _kernel, "direction_v2_v2", helperScriptMethod, &HelperScriptMethod::mt_direction_v2_v2 );
        pybind::def_functor( _kernel, "direction_v3_v3", helperScriptMethod, &HelperScriptMethod::mt_direction_v3_v3 );
        pybind::def_functor( _kernel, "angle_from_v2_v2", helperScriptMethod, &HelperScriptMethod::mt_angle_from_v2_v2 );
        pybind::def_functor( _kernel, "norm_v2", helperScriptMethod, &HelperScriptMethod::mt_norm_v2 );
        pybind::def_functor( _kernel, "norm_v3", helperScriptMethod, &HelperScriptMethod::mt_norm_v3 );

        pybind::def_functor_kernel( _kernel, "angle_correct_interpolate_from_to", helperScriptMethod, &HelperScriptMethod::s_angle_correct_interpolate_from_to );
        pybind::def_functor( _kernel, "angle_between_two_vectors", helperScriptMethod, &HelperScriptMethod::s_angle_between_two_vectors );

        pybind::def_functor( _kernel, "projectionPointToLine", helperScriptMethod, &HelperScriptMethod::projectionPointToLine );

        pybind::def_functor( _kernel, "isPointInsidePolygon", helperScriptMethod, &HelperScriptMethod::s_isPointInsidePolygon );

        pybind::def_functor( _kernel, "getTime", helperScriptMethod, &HelperScriptMethod::s_getTime );
        pybind::def_functor( _kernel, "getTimeMs", helperScriptMethod, &HelperScriptMethod::s_getTimeMs );

        pybind::def_functor( _kernel, "getDate", helperScriptMethod, &HelperScriptMethod::s_getDate );

        pybind::def_functor( _kernel, "getTimeString", helperScriptMethod, &HelperScriptMethod::s_getTimeString );

        pybind::def_functor( _kernel, "getConfigBool", helperScriptMethod, &HelperScriptMethod::s_getConfigBool );
        pybind::def_functor( _kernel, "getConfigInt", helperScriptMethod, &HelperScriptMethod::s_getConfigInt );
        pybind::def_functor( _kernel, "getConfigUInt", helperScriptMethod, &HelperScriptMethod::s_getConfigUInt );
        pybind::def_functor( _kernel, "getConfigUInt64", helperScriptMethod, &HelperScriptMethod::s_getConfigUInt64 );
        pybind::def_functor( _kernel, "getConfigFloat", helperScriptMethod, &HelperScriptMethod::s_getConfigFloat );
        pybind::def_functor( _kernel, "getConfigString", helperScriptMethod, &HelperScriptMethod::s_getConfigString );

        pybind::def_functor( _kernel, "getAccounts", helperScriptMethod, &HelperScriptMethod::s_getAccounts );

        pybind::def_functor_kernel( _kernel, "getCurrentAccountUID", helperScriptMethod, &HelperScriptMethod::s_getCurrentAccountUID );

        pybind::def_functor_kernel( _kernel, "getSetting", helperScriptMethod, &HelperScriptMethod::s_getSetting );
        pybind::def_functor_kernel( _kernel, "getSettingBool", helperScriptMethod, &HelperScriptMethod::s_getSettingBool );
        pybind::def_functor_kernel( _kernel, "getSettingInt", helperScriptMethod, &HelperScriptMethod::s_getSettingInt );
        pybind::def_functor_kernel( _kernel, "getSettingUInt", helperScriptMethod, &HelperScriptMethod::s_getSettingUInt );
        pybind::def_functor_kernel( _kernel, "getSettingUInt64", helperScriptMethod, &HelperScriptMethod::s_getSettingUInt64 );
        pybind::def_functor_kernel( _kernel, "getSettingFloat", helperScriptMethod, &HelperScriptMethod::s_getSettingFloat );
        pybind::def_functor_kernel( _kernel, "getSettingStrings", helperScriptMethod, &HelperScriptMethod::s_getSettingStrings );
        pybind::def_functor( _kernel, "getSettingFloatDefault", helperScriptMethod, &HelperScriptMethod::s_getSettingFloatDefault );

        pybind::def_functor_kernel_args( _kernel, "addSetting", helperScriptMethod, &HelperScriptMethod::s_addSetting );
        pybind::def_functor( _kernel, "hasSetting", helperScriptMethod, &HelperScriptMethod::s_hasSetting );
        pybind::def_functor_kernel( _kernel, "changeSetting", helperScriptMethod, &HelperScriptMethod::s_changeSetting );
        pybind::def_functor( _kernel, "changeSettingBool", helperScriptMethod, &HelperScriptMethod::s_changeSettingBool );
        pybind::def_functor( _kernel, "changeSettingInt", helperScriptMethod, &HelperScriptMethod::s_changeSettingInt );
        pybind::def_functor( _kernel, "changeSettingUInt", helperScriptMethod, &HelperScriptMethod::s_changeSettingUInt );
        pybind::def_functor( _kernel, "changeSettingUInt64", helperScriptMethod, &HelperScriptMethod::s_changeSettingUInt64 );
        pybind::def_functor( _kernel, "changeSettingFloat", helperScriptMethod, &HelperScriptMethod::s_changeSettingFloat );
        pybind::def_functor( _kernel, "changeSettingStrings", helperScriptMethod, &HelperScriptMethod::s_changeSettingStrings );

        pybind::def_functor_kernel( _kernel, "getAccountUID", helperScriptMethod, &HelperScriptMethod::s_getAccountUID );

        pybind::def_functor_kernel( _kernel, "getAccountSetting", helperScriptMethod, &HelperScriptMethod::s_getAccountSetting );
        pybind::def_functor_kernel( _kernel, "getAccountSettingBool", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingBool );
        pybind::def_functor_kernel( _kernel, "getAccountSettingInt", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingInt );
        pybind::def_functor_kernel( _kernel, "getAccountSettingUInt", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingUInt );
        pybind::def_functor_kernel( _kernel, "getAccountSettingUInt64", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingUInt64 );
        pybind::def_functor_kernel( _kernel, "getAccountSettingFloat", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingFloat );
        pybind::def_functor_kernel( _kernel, "getAccountSettingStrings", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingStrings );
        pybind::def_functor( _kernel, "getAccountSettingFloatDefault", helperScriptMethod, &HelperScriptMethod::s_getAccountSettingFloatDefault );

        pybind::def_functor_kernel_args( _kernel, "addAccountSetting", helperScriptMethod, &HelperScriptMethod::s_addAccountSetting );
        pybind::def_functor( _kernel, "hasAccountSetting", helperScriptMethod, &HelperScriptMethod::s_hasAccountSetting );
        pybind::def_functor_kernel( _kernel, "changeAccountSetting", helperScriptMethod, &HelperScriptMethod::s_changeAccountSetting );
        pybind::def_functor( _kernel, "changeAccountSettingBool", helperScriptMethod, &HelperScriptMethod::s_changeAccountSettingBool );
        pybind::def_functor( _kernel, "changeAccountSettingInt", helperScriptMethod, &HelperScriptMethod::s_changeAccountSettingInt );
        pybind::def_functor( _kernel, "changeAccountSettingUInt", helperScriptMethod, &HelperScriptMethod::s_changeAccountSettingUInt );
        pybind::def_functor( _kernel, "changeAccountSettingUInt64", helperScriptMethod, &HelperScriptMethod::s_changeAccountSettingUInt64 );
        pybind::def_functor( _kernel, "changeAccountSettingFloat", helperScriptMethod, &HelperScriptMethod::s_changeAccountSettingFloat );
        pybind::def_functor( _kernel, "changeAccountSettingStrings", helperScriptMethod, &HelperScriptMethod::s_changeAccountSettingStrings );

        pybind::def_functor_kernel( _kernel, "getGlobalAccountUID", helperScriptMethod, &HelperScriptMethod::s_getGlobalAccountUID );

        pybind::def_functor_kernel( _kernel, "getGlobalSetting", helperScriptMethod, &HelperScriptMethod::s_getGlobalSetting );
        pybind::def_functor_kernel( _kernel, "getGlobalSettingBool", helperScriptMethod, &HelperScriptMethod::s_getGlobalSettingBool );
        pybind::def_functor_kernel( _kernel, "getGlobalSettingInt", helperScriptMethod, &HelperScriptMethod::s_getGlobalSettingInt );
        pybind::def_functor_kernel( _kernel, "getGlobalSettingUInt", helperScriptMethod, &HelperScriptMethod::s_getGlobalSettingUInt );
        pybind::def_functor_kernel( _kernel, "getGlobalSettingUInt64", helperScriptMethod, &HelperScriptMethod::s_getGlobalSettingUInt64 );
        pybind::def_functor_kernel( _kernel, "getGlobalSettingFloat", helperScriptMethod, &HelperScriptMethod::s_getGlobalSettingFloat );
        pybind::def_functor_kernel( _kernel, "getGlobalSettingStrings", helperScriptMethod, &HelperScriptMethod::s_getGlobalSettingStrings );

        pybind::def_functor_kernel_args( _kernel, "addGlobalSetting", helperScriptMethod, &HelperScriptMethod::s_addGlobalSetting );
        pybind::def_functor( _kernel, "hasGlobalSetting", helperScriptMethod, &HelperScriptMethod::s_hasGlobalSetting );
        pybind::def_functor_kernel( _kernel, "changeGlobalSetting", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSetting );
        pybind::def_functor( _kernel, "changeGlobalSettingBool", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSettingBool );
        pybind::def_functor( _kernel, "changeGlobalSettingInt", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSettingInt );
        pybind::def_functor( _kernel, "changeGlobalSettingUint", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSettingUInt );
        pybind::def_functor( _kernel, "changeGlobalSettingUInt64", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSettingUInt64 );
        pybind::def_functor( _kernel, "changeGlobalSettingFloat", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSettingFloat );
        pybind::def_functor( _kernel, "changeGlobalSettingStrings", helperScriptMethod, &HelperScriptMethod::s_changeGlobalSettingStrings );

        pybind::def_functor_kernel( _kernel, "createAccount", helperScriptMethod, &HelperScriptMethod::s_createAccount );
        pybind::def_functor_kernel( _kernel, "createGlobalAccount", helperScriptMethod, &HelperScriptMethod::s_createGlobalAccount );

        pybind::def_functor( _kernel, "selectAccount", helperScriptMethod, &HelperScriptMethod::s_selectAccount );
        pybind::def_functor( _kernel, "deleteAccount", helperScriptMethod, &HelperScriptMethod::s_deleteAccount );

        pybind::def_functor( _kernel, "saveAccount", helperScriptMethod, &HelperScriptMethod::s_saveAccount );
        pybind::def_functor( _kernel, "saveAccounts", helperScriptMethod, &HelperScriptMethod::s_saveAccounts );
        pybind::def_functor( _kernel, "saveAccountsInfo", helperScriptMethod, &HelperScriptMethod::s_saveAccountsInfo );
        pybind::def_functor( _kernel, "hasCurrentAccount", helperScriptMethod, &HelperScriptMethod::s_hasCurrentAccount );
        pybind::def_functor( _kernel, "getCurrentAccountName", helperScriptMethod, &HelperScriptMethod::s_getCurrentAccountName );

        pybind::def_functor( _kernel, "setGlobalAccount", helperScriptMethod, &HelperScriptMethod::s_setGlobalAccount );
        pybind::def_functor( _kernel, "hasGlobalAccount", helperScriptMethod, &HelperScriptMethod::s_hasGlobalAccount );
        pybind::def_functor( _kernel, "getGlobalAccountName", helperScriptMethod, &HelperScriptMethod::s_getGlobalAccountName );

        pybind::def_functor( _kernel, "setDefaultAccount", helperScriptMethod, &HelperScriptMethod::s_setDefaultAccount );
        pybind::def_functor( _kernel, "getDefaultAccount", helperScriptMethod, &HelperScriptMethod::s_getDefaultAccount );
        pybind::def_functor( _kernel, "hasDefaultAccount", helperScriptMethod, &HelperScriptMethod::s_hasDefaultAccount );
        pybind::def_functor( _kernel, "isCurrentDefaultAccount", helperScriptMethod, &HelperScriptMethod::s_isCurrentDefaultAccount );
        pybind::def_functor( _kernel, "selectDefaultAccount", helperScriptMethod, &HelperScriptMethod::s_selectDefaultAccount );

        pybind::def_functor_kernel( _kernel, "writeAccountPickleFile", helperScriptMethod, &HelperScriptMethod::s_writeAccountPickleFile );
        pybind::def_functor_kernel( _kernel, "loadAccountPickleFile", helperScriptMethod, &HelperScriptMethod::s_loadAccountPickleFile );
        pybind::def_functor( _kernel, "hasAccountPickleFile", helperScriptMethod, &HelperScriptMethod::s_hasAccountPickleFile );

        pybind::def_functor( _kernel, "getDefaultFileGroup", helperScriptMethod, &HelperScriptMethod::s_getDefaultFileGroup );
        pybind::def_functor( _kernel, "hasFileGroup", helperScriptMethod, &HelperScriptMethod::s_hasFileGroup );
        pybind::def_functor( _kernel, "getFileGroup", helperScriptMethod, &HelperScriptMethod::s_getFileGroup );

        pybind::def_functor( _kernel, "setParticlesEnabled", helperScriptMethod, &HelperScriptMethod::s_setParticlesEnabled );

        pybind::def_functor( _kernel, "getNodeScreenPosition", helperScriptMethod, &HelperScriptMethod::s_getNodeScreenPosition );

        pybind::def_functor( _kernel, "hasTextByKey", helperScriptMethod, &HelperScriptMethod::s_hasTextByKey );
        pybind::def_functor( _kernel, "getTextByKey", helperScriptMethod, &HelperScriptMethod::s_getTextByKey );
        pybind::def_functor( _kernel, "getTextCharCountByKey", helperScriptMethod, &HelperScriptMethod::s_getTextCharCountByKey );


        pybind::def_functor( _kernel, "setVSync", helperScriptMethod, &HelperScriptMethod::s_setVSync );
        pybind::def_functor( _kernel, "getVSync", helperScriptMethod, &HelperScriptMethod::s_getVSync );
        pybind::def_functor( _kernel, "setCursorMode", helperScriptMethod, &HelperScriptMethod::s_setCursorMode );
        pybind::def_functor( _kernel, "getCursorMode", helperScriptMethod, &HelperScriptMethod::s_getCursorMode );
        pybind::def_functor( _kernel, "setCursorIcon", helperScriptMethod, &HelperScriptMethod::s_setCursorIcon );

        pybind::def_functor( _kernel, "intersectsEllipseVsPoint", helperScriptMethod, &HelperScriptMethod::s_intersectsEllipseVsPoint );
        pybind::def_functor( _kernel, "intersectsBoxes", helperScriptMethod, &HelperScriptMethod::s_intersectsBoxes );

        pybind::def_functor_kernel( _kernel, "getPolygonPoints", helperScriptMethod, &HelperScriptMethod::s_getPolygonPoints );
        pybind::def_functor( _kernel, "intersectionPolygons", helperScriptMethod, &HelperScriptMethod::s_intersectionPolygons );
        pybind::def_functor( _kernel, "intersectsPolygons", helperScriptMethod, &HelperScriptMethod::s_intersectsPolygons );
        pybind::def_functor( _kernel, "intersectsPolygonsWM", helperScriptMethod, &HelperScriptMethod::s_intersectsPolygonsWM );
        pybind::def_functor( _kernel, "intersectsPolygonsWMP", helperScriptMethod, &HelperScriptMethod::s_intersectsPolygonsWMP );


        pybind::def_functor_kernel( _kernel, "objects", helperScriptMethod, &HelperScriptMethod::s_objects );
        pybind::def_functor_kernel( _kernel, "textures", helperScriptMethod, &HelperScriptMethod::s_textures );

        pybind::def_functor( _kernel, "watchdog", helperScriptMethod, &HelperScriptMethod::s_watchdog );

        pybind::def_functor( _kernel, "addGlobalModule", helperScriptMethod, &HelperScriptMethod::s_addGlobalModule );

        pybind::def_function( _kernel, "angle_norm", &mt::angle_norm );
        pybind::def_function( _kernel, "angle_delta_deg", &mt::angle_delta_deg );

        pybind::def_function( _kernel, "length_v2", &mt::length_v2 );

        pybind::def_function( _kernel, "length_v2_v2", &mt::length_v2_v2 );
        pybind::def_function( _kernel, "sqrlength_v2_v2", &mt::sqrlength_v2_v2 );

        pybind::def_function( _kernel, "length_v3_v3", &mt::length_v3_v3 );
        pybind::def_function( _kernel, "sqrlength_v3_v3", &mt::sqrlength_v3_v3 );

        pybind::def_function( _kernel, "length_v3", &mt::length_v3 );
        pybind::def_function( _kernel, "sqrlength_v3", &mt::sqrlength_v3 );

        pybind::def_function( _kernel, "signed_angle", &mt::signed_angle );
        pybind::def_function( _kernel, "angle_length", &mt::angle_length );
        pybind::def_function( _kernel, "perp_v2", &mt::perp );
        pybind::def_function_deprecate( _kernel, "perp", &mt::perp, "use perp_v2" );
        pybind::def_function( _kernel, "perp_left", &mt::perp_left );

        pybind::def_functor_kernel( _kernel, "getrefcount", helperScriptMethod, &HelperScriptMethod::s_refcount );
        pybind::def_functor_kernel( _kernel, "is_class", helperScriptMethod, &HelperScriptMethod::s_is_class );
        pybind::def_functor_kernel( _kernel, "is_type_class", helperScriptMethod, &HelperScriptMethod::s_is_type_class );
        pybind::def_functor_kernel( _kernel, "is_wrap", helperScriptMethod, &HelperScriptMethod::s_is_wrap );

        pybind::def_functor( _kernel, "getLanguagePack", helperScriptMethod, &HelperScriptMethod::s_getLanguagePack );

        pybind::def_functor( _kernel, "isometric_length_v3_v3", helperScriptMethod, &HelperScriptMethod::s_isometric_length_v3_v3 );
        pybind::def_functor( _kernel, "isometric_sqrlength_v3_v3", helperScriptMethod, &HelperScriptMethod::s_isometric_sqrlength_v3_v3 );

        pybind::def_functor( _kernel, "isValidWindowMode", helperScriptMethod, &HelperScriptMethod::s_isValidWindowMode );

        pybind::def_functor( _kernel, "utf8ToUnicode", helperScriptMethod, &HelperScriptMethod::s_utf8ToUnicode );
        pybind::def_functor( _kernel, "unicodeToUtf8", helperScriptMethod, &HelperScriptMethod::s_unicodeToUtf8 );
        pybind::def_functor( _kernel, "setCursorPosition", helperScriptMethod, &HelperScriptMethod::s_setCursorPosition );

        pybind::def_functor( _kernel, "debug", helperScriptMethod, &HelperScriptMethod::s_debug );
        pybind::def_functor( _kernel, "debugNode", helperScriptMethod, &HelperScriptMethod::s_debugNode );

        pybind::def_functor( _kernel, "hasGameData", helperScriptMethod, &HelperScriptMethod::s_hasGameData );
        pybind::def_functor_kernel( _kernel, "writeGameData", helperScriptMethod, &HelperScriptMethod::s_writeGameData );
        pybind::def_functor_kernel( _kernel, "loadGameData", helperScriptMethod, &HelperScriptMethod::s_loadGameData );


        pybind::def_functor( _kernel, "isAltDown", helperScriptMethod, &HelperScriptMethod::s_isAltDown );
        pybind::def_functor( _kernel, "isShiftDown", helperScriptMethod, &HelperScriptMethod::s_isShiftDown );
        pybind::def_functor( _kernel, "isCtrlDown", helperScriptMethod, &HelperScriptMethod::s_isCtrlDown );

        pybind::def_functor( _kernel, "isKeyDown", helperScriptMethod, &HelperScriptMethod::s_isKeyDown );
        pybind::def_functor( _kernel, "isExclusiveKeyDown", helperScriptMethod, &HelperScriptMethod::s_isExclusiveKeyDown );
        pybind::def_functor( _kernel, "isAnyKeyDown", helperScriptMethod, &HelperScriptMethod::s_isAnyKeyDown );

        pybind::def_functor( _kernel, "isAnyMouseButtonDown", helperScriptMethod, &HelperScriptMethod::s_isAnyMouseButtonDown );
        pybind::def_functor( _kernel, "isMouseButtonDown", helperScriptMethod, &HelperScriptMethod::s_isMouseButtonDown );

        pybind::def_functor( _kernel, "printChildren", helperScriptMethod, &HelperScriptMethod::s_printChildren );

        pybind::def_functor( _kernel, "getGroupResourcesMemoryUse", helperScriptMethod, &HelperScriptMethod::s_getGroupResourcesMemoryUse );

        pybind::def_functor( _kernel, "intersectPathVsCircle", helperScriptMethod, &HelperScriptMethod::s_intersectPathVsCircle );
        pybind::def_functor_kernel( _kernel, "selectRandomPointFromPathRadius", helperScriptMethod, &HelperScriptMethod::s_selectRandomPointFromPathRadius );

        pybind::def_functor_kernel( _kernel, "reloadModule", helperScriptMethod, &HelperScriptMethod::s_reloadModule );

        pybind::def_functor( _kernel, "makeUID", helperScriptMethod, &HelperScriptMethod::s_makeUID );

        pybind::def_functor( _kernel, "getTextFromID", helperScriptMethod, &HelperScriptMethod::s_getTextFromID );

        pybind::def_functor( _kernel, "setTextAlias", helperScriptMethod, &HelperScriptMethod::s_setTextAlias );
        pybind::def_functor( _kernel, "removeTextAlias", helperScriptMethod, &HelperScriptMethod::s_removeTextAlias );
        pybind::def_functor( _kernel, "hasTextAlias", helperScriptMethod, &HelperScriptMethod::s_hasTextAlias );
        pybind::def_functor( _kernel, "getTextAlias", helperScriptMethod, &HelperScriptMethod::s_getTextAlias );

        pybind::def_functor_args( _kernel, "setTextAliasArguments", helperScriptMethod, &HelperScriptMethod::s_setTextAliasArguments );
        pybind::def_functor( _kernel, "removeTextAliasArguments", helperScriptMethod, &HelperScriptMethod::s_removeTextAliasArguments );

        pybind::def_functor( _kernel, "getJoystickAxis", helperScriptMethod, &HelperScriptMethod::s_getJoystickAxis );

        pybind::def_functor_kernel_args( _kernel, "addChronometer", helperScriptMethod, &HelperScriptMethod::s_addChronometer );
        pybind::def_functor( _kernel, "removeChronometer", helperScriptMethod, &HelperScriptMethod::s_removeChronometer );

        pybind::def_functor( _kernel, "getHotSpotPolygonBoundingBox", helperScriptMethod, &HelperScriptMethod::s_getHotSpotPolygonBoundingBox );

        m_implement = helperScriptMethod;

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    void HelperScriptEmbedding::ejecting()
    {
        m_implement = nullptr;

        pybind::kernel_interface * kernel = SCRIPT_SERVICE()
            ->getKernel();

        pybind::unregistration_type_cast<Blobject>(kernel);
        pybind::unregistration_type_cast<Tags>(kernel);

        pybind::unregistration_stl_vector_type_cast<ResourceImage *, VectorResourceImage>(kernel);
        pybind::unregistration_stl_vector_type_cast<HotSpotPolygon *, VectorHotSpotPolygons>(kernel);

        pybind::unregistration_stl_map_type_cast<ConstString, WString, MapWParams>(kernel);
        pybind::unregistration_stl_map_type_cast<ConstString, String, MapParams>(kernel);

        pybind::unregistration_type_cast<String>(kernel);
        pybind::unregistration_type_cast<WString>(kernel);

        pybind::unregistration_stl_vector_type_cast<String, Vector<String>>(kernel);
        pybind::unregistration_stl_vector_type_cast<WString, Vector<WString>>(kernel);
    }
}
