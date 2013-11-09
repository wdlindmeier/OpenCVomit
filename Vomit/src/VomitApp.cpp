#include "cinder/qtime/QuickTime.h"
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "CinderOpenCv.h"
#include "ciFaceTracker.h"
#include "VideoServer.h"
#include "VideoClient.h"

using namespace ci;
using namespace ci::app;

const static int kVideoWidth = 640;
const static int kVideoHeight = 480;
const static int kServerPort = 3333;

class VomitApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();

    void update();
    void trackClientFace();
	
    void draw();
    void renderTracker();

    VideoServer         mVideoServer;
    VideoClient         mVideoClient;

	ciFaceTracker       mFaceTracker;
    qtime::MovieGlRef   mPukeSound;
    
    float               mAmtMouthOpen;
    float               mAmtJawOpen;
};

void VomitApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(kVideoWidth * 2, kVideoHeight);
}

void VomitApp::setup()
{
    Vec2i videoSize(kVideoWidth,kVideoHeight);
    
    // Stream video
    mVideoServer.start(videoSize, kServerPort);
    
    // Connecting to self for now.
    // This will / should be another person.
    mVideoClient.connect("localhost", kServerPort, videoSize);
    
    mFaceTracker.setup();
    
    mPukeSound = qtime::MovieGl::create( getResourcePath("puking.m4a") );
    mPukeSound->setLoop();
}

void VomitApp::update()
{
    mVideoServer.update();
    mVideoClient.update();
    trackClientFace();
}

void VomitApp::trackClientFace()
{
    Surface incomingSurf = mVideoClient.getSurface();
    cv::Mat input(toOcv(incomingSurf));
    mFaceTracker.update(input);

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

void VomitApp::draw()
{
	gl::clear(Color(0,0,0));
    
    mVideoServer.render();
    mVideoClient.render();
    renderTracker();
}

void VomitApp::renderTracker()
{
    gl::pushMatrices();
    
    gl::translate(Vec2i(kVideoWidth,0));

    mFaceTracker.draw(true);
    
    gl::color(Color::black());
    
    gl::drawSolidRect(Rectf(0,30,150,70));
    
    gl::drawString("Mouth Open: " + std::to_string(mAmtMouthOpen), Vec2i(10,40));
    gl::drawString("Jaw Open: " + std::to_string(mAmtJawOpen), Vec2i(10,55));

    gl::popMatrices();
}

CINDER_APP_NATIVE( VomitApp, RendererGl )
