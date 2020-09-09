#include "Fx/DoFFx.h"
#include "cinder/app/AppBase.h"
#include "cinder/Log.h"
#include "cinder/Camera.h"
#if defined( USE_CINDER_IMGUI )
    #include "CinderImGui.h"
#endif

using namespace ci;

DoFFx::DoFFx()
{
}

void DoFFx::update()
{
#if defined( USE_CINDER_IMGUI )
    ui::ScopedWindow window( "DoFFx" );
    ui::Checkbox( "Enable", &mEnabled );
	ui::DragFloat( "Focus", &mFocus, .05f, 0.1, 2000, "%0.5f" );
	ui::SliderInt( "Max CoC Radius", &mMaxCoCRadiusPixels, 1, 20 );
	ui::DragFloat( "Far scale", &mFarRadiusRescale, .05f, 1.0, 2000, "%0.5f" );
	static const std::vector<std::string> fstops{ "0.7f", "0.8f", "1.0f", "1.2f", "1.4f", "1.7f", "2.0f", "2.4f", "2.8f", "3.3f", "4.0f", "4.8f", "5.6f", "6.7f", "8.0f", "9.5f", "11.0f", "16.0f", "22.0f", "25.0f", "29.f", "34.f", "40.f", "60.f", "80.f", "100.f", "50.f" };
	static int sSelectedFStop = 14;
	if( ui::Combo( "F-Stops", &sSelectedFStop, fstops ) ) {
		mAperture = mFocalLength / std::stof( fstops[sSelectedFStop] );
	}
	static const std::vector<std::string> sDebugOptions{ "off", "Show CoC", "Show Region", "Show Near", "Show Blurry", "Show Input", "Show Mid & Far", "Show Signed CoC" };
	static int sSelectedDebugOption = mDebugOption;
	if( ui::Combo( "Debug option", &sSelectedDebugOption, sDebugOptions ) ) {
		mDebugOption = sSelectedDebugOption;;
	}
#endif
}

void DoFFx::createBatch( const ci::Rectf bounds )
{
	int width = bounds.getWidth();
#if ! defined( RUN_PRODUCTION )
	width >>= 2;
#endif
	CI_LOG_I( width );
	int height = bounds.getHeight();
	// Create blur fbos
	auto createBlurFbo = [&]( bool horizontal ) -> gl::FboRef {
#if ! defined( RUN_PRODUCTION )
		if( ! horizontal ) height >>= 2;
#endif
		auto fmt = ci::gl::Fbo::Format();
		fmt.attachment( GL_COLOR_ATTACHMENT0, ci::gl::Texture2d::create( width, height, ci::gl::Texture2d::Format().internalFormat( GL_RGBA16F ) ) );	
		fmt.attachment( GL_COLOR_ATTACHMENT1, ci::gl::Texture2d::create( width, height, ci::gl::Texture2d::Format().internalFormat( GL_RGBA16F ) ) );	
		return ci::gl::Fbo::create( width, height, fmt );
	};	
	mFboBlur[0] = createBlurFbo( true );
	mFboBlur[1] = createBlurFbo( false );
	// Create blur glsl progs
	auto createBlurGlslProgs = [] ( bool horizontal ) -> gl::GlslProgRef {
		try {
			DataSourceRef vert = app::loadAsset( "common_shaders/pass_through.vert" );
			DataSourceRef frag = app::loadAsset( "common_shaders/post/dof_blur.frag" );
			auto fmt = ci::gl::GlslProg::Format()
				.vertex(  vert )
				.fragment( frag )
#if defined( RUN_PRODUCTION )
				.define( "RUN_PRODUCTION", "1" )
#else
				.define( "RUN_PRODUCTION", "0" )
#endif
				.define( "HORIZONTAL", horizontal ? "1" : "0" );
			return gl::GlslProg::create( fmt );	
		}
		catch( const std::exception& e ) {
			CI_LOG_EXCEPTION( "Failed to load shader with exception: " << e.what(), e );
			return nullptr;
		}
	};
	mGlslBlur[0] = createBlurGlslProgs( true );
	mGlslBlur[1] = createBlurGlslProgs( false );
	// Create composite glsl
	try {
		DataSourceRef vert = app::loadAsset( "common_shaders/pass_through.vert" );
		DataSourceRef frag = app::loadAsset( "common_shaders/post/dof_composite.frag" );
		auto fmt = ci::gl::GlslProg::Format()
			.vertex(  vert )
			.fragment( frag );
		mGlslComposite = gl::GlslProg::create( fmt );	
	}
	catch( const std::exception& e ) {
		CI_LOG_EXCEPTION( "Failed to load shader with exception: " << e.what(), e );
	}
	// Create batch with default glsl which will be replace with one of the above
	// during rendering.
	const auto dofRect = gl::VboMesh::create( geom::Rect( bounds ) );
	mDoFBatch = gl::Batch::create( dofRect, gl::getStockShader( gl::ShaderDef() ) );
}

