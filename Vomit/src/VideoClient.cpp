//
//  VideoClient.cpp
//  Vomit
//
//  Created by William Lindmeier on 11/9/13.
//
//

#include "VideoClient.h"

using namespace ci;
using namespace ci::gl;
using namespace std;

VideoClient::~VideoClient()
{
    disconnect();
}

ci::Surface8u VideoClient::getSurface()
{
    boost::mutex::scoped_lock lock(mSurfaceMutex);
    return mStreamSurface;
}

void VideoClient::connect(const std::string & hostName,
                          const int portNum,
                          const ci::Vec2i & size)
{
    mSize = size;
    mHostName = hostName;
    mPortNum = portNum;
    mIsConnected = true;
    
    mClientStatus = new std::string();
    mQueueFromServer = new ph::ConcurrentQueue<uint8_t*>();
    
    mClientThreadRef = std::shared_ptr<std::thread>(new std::thread(boost::bind(&VideoClient::threadLoop, this)));
    
    mStreamSurface = Surface8u(mSize.x, mSize.y, mSize.x*3, SurfaceChannelOrder::RGB);
    mStatus = "Starting";
}

void VideoClient::disconnect()
{
    mIsConnected = false;

    if(mClientThreadRef)
    {
        // TODO: How can we kill the thread?
        mClient->stop();
        //        mClientThreadRef->join();
    }

    if (mQueueFromServer)
    {
        delete mQueueFromServer;
        mQueueFromServer = NULL;
    }
}

void VideoClient::update()
{
    // boost::mutex::scoped_lock lock(mSurfaceMutex);
    // mQueueFromServer->mMutex.lock();
    if (mQueueFromServer->try_pop(mData))
    {
        mSurfaceMutex.lock();
        memcpy(mStreamSurface.getData(), mData, mSize.x * mSize.y * 3);
        mTexture = gl::Texture( mStreamSurface );
        mSurfaceMutex.unlock();
    }
    // mQueueFromServer->mMutex.unlock();
    
    if (mIsConnected)
    {
        mStatus = "Client: " + *mClientStatus + " fps: " + std::to_string((int)ci::app::getFrameRate());
    }
    else
    {
        mStatus = "Not Connected";
    }
}

void VideoClient::render()
{
    gl::enableAlphaBlending();
    
    // Draw capture frame on the left side of the window
    gl::color(Color::white());
    if (mTexture)
    {
        // NOTE: Flipped horizontally
        gl::draw(mTexture, Rectf(mSize.x, 0, mSize.x*2, mSize.y));
    }
    
    // draw status
    gl::drawString(mStatus, Vec2f( mSize.x + 10 + 1, 10 + 1 ), Color::black());
    gl::drawString(mStatus, Vec2f( mSize.x + 10, 10 ), Color::white());
}

bool VideoClient::isConnected()
{
    return mIsConnected;
}

void VideoClient::threadLoop()
{
    while (true)
    {
        try
        {
            mClient = ClientRef(new CinderVideoStreamClientUint8(mHostName,
                                                                 std::to_string(mPortNum))); // Strange that this is a string
            mClient->setup(mQueueFromServer,
                           mClientStatus,
                           mSize.x * mSize.y * 3);  //3 - for RGB mode
            mClient->run();
        }
        catch (std::exception& e)
        {
            ci::app::console() << "Exception: " << e.what() << std::endl;
        }
    }
}
