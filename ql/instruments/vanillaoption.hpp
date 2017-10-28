/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file vanillaoption.hpp
    \brief Vanilla option on a single asset
*/

#ifndef quantlib_vanilla_option_hpp
#define quantlib_vanilla_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! Vanilla option (no discrete dividends, no barriers) on a single asset
    /*! \ingroup instruments */
    class VanillaOption : public OneAssetOption {
      public:
        VanillaOption(const boost::shared_ptr<StrikedTypePayoff>&,
                      const boost::shared_ptr<Exercise>&);

        /*! \warning currently, this method returns the Black-Scholes
                     implied volatility using analytic formulas for
                     European options and a finite-difference method
                     for American and Bermudan options. It will give
                     unconsistent results if the pricing was performed
                     with any other methods (such as jump-diffusion
                     models.)

            \warning options with a gamma that changes sign (e.g.,
                     binary options) have values that are <b>not</b>
                     monotonic in the volatility. In these cases, the
                     calculation can fail and the result (if any) is
                     almost meaningless.  Another possible source of
                     failure is to have a target value that is not
                     attainable with any volatility, e.g., a target
                     value lower than the intrinsic value in the case
                     of American options.
        */
        Volatility impliedVolatility(
             Real price,
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1.0e-7,
             Volatility maxVol = 4.0) const;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/instruments/impliedvolatility.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/fdamericanengine.hpp>
#include <ql/pricingengines/vanilla/fdbermudanengine.hpp>
#include <ql/exercise.hpp>
#include <boost/scoped_ptr.hpp>

namespace QuantLib {

    inline VanillaOption::VanillaOption(
        const boost::shared_ptr<StrikedTypePayoff>& payoff,
        const boost::shared_ptr<Exercise>& exercise)
    : OneAssetOption(payoff, exercise) {}


    inline Volatility VanillaOption::impliedVolatility(
             Real targetValue,
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy,
             Size maxEvaluations,
             Volatility minVol,
             Volatility maxVol) const {

        QL_REQUIRE(!isExpired(), "option expired");

        boost::shared_ptr<SimpleQuote> volQuote(new SimpleQuote);

        boost::shared_ptr<GeneralizedBlackScholesProcess> newProcess =
            detail::ImpliedVolatilityHelper::clone(process, volQuote);

        // engines are built-in for the time being
        boost::scoped_ptr<PricingEngine> engine;
        switch (exercise_->type()) {
          case Exercise::European:
            engine.reset(new AnalyticEuropeanEngine(newProcess));
            break;
          case Exercise::American:
            engine.reset(new FDAmericanEngine<CrankNicolson>(newProcess));
            break;
          case Exercise::Bermudan:
            engine.reset(new FDBermudanEngine<CrankNicolson>(newProcess));
            break;
          default:
            QL_FAIL("unknown exercise type");
        }

        return detail::ImpliedVolatilityHelper::calculate(*this,
                                                          *engine,
                                                          *volQuote,
                                                          targetValue,
                                                          accuracy,
                                                          maxEvaluations,
                                                          minVol, maxVol);
    }

}


#endif
