/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#ifndef _MUD_SMART_POINTERS_
#define _MUD_SMART_POINTERS_

#if defined(_WIN32)

    #if _MSC_VER == 1500 //Visual Studio 2008
        #include <memory>
        using std::tr1::shared_ptr;
        using std::tr1::weak_ptr;
        using std::tr1::enable_shared_from_this;
    #elif _MSC_VER >= 1600 //Visual Studio 2010, Visual Studio 2012, ... ?
        #include <memory>
        using std::shared_ptr;
        using std::weak_ptr;
        using std::enable_shared_from_this;
    // #elif for future version of Visual Studio if needed.
    #else
        #include <boost/shared_ptr.hpp>
        #include <boost/weak_ptr.hpp>
        using boost::shared_ptr;
        using boost::weak_ptr;
        using boost::enable_shared_from_this;
    #endif

#elif defined(ANDROID)

    // We'll just use Boost on Android.
    #include <boost/shared_ptr.hpp>
    #include <boost/weak_ptr.hpp>
    #include <boost/enable_shared_from_this.hpp>
    using boost::shared_ptr;
    using boost::weak_ptr;
    using boost::enable_shared_from_this;

#elif defined(__linux__)

    #if (__GNUC__ >= 4) && (__GNUC_MINOR__ < 3)
        #include <tr1/memory>
        using std::tr1::shared_ptr;
        using std::tr1::weak_ptr;
        using std::tr1::enable_shared_from_this;
    #elif (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 3)
        #include <memory>
        using std::shared_ptr;
        using std::weak_ptr;
        using std::enable_shared_from_this;
    #else
        #include <boost/shared_ptr.hpp>
        #include <boost/weak_ptr.hpp>
        using boost::shared_ptr;
        using boost::weak_ptr;
        using boost::enable_shared_from_this;
    #endif

#elif __APPLE_CPP__ || __APPLE_CC__
//iOS and Mac OS stuff
    #include <memory>
    using std::shared_ptr;
    using std::weak_ptr;
    using std::enable_shared_from_this;

//#elif For Mac OS
//#else

#endif

#endif //_MUD_SMART_POINTERS_
