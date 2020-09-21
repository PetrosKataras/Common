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
	IndieResolutionRendererPtr mIndieResRenderer;
	IndieResolutionRendererPtr mIndieResRendererB;
};

void prepareSettings( RIRApp::Settings* settings )
{
	settings->setMultiTouchEnabled( false );
}

void RIRApp::setup()
{
	auto options = IndieResolutionRenderer::Options().name( "Renderer 1" )
						.virtualPixelSize( ci::app::getWindowSize() * 2 )
						.screenPixelSize( ci::vec2( ci::app::getWindowSize() ) * .5f )
						.debugColor( ci::Color( 1.0f, 1.0f, .0f ) ) ;
	mIndieResRenderer = std::make_unique<IndieResolutionRenderer>( options );
	mIndieResRenderer->setClearColor( ci::Color( 1.0f, 0.f, 0.f ) );
	mIndieResRenderer->setCenter( ci::vec2( ci::app::getWindowSize() ) * .25f );

	options.name( "Renderer 2" ).debugColor( ci::Color( 0.f, 1.0f, 1.0f ) );
	mIndieResRendererB = std::make_unique<IndieResolutionRenderer>( options );
	mIndieResRendererB->setClearColor( ci::Color( .0f, 1.f, 0.f ) );
	mIndieResRendererB->setCenter( ci::vec2( ci::app::getWindowSize() ) * .75f );
}

void RIRApp::update()
{
	if( mIndieResRenderer ) {
		mIndieResRenderer->update();
		mIndieResRenderer->bind();
		mIndieResRenderer->unbind();
	}

	if( mIndieResRendererB ) {
		mIndieResRendererB->update();
		mIndieResRendererB->bind();
		mIndieResRendererB->unbind();
	}

}

void RIRApp::draw()
{
	gl::clear( Color::gray( 0.1f ) );
	if( mIndieResRenderer ) 
		mIndieResRenderer->render();

	if( mIndieResRendererB ) 
		mIndieResRendererB->render();
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
