#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "Fx/FxChain.h"
#include "Fx/VignetteFx.h"

using namespace ci;
using namespace ci::app;

class FxApp : public App {
  public:
	void mouseDrag( MouseEvent event ) final;
	void mouseDown( MouseEvent event ) final;	
	void mouseUp( MouseEvent event ) final;	
	void keyDown( KeyEvent event ) override;

	void setup() final;
	void update() final;
	void draw() override;

  private:
	std::unique_ptr<FxChain> mFxChain;
};

void prepareSettings( FxApp::Settings* settings )
{
	settings->setMultiTouchEnabled( false );
}

void FxApp::setup()
{
	mFxChain = std::make_unique<FxChain>();
	auto vignette = std::make_shared<VignetteFx>();
	vignette->enable();
	mFxChain->chain( vignette, "vignette" );
}

void FxApp::update()
{
	if( mFxChain ) mFxChain->update();
}

void FxApp::draw()
{
	gl::clear( Color::gray( 0.9f ) );
}

void FxApp::mouseDrag( MouseEvent event )
{
}

void FxApp::mouseDown( MouseEvent event )
{
}

void FxApp::mouseUp( MouseEvent event )
{
}

void FxApp::keyDown( KeyEvent event )
{
	if( event.getChar() == 'f' ) {
		setFullScreen( ! isFullScreen() );
	}
	else if( event.getCode() == KeyEvent::KEY_SPACE ) {
	}
	else if( event.getCode() == KeyEvent::KEY_ESCAPE ) {
		if( isFullScreen() )
			setFullScreen( false );
		else
			quit();
	}
}

CINDER_APP( FxApp, RendererGl, prepareSettings )
