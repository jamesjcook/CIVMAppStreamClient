
#import <Cocoa/Cocoa.h>

@interface AppStreamSampleClientAppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;

-(IBAction) setVDAPrefs:(id)sender;
- (IBAction)disconnectFromServer:(id)sender;

@end
