#	pragma once

#	include "Interface/ServiceInterface.h"

#	include "Utils/Core/ConstString.h"

namespace Menge
{
	class AmplifierInterface
		: public ServiceInterface
	{
        SERVICE_DECLARE("Amplifier")

	public:
		virtual bool playTrack( const ConstString& _playlistResource, size_t _index, float _pos, bool _looped ) = 0;
		
		virtual const ConstString & getPlayTrack() const = 0;
		virtual size_t getCurrentTrack() const = 0;
		virtual size_t getNumTracks() const = 0;

		virtual void shuffle( const ConstString& _playlist ) = 0;

		virtual void stop() = 0;
		virtual void pause() = 0;
		virtual void resume() = 0;
				
		virtual void setVolume( float _value ) = 0;
		virtual float getVolume() const = 0;
		
		//virtual void onTurnSound( bool _turn ) = 0;

		virtual void setPosMs( float _posMs ) = 0;
		virtual float getPosMs() const = 0;
	};

#   define AMPLIFIER_SERVICE( serviceProvider )\
    SERVICE_GET(serviceProvider, Menge::AmplifierInterface)
}