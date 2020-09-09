#pragma once

#include "Fx/Fx.h"

class ColorCorrectionFx : public Fx {
public:
	ColorCorrectionFx();
	virtual ~ColorCorrectionFx(){}
	virtual void update() final;
	virtual void apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture = nullptr, ci::CameraPersp* camera = nullptr ) final;
protected:
	virtual void createBatch( const ci::Rectf bounds ) final;
private:
	ci::gl::BatchRef mCCBatch;
	float mBlend{ .65f };
	float mBrightness{ 0.5f };
	float mChroma{ 0.000f };
	float mContrast{ 0.3f };
	float mExposure{ 1.5f };
	ci::vec3 mSaturation{ 1.03f, 1.f, 1.06f };
};
