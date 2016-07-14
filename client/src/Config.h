#ifndef _included_Config_h
#define _included_Config_h

/**
 * @file Config.h
 *
 * Miscellaneous configuration options.
 */


// OPENGLES

#if TARGET_OS_IPHONE
#define GL_USES_OPENGLES 1
#elif TARGET_OS_MAC
#define GL_USES_OPENGL 1
#elif __ANDROID__
#define GL_USES_GLES2 1
#endif


#endif //_included_Config_h
