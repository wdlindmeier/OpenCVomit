//
//  VideoClient.h
//  Vomit
//
//  Created by William Lindmeier on 11/9/13.
//
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "cinder/Cinder.h"
#include "CinderVideoStreamClient.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Text.h"
#include "ConcurrentQueue.h"

typedef CinderVideoStreamClient<uint8_t> CinderVideoStreamClientUint8;
typedef boost::shared_ptr<CinderVideoStreamClientUint8> ClientRef;

class VideoClient
{
    
public:
    
    VideoClient(){};
    ~VideoClient();
    
    void connect(const std::string & hostName,
                 const int portNum,
                 const ci::Vec2i & size);
    void disconnect();
    void update();
    void render();
    ci::Surface8u & getSurface();
    bool isConnected();
    
protected:
    
    void                threadLoop();
    
    bool                mIsConnected;
    std::string         mHostName;
    int                 mPortNum;
    ci::gl::Texture     mTexture;
    ci::Vec2i           mSize;
    
    std::shared_ptr<std::thread> mClientThreadRef;
    ClientRef           mClient;
    
    uint8_t             *mData;
    ci::Surface8u       mStreamSurface;
    
    std::string*        mClientStatus;
    std::string         mStatus;

    ph::ConcurrentQueue<uint8_t*>* mQueueFromServer;
};