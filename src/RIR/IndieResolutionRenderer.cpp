#include "IndieResolutionRenderer.h"

#include "cinder/gl/gl.h"
#include "cinder/Rect.h"
#include "cinder/Log.h"

#if defined( USE_CINDER_IMGUI )
    #include "CinderImGui.h"
#endif

IndieResolutionRenderer::IndieResolutionRenderer( ci::vec2 virtualSize, const ci::vec2 windowSize, ci::gl::Fbo::Format fboFormat )
: mVirtualSize( virtualSize )
, mWindowSize( windowSize )
{
    mFbo            = ci::gl::Fbo::create( virtualSize.x, virtualSize.y, fboFormat );
    mShader         = ci::gl::context()->getStockShader( ci::gl::ShaderDef().color().texture() );
    mCenter         = mWindowSize / 2.0f;
    mCenterLerp     = mCenter;

    connectSignals();

    resize();

    ci::gl::ScopedFramebuffer scopedBuffer( mFbo );
    ci::gl::clear( mClearColor );
}

void IndieResolutionRenderer::connectSignals()
{
    mConnectionAppUpdate = ci::app::AppBase::get()->getSignalUpdate().connect ( 
        std::bind( &IndieResolutionRenderer::update, this )
    );
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
        std::bind( &IndieResolutionRenderer::resize, this )
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
    mFbo->bindFramebuffer();
	ci::gl::context()->blendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
    ci::gl::clear( mClearColor );
    ci::gl::pushViewport();
    ci::gl::viewport( std::make_pair( ci::vec2( 0 ), mVirtualSize ) );
    ci::gl::pushMatrices();
    ci::gl::setMatricesWindow( mVirtualSize );
}

void IndieResolutionRenderer::unbind()
{
    ci::gl::popMatrices();
    ci::gl::popViewport();
    mFbo->unbindFramebuffer();
}

void IndieResolutionRenderer::update()
{
#if defined( USE_CINDER_IMGUI )
    if( mShowUI ) {
        ui::ScopedWindow window( "IndieResolutionRenderer" );
        if( ui::Button( "Re-Center" ) ) {
            setCenter( mWindowSize * .5f );
        }
        ui::SameLine();
        if( ui::Button( "Reset View" ) ) {
            resetView();
        }
        ui::Separator();
        {
            ui::SliderFloat( "Center X", &mCenterLerp[0], 0.0f, mWindowSize.x, "%5.0f" );
            ui::SliderFloat( "Center Y", &mCenterLerp[1], 0.0f, mWindowSize.y, "%5.0f" );
        }
        ui::Separator();
        if( ui::Button( "Reset Zoom" ) ) {
            setZoomFactor( 1.0f );
        }
        if( ui::SliderFloat( "Zoom", &mZoomFactor, 0.0f, 3.0f, "%.2f" ) ) {
            resize();
        }
    }
#endif
    mCenter = ci::lerp( mCenter, mCenterLerp, mLerpSpeed );
}

void IndieResolutionRenderer::resize()
{
	auto fboBounds			= mFbo->getBounds();
    mScreenRect             = ci::Rectf( fboBounds ).getCenteredFit( ci::Rectf( mCenter.x - mWindowSize.x * .5f, mCenter.y - mWindowSize.y * .5f, mCenter.x + mWindowSize.x * .5f, mCenter.y + mWindowSize.y * .5f ), false ).scaledCentered( mZoomFactor );
    auto rectGeom           = ci::geom::Rect().rect( mScreenRect );
    auto rectMesh           = ci::gl::VboMesh::create( rectGeom );
    mBatch                  = ci::gl::Batch::create( rectMesh, mShader );
	mCenter					= mWindowSize * .5f;
    mCenterLerp             = mCenter;
    mLerpSpeed              = 1.0f;
}

void IndieResolutionRenderer::render()
{

    {
        ci::gl::ScopedTextureBind scopedTexture( mFbo->getColorTexture() );
        ci::gl::ScopedModelMatrix modelMatrix;
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
    resize();
}

void IndieResolutionRenderer::setCenter( const ci::vec2& center )
{
    mCenterLerp = center;
	mCenter = center;
	mScreenRect.offsetCenterTo( center );
    setLerpSpeed( 1.0f );
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
    mCenter     = mWindowSize * .5f;
    mZoomFactor = 1.0f;
    resize();
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
#endif
