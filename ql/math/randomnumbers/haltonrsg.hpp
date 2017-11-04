/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file haltonrsg.hpp
    \brief Halton low-discrepancy sequence generator
*/

#ifndef quantlib_halton_ld_rsg_h
#define quantlib_halton_ld_rsg_h

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    //! Halton low-discrepancy sequence generator
    /*! Halton algorithm for low-discrepancy sequence.  For more
        details see chapter 8, paragraph 2 of "Monte Carlo Methods in
        Finance", by Peter Jäckel

        \test
        - the correctness of the returned values is tested by
          reproducing known good values.
        - the correctness of the returned values is tested by checking
          their discrepancy against known good values.
    */
    class HaltonRsg {
      public:
        typedef Sample<std::vector<Real> > sample_type;
        HaltonRsg(Size dimensionality,
                  unsigned long seed = 0,
                  bool randomStart = true,
                  bool randomShift = false);
        const sample_type& nextSequence() const;
        const sample_type& lastSequence() const {
            return sequence_;
        }
        Size dimension() const {return dimensionality_;}
      private:
        Size dimensionality_;
        mutable unsigned long sequenceCounter_;
        mutable sample_type sequence_;
        std::vector<unsigned long> randomStart_;
        std::vector<Real>  randomShift_;
    };
}



/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
//
// Copyright (C) 2002 Peter Jäckel "Monte Carlo Methods in Finance".
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/primenumbers.hpp>

namespace QuantLib {

    inline HaltonRsg::HaltonRsg(Size dimensionality, unsigned long seed,
                         bool randomStart, bool randomShift)
    : dimensionality_(dimensionality), sequenceCounter_(0),
      sequence_(std::vector<Real> (dimensionality), 1.0),
      randomStart_(dimensionality, 0UL),
      randomShift_(dimensionality, 0.0) {

        QL_REQUIRE(dimensionality>0, 
                   "dimensionality must be greater than 0");

        if (randomStart || randomShift) {
            RandomSequenceGenerator<MersenneTwisterUniformRng>
                uniformRsg(dimensionality_, seed);
            if (randomStart)
                randomStart_ = uniformRsg.nextInt32Sequence();
            if (randomShift)
                randomShift_ = uniformRsg.nextSequence().value;
        }

    }

    inline const HaltonRsg::sample_type& HaltonRsg::nextSequence() const {
        ++sequenceCounter_;
        for (Size i=0; i<dimensionality_; ++i) {
            double h = 0.0;
            unsigned long b = PrimeNumbers::instance().get(i);
            double f = 1.0;
            unsigned long k = sequenceCounter_+randomStart_[i];
            while (k) {
                f /= b;
                h += (k%b)*f;
                k /= b;
            }
            sequence_.value[i] = h+randomShift_[i];
            sequence_.value[i] -= long(sequence_.value[i]);
        }
        return sequence_;
    }

}

#endif
