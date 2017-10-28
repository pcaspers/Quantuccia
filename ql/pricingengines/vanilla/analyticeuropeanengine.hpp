/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl

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

/*! \file analyticeuropeanengine.hpp
    \brief Analytic European engine
*/

#ifndef quantlib_analytic_european_engine_hpp
#define quantlib_analytic_european_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for European vanilla options using analytical formulae
    /*! \ingroup vanillaengines

        \test
        - the correctness of the returned value is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
        - the correctness of the returned implied volatility is tested
          by using it for reproducing the target value.
        - the implied-volatility calculation is tested by checking
          that it does not modify the option.
        - the correctness of the returned value in case of
          cash-or-nothing digital payoff is tested by reproducing
          results available in literature.
        - the correctness of the returned value in case of
          asset-or-nothing digital payoff is tested by reproducing
          results available in literature.
        - the correctness of the returned value in case of gap digital
          payoff is tested by reproducing results available in
          literature.
        - the correctness of the returned greeks in case of
          cash-or-nothing digital payoff is tested by reproducing
          numerical derivatives.
    */
    class AnalyticEuropeanEngine : public VanillaOption::engine {
      public:
        /*! This constructor triggers the usual calculation, in which
            the risk-free rate in the given process is used for both
            forecasting and discounting.
        */
        AnalyticEuropeanEngine(
                    const boost::shared_ptr<GeneralizedBlackScholesProcess>&);

        /*! This constructor allows to use a different term structure
            for discounting the payoff. As usual, the risk-free rate
            from the given process is used for forecasting the forward
            price.
        */
        AnalyticEuropeanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const Handle<YieldTermStructure>& discountCurve);
        void calculate() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Handle<YieldTermStructure> discountCurve_;
    };

}


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

#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    inline AnalyticEuropeanEngine::AnalyticEuropeanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
    : process_(process) {
        registerWith(process_);
    }

    inline AnalyticEuropeanEngine::AnalyticEuropeanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             const Handle<YieldTermStructure>& discountCurve)
    : process_(process), discountCurve_(discountCurve) {
        registerWith(process_);
        registerWith(discountCurve_);
    }

    inline void AnalyticEuropeanEngine::calculate() const {

        // if the discount curve is not specified, we default to the
        // risk free rate curve embedded within the GBM process
        boost::shared_ptr<YieldTermStructure> discountPtr = 
            discountCurve_.empty() ? 
            process_->riskFreeRate().currentLink() :
            discountCurve_.currentLink();

        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European option");

        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-striked payoff given");

        Real variance =
            process_->blackVolatility()->blackVariance(
                                              arguments_.exercise->lastDate(),
                                              payoff->strike());
        DiscountFactor dividendDiscount =
            process_->dividendYield()->discount(
                                             arguments_.exercise->lastDate());
        DiscountFactor df = discountPtr->discount(arguments_.exercise->lastDate());
        DiscountFactor riskFreeDiscountForFwdEstimation =
            process_->riskFreeRate()->discount(arguments_.exercise->lastDate());
        Real spot = process_->stateVariable()->value();
        QL_REQUIRE(spot > 0.0, "negative or null underlying given");
        Real forwardPrice = spot * dividendDiscount / riskFreeDiscountForFwdEstimation;

        BlackCalculator black(payoff, forwardPrice, std::sqrt(variance),df);


        results_.value = black.value();
        results_.delta = black.delta(spot);
        results_.deltaForward = black.deltaForward();
        results_.elasticity = black.elasticity(spot);
        results_.gamma = black.gamma(spot);

        DayCounter rfdc  = discountPtr->dayCounter();
        DayCounter divdc = process_->dividendYield()->dayCounter();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        Time t = rfdc.yearFraction(process_->riskFreeRate()->referenceDate(),
                                   arguments_.exercise->lastDate());
        results_.rho = black.rho(t);

        t = divdc.yearFraction(process_->dividendYield()->referenceDate(),
                               arguments_.exercise->lastDate());
        results_.dividendRho = black.dividendRho(t);

        t = voldc.yearFraction(process_->blackVolatility()->referenceDate(),
                               arguments_.exercise->lastDate());
        results_.vega = black.vega(t);
        try {
            results_.theta = black.theta(spot, t);
            results_.thetaPerDay =
                black.thetaPerDay(spot, t);
        } catch (Error&) {
            results_.theta = Null<Real>();
            results_.thetaPerDay = Null<Real>();
        }

        results_.strikeSensitivity  = black.strikeSensitivity();
        results_.itmCashProbability = black.itmCashProbability();
    }

}


#endif