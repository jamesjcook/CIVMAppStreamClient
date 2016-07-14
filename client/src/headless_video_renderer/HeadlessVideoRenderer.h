#ifndef _included_HeadlessVideoRenderer_h
#define _included_HeadlessVideoRenderer_h

#include "VideoRenderer.h"
#include "MUD/threading/ScopeLock.h"
#include "MUD/threading/SimpleLock.h"

#include <sstream>
#include <string>
#include "XStx/common/XStxUtil.h"
/**
 * This is a headless renderer
 */
class HeadlessVideoRenderer : public VideoRenderer
{
public:
    //constructor
    HeadlessVideoRenderer();
    //destructor
    ~HeadlessVideoRenderer();

    /**
     * Set up the size for the backbuffer
     * System-side and GPU-side textures are always 1080p
     * We change the resolution by changing the uv co-cords of the fullscreen quad to crop
     * the textures, instead of changing GPU texture sizes
     */
    virtual bool init(uint32_t w, uint32_t h);

    /**
     * Renders the image onto the display. We do this by drawing a full-screen quad
     * and using a fragment shader to convert the YUV data into the viewable RGB image.
     */     
    virtual int draw();
 
    /**
     * This function:
     * Check for resolution change in mFrame and adapt(change quad uv cord) if neccesary
     * Upload mFrame to GPU texture, get ready for draw to be called
     */
    virtual void render();

    virtual void clearScreen();
    virtual void post();

};



#endif
