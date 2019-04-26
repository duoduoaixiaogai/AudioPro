#ifndef TYPES_H
#define TYPES_H

#include <algorithm>
#include <QString>
#include <type_traits>

namespace RF {
    class sampleCount
    {
    public:
        using type = long long;
        static_assert(sizeof(type) == 8, "Wrong width of sampleCount");

        sampleCount () : value { 0 } {}

        // Allow implicit conversion from integral types
        sampleCount ( type v ) : value { v } {}
        sampleCount ( unsigned long long v ) : value ( v ) {}
        sampleCount ( int v ) : value { v } {}
        sampleCount ( unsigned v ) : value { v } {}
        sampleCount ( long v ) : value { v } {}

        // unsigned long is 64 bit on some platforms.  Let it narrow.
        sampleCount ( unsigned long v ) : value ( v ) {}

        // Beware implicit conversions from floating point values!
        // Otherwise the meaning of binary operators with sampleCount change
        // their meaning when sampleCount is not an alias!
        explicit sampleCount ( float f ) : value ( f ) {}
        explicit sampleCount ( double d ) : value ( d ) {}

        sampleCount ( const sampleCount& ) = default;
        sampleCount &operator= ( const sampleCount& ) = default;

        float as_float() const { return value; }
        double as_double() const { return value; }

        long long as_long_long() const { return value; }

        size_t as_size_t() const {
            //wxASSERT(value >= 0);
            //wxASSERT(static_cast<std::make_unsigned<type>::type>(value) <= std::numeric_limits<size_t>::max());
            return value;
        }

        sampleCount &operator += (sampleCount b) { value += b.value; return *this; }
        sampleCount &operator -= (sampleCount b) { value -= b.value; return *this; }
        sampleCount &operator *= (sampleCount b) { value *= b.value; return *this; }
        sampleCount &operator /= (sampleCount b) { value /= b.value; return *this; }
        sampleCount &operator %= (sampleCount b) { value %= b.value; return *this; }

        sampleCount operator - () const { return -value; }

        sampleCount &operator ++ () { ++value; return *this; }
        sampleCount operator ++ (int)
        { sampleCount result{ *this }; ++value; return result; }

        sampleCount &operator -- () { --value; return *this; }
        sampleCount operator -- (int)
        { sampleCount result{ *this }; --value; return result; }

    private:
        type value;
    };

    inline bool operator == (sampleCount a, sampleCount b)
    {
        return a.as_long_long() == b.as_long_long();
    }

    inline bool operator != (sampleCount a, sampleCount b)
    {
        return !(a == b);
    }

    inline bool operator < (sampleCount a, sampleCount b)
    {
        return a.as_long_long() < b.as_long_long();
    }

    inline bool operator >= (sampleCount a, sampleCount b)
    {
        return !(a < b);
    }

    inline bool operator > (sampleCount a, sampleCount b)
    {
        return b < a;
    }

    inline bool operator <= (sampleCount a, sampleCount b)
    {
        return !(b < a);
    }

    inline sampleCount operator + (sampleCount a, sampleCount b)
    {
        return sampleCount{ a } += b;
    }

    inline sampleCount operator - (sampleCount a, sampleCount b)
    {
        return sampleCount{ a } -= b;
    }

    inline sampleCount operator * (sampleCount a, sampleCount b)
    {
        return sampleCount{ a } *= b;
    }

    inline sampleCount operator / (sampleCount a, sampleCount b)
    {
        return sampleCount{ a } /= b;
    }

    inline sampleCount operator % (sampleCount a, sampleCount b)
    {
        return sampleCount{ a } %= b;
    }

    // ----------------------------------------------------------------------------
    // Function returning the minimum of a sampleCount and a size_t,
    // hiding the casts
    // ----------------------------------------------------------------------------

    inline size_t limitSampleBufferSize( size_t bufferSize, sampleCount limit )
    {
        return
                std::min( sampleCount( bufferSize ), std::max( sampleCount(0), limit ) )
                .as_size_t();
    }

    // ----------------------------------------------------------------------------
    // Supported sample formats
    // ----------------------------------------------------------------------------
    enum sampleFormat : unsigned
    {
        int16Sample = 0x00020001,
        int24Sample = 0x00040001,
        floatSample = 0x0004000F
    };

    // ----------------------------------------------------------------------------
    // Provide the number of bytes a specific sample will take
    // ----------------------------------------------------------------------------
#define SAMPLE_SIZE(SampleFormat) (SampleFormat >> 16)

    // ----------------------------------------------------------------------------
    // Generic pointer to sample data
    // ----------------------------------------------------------------------------
    typedef char *samplePtr;
    typedef const char *constSamplePtr;

    // ----------------------------------------------------------------------------
    // The type for plugin IDs
    // ----------------------------------------------------------------------------
    typedef QString PluginID;

    // ----------------------------------------------------------------------------
    // Supported channel assignments
    // ----------------------------------------------------------------------------

    typedef enum
    {
        // Use to mark end of list
        ChannelNameEOL = -1,
        // The default channel assignment
        ChannelNameMono,
        // From this point, the channels follow the 22.2 surround sound format
        ChannelNameFrontLeft,
        ChannelNameFrontRight,
        ChannelNameFrontCenter,
        ChannelNameLowFrequency1,
        ChannelNameBackLeft,
        ChannelNameBackRight,
        ChannelNameFrontLeftCenter,
        ChannelNameFrontRightCenter,
        ChannelNameBackCenter,
        ChannelNameLowFrequency2,
        ChannelNameSideLeft,
        ChannelNameSideRight,
        ChannelNameTopFrontLeft,
        ChannelNameTopFrontRight,
        ChannelNameTopFrontCenter,
        ChannelNameTopCenter,
        ChannelNameTopBackLeft,
        ChannelNameTopBackRight,
        ChannelNameTopSideLeft,
        ChannelNameTopSideRight,
        ChannelNameTopBackCenter,
        ChannelNameBottomFrontCenter,
        ChannelNameBottomFrontLeft,
        ChannelNameBottomFrontRight,
    } ChannelName, *ChannelNames;
}

#endif
