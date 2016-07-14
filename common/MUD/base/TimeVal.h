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

#ifndef _MUD_TIME_VAL_H_
#define _MUD_TIME_VAL_H_

#include <stdint.h>

namespace mud
{

/**
 *  Time value class.
 */
class TimeVal
{
public:
    /// Zero time value.
    static const TimeVal ZERO;

    /// Infinite time value.
    static const TimeVal INFIN;

public:
    /** Get the current system time. */
    static TimeVal now();
    
    /** Get the current mono time. */
    static TimeVal mono();
 
    /** Get the time from various units */
    static TimeVal fromSeconds( uint64_t seconds )
    {
        return TimeVal( seconds * 1000000 );
    }

    static TimeVal fromMilliSeconds( uint64_t milliSeconds )
    {
        return TimeVal( milliSeconds * 1000 );
    }

    static TimeVal fromMicroSeconds( uint64_t microSeconds )
    {
        return TimeVal( microSeconds );
    }

public:
    /** Constructor. */
    TimeVal() : mValue( 0 )
    { }
 
    /** Constructor. */
    template<class T>
    TimeVal( T value ) : mValue( (uint64_t) value )
    { }
 
    /** Copy constructor. */
    TimeVal( const TimeVal& refTimeVal ) : mValue( refTimeVal.mValue )
    { }
   
    /**
     *  Constructor, and default constructor.
     *
     *  @param[in] boolCurTimeFlag Set to true for current time.
     */
    TimeVal( bool boolCurTimeFlag );

    /** Default destructor. */
    ~TimeVal()
    {}

    /** Get the time in various units. */
    uint64_t toSeconds() const
    {
        return mValue / 1000000;
    }

    uint64_t toMilliSeconds() const
    {
        return mValue / 1000;
    }

    uint64_t toMicroSeconds() const
    {
        return mValue;
    }

    /** Convert the time into broken-time. */
    void toLocal( 
                uint32_t& year, 
                uint32_t& month, 
                uint32_t& day,
                uint32_t& hour,
                uint32_t& minute,
                uint32_t& second,
                uint32_t& milli ) const;
 
    TimeVal elapsedMono() 
    {
        return mono() - *this;
    }

    TimeVal elapsedClock() 
    {
        return now() - *this;
    }

    void resetMono() 
    {
        mValue = mono().toMicroSeconds();
    }

    void resetClock() 
    {
        mValue = now().toMicroSeconds();
    }

    /** == operator. */
    bool operator==( const TimeVal& refTimeVal ) const
    {
        return mValue == refTimeVal.mValue;
    }

    /** != operator. */
    bool operator!=( const TimeVal& refTimeVal ) const
    {
        return mValue != refTimeVal.mValue;
    }

   /** < operator. */
    bool operator<( const TimeVal& refTimeVal ) const
    {
        return mValue < refTimeVal.mValue;
    }

    /** > operator. */
    bool operator>( const TimeVal& refTimeVal ) const
    {
        return mValue > refTimeVal.mValue;
    }

    /** <= operator. */
    bool operator<=( const TimeVal& refTimeVal ) const
    {
        return mValue <= refTimeVal.mValue;
    }

    /** >= operator. */
    bool operator>=( const TimeVal& refTimeVal ) const
    {
        return mValue >= refTimeVal.mValue;
    }

    /**Assignment operator. */
    TimeVal& operator=( const TimeVal& refTimeVal )
    {
        if (this != &refTimeVal)
        {
            mValue = refTimeVal.mValue;
        }

        return *this;
    }

    /** += operator. */
    TimeVal& operator+=( const TimeVal& refTimeVal )
    {
        if ((*this == TimeVal::INFIN) || (refTimeVal == TimeVal::INFIN))
        {
            *this = TimeVal::INFIN;
        }
        else
        {
            mValue += refTimeVal.mValue; 
        }

        return *this;
    }

    /** - operator. */
    TimeVal operator-( const TimeVal& other )
    {
        TimeVal meCopy = *this;
        meCopy -= other;
        return meCopy;
    }

    /** -= operator. */
    TimeVal& operator-=( const TimeVal& refTimeVal )
    {
        if ((*this != TimeVal::INFIN) && (refTimeVal != TimeVal::INFIN))
        {
            if (mValue >= refTimeVal.mValue)
            {
                mValue -= refTimeVal.mValue;
            }
            else
            {
                mValue = 0;
            }
        }
        else if ((*this == TimeVal::INFIN) && (refTimeVal != TimeVal::INFIN))
        {
            *this = TimeVal::INFIN;
        }
        else
        {
            mValue = 0;
        }

        return *this;
    }
    
private:
    /// Total time, in microseconds.
    uint64_t mValue;

    /// for Windows high-resolution system clock
    static uint64_t getStaticFrequency();
};

//+ operator.
inline TimeVal operator+( const TimeVal& refLeft, const TimeVal& refRight )
{
    return TimeVal( refLeft ).operator+=( refRight );
}

//- operator.
inline TimeVal operator-( const TimeVal& refLeft, const TimeVal& refRight )
{
    return TimeVal( refLeft ).operator-=(refRight);
}

} //namespace mud

#endif //_MUD_TIME_VAL_H_

