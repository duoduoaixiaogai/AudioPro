#include "Envelope.h"

namespace RF {

    Envelope::Envelope(bool exponential, double minValue, double maxValue, double defaultValue)
       : mDB(exponential)
       , mMinValue(minValue)
       , mMaxValue(maxValue)
       , mDefaultValue { ClampValue(defaultValue) }
    {
    }

    void Envelope::SetOffset(double newOffset)
    {
       mOffset = newOffset;
    }

    void Envelope::SetTrackLen( double trackLen, double sampleDur )
    {
       auto range = EqualRange( trackLen, sampleDur );
       bool needPoint = ( range.first == range.second && trackLen < mTrackLen );
       double value=0.0;
       if ( needPoint )
          value = GetValueRelative( trackLen );

       mTrackLen = trackLen;

       // Shrink the array.
       // If more than one point already at the end, keep only the first of them.
       int newLen = std::min( 1 + range.first, range.second );
       mEnv.resize( newLen );

       if ( needPoint )
          AddPointAtEnd( mTrackLen, value );
    }

    double Envelope::GetValueRelative(double t, bool leftLimit) const
    {
       double temp;

       GetValuesRelative(&temp, 1, t, 0.0, leftLimit);
       return temp;
    }

    void Envelope::GetValuesRelative
       (double *buffer, int bufferLen, double t0, double tstep, bool leftLimit)
       const
    {
       // JC: If bufferLen ==0 we have probably just allocated a zero sized buffer.
       // wxASSERT( bufferLen > 0 );

       const auto epsilon = tstep / 2;
       int len = mEnv.size();

       double t = t0;
       double increment = 0;
       if ( len > 1 && t <= mEnv[0].GetT() && mEnv[0].GetT() == mEnv[1].GetT() )
          increment = leftLimit ? -epsilon : epsilon;

       double tprev, vprev, tnext = 0, vnext, vstep = 0;

       for (int b = 0; b < bufferLen; b++) {

          // Get easiest cases out the way first...
          // IF empty envelope THEN default value
          if (len <= 0) {
             buffer[b] = mDefaultValue;
             t += tstep;
             continue;
          }

          auto tplus = t + increment;

          // IF before envelope THEN first value
          if ( leftLimit ? tplus <= mEnv[0].GetT() : tplus < mEnv[0].GetT() ) {
             buffer[b] = mEnv[0].GetVal();
             t += tstep;
             continue;
          }
          // IF after envelope THEN last value
          if ( leftLimit
                ? tplus > mEnv[len - 1].GetT() : tplus >= mEnv[len - 1].GetT() ) {
             buffer[b] = mEnv[len - 1].GetVal();
             t += tstep;
             continue;
          }

          // be careful to get the correct limit even in case epsilon == 0
          if ( b == 0 ||
               ( leftLimit ? tplus > tnext : tplus >= tnext ) ) {

             // We're beyond our tnext, so find the next one.
             // Don't just increment lo or hi because we might
             // be zoomed far out and that could be a large number of
             // points to move over.  That's why we binary search.

             int lo,hi;
             if ( leftLimit )
                BinarySearchForTime_LeftLimit( lo, hi, tplus );
             else
                BinarySearchForTime( lo, hi, tplus );

             // mEnv[0] is before tplus because of eliminations above, therefore lo >= 0
             // mEnv[len - 1] is after tplus, therefore hi <= len - 1
//             wxASSERT( lo >= 0 && hi <= len - 1 );

             tprev = mEnv[lo].GetT();
             tnext = mEnv[hi].GetT();

             if ( hi + 1 < len && tnext == mEnv[ hi + 1 ].GetT() )
                // There is a discontinuity after this point-to-point interval.
                // Usually will stop evaluating in this interval when time is slightly
                // before tNext, then use the right limit.
                // This is the right intent
                // in case small roundoff errors cause a sample time to be a little
                // before the envelope point time.
                // Less commonly we want a left limit, so we continue evaluating in
                // this interval until shortly after the discontinuity.
                increment = leftLimit ? -epsilon : epsilon;
             else
                increment = 0;

             vprev = GetInterpolationStartValueAtPoint( lo );
             vnext = GetInterpolationStartValueAtPoint( hi );

             // Interpolate, either linear or log depending on mDB.
             double dt = (tnext - tprev);
             double to = t - tprev;
             double v;
             if (dt > 0.0)
             {
                v = (vprev * (dt - to) + vnext * to) / dt;
                vstep = (vnext - vprev) * tstep / dt;
             }
             else
             {
                v = vnext;
                vstep = 0.0;
             }

             // An adjustment if logarithmic scale.
             if( mDB )
             {
                v = pow(10.0, v);
                vstep = pow( 10.0, vstep );
             }

             buffer[b] = v;
          } else {
             if (mDB){
                buffer[b] = buffer[b - 1] * vstep;
             }else{
                buffer[b] = buffer[b - 1] + vstep;
             }
          }

          t += tstep;
       }
    }

