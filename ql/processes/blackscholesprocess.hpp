/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2006, 2007, 2009 StatPro Italia srl
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

/*! \file blackscholesprocess.hpp
    \brief Black-Scholes processes
*/

#ifndef quantlib_black_scholes_process_hpp
#define quantlib_black_scholes_process_hpp

#include <ql/stochasticprocess.hpp>
#include <ql/processes/eulerdiscretization.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class LocalConstantVol;
    class LocalVolCurve;

    //! Generalized Black-Scholes stochastic process
    /*! This class describes the stochastic process \f$ S \f$ governed by
        \f[
            d\ln S(t) = (r(t) - q(t) - \frac{\sigma(t, S)^2}{2}) dt
                     + \sigma dW_t.
        \f]

        \warning while the interface is expressed in terms of \f$ S \f$,
                 the internal calculations work on \f$ ln S \f$.

        \ingroup processes
    */
    class GeneralizedBlackScholesProcess : public StochasticProcess1D {
      public:
        GeneralizedBlackScholesProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const boost::shared_ptr<discretization>& d =
                  boost::shared_ptr<discretization>(new EulerDiscretization),
            bool forceDiscretization = false);
        //! \name StochasticProcess1D interface
        //@{
        Real x0() const;
        /*! \todo revise extrapolation */
        Real drift(Time t, Real x) const;
        /*! \todo revise extrapolation */
        Real diffusion(Time t, Real x) const;
        Real apply(Real x0, Real dx) const;
        /*! \warning in general raises a "not implemented" exception.
                     It should be rewritten to return the expectation E(S)
                     of the process, not exp(E(log S)).
        */
        Real expectation(Time t0, Real x0, Time dt) const;
        Real stdDeviation(Time t0, Real x0, Time dt) const;
        Real variance(Time t0, Real x0, Time dt) const;
        Real evolve(Time t0, Real x0, Time dt, Real dw) const;
        //@}
        Time time(const Date&) const;
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        const Handle<Quote>& stateVariable() const;
        const Handle<YieldTermStructure>& dividendYield() const;
        const Handle<YieldTermStructure>& riskFreeRate() const;
        const Handle<BlackVolTermStructure>& blackVolatility() const;
        const Handle<LocalVolTermStructure>& localVolatility() const;
        //@}
      private:
        Handle<Quote> x0_;
        Handle<YieldTermStructure> riskFreeRate_, dividendYield_;
        Handle<BlackVolTermStructure> blackVolatility_;
        bool forceDiscretization_;
        mutable RelinkableHandle<LocalVolTermStructure> localVolatility_;
        mutable bool updated_, isStrikeIndependent_;
    };

    //! Black-Scholes (1973) stochastic process
    /*! This class describes the stochastic process \f$ S \f$ for a stock
        given by
        \f[
            d\ln S(t) = (r(t) - \frac{\sigma(t, S)^2}{2}) dt + \sigma dW_t.
        \f]

        \warning while the interface is expressed in terms of \f$ S \f$,
                 the internal calculations work on \f$ ln S \f$.

        \ingroup processes
    */
    class BlackScholesProcess : public GeneralizedBlackScholesProcess {
      public:
        BlackScholesProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const boost::shared_ptr<discretization>& d =
                  boost::shared_ptr<discretization>(new EulerDiscretization),
            bool forceDiscretization = false);
    };

    //! Merton (1973) extension to the Black-Scholes stochastic process
    /*! This class describes the stochastic process ln(S) for a stock or
        stock index paying a continuous dividend yield given by
        \f[
            d\ln S(t, S) = (r(t) - q(t) - \frac{\sigma(t, S)^2}{2}) dt
                     + \sigma dW_t.
        \f]

        \ingroup processes
    */
    class BlackScholesMertonProcess : public GeneralizedBlackScholesProcess {
      public:
        BlackScholesMertonProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& dividendTS,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const boost::shared_ptr<discretization>& d =
                  boost::shared_ptr<discretization>(new EulerDiscretization),
            bool forceDiscretization = false);
    };

    //! Black (1976) stochastic process
    /*! This class describes the stochastic process \f$ S \f$ for a
        forward or futures contract given by
        \f[
            d\ln S(t) = -\frac{\sigma(t, S)^2}{2} dt + \sigma dW_t.
        \f]

        \warning while the interface is expressed in terms of \f$ S \f$,
                 the internal calculations work on \f$ ln S \f$.

        \ingroup processes
    */
    class BlackProcess : public GeneralizedBlackScholesProcess {
      public:
        BlackProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& riskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const boost::shared_ptr<discretization>& d =
                  boost::shared_ptr<discretization>(new EulerDiscretization),
            bool forceDiscretization = false);
    };

    //! Garman-Kohlhagen (1983) stochastic process
    /*! This class describes the stochastic process \f$ S \f$ for an exchange
        rate given by
        \f[
            d\ln S(t) = (r(t) - r_f(t) - \frac{\sigma(t, S)^2}{2}) dt
                     + \sigma dW_t.
        \f]

        \warning while the interface is expressed in terms of \f$ S \f$,
                 the internal calculations work on \f$ ln S \f$.

        \ingroup processes
    */
    class GarmanKohlagenProcess : public GeneralizedBlackScholesProcess {
      public:
        GarmanKohlagenProcess(
            const Handle<Quote>& x0,
            const Handle<YieldTermStructure>& foreignRiskFreeTS,
            const Handle<YieldTermStructure>& domesticRiskFreeTS,
            const Handle<BlackVolTermStructure>& blackVolTS,
            const boost::shared_ptr<discretization>& d =
                  boost::shared_ptr<discretization>(new EulerDiscretization),
            bool forceDiscretization = false);
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005, 2006, 2007 StatPro Italia srl
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

#include <ql/termstructures/volatility/equityfx/localvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/localvolcurve.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

#include <boost/make_shared.hpp>

namespace QuantLib {

    inline GeneralizedBlackScholesProcess::GeneralizedBlackScholesProcess(
             const Handle<Quote>& x0,
             const Handle<YieldTermStructure>& dividendTS,
             const Handle<YieldTermStructure>& riskFreeTS,
             const Handle<BlackVolTermStructure>& blackVolTS,
             const boost::shared_ptr<discretization>& disc,
             bool forceDiscretization)
    : StochasticProcess1D(disc), x0_(x0), riskFreeRate_(riskFreeTS),
      dividendYield_(dividendTS), blackVolatility_(blackVolTS),
      forceDiscretization_(forceDiscretization), updated_(false) {
        registerWith(x0_);
        registerWith(riskFreeRate_);
        registerWith(dividendYield_);
        registerWith(blackVolatility_);
    }

    inline Real GeneralizedBlackScholesProcess::x0() const {
        return x0_->value();
    }

    inline Real GeneralizedBlackScholesProcess::drift(Time t, Real x) const {
        Real sigma = diffusion(t,x);
        // we could be more anticipatory if we know the right dt
        // for which the drift will be used
        Time t1 = t + 0.0001;
        return riskFreeRate_->forwardRate(t,t1,Continuous,NoFrequency,true)
             - dividendYield_->forwardRate(t,t1,Continuous,NoFrequency,true)
             - 0.5 * sigma * sigma;
    }

    inline Real GeneralizedBlackScholesProcess::diffusion(Time t, Real x) const {
        return localVolatility()->localVol(t, x, true);
    }

    inline Real GeneralizedBlackScholesProcess::apply(Real x0, Real dx) const {
        return x0 * std::exp(dx);
    }

    inline Real GeneralizedBlackScholesProcess::expectation(Time t0,
                                                     Real x0,
                                                     Time dt) const {
        localVolatility(); // trigger update
        if(isStrikeIndependent_ && !forceDiscretization_) {
            // exact value for curves
            return x0 *
                std::exp(dt * (riskFreeRate_->forwardRate(t0, t0 + dt, Continuous,
                                                          NoFrequency, true) -
                             dividendYield_->forwardRate(
                                 t0, t0 + dt, Continuous, NoFrequency, true)));
        } else {
            QL_FAIL("not implemented");
        }
    }

    inline Real GeneralizedBlackScholesProcess::stdDeviation(Time t0, Real x0, Time dt) const {
        localVolatility(); // trigger update
        if(isStrikeIndependent_ && !forceDiscretization_) {
            // exact value for curves
            return std::sqrt(variance(t0,x0,dt));
        }
        else{
            return discretization_->diffusion(*this,t0,x0,dt);
        }
    }

    inline Real GeneralizedBlackScholesProcess::variance(Time t0, Real x0, Time dt) const {
        localVolatility(); // trigger update
        if(isStrikeIndependent_ && !forceDiscretization_) {
            // exact value for curves
            return blackVolatility_->blackVariance(t0 + dt, 0.01) -
                   blackVolatility_->blackVariance(t0, 0.01);
        }
        else{
            return discretization_->variance(*this,t0,x0,dt);
        }
    }

    inline Real GeneralizedBlackScholesProcess::evolve(Time t0, Real x0,
                                                Time dt, Real dw) const {
        localVolatility(); // trigger update
        if (isStrikeIndependent_ && !forceDiscretization_) {
            // exact value for curves
            Real var = variance(t0, x0, dt);
            Real drift = (riskFreeRate_->forwardRate(t0, t0 + dt, Continuous,
                                                     NoFrequency, true) -
                          dividendYield_->forwardRate(t0, t0 + dt, Continuous,
                                                      NoFrequency, true)) *
                             dt -
                         0.5 * var;
            return apply(x0, std::sqrt(var) * dw + drift);
        } else
            return apply(x0, discretization_->drift(*this, t0, x0, dt) +
                                 stdDeviation(t0, x0, dt) * dw);
    }

    inline Time GeneralizedBlackScholesProcess::time(const Date& d) const {
        return riskFreeRate_->dayCounter().yearFraction(
                                           riskFreeRate_->referenceDate(), d);
    }

    inline void GeneralizedBlackScholesProcess::update() {
        updated_ = false;
        StochasticProcess1D::update();
    }

    inline const Handle<Quote>&
    GeneralizedBlackScholesProcess::stateVariable() const {
        return x0_;
    }

    inline const Handle<YieldTermStructure>&
    GeneralizedBlackScholesProcess::dividendYield() const {
        return dividendYield_;
    }

    inline const Handle<YieldTermStructure>&
    GeneralizedBlackScholesProcess::riskFreeRate() const {
        return riskFreeRate_;
    }

    inline const Handle<BlackVolTermStructure>&
    GeneralizedBlackScholesProcess::blackVolatility() const {
        return blackVolatility_;
    }

    inline const Handle<LocalVolTermStructure>&
    GeneralizedBlackScholesProcess::localVolatility() const {
        if (!updated_) {
            isStrikeIndependent_=true;

            // constant Black vol?
            boost::shared_ptr<BlackConstantVol> constVol =
                boost::dynamic_pointer_cast<BlackConstantVol>(
                                                          *blackVolatility());
            if (constVol) {
                // ok, the local vol is constant too.
                localVolatility_.linkTo(boost::make_shared<LocalConstantVol>(
                    constVol->referenceDate(),
                    constVol->blackVol(0.0, x0_->value()),
                    constVol->dayCounter()));
                updated_ = true;
                return localVolatility_;
            }

            // ok, so it's not constant. Maybe it's strike-independent?
            boost::shared_ptr<BlackVarianceCurve> volCurve =
                boost::dynamic_pointer_cast<BlackVarianceCurve>(
                                                          *blackVolatility());
            if (volCurve) {
                // ok, we can use the optimized algorithm
                localVolatility_.linkTo(boost::make_shared<LocalVolCurve>(
                    Handle<BlackVarianceCurve>(volCurve)));
                updated_ = true;
                return localVolatility_;
            }

            // ok, so it's strike-dependent. Never mind.
            localVolatility_.linkTo(
                boost::make_shared<LocalVolSurface>(blackVolatility_, riskFreeRate_,
                                                    dividendYield_, x0_->value()));
            updated_ = true;
            isStrikeIndependent_ = false;
            return localVolatility_;

        } else {
            return localVolatility_;
        }
    }


    // specific models

    inline BlackScholesProcess::BlackScholesProcess(
                              const Handle<Quote>& x0,
                              const Handle<YieldTermStructure>& riskFreeTS,
                              const Handle<BlackVolTermStructure>& blackVolTS,
                              const boost::shared_ptr<discretization>& d,
                              bool forceDiscretization)
    : GeneralizedBlackScholesProcess(
             x0,
             // no dividend yield
             Handle<YieldTermStructure>(boost::shared_ptr<YieldTermStructure>(
                  new FlatForward(0, NullCalendar(), 0.0, Actual365Fixed()))),
             riskFreeTS,
             blackVolTS,
             d,forceDiscretization) {}


    inline BlackScholesMertonProcess::BlackScholesMertonProcess(
                              const Handle<Quote>& x0,
                              const Handle<YieldTermStructure>& dividendTS,
                              const Handle<YieldTermStructure>& riskFreeTS,
                              const Handle<BlackVolTermStructure>& blackVolTS,
                              const boost::shared_ptr<discretization>& d,
                              bool forceDiscretization)
    : GeneralizedBlackScholesProcess(x0,dividendTS,riskFreeTS,blackVolTS,d,
                                     forceDiscretization) {}


    inline BlackProcess::BlackProcess(const Handle<Quote>& x0,
                               const Handle<YieldTermStructure>& riskFreeTS,
                               const Handle<BlackVolTermStructure>& blackVolTS,
                               const boost::shared_ptr<discretization>& d,
                               bool forceDiscretization)
    : GeneralizedBlackScholesProcess(x0,riskFreeTS,riskFreeTS,blackVolTS,d,
                                     forceDiscretization) {}


    inline GarmanKohlagenProcess::GarmanKohlagenProcess(
                          const Handle<Quote>& x0,
                          const Handle<YieldTermStructure>& foreignRiskFreeTS,
                          const Handle<YieldTermStructure>& domesticRiskFreeTS,
                          const Handle<BlackVolTermStructure>& blackVolTS,
                          const boost::shared_ptr<discretization>& d,
                          bool forceDiscretization)
    : GeneralizedBlackScholesProcess(x0,foreignRiskFreeTS,domesticRiskFreeTS,
                                     blackVolTS,d,forceDiscretization) {}

}

#endif