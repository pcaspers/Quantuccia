/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

/*! \file gumbelcopula.hpp
    \brief Gumbel copula
*/

#ifndef quantlib_math_Gumbel_copula_h
#define quantlib_math_Gumbel_copula_h

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Gumbel copula
    class GumbelCopula : public std::binary_function<Real,Real,Real> {
      public:
        GumbelCopula(Real theta);
        Real operator()(Real x, Real y) const;
      private:
        Real theta_;
    };
    
}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

#include <ql/errors.hpp>

namespace QuantLib {

    inline GumbelCopula::GumbelCopula(Real theta) : theta_(theta)
    {
        QL_REQUIRE(theta >= 1.0,
                   "theta (" << theta << ") must be greater or equal to 1");
    }
    
    inline Real GumbelCopula::operator()(Real x, Real y) const 
    {
        QL_REQUIRE(x >= 0.0 && x <=1.0 ,
                   "1st argument (" << x << ") must be in [0,1]");
        QL_REQUIRE(y >= 0.0 && y <=1.0 ,
                   "2nd argument (" << y << ") must be in [0,1]");
        using namespace std;
        return exp(-pow( pow( -log(x), theta_)+pow( -log(y), theta_),1/theta_));
    }

}

#endif