void DoFFx::apply( ci::gl::FboRef readFbo, ci::gl::FboRef writeFbo, const ci::gl::TextureRef depthTexture, CameraPersp* camera )
{
	if( ! mDoFBatch )
		createBatch( writeFbo->getBounds() );

	if( camera ) {
		mZnear = camera->getNearClip();
		mZfar = camera->getFarClip();
		mFocalPlane = exp( glm::mix( ::log( mFocalPlane ), ::log( mFocus ), 0.2f ) );
		mFocalLength = camera->getFocalLength();
	}
	{ // HORIZONTAL BLUR
		const gl::ScopedFramebuffer scopedBlurBuffer( mFboBlur[0] );	
		gl::clear( ColorA( 0, 0, 0, 0 ) );
		const gl::ScopedViewport scopedViewport( mFboBlur[0]->getSize() );
		const gl::ScopedMatrices scopedMatrices;
		gl::setMatricesWindow( mFboBlur[0]->getSize() );
		gl::ScopedColor scopedColor( 1, 1, 1 );
		gl::ScopedBlendPremult scopedBlend;
		gl::ScopedGlslProg scopedBlurHor( mGlslBlur[0] );
		gl::ScopedTextureBind scopedTextureColor( readFbo->getColorTexture(), 0 );
		gl::ScopedTextureBind scopedTextureDepth( depthTexture, 1 );
		mGlslBlur[0]->uniform( "uBlurSource", 0 );
		mGlslBlur[0]->uniform( "uDepthSource", 1 );
		mGlslBlur[0]->uniform( "uAperture", mAperture );
		mGlslBlur[0]->uniform( "uFocalPlane", mFocalPlane );
		mGlslBlur[0]->uniform( "uFocalLength", mFocalLength );
		mGlslBlur[0]->uniform( "uZnear", mZnear );
		mGlslBlur[0]->uniform( "uZfar", mZfar );
		mGlslBlur[0]->uniform( "uMaxCoCRadiusPixels", mMaxCoCRadiusPixels );
		mGlslBlur[0]->uniform( "uNearBlurRadiusPixels", mMaxCoCRadiusPixels );
		mGlslBlur[0]->uniform( "uInvNearBlurRadiusPixels", 1.f / mMaxCoCRadiusPixels );
		mGlslComposite->uniform( "uInputSourceInvSize", 1.f / vec2( readFbo->getSize() ) );	
		mDoFBatch->replaceGlslProg( mGlslBlur[0] );
		mDoFBatch->draw();
	}
	{ // VERTICAL BLUR
		const gl::ScopedFramebuffer scopedBlurBuffer( mFboBlur[1] );	
		gl::clear( ColorA( 0, 0, 0, 0 ) );
		const gl::ScopedViewport scopedViewport( ivec2( 0 ), mFboBlur[1]->getSize() );
		const gl::ScopedMatrices scopedMatrices;
		gl::setMatricesWindow( mFboBlur[1]->getSize() );
		gl::ScopedColor scopedColor( 1, 1, 1 );
		gl::ScopedBlendPremult scopedBlend;
		gl::ScopedGlslProg scopedBlurVert( mGlslBlur[1] );
		gl::ScopedTextureBind scopedNear( mFboBlur[0]->getTexture2d( GL_COLOR_ATTACHMENT0 ), 0 );
		gl::ScopedTextureBind scopedTextureBlur( mFboBlur[0]->getTexture2d( GL_COLOR_ATTACHMENT1 ), 1 );
		gl::ScopedTextureBind scopedTextureDepth( depthTexture, 2 );
		mGlslBlur[1]->uniform( "uNearSource", 0 );
		mGlslBlur[1]->uniform( "uBlurSource", 1 );
		mGlslBlur[1]->uniform( "uDepthSource", 2 );
		mGlslBlur[1]->uniform( "uAperture", mAperture );
		mGlslBlur[1]->uniform( "uFocalPlane", mFocalPlane );
		mGlslBlur[1]->uniform( "uFocalLength", mFocalLength );
		mGlslBlur[1]->uniform( "uZnear", mZnear );
		mGlslBlur[1]->uniform( "uZfar", mZfar );
		mGlslBlur[1]->uniform( "uMaxCoCRadiusPixels", mMaxCoCRadiusPixels );
		mGlslBlur[1]->uniform( "uNearBlurRadiusPixels", mMaxCoCRadiusPixels );
		mGlslComposite->uniform( "uInputSourceInvSize", 1.f / vec2( readFbo->getSize() ) );	
		mDoFBatch->replaceGlslProg( mGlslBlur[1] );
		mDoFBatch->draw();
	}
	const gl::ScopedFramebuffer scopedWriteBuffer( writeFbo );	
	gl::clear( Color::white() );
	const gl::ScopedViewport scopedViewport( ivec2( 0 ), writeFbo->getSize() );
	const gl::ScopedMatrices scopedMatrices;
	gl::setMatricesWindow( writeFbo->getSize() );
#if 0
	gl::ScopedTextureBind scopedTex( mFboBlur[1]->getTexture2d( GL_COLOR_ATTACHMENT1 ) );
	gl::ScopedGlslProg shaderScp( gl::getStockShader( gl::ShaderDef().color().texture() ) );
	gl::ScopedColor scpColor( Color::white() );
	gl::drawSolidRect( writeFbo->getBounds() );
#endif
	{
		{ // COMPOSITE
			gl::ScopedColor scopedColor( 1, 1, 1 );
			gl::ScopedBlend scopedBlend( false );
			gl::ScopedGlslProg scopedComposite( mGlslComposite );
			gl::ScopedTextureBind scopedTextureColor( readFbo->getColorTexture(), 0 );
			gl::ScopedTextureBind scopedTextureBlur1( mFboBlur[1]->getTexture2d( GL_COLOR_ATTACHMENT0 ), 1 );
			gl::ScopedTextureBind scopedTextureBlur2( mFboBlur[1]->getTexture2d( GL_COLOR_ATTACHMENT1 ), 2 );
			mGlslComposite->uniform( "uInputSource", 0 );
			mGlslComposite->uniform( "uNearSource", 1 );	
			mGlslComposite->uniform( "uBlurSource", 2 );
			mGlslComposite->uniform( "uMaxCoCRadiusPixels", mMaxCoCRadiusPixels );
			mGlslComposite->uniform( "uInputSourceInvSize", 1.f / vec2( readFbo->getSize() ) );	
			mGlslComposite->uniform( "uFarRadiusRescale", mFarRadiusRescale );	
			mGlslComposite->uniform( "uDebugOption", mDebugOption );
			mDoFBatch->replaceGlslProg( mGlslComposite );
			mDoFBatch->draw();
		}
	}
}

void DoFFx::setFocus( const float focus )
{
	mFocus = focus;
}

void DoFFx::setMaxCoCRadius( const int cocRadius )
{
	mMaxCoCRadiusPixels = cocRadius;
}
