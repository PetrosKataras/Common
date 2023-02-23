#pragma once

#include "Fx/Fx.h"

class BlurFx : public Fx {
public:
	BlurFx();
	virtual ~BlurFx(){}
	virtual void update() final;
	virtual void apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture = nullptr, ci::CameraPersp* camera = nullptr ) final;
protected:
	virtual void createBatch( const ci::Rectf bounds ) final;
private:
	ci::gl::BatchRef mBlurBatch;
	ci::gl::FboRef mFboBlur[2];
};
