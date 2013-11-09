//
//  VideoServer.cpp
//  Vomit
//
//  Created by William Lindmeier on 11/9/13.
//
//

#include "VideoServer.h"
#include "CinderOpenCv.h"

using namespace ci;
using namespace std;
using namespace ph;

VideoServer::~VideoServer()
{
    stop();
}

void VideoServer::start(const ci::Vec2i & size, const int portNum)
{
    mSize = size;
    mPortNum = portNum;

	vector<Capture::DeviceRef> devices( Capture::getDevices() );
    
    for(Capture::DeviceRef & device : devices)
    {
		try
        {
			if( device->checkAvailable() )
            {
				mCapture =  Capture(mSize.x, mSize.y, device);
				mCapture.start();
                
                // placeholder
				mTexture = gl::Texture();
                
				// render the name as a texture
				TextLayout layout;
				layout.setFont( Font( "Arial", 12 ) );
				layout.setColor( Color( 1, 1, 1 ) );
				layout.addLine( device->getName() );
				mNameTexture = gl::Texture( layout.render( true ) );
                
                break;
			}
		}
		catch( CaptureExc & )
        {
            ci::app::console() << "Unable to initialize device: " << device->getName() << endl;
		}
	}
    
    mQueueToServer = new ph::ConcurrentQueue<uint8_t*>();
    mServerThreadRef = std::shared_ptr<std::thread>(new std::thread(boost::bind(&VideoServer::threadLoop, this)));
    if (!mIsRunning) mIsRunning = true;
}

void VideoServer::stop()
{
    mIsRunning = false;
    
    if(mServerThreadRef)
    {
        // TODO: How can we kill the thread?
        mServer->stop();
//        mServerThreadRef->join();
    }
    
    if (mQueueToServer)
    {
        delete mQueueToServer;
        mQueueToServer = NULL;
    }
}

bool VideoServer::isRunning()
{
    return mIsRunning;
}
/*
Surface8u & VideoServer::getSurface()
{
    return mSurface;
}
*/
void VideoServer::update()
{
    if (mCapture.checkNewFrame())
    {
        Surface8u surf = mCapture.getSurface();

        // Flip horizontally to be more intuitive
        cv::Mat currentFrame(toOcv(surf));
        cv::flip(currentFrame, currentFrame, 1);
        surf = fromOcv(currentFrame);

        mQueueToServer->push(surf.getData());
        mTexture = gl::Texture( surf );
    }
    if (mIsRunning)
    {
        mStatus = "Streaming @ " + std::to_string(ci::app::getFrameRate()) + " fps";
    }
    else
    {
        mStatus = "Not Streaming";
    }
}

void VideoServer::render()
{
    gl::enableAlphaBlending();
    
    // Draw capture frame on the left side of the window
    gl::color(Color::white());
    if (mTexture)
    {
        // NOTE: Flipped horizontally
        gl::draw(mTexture, Rectf(0, 0, mSize.x, mSize.y));
    }
    
    if (mNameTexture)
    {
        // draw the name w/ drop shadow
        gl::color(Color::black());
        gl::draw(mNameTexture, Vec2f(11, 11));
        gl::color(Color(0.5, 0.75, 1));
        gl::draw(mNameTexture, Vec2f(10, 10));
    }
}

void VideoServer::threadLoop()
{
    while (mIsRunning)
    {
        try
        {
            mServer = ServerRef(new CinderVideoStreamServerUint8(mPortNum,
                                                                 mQueueToServer,
                                                                 mSize.x,
                                                                 mSize.y));
            mServer->run();
        }
        catch (std::exception& e)
        {
            ci::app::console() << "Exception: " << e.what() << endl;
        }
    }
}
