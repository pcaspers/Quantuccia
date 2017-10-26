/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

/*! \file quadratic.hpp
    \brief quadratic formula
*/

#ifndef quantlib_quadratic_hpp
#define quantlib_quadratic_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib
{
    class quadratic
    {
      public:
        quadratic(Real a, Real b, Real c);
        Real turningPoint() const;
        Real valueAtTurningPoint() const;
        Real operator()(Real x) const;
        Real discriminant() const;
        // return false if roots not real, and give turning point instead
        bool roots(Real& x, Real& y) const;
      private:
        Real a_, b_, c_;

    };
}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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


namespace QuantLib
{

    inline quadratic::quadratic(Real a, Real b, Real c) : a_(a), b_(b), c_(c) {}

    inline Real quadratic::turningPoint() const {
        return -b_/(2.0*a_);
    }

    inline Real quadratic::valueAtTurningPoint() const {
        return (*this)(turningPoint());
    }

    inline Real quadratic::operator()(Real x) const {
        return x*(x*a_+b_)+c_;
    }

    inline Real quadratic::discriminant() const {
        return b_*b_-4*a_*c_;
    }

    // return false if roots not real, and give turning point instead
    inline bool quadratic::roots(Real& x, Real& y) const {
        Real d = discriminant();
        if (d<0) {
            x = y = turningPoint();
            return false;
        }
        d = std::sqrt(d);
        x = (-b_ -  d)/(2*a_);
        y = (-b_ + d)/(2*a_);
        return true;

    }
}


#endif
