#include "Fx/FXAAFx.h"
#include "cinder/app/AppBase.h"

using namespace ci;

FXAAFx::FXAAFx()
{
}

void FXAAFx::update()
{
#if defined( USE_CINDER_IMGUI )
    ui::ScopedWindow window( "FXAA" );
    ui::Checkbox( "Enable", &mEnabled );
#endif
}

void FXAAFx::createBatch( const ci::Rectf bounds )
{
	DataSourceRef fxaaVert = app::loadAsset( "common_haders/pass_through.vert" );
	DataSourceRef fxaaFrag = app::loadAsset( "common_shaders/post/fxaa.frag" );
	const auto fxaaGlsl = gl::GlslProg::create( 
		gl::GlslProg::Format()
		.vertex( fxaaVert )
		.fragment( fxaaFrag )
	);
	
	const auto fxaaRec = gl::VboMesh::create( geom::Rect( bounds ) );
	mFXAABatch = gl::Batch::create( fxaaRec, fxaaGlsl );
}

void FXAAFx::apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture, ci::CameraPersp* camera ){
	if( ! mFXAABatch )
		createBatch( writeFbo->getBounds() );
	
	const gl::ScopedFramebuffer scopedReadBuffer( writeFbo );
	ci::gl::clear( ci::ColorAf( 0.0f, 0.f, 0.f, 0.f ));
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), writeFbo->getSize() );
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow( writeFbo->getSize() );
	const gl::ScopedTextureBind scopedTexture( readFbo->getColorTexture(), 0 );
	mFXAABatch->getGlslProg()->uniform( "uTexture", 0 );
	mFXAABatch->getGlslProg()->uniform( "uDims", ci::vec2( writeFbo->getSize() ) );
	mFXAABatch->getGlslProg()->uniform( "uRcpBufferSize", vec2( 1.0f / vec2( writeFbo->getSize() ) ) );
	mFXAABatch->draw();
}
