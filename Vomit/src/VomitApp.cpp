#include "cinder/qtime/QuickTime.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCv.h"
#include "ciFaceTracker.h"

using namespace ci;
using namespace ci::app;

class VomitApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();
	void draw();
    void update();
	
	gl::TextureRef      mTexture;
    CaptureRef          mCapture;
    ciFaceTracker       mFaceTracker;
    qtime::MovieGlRef   mPukeSound;
    
    float               mAmtMouthOpen;
    float               mAmtJawOpen;
};

void VomitApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(640, 480); //image size
}

void VomitApp::setup()
{
    mFaceTracker.setup();
    mCapture = Capture::create( 640, 480 );
    mCapture->start();
    
    mPukeSound = qtime::MovieGl::create( getResourcePath("puking.m4a") );
    mPukeSound->setLoop();
}

void VomitApp::update()
{
    if(mCapture && mCapture->checkNewFrame())
    {
        Surface camSurf = mCapture->getSurface();
        cv::Mat input(toOcv(camSurf));
        mFaceTracker.update(input);
        mTexture = gl::Texture::create(camSurf);

        mAmtMouthOpen = mFaceTracker.getGesture(ciFaceTracker::MOUTH_HEIGHT);
        mAmtJawOpen = mFaceTracker.getGesture(ciFaceTracker::JAW_OPENNESS);
        
        if (mAmtMouthOpen > 3)
        {
            mPukeSound->play();
        }
        else
        {
            mPukeSound->stop();
        }
    }
}

void VomitApp::draw()
{
    gl::color(Color::white());
    gl::enableAlphaBlending();

	gl::clear(Color(0,0,0));
    if (mTexture)
    {
        gl::draw(mTexture);
    
        mFaceTracker.draw(true);
        
        gl::color(Color::black());
        gl::drawSolidRect(Rectf(5,5,150,45));

        gl::drawString("Mouth Open: " + std::to_string(mAmtMouthOpen), Vec2i(10,10));
        gl::drawString("Jaw Open: " + std::to_string(mAmtJawOpen), Vec2i(10,25));
    }
}

CINDER_APP_NATIVE( VomitApp, RendererGl )
