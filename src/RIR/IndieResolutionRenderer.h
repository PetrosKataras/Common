#pragma once

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Batch.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/app/AppBase.h"

class IndieResolutionRenderer;
using IndieResolutionRendererPtr = std::unique_ptr<IndieResolutionRenderer>;

class IndieResolutionRenderer {
public:
    IndieResolutionRenderer( ci::vec2 virtualSize, const ci::vec2 windowSize = ci::app::getWindowSize(), ci::gl::Fbo::Format fboFormat = ci::gl::Fbo::Format().samples( 0 ).disableDepth() );
	~IndieResolutionRenderer();
    void                    bind();
    void                    unbind();
    void                    update();
    void                    resize();
    void                    render();
	void 					setClearColor( ci::ColorAf clearColor );
    void                    setZoomFactor( const float& zoomFactor );
    ci::vec2                getVirtualMousePosition( ci::vec2 mousePos );
    void                    setCenter( const ci::vec2& center );
    void                    panTo( const ci::vec2& panTo, const float& lerpSpeed = .1f );
    void                    setLerpSpeed( const float& lerpSpeed );
    void                    mouseDown( ci::app::MouseEvent event );
    void                    mouseUp( ci::app::MouseEvent event );
    void                    mouseDrag( ci::app::MouseEvent event );
    void                    resetView();
    const ci::vec2&         getVirtualSize() const;
    void                    setEnablePanning( bool enable );
	ci::gl::TextureRef		getTexture();
#if defined( USE_CINDER_IMGUI )
    void                    showUI( bool show = true );
private:
    bool                    mShowUI{ true };
#endif
private:
    void                    connectSignals();
	void 					disconnectSignals();
private:
    ci::gl::FboRef          mFbo;
    ci::vec2                mVirtualSize{ 0 };
    ci::vec2                mWindowSize{ 0 };
    ci::gl::BatchRef        mBatch;
    float                   mZoomFactor{ 1.0f };
    ci::gl::GlslProgRef     mShader;
    ci::vec2                mCenter{ ci::vec2( 0.0f ) };
    ci::vec2                mCenterLerp{ ci::vec2( 0.0f ) };
    ci::vec2                mStartDragPos{ ci::vec2( 0.0f ) };
    float                   mLerpSpeed{ 1.0f };
    ci::Rectf               mScreenRect{ ci::Rectf() };
    bool                    mPanningEnabled{ false };
	ci::ColorAf				mClearColor{ ci::ColorAf( 1.0f, 1.0f, 1.0f, 1.0f ) };
	ci::signals::Connection	mConnectionAppUpdate;
	ci::signals::Connection	mConnectionAppMouseDown;
	ci::signals::Connection	mConnectionAppMouseUp;
	ci::signals::Connection	mConnectionAppMouseDrag;
	ci::signals::Connection	mConnectionAppResize;
};
