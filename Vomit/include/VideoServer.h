//
//  VideoServer.h
//  Vomit
//
//  Created by William Lindmeier on 11/9/13.
//
//

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "cinder/Cinder.h"
#include "CinderVideoStreamServer.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Capture.h"
#include "cinder/Text.h"
#include "ConcurrentQueue.h"

typedef CinderVideoStreamServer<uint8_t> CinderVideoStreamServerUint8;
typedef boost::shared_ptr<CinderVideoStreamServerUint8> ServerRef;

class VideoServer
{
    
public:
    
    VideoServer() : mPortNum(-1) {};
    ~VideoServer();
    
    void start(const ci::Vec2i & size, const int portNum);
    void stop();
    void update();
    void render();
    bool isRunning();
    //ci::Surface8u & getSurface();
    
protected:
    
    void                threadLoop();

    bool                mIsRunning;
    int                 mPortNum;
    ci::Vec2i           mSize;
    ci::Capture         mCapture;
    ci::gl::Texture     mTexture;
    ci::gl::Texture     mNameTexture;
    std::string         mStatus;
    //ci::Surface8u       mSurface;
    
    std::shared_ptr<std::thread> mServerThreadRef;
    ServerRef           mServer;
    
    ph::ConcurrentQueue<uint8_t*>* mQueueToServer;
};