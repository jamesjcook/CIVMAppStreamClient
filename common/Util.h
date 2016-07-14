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

#ifndef _included_Util_h
#define _included_Util_h

#include <stddef.h>

#include "XStx/common/XStxAPI.h"

/** @ingroup XStxExampleCommon
 * @{
 */

/**
 * Various utilities for XStx examples
 */

/**
 * Make MSVC (kind of ) support snprintf
 */
#ifdef _WIN32
#ifndef snprintf
#pragma warning(disable:4996)
#define snprintf _snprintf
#endif
#endif

/**
 * Macro for initializing the mSize member of an XStx interface structure
 * inherited by a class implementing it.
 */

#define INIT_INTERFACE_SIZE(Type) Type::mSize = sizeof(Type);

/**
 * Macro for initializing an XStx interface callback / context member pair.
 * This assumes that there is a corresponding static funciton with a name
 * of the form static<Type><Name>.  It is also assumed that 'this' is
 * an appropriate context for the the statuic function.
 *
 * @param[in] Type The type of interface to initialize
 * @param[in] Name The name of the 'method' to initialize
 */

#define INIT_CALLBACK(Type, Name)                                           \
    Type::m##Name##Fcn = static##Type##Name;                                \
    Type::m##Name##Ctx = this;

/**
 * Macros for defining a static function corresponding to those referenced
 * by invocations of INIT_CALLBACK.  These static will be named static<Fcn>
 * and they will call a non-static method named <Fcn>.
 *
 * @param[in] Type The type of the object pointed to by the 'context'
 * parameter of the statuc function.
 * @param[in] Fcn The name of the function.  This is used to create a
 * static method with a name of the form static<Fcn> which calls a non-static
 * method with the name <Fcn>.
 * DECLARE_CALLBACK_N variantes allow for 'N' arguments to be specified
 * for <Fcn> as additional macro arguments.  These additional arguments
 * specify the type of each additional argument in order.
 */

#define DECLARE_CALLBACK_0(Type, Fcn)                                       \
    static XStxResult XSTX_API static##Fcn(void* context)                   \
    {                                                                       \
        if (context != NULL)                                                \
        {                                                                   \
            return ((Type*)context)->Fcn();                                 \
        }                                                                   \
        else                                                                \
        {                                                                   \
            return XSTX_RESULT_INVALID_ARGUMENTS;                           \
        }                                                                   \
    }                                                                       \
    XStxResult Fcn();

#define DECLARE_CALLBACK_1(Type, Fcn, t1)                                   \
    static XStxResult XSTX_API static##Fcn(                                 \
        void* context, t1 a1)                                               \
    {                                                                       \
        if (context != NULL)                                                \
        {                                                                   \
            return ((Type*)context)->Fcn(a1);                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            return XSTX_RESULT_INVALID_ARGUMENTS;                           \
        }                                                                   \
    }                                                                       \
    XStxResult Fcn(t1);

#define DECLARE_CALLBACK_2(Type, Fcn, t1, t2)                               \
    static XStxResult XSTX_API static##Fcn(                                 \
        void* context, t1 a1, t2 a2)                                        \
    {                                                                       \
        if (context != NULL)                                                \
        {                                                                   \
            return ((Type*)context)->Fcn(a1, a2);                           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            return XSTX_RESULT_INVALID_ARGUMENTS;                           \
        }                                                                   \
    }                                                                       \
    XStxResult Fcn(t1, t2);

#define DECLARE_CALLBACK_3(Type, Fcn, t1, t2, t3)                           \
    static XStxResult XSTX_API static##Fcn(                                 \
        void* context, t1 a1, t2 a2, t3 a3)                                 \
    {                                                                       \
        if (context != NULL)                                                \
        {                                                                   \
            return ((Type*)context)->Fcn(a1, a2, a3);                       \
        }                                                                   \
        else                                                                \
        {                                                                   \
            return XSTX_RESULT_INVALID_ARGUMENTS;                           \
        }                                                                   \
    }                                                                       \
    XStxResult Fcn(t1, t2, t3);

/** @} */ //end doxygen group

#endif // _included_Util_h

