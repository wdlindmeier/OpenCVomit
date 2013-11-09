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
#include "cinder/Function.h"

#import <Cocoa/Cocoa.h>
#import "ControlView.h"

using namespace ci;
using namespace ci::app;

const static int kVideoWidth = 640;
const static int kVideoHeight = 480;
const static int kServerPortSend = 3333;
const static int kServerPortReceive = 3333;

class VomitApp : public AppNative {
  public:
    void prepareSettings(Settings *settings);
	void setup();
    void addControls();
    
    void update();
    void trackClientFace();
	
    void draw();
    void renderTracker();
    
    void keyUp(KeyEvent event);
    void buttonConnectPressed(const std::string & serverName);

    VideoServer         mVideoServer;
    VideoClient         mVideoClient;

	ciFaceTracker       mFaceTracker;
    qtime::MovieGlRef   mPukeSound;
    
    float               mAmtMouthOpen;
    float               mAmtJawOpen;
    
    ControlView         *mControlView;
};

void VomitApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(kVideoWidth * 2, kVideoHeight);
}

void VomitApp::setup()
{
    // Stream video
    mVideoServer.start(Vec2i(kVideoWidth,kVideoHeight), kServerPortSend);

    // Inaccurate is ok
    mFaceTracker.setIterations(5);
    mFaceTracker.setup();
    
    mPukeSound = qtime::MovieGl::create( getResourcePath("puking.m4a") );
    mPukeSound->setLoop();
    
    addControls();
}

void VomitApp::keyUp(KeyEvent event)
{
    if (event.getChar() == ' ')
    {
        console() << "Resetting face tracker\n";
        mFaceTracker.reset();
    }
}

void VomitApp::addControls()
{
    NSArray *nibViews = nil;
    [[NSBundle mainBundle] loadNibNamed:@"ControlView"
                                  owner:nil
                        topLevelObjects:&nibViews];
    
    for (NSObject *nibMember in nibViews)
    {
        if ([nibMember isKindOfClass:[ControlView class]])
        {
            mControlView = (ControlView *)nibMember;
            mControlView.frame = NSMakeRect(0, kVideoHeight - 50, kVideoWidth, 50);
            ConnectCallback callback = std::bind( &VomitApp::buttonConnectPressed,
                                                 this,
                                                 std::placeholders::_1);
            [mControlView setConnectButtonCallback:callback];
            
            [(NSView *)(ci::app::getWindow()->getNative()) addSubview:(NSView *)mControlView];
            break;
        }
    }
}

void VomitApp::buttonConnectPressed(const std::string & serverName)
{
    mVideoClient.connect(serverName, kServerPortReceive, Vec2i(kVideoWidth,kVideoHeight));
    mFaceTracker.reset();
}

void VomitApp::update()
{
    if (mVideoServer.isRunning())
    {
        mVideoServer.update();
    }
    if (mVideoClient.isConnected())
    {
        mVideoClient.update();
        trackClientFace();
    }
}

void VomitApp::trackClientFace()
{
    Surface incomingSurf = mVideoClient.getSurface();
    cv::Mat input(toOcv(incomingSurf));
    
    // NOTE: Only check if the prev frame failed every N frames,
    // so we get more tracked frames, but it can correct itself
    // every N frames.
    bool failCheck = getElapsedFrames() % 30 == 0;
    mFaceTracker.update(input, failCheck);

    if (mFaceTracker.getFound())
    {
        // If we didn't find a face, just keep the previous values
        mAmtMouthOpen = mFaceTracker.getGesture(ciFaceTracker::MOUTH_HEIGHT);
        mAmtJawOpen = mFaceTracker.getGesture(ciFaceTracker::JAW_OPENNESS);
    }
    
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
    
    if (mVideoServer.isRunning())
    {
        mVideoServer.render();
    }
    
    if (mVideoClient.isConnected())
    {
        mVideoClient.render();
    }
    renderTracker();
}

void VomitApp::renderTracker()
{
    gl::pushMatrices();
    
    gl::translate(Vec2i(kVideoWidth,0));

    //mFaceTracker.draw(true);
    
    // Draw selective features
    gl::draw( mFaceTracker.getImageFeature(ciFaceTracker::LEFT_EYE) );
	gl::draw( mFaceTracker.getImageFeature(ciFaceTracker::RIGHT_EYE) );
	gl::draw( mFaceTracker.getImageFeature(ciFaceTracker::INNER_MOUTH) );
	gl::draw( mFaceTracker.getImageFeature(ciFaceTracker::OUTER_MOUTH) );
	gl::draw( mFaceTracker.getImageFeature(ciFaceTracker::JAW) );
    
    gl::color(Color::black());
    
    gl::drawSolidRect(Rectf(0,30,150,70));
    
    gl::drawString("Mouth Open: " + std::to_string(mAmtMouthOpen), Vec2i(10,40));
    gl::drawString("Jaw Open: " + std::to_string(mAmtJawOpen), Vec2i(10,55));

    gl::popMatrices();
}

CINDER_APP_NATIVE( VomitApp, RendererGl )
