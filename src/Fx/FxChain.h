#pragma once

#include "Fx/Fx.h"

using FxMap = std::map<std::string, FxRef>;
class FxChain {
public:
	FxChain();
	FxChain& chain( FxRef fx, const std::string fxName );	
	void update();
	ci::gl::TextureRef process( const ci::gl::TextureRef colorTexture, const ci::gl::TextureRef depthTexture = nullptr, ci::CameraPersp* camera = nullptr );
	void enable( const std::string fxName );
	void disable( const std::string fxName );
	FxRef getFx( const std::string fxName );
private:
	void createPingPongFbos( const ci::vec2 size );
	void fillReadFbo( const ci::gl::TextureRef frame );
private:
	FxMap 					mFxMap;
	ci::gl::FboRef			mPingPongFbo[2];
	ci::gl::BatchRef		mBatch;
	size_t ping = 0;
	size_t pong = 1;
};
