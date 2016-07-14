
#import "AppStreamSampleClientAppDelegate.h"

#import "AppStreamSampleClientWindowController.h"


@interface AppStreamSampleClientAppDelegate()
{
    AppStreamSampleClientWindowController *_windowController;
    BOOL disableHardwareDecode;
}

@end

@implementation AppStreamSampleClientAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //Create the main app window
    _windowController = [[AppStreamSampleClientWindowController alloc] initWithWindowNibName:@"AppStreamSampleClientWindowController"];
    [_windowController showWindow:self];
    
    disableHardwareDecode = [[NSUserDefaults standardUserDefaults] boolForKey:@"disableVDADecode"];
    
    if ( disableHardwareDecode == YES)
    {
        // set the disable Hardware Decode option in application menu checked or not
        [[[[[[NSApplication sharedApplication]mainMenu]itemAtIndex:0] submenu]itemAtIndex:2]setState:NSOnState];
    }
    
    //Make sure the windowController is the first responder so it will capture
    // all keypresses (specifically TAB keyDown does not happen without this)
    [_windowController.window makeFirstResponder:_windowController];
    
    //Make sure the DES Dialog is showing
    [_windowController showDESDialog];
}

//We want to close the app when the user closes the window
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return true;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSNotification *) notification
{
    //Tell the window controller we are stopping so it doesn't show an
    // error dialog
    _windowController.isStopping = true;
    
    //Stop the AppStreamClient
    [[AppStreamClient sharedClient] stop];
    [[AppStreamClient sharedClient] recycle];

    
    // give xstx client some time to close
    double delayInSeconds = 1.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        
        // now really quit the app
        [NSApp replyToApplicationShouldTerminate:YES];
        
    });
        
    return NSTerminateLater;
    
}

- (void)applicationWillTerminate:(NSNotification *)notification
{

    NSLog(@"Application exiting.");
}

-(IBAction) setVDAPrefs:(id)sender{
    
    if( disableHardwareDecode == YES)
    {
        disableHardwareDecode = NO;
        [sender setState:NSOffState];
    }
    else
    {
        disableHardwareDecode = YES;
        [sender setState:NSOnState];
    }
   
    [[NSUserDefaults standardUserDefaults]setBool:disableHardwareDecode forKey:@"disableVDADecode"];
    [[NSUserDefaults standardUserDefaults]synchronize];
    
}

- (IBAction)disconnectFromServer:(id)sender {
    [[AppStreamClient sharedClient] stop];
    
    //Make sure the DES Dialog is showing
    [_windowController showDESDialog];
}

@end
