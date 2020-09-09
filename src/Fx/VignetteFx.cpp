#include "Fx/VignetteFx.h"
#include "cinder/app/AppBase.h"

#if defined( USE_CINDER_IMGUI )
    #include "CinderImGui.h"
#endif

using namespace ci;

VignetteFx::VignetteFx()
{
}

void VignetteFx::createBatch( const ci::Rectf bounds )
{
	DataSourceRef vignetteVert = app::loadAsset( "common_shaders/pass_through.vert" );
	DataSourceRef vignetteFrag = app::loadAsset( "common_shaders/post/vignette.frag" );
	const auto vignetteGlsl = gl::GlslProg::create( 
		gl::GlslProg::Format()
		.version( 330 )
		.vertex( vignetteVert )
		.fragment( vignetteFrag )
	);
	const auto vignetteRect = gl::VboMesh::create( geom::Rect( bounds ) );
	mVignetteBatch = gl::Batch::create( vignetteRect, vignetteGlsl );
}

void VignetteFx::update()
{
#if defined( USE_CINDER_IMGUI )
    ui::ScopedWindow window( "VignetteFx" );
    ui::SliderFloat( "FallOfff", &mFallOff, 0.0f, 1.0f, "%.05f" );
    ui::Separator();
    ui::Checkbox( "Enable", &mEnabled );
#endif
}

void VignetteFx::apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const gl::TextureRef depthTexture, CameraPersp* camera )
{
	if( ! mVignetteBatch )
		createBatch( writeFbo->getBounds() );

	const gl::ScopedFramebuffer scopedReadBuffer( writeFbo );
	ci::gl::clear( ci::Color::white() );
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), writeFbo->getSize() );
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow( writeFbo->getSize() );
	const gl::ScopedTextureBind scopedTexture( readFbo->getColorTexture(), 0 );
	mVignetteBatch->getGlslProg()->uniform( "uTexture", 0 );
	mVignetteBatch->getGlslProg()->uniform( "uDims", ci::vec2( writeFbo->getSize() ) );
	mVignetteBatch->getGlslProg()->uniform( "FallOff", mFallOff );
	mVignetteBatch->draw();
}

