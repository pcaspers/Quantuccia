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

/*! \file primenumbers.hpp
    \brief Prime numbers calculator
*/

#ifndef quantlib_primenumbers_h
#define quantlib_primenumbers_h

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Prime numbers calculator
    /*! Taken from "Monte Carlo Methods in Finance", by Peter Jäckel
     */
    class PrimeNumbers {
      public:
        //! Get and store one after another.
        static BigNatural get(Size absoluteIndex);
      private:
        PrimeNumbers() {}
        static BigNatural nextPrimeNumber();
        static std::vector<BigNatural> primeNumbers_;
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

#include <iterator>

namespace QuantLib {

    namespace {

        const BigNatural firstPrimes[] = {
            // the first two primes are mandatory for bootstrapping
            2,  3,
            // optional additional precomputed primes
            5,  7, 11, 13, 17, 19, 23, 29,
            31, 37, 41, 43, 47 };
    }
	
	//std::vector<BigNatural> PrimeNumbers::primeNumbers_;
	
    inline BigNatural PrimeNumbers::get(Size absoluteIndex) {
        if (primeNumbers_.empty()) {
            Size n = sizeof(firstPrimes)/sizeof(firstPrimes[0]);
            primeNumbers_.insert(primeNumbers_.end(),
                                 firstPrimes, firstPrimes+n);
        }
        while (primeNumbers_.size()<=absoluteIndex)
            nextPrimeNumber();
        return primeNumbers_[absoluteIndex];
    }

    inline BigNatural PrimeNumbers::nextPrimeNumber() {
        BigNatural p, n, m = primeNumbers_.back();
        do {
            // skip the even numbers
            m += 2;
            n = static_cast<BigNatural>(std::sqrt(Real(m)));
            // i=1 since the even numbers have already been skipped
            Size i = 1;
            do {
                p = primeNumbers_[i];
                ++i;
            } while ( m%p && p<=n );
        } while ( p<=n );
        primeNumbers_.push_back(m);
        return m;
    }

}


#endif