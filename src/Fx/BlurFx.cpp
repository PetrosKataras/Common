#include "Fx/BlurFx.h"
#include "cinder/Log.h"
#include "cinder/app/AppBase.h"

using namespace ci;

BlurFx::BlurFx()
{
}

void BlurFx::createBatch( const ci::Rectf bounds )
{
	DataSourceRef blurVert = app::loadAsset( "common_shaders/pass_through.vert" );
	DataSourceRef blurFrag = app::loadAsset( "common_shaders/post/blur.frag" );
	const auto blurGlsl = gl::GlslProg::create( 
		gl::GlslProg::Format()
		.version( 330 )
		.vertex( blurVert )
		.fragment( blurFrag )
	);
	blurGlsl->uniform( "uResolution", ci::vec2( bounds.getSize() ) );
	const auto blurRect = gl::VboMesh::create( geom::Rect( bounds ) );
	mBlurBatch = gl::Batch::create( blurRect, blurGlsl );
	
	auto fmt = gl::Fbo::Format();
	auto texFmt = gl::Texture2d::Format();
	texFmt.setMinFilter( GL_LINEAR );
	texFmt.setMagFilter( GL_LINEAR );
	fmt.colorTexture( texFmt );
	
	mFboBlur[0] = ci::gl::Fbo::create( bounds.getWidth(), bounds.getHeight(), fmt );
	mFboBlur[1] = ci::gl::Fbo::create( bounds.getWidth(), bounds.getHeight(), fmt );
}

void BlurFx::update()
{
#if defined( USE_CINDER_IMGUI )
    ui::ScopedWindow window( "BlurFx" );
    //ui::SliderFloat( "FallOfff", &mFallOff, 0.0f, 1.0f, "%.05f" );
    ui::Separator();
    ui::Checkbox( "Enable", &mEnabled );
#endif
}

void BlurFx::apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const gl::TextureRef depthTexture, CameraPersp* camera )
{
	if( ! mBlurBatch )
		createBatch( writeFbo->getBounds() );

	size_t iterations = 8;
	for( size_t i = 0; i < iterations; i++ ) {
		auto radius = ( iterations - i - 1 ) * mIntensity;
		{ // HORIZONTAL BLUR
			const gl::ScopedFramebuffer scopedBlurBuffer( mFboBlur[0] );	
			gl::clear( ColorA( 0, 0, 0, 0 ) );
			const gl::ScopedViewport scopedViewport( mFboBlur[0]->getSize() );
			const gl::ScopedMatrices scopedMatrices;
			gl::setMatricesWindow( mFboBlur[0]->getSize() );
			gl::ScopedColor scopedColor( 1, 1, 1 );
			gl::ScopedTextureBind scopedTextureColor( i == 0 ? readFbo->getColorTexture() : mFboBlur[1]->getColorTexture(), 0 );
			mBlurBatch->getGlslProg()->uniform( "uOffset", ci::vec2( 0.f ) );
			mBlurBatch->getGlslProg()->uniform( "uDims", ci::vec2( mFboBlur[0]->getSize() ) );
			mBlurBatch->getGlslProg()->uniform( "uDirection", i % 2 == 0 ? ci::vec2( radius, 0.f ) : ci::vec2( 0.f, radius ) );
			mBlurBatch->draw();
		}
		std::swap( mFboBlur[0], mFboBlur[1] );
	}
	{ // render final result for output
		const gl::ScopedFramebuffer scopedWriteBuffer( writeFbo );
		ci::gl::clear( ci::ColorAf( 0.f, 0.f, 0.f, 0.f ) );
		const gl::ScopedViewport scopedViewport( ivec2( 0 ), writeFbo->getSize() );
		const gl::ScopedMatrices scopedMatrices;
		gl::setMatricesWindow( writeFbo->getSize() );
		ci::gl::draw( mFboBlur[1]->getColorTexture() );
	}
}

