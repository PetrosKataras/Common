#include "Fx/FogFx.h"
#include "cinder/app/AppBase.h"
#include "cinder/Log.h"

using namespace ci;

FogFx::FogFx()
{
}

void FogFx::createBatch( const ci::Rectf bounds )
{
	DataSourceRef fogVert = app::loadAsset( "common_shaders/pass_through.vert" );
	DataSourceRef fogFrag = app::loadAsset( "common_shaders/post/fog.frag" );
	const auto fogGlsl = gl::GlslProg::create( 
		gl::GlslProg::Format()
		.version( 330 )
		.vertex( fogVert )
		.fragment( fogFrag )
	);
	const auto fogRect = gl::VboMesh::create( geom::Rect( bounds ) );
	mFogBatch = gl::Batch::create( fogRect, fogGlsl );
}

void FogFx::update()
{
#if defined( USE_CINDER_IMGUI )
    ui::ScopedWindow window( "FogFx" );
    ui::SliderFloat( "Density", &mDensity, 0.0f, 1.0f, "%.05f" );
    ui::Separator();
	ui::ColorEdit3( "Fog Color", &mFogColor[0] );
    ui::Separator();
    ui::Checkbox( "Enable", &mEnabled );
#endif

}

void FogFx::apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture, CameraPersp* camera )
{
	if( ! mFogBatch )
		createBatch( writeFbo->getBounds() );

	if( ! depthTexture )
		CI_LOG_W( "Required depth texture in null - Fog will not work!" );
	const gl::ScopedFramebuffer scopedWriteBuffer( writeFbo );
	ci::gl::clear( ci::Color::white() );
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), writeFbo->getSize() );
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow( writeFbo->getSize() );
	const gl::ScopedTextureBind scopedTextureColor( readFbo->getColorTexture(), 0 );
	mFogBatch->getGlslProg()->uniform( "uDims", ci::vec2( readFbo->getColorTexture()->getSize() ) );
	mFogBatch->getGlslProg()->uniform( "uFogColor", mFogColor );
	mFogBatch->getGlslProg()->uniform( "uDensity", mDensity );
	mFogBatch->getGlslProg()->uniform( "uSamplerColor", 0 );
	mFogBatch->draw();
}
