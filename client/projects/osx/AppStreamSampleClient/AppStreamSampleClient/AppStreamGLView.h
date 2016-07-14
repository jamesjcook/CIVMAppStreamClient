
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>

@interface AppStreamGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}

@end
