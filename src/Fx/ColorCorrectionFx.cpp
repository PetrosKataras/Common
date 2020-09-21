#include "Fx/ColorCorrectionFx.h"
#include "cinder/app/AppBase.h"

using namespace ci;

ColorCorrectionFx::ColorCorrectionFx()
{
}

void ColorCorrectionFx::update()
{
#if defined( USE_CINDER_IMGUI )
    ui::ScopedWindow window( "ColorCorrectionFx" );
    ui::DragFloat( "Blend", &mBlend, .01f, 0.0f, 1.0f );
    ui::DragFloat( "Brightness", &mBrightness, 0.01f, 0.0, 1.0f );
    ui::DragFloat( "Chroma", &mChroma, .001, 0.0001f, 0.1f );
    ui::DragFloat( "Contrast", &mContrast, 0.01f, 0.f, 2.0f );
    ui::DragFloat( "Exposure", &mExposure, 0.01f, 0.f, 3.0f );
	ui::DragFloat3( "Saturation", &mSaturation[0], .01f, 0.0f, 2.0f );
    ui::Checkbox( "Enable", &mEnabled );
#endif
}

void ColorCorrectionFx::apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture, ci::CameraPersp* camera )
{
	if( ! mCCBatch )
		createBatch( writeFbo->getBounds() );	

	const gl::ScopedFramebuffer scopedReadBuffer( writeFbo );
	ci::gl::clear( ci::Color::white() );
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), writeFbo->getSize() );
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow( writeFbo->getSize() );
	const gl::ScopedTextureBind scopedTexture( readFbo->getColorTexture(), 0 );
	mCCBatch->getGlslProg()->uniform( "uTexture", 0 );
	mCCBatch->getGlslProg()->uniform( "uDims", ci::vec2( writeFbo->getSize() ) );
	mCCBatch->getGlslProg()->uniform( "uBlend", mBlend );
	mCCBatch->getGlslProg()->uniform( "uBrightness", mBrightness );
	mCCBatch->getGlslProg()->uniform( "uChromaticAberration", mChroma );
	mCCBatch->getGlslProg()->uniform( "uContrast", mContrast );
	mCCBatch->getGlslProg()->uniform( "uExposure", mExposure );
	mCCBatch->getGlslProg()->uniform( "uSaturation", mSaturation );
	mCCBatch->draw();
}

void ColorCorrectionFx::createBatch( const ci::Rectf bounds )
{
	DataSourceRef ccVert = app::loadAsset( "common_shaders/pass_through.vert" );
	DataSourceRef ccFrag = app::loadAsset( "common_shaders/post/color_correction.frag" );
	const auto ccGlsl = gl::GlslProg::create( 
		gl::GlslProg::Format()
		.version( 330 )
		.vertex( ccVert )
		.fragment( ccFrag )
	);
		const auto ccRec = gl::VboMesh::create( geom::Rect( bounds ) );
	mCCBatch = gl::Batch::create( ccRec, ccGlsl );
}
