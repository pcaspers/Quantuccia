/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

/*! \file noarbsabrsmilesection.hpp
    \brief no arbitrage sabr smile section
*/

#ifndef quantlib_noarbsabr_smile_section_hpp
#define quantlib_noarbsabr_smile_section_hpp

#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/experimental/volatility/noarbsabr.hpp>
#include <vector>

namespace QuantLib {

class NoArbSabrSmileSection : public SmileSection {

  public:
    NoArbSabrSmileSection(Time timeToExpiry, Rate forward,
                          const std::vector<Real> &sabrParameters,
                          const Real shift = 0.0);
    NoArbSabrSmileSection(const Date &d, Rate forward,
                          const std::vector<Real> &sabrParameters,
                          const DayCounter &dc = Actual365Fixed(),
                          const Real shift = 0.0);
    Real minStrike() const { return 0.0; }
    Real maxStrike() const { return QL_MAX_REAL; }
    Real atmLevel() const { return forward_; }
    Real optionPrice(Rate strike, Option::Type type = Option::Call,
                     Real discount = 1.0) const;
    Real digitalOptionPrice(Rate strike, Option::Type type = Option::Call,
                            Real discount = 1.0, Real gap = 1.0e-5) const;
    Real density(Rate strike, Real discount = 1.0, Real gap = 1.0E-4) const;

    boost::shared_ptr<NoArbSabrModel> model() { return model_; }

  protected:
    Volatility volatilityImpl(Rate strike) const;

  private:
    void init();
    boost::shared_ptr<NoArbSabrModel> model_;
    Rate forward_;
    std::vector<Real> params_;
    Real shift_;
};
}



/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Peter Caspers

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

#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/sabr.hpp>

#include <boost/make_shared.hpp>

namespace QuantLib {

NoArbSabrSmileSection::NoArbSabrSmileSection(
    Time timeToExpiry, Rate forward, const std::vector<Real> &sabrParams,
    Real shift)
    : SmileSection(timeToExpiry, DayCounter()), forward_(forward),
      params_(sabrParams), shift_(shift) {
    init();
}

NoArbSabrSmileSection::NoArbSabrSmileSection(
    const Date &d, Rate forward, const std::vector<Real> &sabrParams,
    const DayCounter &dc, Real shift)
    : SmileSection(d, dc, Date()), forward_(forward), params_(sabrParams), shift_(shift) {
    init();
}

void NoArbSabrSmileSection::init() {
    QL_REQUIRE(params_.size() >= 4,
               "sabr expects 4 parameters (alpha,beta,nu,rho) but ("
                   << params_.size() << ") given");
    QL_REQUIRE(forward_ > 0.0, "forward (" << forward_ << ") must be positive");
    QL_REQUIRE(
        shift_ == 0.0,
        "shift (" << shift_
                  << ") must be zero, other shifts are not implemented yet");
    model_ =
        boost::make_shared<NoArbSabrModel>(exerciseTime(), forward_, params_[0],
                                           params_[1], params_[2], params_[3]);
}

Real NoArbSabrSmileSection::optionPrice(Rate strike, Option::Type type,
                                        Real discount) const {
    Real call = model_->optionPrice(strike);
    return discount *
           (type == Option::Call ? call : call - (forward_ - strike));
}

Real NoArbSabrSmileSection::digitalOptionPrice(Rate strike, Option::Type type,
                                               Real discount, Real) const {
    Real call = model_->digitalOptionPrice(strike);
    return discount * (type == Option::Call ? call : 1.0 - call);
}

Real NoArbSabrSmileSection::density(Rate strike, Real discount, Real) const {
    return discount * model_->density(strike);
}

Real NoArbSabrSmileSection::volatilityImpl(Rate strike) const {

    Real impliedVol = 0.0;
    try {
        Option::Type type;
        if (strike >= forward_)
            type = Option::Call;
        else
            type = Option::Put;
        impliedVol =
            blackFormulaImpliedStdDev(type, strike, forward_,
                                      optionPrice(strike, type, 1.0), 1.0) /
            std::sqrt(exerciseTime());
    } catch (...) {
    }
    if (impliedVol == 0.0)
        // fall back on Hagan 2002 expansion
        impliedVol =
            unsafeSabrVolatility(strike, forward_, exerciseTime(), params_[0],
                                 params_[1], params_[2], params_[3]);

    return impliedVol;
}
} // namespace QuantLib

#endif