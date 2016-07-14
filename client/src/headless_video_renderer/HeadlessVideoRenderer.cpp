
#include "HeadlessVideoRenderer.h"
#include "MUD/threading/ThreadUtil.h"

/**
 * HeadlessVideoRenderer
 */

    /** Constructor */
    HeadlessVideoRenderer::HeadlessVideoRenderer()
    {
    }

    /** Destructor */
    HeadlessVideoRenderer::~HeadlessVideoRenderer()
    {
       
    }

	bool HeadlessVideoRenderer::init(uint32_t w, uint32_t h)
    {
        mWidth = w;
        mHeight = h;
        return true;
    }

    void HeadlessVideoRenderer::clearScreen()
    {    
    }

    void HeadlessVideoRenderer::render()
    {
        //convertAndCopyFrameData();
        mud::ThreadUtil::sleep(5);
    }

    int HeadlessVideoRenderer::draw()
    {
		int nFrameRendered = 0;

        if (mFrame == NULL)
        {
            mud::ThreadUtil::sleep(5);
            return 0;
		}

		nFrameRendered = checkQueue();

        //renderFps();
        //renderErrorText();
        return nFrameRendered;

    }

    void HeadlessVideoRenderer::post()
    {
       
    }