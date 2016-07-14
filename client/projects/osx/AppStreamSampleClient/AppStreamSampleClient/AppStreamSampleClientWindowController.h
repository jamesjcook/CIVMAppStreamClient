
#import <Cocoa/Cocoa.h>

// XSTX classes
#include "XStx/client/XStxClientAPI.h"

// AppStream client singleton
#import "AppStreamClient.h"

#import "EntitlementRetriever.h"

@class AppStreamGLView;

@interface AppStreamSampleClientWindowController : NSWindowController <AppStreamClientListenerDelegate, EntitlementRetrieverDelegate>

@property (strong) IBOutlet AppStreamGLView *glView;
@property (strong) IBOutlet NSView *reconnectingView;
@property (weak) IBOutlet NSTextField *reconnectingTextField;
@property (weak) IBOutlet NSTextField *statusTextField;
@property (weak) IBOutlet NSTextField *fpsTextField;
@property (nonatomic) BOOL isStopping;

-(void) showDESDialog;

@end