    void Envelope::BinarySearchForTime_LeftLimit( int &Lo, int &Hi, double t ) const
    {
       Lo = -1;
       Hi = mEnv.size();

       // Invariants:  Lo is not less than -1, Hi not more than size
       while (Hi > (Lo + 1)) {
          int mid = (Lo + Hi) / 2;
          // mid must be strictly between Lo and Hi, therefore a valid index
          if (t <= mEnv[mid].GetT())
             Hi = mid;
          else
             Lo = mid;
       }
//       wxASSERT( Hi == ( Lo+1 ));

       mSearchGuess = Lo;
    }

    void Envelope::BinarySearchForTime( int &Lo, int &Hi, double t ) const
    {
       // Optimizations for the usual pattern of repeated calls with
       // small increases of t.
       {
          if (mSearchGuess >= 0 && mSearchGuess < (int)mEnv.size()) {
             if (t >= mEnv[mSearchGuess].GetT() &&
                 (1 + mSearchGuess == (int)mEnv.size() ||
                  t < mEnv[1 + mSearchGuess].GetT())) {
                Lo = mSearchGuess;
                Hi = 1 + mSearchGuess;
                return;
             }
          }

          ++mSearchGuess;
          if (mSearchGuess >= 0 && mSearchGuess < (int)mEnv.size()) {
             if (t >= mEnv[mSearchGuess].GetT() &&
                 (1 + mSearchGuess == (int)mEnv.size() ||
                  t < mEnv[1 + mSearchGuess].GetT())) {
                Lo = mSearchGuess;
                Hi = 1 + mSearchGuess;
                return;
             }
          }
       }

       Lo = -1;
       Hi = mEnv.size();

       // Invariants:  Lo is not less than -1, Hi not more than size
       while (Hi > (Lo + 1)) {
          int mid = (Lo + Hi) / 2;
          // mid must be strictly between Lo and Hi, therefore a valid index
          if (t < mEnv[mid].GetT())
             Hi = mid;
          else
             Lo = mid;
       }
//       wxASSERT( Hi == ( Lo+1 ));

       mSearchGuess = Lo;
    }

    double Envelope::GetInterpolationStartValueAtPoint( int iPoint ) const
    {
       double v = mEnv[ iPoint ].GetVal();
       if( !mDB )
          return v;
       else
          return log10(v);
    }

    void Envelope::AddPointAtEnd( double t, double val )
    {
       mEnv.push_back( EnvPoint{ t, val } );

       // Assume copied points were stored by nondecreasing time.
       // Allow no more than two points at exactly the same time.
       // Maybe that happened, because extra points were inserted at the boundary
       // of the copied range, which were not in the source envelope.
       auto nn = mEnv.size() - 1;
       while ( nn >= 2 && mEnv[ nn - 2 ].GetT() == t ) {
          // Of three or more points at the same time, erase one in the middle,
          // not the one newly added.
          mEnv.erase( mEnv.begin() + nn - 1 );
          --nn;
       }
    }

    std::pair<int, int> Envelope::EqualRange( double when, double sampleDur ) const
    {
       // Find range of envelope points matching the given time coordinate
       // (within an interval of length sampleDur)
       // by binary search; if empty, it still indicates where to
       // insert.
       const auto tolerance = sampleDur / 2;
       auto begin = mEnv.begin();
       auto end = mEnv.end();
       auto first = std::lower_bound(
          begin, end,
          EnvPoint{ when - tolerance, 0.0 },
          []( const EnvPoint &point1, const EnvPoint &point2 )
             { return point1.GetT() < point2.GetT(); }
       );
       auto after = first;
       while ( after != end && after->GetT() <= when + tolerance )
          ++after;
       return { first - begin, after - begin };
    }
}
