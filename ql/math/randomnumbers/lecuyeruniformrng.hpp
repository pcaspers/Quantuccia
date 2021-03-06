/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file lecuyeruniformrng.hpp
    \brief L'Ecuyer uniform random number generator
*/

#ifndef quantlib_lecuyer_uniform_rng_h
#define quantlib_lecuyer_uniform_rng_h

#include <ql/methods/montecarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    //! Uniform random number generator
    /*! Random number generator of L'Ecuyer with added Bays-Durham
        shuffle (know as ran2 in Numerical recipes)

        For more details see Section 7.1 of Numerical Recipes in C, 2nd
        Edition, Cambridge University Press (available at
        http://www.nr.com/)
    */
    class LecuyerUniformRng {
      public:
        typedef Sample<Real> sample_type;
        /*! if the given seed is 0, a random seed will be chosen
             based on clock() */
        explicit LecuyerUniformRng(long seed = 0);
        /*! returns a sample with weight 1.0 containing a random number
             uniformly chosen from (0.0,1.0) */
        sample_type next() const;
      private:
        mutable long temp1, temp2;
        mutable long y;
        mutable std::vector<long> buffer;
		
		static const long m1 = 2147483563L;
		static const long a1 = 40014L;
		static const long q1 = 53668L;
		static const long r1 = 12211L;
		
		static const long m2 = 2147483399L;
		static const long a2 = 40692L;
		static const long q2 = 52774L;
		static const long r2 = 3791L;
		
		
        static const int bufferSize=32;
        static const long bufferNormalizer= 67108862L;
        static const long double maxRandom();
    };

}



/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/math/randomnumbers/seedgenerator.hpp>

namespace QuantLib {


	inline const long double LecuyerUniformRng::maxRandom(){return 1.0-QL_EPSILON;}
    inline LecuyerUniformRng::LecuyerUniformRng(long seed)
    : buffer(LecuyerUniformRng::bufferSize) {
        // Need to prevent seed=0, so use seed=0 to have a "random" seed
        temp2 = temp1 = (seed != 0 ? seed : SeedGenerator::instance().get());
        // Load the shuffle table (after 8 warm-ups)
        for (int j=bufferSize+7; j>=0; j--) {
            long k = temp1/q1;
            temp1 = a1*(temp1-k*q1)-k*r1;
            if (temp1 < 0)
                temp1 += m1;
            if (j < bufferSize)
                buffer[j] = temp1;
        }
        y = buffer[0];
    }

    inline LecuyerUniformRng::sample_type LecuyerUniformRng::next() const {
        long k = temp1/q1;
        // Compute temp1=(a1*temp1) % m1
        // without overflows (Schrage's method)
        temp1 = a1*(temp1-k*q1)-k*r1;
        if (temp1 < 0)
            temp1 += m1;
        k = temp2/q2;
        // Compute temp2=(a2*temp2) % m2
        // without overflows (Schrage's method)
        temp2 = a2*(temp2-k*q2)-k*r2;
        if (temp2 < 0)
            temp2 += m2;
        // Will be in the range 0..bufferSize-1
        int j = y/bufferNormalizer;
        // Here temp1 is shuffled, temp1 and temp2 are
        // combined to generate output
        y = buffer[j]-temp2;
        buffer[j] = temp1;
        if (y < 1)
            y += m1-1;
        double result = y/double(m1);
        // users don't expect endpoint values
        if (result > maxRandom())
            result = (double) maxRandom();
        return sample_type(result,1.0);
    }

}



#endif
