#pragma once
#include "cinder/gl/gl.h"
#include "cinder/app/AppBase.h"

#if defined( USE_CINDER_IMGUI )
    #include "cinder/CinderImGui.h"
	namespace ui = ImGui;
#endif

class Fx { 
public:
	Fx(){
	#if defined( USE_CINDER_IMGUI )
		// it's a no-op if already initialized
		ui::Initialize( ui::Options().iniPath( ci::app::getAssetPath( "imgui.ini" ) ) );
	#endif
	}
	virtual ~Fx(){}
	virtual void update() {};
	virtual void apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture = nullptr, ci::CameraPersp* camera = nullptr ) = 0;
	virtual Fx& enable( bool enable = true ) { mEnabled = enable; return *this; }
	virtual const bool enabled() const { return mEnabled; }
protected:
	virtual void createBatch( const ci::Rectf bounds ) = 0;
protected:
	bool mEnabled{ false };
private:
};
using FxRef = std::shared_ptr<Fx>;
