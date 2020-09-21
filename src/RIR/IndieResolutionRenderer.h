#pragma once

#include "cinder/gl/Fbo.h"
#include "cinder/gl/Batch.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/app/AppBase.h"
#include "cinder/Rand.h"

class IndieResolutionRenderer;
using IndieResolutionRendererPtr = std::unique_ptr<IndieResolutionRenderer>;

class IndieResolutionRenderer {
public:
	struct Options {
	public:
		Options() {
			mFboFormat = ci::gl::Fbo::Format().samples( 0 ).disableDepth();
			auto debugColor	= ci::randVec3();
			mDebugColor		= ci::Color( debugColor.x, debugColor.y, debugColor.z );
		}
		Options& name( const std::string name )
		{
			mName = name;
			return *this;
		}
		const std::string name() const { return mName; }

		Options& virtualPixelSize( const ci::vec2 virtualSize )
		{
			mVirtualSize = virtualSize;
			return *this;
		}
		const ci::vec2& virtualPixelSize() const { return mVirtualSize; }

		Options& screenPixelSize( const ci::vec2 screenSize )
		{
			mScreenSize = screenSize;
			return *this;
		}
		const ci::vec2& screenPixelSize() const { return mScreenSize; }

		Options& fboFormat( const ci::gl::Fbo::Format fboFormat )
		{
			mFboFormat = fboFormat;
			return *this;
		}
		const ci::gl::Fbo::Format& fboFormat() const { return mFboFormat; }
		
		Options& debugColor( const ci::Color debugColor )
		{
			mDebugColor = debugColor;
			return *this;
		}
		const ci::Color debugColor() const { return mDebugColor; }
		
	private:
		std::string mName;
		ci::vec2 mVirtualSize;
		ci::vec2 mScreenSize;
		ci::gl::Fbo::Format mFboFormat;
		ci::Color mDebugColor;
	};
    IndieResolutionRenderer( const Options options = Options() );
	~IndieResolutionRenderer();
    void                    bind();
    void                    unbind();
    void                    update();
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
	void					debug( bool enable = true );
#if defined( USE_CINDER_IMGUI )
    void                    showUI( bool show = true );
private:
    bool                    mShowUI{ true };
	void					updateUI();
    void                    updateView();
#endif
private:
    void                    connectSignals();
	void 					disconnectSignals();
private:
    ci::gl::FboRef          mFbo;
    ci::vec2                mVirtualSize{ 0 };
    ci::vec2                mScreenSize{ 0 };
    ci::gl::BatchRef        mBatch;
    float                   mZoomFactor{ 1.0f };
    ci::gl::GlslProgRef     mShader;
    ci::vec2                mCenter{ ci::vec2( 0.0f ) };
    ci::vec2                mCenterLerp{ ci::vec2( 0.0f ) };
    ci::vec2                mStartDragPos{ ci::vec2( 0.0f ) };
    float                   mLerpSpeed{ 1.0f };
    ci::Rectf               mScreenRect{ ci::Rectf() };
    bool                    mPanningEnabled{ false };
	bool					mDebug{ false };
	ci::ColorAf				mClearColor{ ci::ColorAf( 1.0f, 1.0f, 1.0f, 1.0f ) };
	ci::signals::Connection	mConnectionAppUpdate;
	ci::signals::Connection	mConnectionAppMouseDown;
	ci::signals::Connection	mConnectionAppMouseUp;
	ci::signals::Connection	mConnectionAppMouseDrag;
	ci::signals::Connection	mConnectionAppResize;
	ci::Color				mDebugColor{ ci::Color( 1.0f, 1.0f, 0.f ) };
	std::string				mRendererName;
};
