#pragma once

#include "Fx/Fx.h"
#include "cinder/Color.h"

class FogFx : public Fx {
public:
	FogFx();
	virtual ~FogFx(){}
	virtual void update() final;
	virtual void apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture = nullptr, ci::CameraPersp* camera = nullptr ) final;
protected:
	virtual void createBatch( const ci::Rectf bounds ) final;
private:
	ci::gl::BatchRef mFogBatch;
	ci::Colorf mFogColor{ ci::Colorf( 1.0f, 1.0f, 1.f ) };
	float mDensity{ .17f };
};
