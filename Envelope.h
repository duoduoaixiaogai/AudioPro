#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "XMLTagHandler.h"

#include "vector"

namespace Renfeng {

    class EnvPoint final : public XMLTagHandler {
    public:
        EnvPoint() {}
        inline EnvPoint( double t, double val ) : mT{ t }, mVal{ val } {}
        double GetT() const { return mT; }
        double GetVal() const { return mVal; }
    private:
        double mT {};
        double mVal {};
    };

    typedef std::vector<EnvPoint> EnvArray;

    class Envelope final : public XMLTagHandler {
    public:
        Envelope(bool exponential, double minValue, double maxValue, double defaultValue);
        void SetOffset(double newOffset);
        void SetTrackLen( double trackLen, double sampleDur = 0.0 );
        double ClampValue(double value) { return std::max(mMinValue, std::min(mMaxValue, value)); }
    private:
        double GetValueRelative(double t, bool leftLimit = false) const;
        void GetValuesRelative
        (double *buffer, int len, double t0, double tstep, bool leftLimit = false)
        const;
        void BinarySearchForTime_LeftLimit( int &Lo, int &Hi, double t ) const;
        void BinarySearchForTime( int &Lo, int &Hi, double t ) const;
        double GetInterpolationStartValueAtPoint( int iPoint ) const;
        void AddPointAtEnd( double t, double val );
    private:
        double mOffset { 0.0 };
        std::pair<int, int> EqualRange( double when, double sampleDur ) const;
        EnvArray mEnv;
        double mDefaultValue;
        mutable int mSearchGuess { -2 };
        bool mDB;
        double mTrackLen { 0.0 };
        double mMinValue, mMaxValue;
    };
}

#endif
