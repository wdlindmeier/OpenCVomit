//
//  ControlView.h
//  Vomit
//
//  Created by William Lindmeier on 11/9/13.
//
//

#import <Cocoa/Cocoa.h>
#include "cinder/Function.h"

typedef std::function<void(const std::string & serverAddress)> ConnectCallback;

@interface ControlView : NSView
{
    ConnectCallback _connectButtonCallback;
    NSTextField *_textFieldServer;
}

@property (nonatomic, assign) ConnectCallback connectButtonCallback;
@property (nonatomic, strong) IBOutlet NSTextField *textFieldServer;

- (IBAction)buttonPressed:(id)sender;
- (IBAction)buttonPressed:(id)sender;

@end
