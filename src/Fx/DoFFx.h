#pragma once

#include "Fx/Fx.h"

class DoFFx : public Fx {
public:
	DoFFx();
	virtual ~DoFFx(){}
	virtual void update() final;
	virtual void apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture = nullptr, ci::CameraPersp* camera = nullptr ) final;
	void setFocus( const float focus );
	void setMaxCoCRadius( const int cocRadius );
protected:
	virtual void createBatch( const ci::Rectf bounds ) final;
private:
	ci::gl::FboRef mFboBlur[2];
	ci::gl::GlslProgRef mGlslBlur[2];
	ci::gl::GlslProgRef mGlslComposite;
	float mAperture{ 1.0f };
	int mMaxCoCRadiusPixels{ 7 };
	float mFocalDistance{ 4.f };
	float mFocalLength{ 1.0f };
	float mFocalPlane{ 35.0f };
	float mFocus{ 4.55f };
	float mZnear{ 0.05f };
	float mZfar{ 100.f };
	int mFocalStop{ 5 };
	int mDebugOption{ 0 };
	float mFarRadiusRescale{ 4.0f };
	ci::gl::BatchRef mDoFBatch;
};
