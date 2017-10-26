/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2015 Peter Caspers

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

/*! \file sabrsmilesection.hpp
    \brief sabr smile section class
*/

#ifndef quantlib_sabr_smile_section_hpp
#define quantlib_sabr_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <vector>

namespace QuantLib {

    class SabrSmileSection : public SmileSection {
      public:
        SabrSmileSection(Time timeToExpiry,
                         Rate forward,
                         const std::vector<Real>& sabrParameters,
                         const Real shift = 0.0);
        SabrSmileSection(const Date& d,
                         Rate forward,
                         const std::vector<Real>& sabrParameters,
                         const DayCounter& dc = Actual365Fixed(),
                         const Real shift = 0.0);
        Real minStrike () const { return -shift_; }
        Real maxStrike () const { return QL_MAX_REAL; }
        Real atmLevel() const { return forward_; }
      protected:
        Real varianceImpl(Rate strike) const;
        Volatility volatilityImpl(Rate strike) const;
      private:
        Real alpha_, beta_, nu_, rho_, forward_, shift_;
    };


}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2015 Peter Caspers

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

#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    inline SabrSmileSection::SabrSmileSection(Time timeToExpiry,
                                       Rate forward,
                                       const std::vector<Real>& sabrParams,
                                       const Real shift)
        : SmileSection(timeToExpiry,DayCounter(),
                       ShiftedLognormal,shift),
          forward_(forward), shift_(shift) {

        alpha_ = sabrParams[0];
        beta_ = sabrParams[1];
        nu_ = sabrParams[2];
        rho_ = sabrParams[3];

        QL_REQUIRE(forward_ + shift_ > 0.0,
                   "at the money forward rate + shift must be "
                   "positive: "
                       << io::rate(forward_) << " with shift "
                       << io::rate(shift_) << " not allowed");
        validateSabrParameters(alpha_, beta_, nu_, rho_);
    }

    inline SabrSmileSection::SabrSmileSection(const Date& d,
                                       Rate forward,
                                       const std::vector<Real>& sabrParams,
                                       const DayCounter& dc,
                                       const Real shift)
        : SmileSection(d, dc,Date(),ShiftedLognormal,shift),
          forward_(forward), shift_(shift) {

        alpha_ = sabrParams[0];
        beta_ = sabrParams[1];
        nu_ = sabrParams[2];
        rho_ = sabrParams[3];

        QL_REQUIRE(forward_ + shift_ > 0.0,
                   "at the money forward rate + shift must be "
                   "positive: "
                       << io::rate(forward_) << " with shift "
                       << io::rate(shift_) << " not allowed");
        validateSabrParameters(alpha_, beta_, nu_, rho_);
    }

    inline Real SabrSmileSection::varianceImpl(Rate strike) const {
        strike = std::max(0.00001 - shift(),strike);
        Volatility vol = unsafeShiftedSabrVolatility(
            strike, forward_, exerciseTime(), alpha_, beta_, nu_, rho_, shift_);
        return vol * vol * exerciseTime();
     }

    inline Real SabrSmileSection::volatilityImpl(Rate strike) const {
        strike = std::max(0.00001 - shift(),strike);
        return unsafeShiftedSabrVolatility(strike, forward_, exerciseTime(),
                                           alpha_, beta_, nu_, rho_, shift_);
     }
}

#endif