//
//  ControlView.m
//  Vomit
//
//  Created by William Lindmeier on 11/9/13.
//
//

#import "ControlView.h"

@implementation ControlView

@synthesize textFieldServer = _textFieldServer;
@synthesize connectButtonCallback = _connectButtonCallback;

- (void)awakeFromNib
{
    [super awakeFromNib];
    assert(self.textFieldServer != nil);
    [self.textFieldServer setStringValue:@"127.0.0.1"];
}

- (IBAction)buttonPressed:(id)sender
{
    if (self.connectButtonCallback)
    {
        NSString *host = self.textFieldServer.stringValue;
        _connectButtonCallback(std::string([host UTF8String]));
    }
}

@end
