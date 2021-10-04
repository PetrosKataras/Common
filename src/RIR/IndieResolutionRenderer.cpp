#include "IndieResolutionRenderer.h"

#include "cinder/gl/gl.h"
#include "cinder/Rect.h"
#include "cinder/Log.h"

#if defined( USE_CINDER_IMGUI )
    #include "cinder/CinderImGui.h"
	namespace ui = ImGui;
#endif

static uint16_t sRendererCount = 0;
IndieResolutionRenderer::IndieResolutionRenderer( const Options options )
{
#if defined( USE_CINDER_IMGUI )
	// it's a no-op if already initialized
	ui::Initialize( ui::Options().iniPath( ci::app::getAssetPath( "imgui.ini" ) ) );
#endif
	mRendererName = options.name();
	if( mRendererName.empty() ) {
		mRendererName = "Renderer_" + std::to_string( sRendererCount );
	}	
	sRendererCount++;

	mVirtualSize = options.virtualPixelSize();
	auto fboFormat = options.fboFormat();
    mFbo            = ci::gl::Fbo::create( mVirtualSize.x, mVirtualSize.y, fboFormat );
    mShader         = ci::gl::context()->getStockShader( ci::gl::ShaderDef().color().texture() );
	mScreenSize = options.screenPixelSize();
    mCenter         = mScreenSize / 2.0f;
    mCenterLerp     = mCenter;

	mDebugColor		= options.debugColor();
	mAutoClear		= options.autoClear();

    connectSignals();

    updateView();

    ci::gl::ScopedFramebuffer scopedBuffer( mFbo );
    ci::gl::clear( mClearColor );
}

void IndieResolutionRenderer::connectSignals()
{
#if defined( USE_CINDER_IMGUI )
    mConnectionAppUpdate = ci::app::AppBase::get()->getSignalUpdate().connect ( 
        std::bind( &IndieResolutionRenderer::updateUI, this )
    );
#endif
    mConnectionAppMouseDown = ci::app::AppBase::get()->getWindow()->getSignalMouseDown().connect( 
        std::bind( &IndieResolutionRenderer::mouseDown, this, std::placeholders::_1 )
    );
    mConnectionAppMouseUp = ci::app::AppBase::get()->getWindow()->getSignalMouseUp().connect( 
        std::bind( &IndieResolutionRenderer::mouseUp, this, std::placeholders::_1 )
    );
    mConnectionAppMouseDrag = ci::app::AppBase::get()->getWindow()->getSignalMouseDrag().connect( 
        std::bind( &IndieResolutionRenderer::mouseDrag, this, std::placeholders::_1 )
    );
    mConnectionAppResize = ci::app::AppBase::get()->getWindow()->getSignalResize().connect( 
        std::bind( &IndieResolutionRenderer::updateView, this )
    );
}

IndieResolutionRenderer::~IndieResolutionRenderer()
{
	disconnectSignals();
}

void IndieResolutionRenderer::disconnectSignals()
{
	mConnectionAppUpdate.disconnect();
	mConnectionAppMouseDown.disconnect();
	mConnectionAppMouseUp.disconnect();
	mConnectionAppMouseDrag.disconnect();
	mConnectionAppResize.disconnect();
}

void IndieResolutionRenderer::bind()
{
	ci::gl::context()->pushFramebuffer( mFbo, GL_FRAMEBUFFER ); 
	ci::gl::context()->blendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	if( mAutoClear ) {
    	ci::gl::clear( mClearColor );
	}
    ci::gl::pushViewport();
    ci::gl::viewport( std::make_pair( ci::vec2( 0 ), mVirtualSize ) );
    ci::gl::pushMatrices();
    ci::gl::setMatricesWindow( mVirtualSize );
}

void IndieResolutionRenderer::unbind()
{
    ci::gl::popMatrices();
    ci::gl::popViewport();
#if ! defined( CINDER_GL_HAS_FBO_MULTISAMPLING )
    ci::gl::context()->popFramebuffer( GL_FRAMEBUFFER );
#else
    ci::gl::context()->popFramebuffer( GL_READ_FRAMEBUFFER );
    ci::gl::context()->popFramebuffer( GL_DRAW_FRAMEBUFFER );
#endif
}

void IndieResolutionRenderer::update()
{
}

