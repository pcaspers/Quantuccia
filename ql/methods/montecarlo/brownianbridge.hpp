/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2009 Bojan Nikolic

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file brownianbridge.hpp
    \brief Browian bridge
*/

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
//
// Copyright (C) 2002 Peter J�ckel "Monte Carlo Methods in Finance".
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#ifndef quantlib_brownian_bridge_hpp
#define quantlib_brownian_bridge_hpp

#include <ql/methods/montecarlo/path.hpp>
#include <ql/methods/montecarlo/sample.hpp>

namespace QuantLib {

    //! Builds Wiener process paths using Gaussian variates
    /*! This class generates normalized (i.e., unit-variance) paths as
        sequences of variations. In order to obtain the actual path of
        the underlying, the returned variations must be multiplied by
        the integrated variance (including time) over the
        corresponding time step.

        \ingroup mcarlo
    */
    class BrownianBridge {
      public:
        /*! The constructor generates the time grid so that each step
            is of unit-time length.

            \param steps The number of steps in the path
        */
        BrownianBridge(Size steps);
        /*! The step times are copied from the supplied vector

            \param times A vector containing the times at which the
                         steps occur. This also defines the number of
                         steps that will be generated.

            \note the starting time of the path is assumed to be 0 and
                  must not be included
        */
        BrownianBridge(const std::vector<Time>& times);
        /*! The step times are copied from the TimeGrid object

            \param timeGrid a time grid containing the times at which
                            the steps will occur
        */
        BrownianBridge(const TimeGrid& timeGrid);
        //! \name inspectors
        //@{
        Size size() const { return size_; }
        const std::vector<Time>& times() const { return t_; }
        const std::vector<Size>& bridgeIndex()  const { return bridgeIndex_; }
        const std::vector<Size>& leftIndex()    const { return leftIndex_; }
        const std::vector<Size>& rightIndex()   const { return rightIndex_; }
        const std::vector<Real>& leftWeight()   const { return leftWeight_; }
        const std::vector<Real>& rightWeight()  const { return rightWeight_; }
        const std::vector<Real>& stdDeviation() const { return stdDev_; }
        //@}

        //! Brownian-bridge generator function
        /*! Transforms an input sequence of random variates into a
            sequence of variations in a Brownian bridge path.

            \param begin  The start iterator of the input sequence.
            \param end    The end iterator of the input sequence.
            \param output The start iterator of the output sequence.

            \note To get the canonical Brownian bridge which starts
                  and finishes at the same value, the first element of
                  the input sequence must be zero. Conversely, to get
                  a sloped bridge set the first element to a non-zero
                  value. In this case, the final value in the bridge
                  will be sqrt(last time point)*(first element of
                  input sequence).
        */
        template <class RandomAccessIterator1,
                  class RandomAccessIterator2>
        void transform(RandomAccessIterator1 begin,
                       RandomAccessIterator1 end,
                       RandomAccessIterator2 output) const {
            QL_REQUIRE(end >= begin, "invalid sequence");
            QL_REQUIRE(Size(end-begin) == size_,
                       "incompatible sequence size");
            // We use output to store the path...
            output[size_-1] = stdDev_[0] * begin[0];
            for (Size i=1; i<size_; ++i) {
                Size j = leftIndex_[i];
                Size k = rightIndex_[i];
                Size l = bridgeIndex_[i];
                if (j != 0) {
                    output[l] =
                        leftWeight_[i] * output[j-1] +
                        rightWeight_[i] * output[k]   +
                        stdDev_[i] * begin[i];
                } else {
                    output[l] =
                        rightWeight_[i] * output[k]   +
                        stdDev_[i] * begin[i];
                }
            }
            // ...after which, we calculate the variations and
            // normalize to unit times
            for (Size i=size_-1; i>=1; --i) {
                output[i] -= output[i-1];
                output[i] /= sqrtdt_[i];
            }
            output[0] /= sqrtdt_[0];
        }
      private:
        void initialize();
        Size size_;
        std::vector<Time> t_;
        std::vector<Real> sqrtdt_;
        std::vector<Size> bridgeIndex_, leftIndex_, rightIndex_;
        std::vector<Real> leftWeight_, rightWeight_, stdDev_;
    };

    // implementation

    inline BrownianBridge::BrownianBridge(Size steps)
    : size_(steps), t_(size_), sqrtdt_(size_),
      bridgeIndex_(size_), leftIndex_(size_), rightIndex_(size_),
      leftWeight_(size_), rightWeight_(size_), stdDev_(size_) {
        for (Size i=0; i<size_; ++i)
            t_[i] = static_cast<Time>(i+1);
        initialize();
    }

    inline BrownianBridge::BrownianBridge(const std::vector<Time>& times)
    : size_(times.size()), t_(times), sqrtdt_(size_),
      bridgeIndex_(size_), leftIndex_(size_), rightIndex_(size_),
      leftWeight_(size_), rightWeight_(size_), stdDev_(size_) {
        initialize();
    }

    inline BrownianBridge::BrownianBridge(const TimeGrid& timeGrid)
    : size_(timeGrid.size()-1), t_(size_), sqrtdt_(size_),
      bridgeIndex_(size_), leftIndex_(size_), rightIndex_(size_),
      leftWeight_(size_), rightWeight_(size_), stdDev_(size_) {
        for (Size i=0; i<size_; ++i)
            t_[i] = timeGrid[i+1];
        initialize();
    }


    inline void BrownianBridge::initialize() {

        sqrtdt_[0] = std::sqrt(t_[0]);
        for (Size i=1; i<size_; ++i)
            sqrtdt_[i] = std::sqrt(t_[i]-t_[i-1]);

        // map is used to indicate which points are already constructed.
        // If map[i] is zero, path point i is yet unconstructed.
        // map[i]-1 is the index of the variate that constructs
        // the path point # i.
        std::vector<Size> map(size_, 0);

        //  The first point in the construction is the global step.
        map[size_-1] = 1;
        //  The global step is constructed from the first variate.
        bridgeIndex_[0] = size_-1;
        //  The variance of the global step
        stdDev_[0] = std::sqrt(t_[size_-1]);
        //  The global step to the last point in time is special.
        leftWeight_[0] = rightWeight_[0] = 0.0;
        for (Size j=0, i=1; i<size_; ++i) {
            // Find the next unpopulated entry in the map.
            while (map[j])
                ++j;
            Size k = j;
            // Find the next populated entry in the map from there.
            while (!map[k])
                ++k;
            // l-1 is now the index of the point to be constructed next.
            Size l = j + ((k-1-j)>>1);
            map[l] = i;
            // The i-th Gaussian variate will be used to set point l-1.
            bridgeIndex_[i] = l;
            leftIndex_[i]   = j;
            rightIndex_[i]  = k;
            if (j != 0) {
                leftWeight_[i]= (t_[k]-t_[l])/(t_[k]-t_[j-1]);
                rightWeight_[i] = (t_[l]-t_[j-1])/(t_[k]-t_[j-1]);
                stdDev_[i] =
                    std::sqrt(((t_[l]-t_[j-1])*(t_[k]-t_[l]))
                              /(t_[k]-t_[j-1]));
            } else {
                leftWeight_[i]  = (t_[k]-t_[l])/t_[k];
                rightWeight_[i] =  t_[l]/t_[k];
                stdDev_[i] = std::sqrt(t_[l]*(t_[k]-t_[l])/t_[k]);
            }
            j=k+1;
            if (j>=size_)
                j=0;    //  wrap around
        }
    }


}

#endif
