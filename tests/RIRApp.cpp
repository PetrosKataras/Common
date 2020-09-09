#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "RIR/IndieResolutionRenderer.h"

using namespace ci;
using namespace ci::app;

class RIRApp : public App {
  public:
	void mouseDrag( MouseEvent event ) final;
	void mouseDown( MouseEvent event ) final;	
	void mouseUp( MouseEvent event ) final;	
	void keyDown( KeyEvent event ) override;

	void setup() final;
	void update() final;
	void draw() override;

  private:
};

void prepareSettings( RIRApp::Settings* settings )
{
	settings->setMultiTouchEnabled( false );
}

void RIRApp::setup()
{
}

void RIRApp::update()
{
}

void RIRApp::draw()
{
	gl::clear( Color::gray( 0.1f ) );
}

void RIRApp::mouseDrag( MouseEvent event )
{
}

void RIRApp::mouseDown( MouseEvent event )
{
}

void RIRApp::mouseUp( MouseEvent event )
{
}

void RIRApp::keyDown( KeyEvent event )
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

CINDER_APP( RIRApp, RendererGl, prepareSettings )