void IndieResolutionRenderer::updateView()
{
	auto fboBounds			= mFbo->getBounds();
    mScreenRect             = ci::Rectf( fboBounds ).getCenteredFit( ci::Rectf( mCenter.x - mScreenSize.x * .5f, mCenter.y - mScreenSize.y * .5f, mCenter.x + mScreenSize.x * .5f, mCenter.y + mScreenSize.y * .5f ), false ).scaledCentered( mZoomFactor );
    auto rectGeom           = ci::geom::Rect().rect( mScreenRect );
    auto rectMesh           = ci::gl::VboMesh::create( rectGeom );
    mBatch                  = ci::gl::Batch::create( rectMesh, mShader );
}

void IndieResolutionRenderer::render()
{

    {
        ci::gl::ScopedTextureBind scopedTexture( mFbo->getColorTexture() );
        ci::gl::ScopedModelMatrix modelMatrix;
		if( mDebug ) {
			ci::gl::ScopedColor debugScreenRectColor( mDebugColor );
			ci::gl::drawSolidRect( mScreenRect.scaledCentered( ci::vec2( 1.03f ) ) );
		}
        ci::gl::translate( mCenter - mScreenRect.getCenter() );
        mBatch->draw();
    }
}

ci::vec2 IndieResolutionRenderer::getVirtualMousePosition( ci::vec2 mousePos )
{
    auto realPos            = mousePos - mScreenRect.getUpperLeft();
    auto ratio              = mVirtualSize / (ci::vec2)mScreenRect.getSize();
    auto virtualPos         = realPos * ratio;
    return virtualPos;
}

void IndieResolutionRenderer::setZoomFactor( const float& zoomFactor )
{
    mZoomFactor = zoomFactor;
    updateView();
}

void IndieResolutionRenderer::setCenter( const ci::vec2& center )
{
    mCenterLerp = center;
	mCenter = center;
	mScreenRect.offsetCenterTo( center );
    setLerpSpeed( 1.0f );
	updateView();
}

void IndieResolutionRenderer::panTo( const ci::vec2& panTo, const float& lerpSpeed )
{
    mCenterLerp = panTo;
    setLerpSpeed( lerpSpeed );
}

void IndieResolutionRenderer::setLerpSpeed( const float& lerpSpeed )
{
    mLerpSpeed = lerpSpeed;
}

void IndieResolutionRenderer::resetView()
{
    mCenter     = mScreenRect.getCenter();
    mZoomFactor = 1.0f;
    updateView();
}

void IndieResolutionRenderer::mouseDown( ci::app::MouseEvent event )
{
    auto eventPos = event.getPos();
    if( mScreenRect.getOffset( mCenter - mScreenRect.getCenter() ).contains( eventPos ) ) {
        mStartDragPos = eventPos;
    }
}

void IndieResolutionRenderer::mouseUp( ci::app::MouseEvent event )
{
}

void IndieResolutionRenderer::mouseDrag( ci::app::MouseEvent event )
{
    if( ! mPanningEnabled )
        return;
    auto eventPos = ci::vec2( event.getPos() );
    if( mScreenRect.getOffset( mCenter - mScreenRect.getCenter() ).contains( eventPos ) ) {
        panTo( mCenter + ( (ci::vec2)eventPos - mStartDragPos ) * 20.0f );
        mStartDragPos = eventPos;
    }
}

const ci::vec2& IndieResolutionRenderer::getVirtualSize() const
{
    return mVirtualSize;
}

void IndieResolutionRenderer::setEnablePanning( bool enable )
{
    mPanningEnabled = enable;
}

void IndieResolutionRenderer::setClearColor( ci::ColorAf clearColor )
{
	mClearColor = clearColor;
}

ci::gl::TextureRef IndieResolutionRenderer::getTexture() 
{
	if( mFbo ) {
		return mFbo->getColorTexture();
	}
	return nullptr;
}

#if defined( USE_CINDER_IMGUI )
void IndieResolutionRenderer::showUI( bool show )
{
    mShowUI = show;
}

void IndieResolutionRenderer::updateUI()
{
    if( mShowUI ) {
        ui::ScopedWindow window( mRendererName.c_str() );
        if( ui::Checkbox( "Debug", &mDebug ) ) {
        }
        ui::Separator();
		if( ui::InputFloat2( "Center", &mCenter[0] ) ) {
			setCenter( mCenter );
		}
        ui::Separator();
        if( ui::Button( "Reset Zoom" ) ) {
            setZoomFactor( 1.0f );
        }
        if( ui::SliderFloat( "Zoom", &mZoomFactor, 0.0f, 3.0f, "%.2f" ) ) {
            setZoomFactor( mZoomFactor );
        }
    }
    mCenter = ci::lerp( mCenter, mCenterLerp, mLerpSpeed );
}
#endif

void IndieResolutionRenderer::debug( bool enable )
{
	mDebug = enable;
}
