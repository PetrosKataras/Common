#include "Fx/FxChain.h"
#include "cinder/Log.h"
#include "cinder/gl/scoped.h"

using namespace ci;

FxChain::FxChain()
{
}

void FxChain::createPingPongFbos( const ci::vec2 size )
{
	auto fmt = gl::Fbo::Format();
	auto texFmt = gl::Texture2d::Format();
	texFmt.internalFormat( GL_RGBA16F );
	texFmt.setMinFilter( GL_LINEAR );
	texFmt.setMagFilter( GL_LINEAR );
	fmt.colorTexture( texFmt );
	for( size_t i = 0; i < 2; ++i ) {
		mPingPongFbo[i] = gl::Fbo::create( size.x, size.y, fmt );
		gl::ScopedFramebuffer scopedBuffer( mPingPongFbo[i] );
		gl::clear( Color::black() );
	}
	mBatch = gl::Batch::create( geom::Rect( mPingPongFbo[ping]->getBounds() ), gl::getStockShader( gl::ShaderDef().texture().color() ) );
}

FxChain& FxChain::chain( FxRef fx, const std::string fxName )
{
	if( ! fx || ( mFxMap.find( fxName ) != mFxMap.end() ) ) {
		CI_LOG_E( "Fx with name: " << fxName << " already exists in fx chain, skipping!" );
		return *this;
	}
	
	CI_LOG_I( "Adding Fx: " << fxName );
	mFxMap[fxName] = fx;
	return *this;
}

void FxChain::update()
{
	for( const auto& fx : mFxMap ) {
		fx.second->update();
	}
}

gl::TextureRef FxChain::process( const gl::TextureRef colorTexture, const gl::TextureRef depthTexture, CameraPersp* camera )
{
	if( ! colorTexture || ! depthTexture )
		return nullptr;
	
	if( ( ! mPingPongFbo[ping] || ! mPingPongFbo[pong] )
		|| ( mPingPongFbo[ping]->getSize() != colorTexture->getSize() )
		|| ( mPingPongFbo[pong]->getSize() != colorTexture->getSize() ) ) {
		createPingPongFbos( colorTexture->getSize() );
	}
	size_t processed = 0;
	for( const auto& fx : mFxMap ) {
		if( fx.second->enabled() ) {
			if( processed == 0 ) 
				fillReadFbo( colorTexture );
			fx.second->apply( mPingPongFbo[ping], mPingPongFbo[pong], depthTexture, camera );
			std::swap( ping, pong );
			processed++;
		}
	}
	return mPingPongFbo[ping]->getColorTexture();
}

void FxChain::fillReadFbo( const ci::gl::TextureRef colorTexture )
{
	if( ! mPingPongFbo[ping] || ! colorTexture )
		return;

	if( mPingPongFbo[ping]->getSize() != colorTexture->getSize() ) {
		createPingPongFbos( colorTexture->getSize() );
	}
	ci::gl::ScopedFramebuffer scopedFbo( mPingPongFbo[ping] );
	ci::gl::ScopedViewport scopedViewport( ci::ivec2( 0 ), mPingPongFbo[ping]->getSize() );
	ci::gl::ScopedMatrices scopedMatrix;
	ci::gl::setMatricesWindow( mPingPongFbo[ping]->getSize() );
	ci::gl::clear( ci::Color::black() );
	ci::gl::ScopedTextureBind scopedTex( colorTexture );
	mBatch->draw();
}

void FxChain::enable( const std::string fxName )
{
	auto fxIt = mFxMap.find( fxName );
	if( fxIt != mFxMap.end() ) {
		fxIt->second->enable();
	}	
}

void FxChain::disable( const std::string fxName )
{
	auto fxIt = mFxMap.find( fxName );
	if( fxIt != mFxMap.end() ) {
		fxIt->second->enable( false );
	}	
}

FxRef FxChain::getFx( const std::string fxName )
{
	if( mFxMap.find( fxName ) != mFxMap.end() ) {
		return mFxMap[fxName];
	}
	return nullptr;
}
