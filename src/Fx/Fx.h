#pragma once
#include "cinder/gl/gl.h"
class Fx { 
public:
	Fx(){}
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
