/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file smilesection.hpp
    \brief Smile section base class
*/

#ifndef quantlib_smile_section_hpp
#define quantlib_smile_section_hpp

#include <ql/patterns/observable.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/utilities/null.hpp>
#include <ql/option.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    //! interest rate volatility smile section
    /*! This abstract class provides volatility smile section interface */
    class SmileSection : public virtual Observable,
                         public virtual Observer {
      public:
        SmileSection(const Date& d,
                     const DayCounter& dc = DayCounter(),
                     const Date& referenceDate = Date(),
                     const VolatilityType type = ShiftedLognormal,
                     const Rate shift = 0.0);
        SmileSection(Time exerciseTime,
                     const DayCounter& dc = DayCounter(),
                     const VolatilityType type = ShiftedLognormal,
                     const Rate shift = 0.0);
        SmileSection() {}

        virtual ~SmileSection() {}

        virtual void update();
        virtual Real minStrike() const = 0;
        virtual Real maxStrike() const = 0;
        Real variance(Rate strike) const;
        Volatility volatility(Rate strike) const;
        virtual Real atmLevel() const = 0;
        virtual const Date& exerciseDate() const { return exerciseDate_; }
        virtual VolatilityType volatilityType() const {
            return volatilityType_;
        }
        virtual Rate shift() const { return shift_; }
        virtual const Date& referenceDate() const;
        virtual Time exerciseTime() const { return exerciseTime_; }
        virtual const DayCounter& dayCounter() const { return dc_; }
        virtual Real optionPrice(Rate strike,
                                 Option::Type type = Option::Call,
                                 Real discount=1.0) const;
        virtual Real digitalOptionPrice(Rate strike,
                                        Option::Type type = Option::Call,
                                        Real discount=1.0,
                                        Real gap=1.0e-5) const;
        virtual Real vega(Rate strike,
                          Real discount=1.0) const;
        virtual Real density(Rate strike,
                             Real discount=1.0,
                             Real gap=1.0E-4) const;
        Volatility volatility(Rate strike, VolatilityType type, Real shift=0.0) const;
      protected:
        virtual void initializeExerciseTime() const;
        virtual Real varianceImpl(Rate strike) const;
        virtual Volatility volatilityImpl(Rate strike) const = 0;
      private:
        bool isFloating_;
        mutable Date referenceDate_;
        Date exerciseDate_;
        DayCounter dc_;
        mutable Time exerciseTime_;
        VolatilityType volatilityType_;
        Rate shift_;
    };


    // inline definitions

    inline Real SmileSection::variance(Rate strike) const {
        return varianceImpl(strike);
    }

    inline Volatility SmileSection::volatility(Rate strike) const {
        return volatilityImpl(strike);
    }

    inline const Date& SmileSection::referenceDate() const {
        QL_REQUIRE(referenceDate_!=Date(),
                   "referenceDate not available for this instance");
        return referenceDate_;
    }

    inline Real SmileSection::varianceImpl(Rate strike) const {
        Volatility v = volatilityImpl(strike);
        return v*v*exerciseTime();
    }

}




/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2013, 2015 Peter Caspers

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
#include <ql/settings.hpp>
#include <ql/math/comparison.hpp>

using std::sqrt;

namespace QuantLib {

    inline void SmileSection::update() {
        if (isFloating_) {
            referenceDate_ = Settings::instance().evaluationDate();
            initializeExerciseTime();
        }
    }

    inline void SmileSection::initializeExerciseTime() const {
        QL_REQUIRE(exerciseDate_>=referenceDate_,
                   "expiry date (" << exerciseDate_ <<
                   ") must be greater than reference date (" <<
                   referenceDate_ << ")");
        exerciseTime_ = dc_.yearFraction(referenceDate_, exerciseDate_);
    }

    inline SmileSection::SmileSection(const Date& d,
                               const DayCounter& dc,
                               const Date& referenceDate,
                               const VolatilityType type,
                               const Rate shift)
        : exerciseDate_(d), dc_(dc), volatilityType_(type), shift_(shift) {
        isFloating_ = referenceDate==Date();
        if (isFloating_) {
            registerWith(Settings::instance().evaluationDate());
            referenceDate_ = Settings::instance().evaluationDate();
        } else
            referenceDate_ = referenceDate;
        initializeExerciseTime();
    }

    inline SmileSection::SmileSection(Time exerciseTime,
                               const DayCounter& dc,
                               const VolatilityType type,
                               const Rate shift)
    : isFloating_(false), referenceDate_(Date()),
      dc_(dc), exerciseTime_(exerciseTime), volatilityType_(type), shift_(shift) {
        QL_REQUIRE(exerciseTime_>=0.0,
                   "expiry time must be positive: " <<
                   exerciseTime_ << " not allowed");
    }

    inline Real SmileSection::optionPrice(Rate strike,
                                   Option::Type type,
                                   Real discount) const {
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute option price");
        // if lognormal or shifted lognormal,
        // for strike at -shift, return option price even if outside
        // minstrike, maxstrike interval
        if (volatilityType() == ShiftedLognormal)
            return blackFormula(type,strike,atm, std::fabs(strike+shift()) < QL_EPSILON ?
                            0.2 : sqrt(variance(strike)),discount,shift());
        else
            return bachelierBlackFormula(type,strike,atm,sqrt(variance(strike)),discount);
    }

    inline Real SmileSection::digitalOptionPrice(Rate strike,
                                          Option::Type type,
                                          Real discount,
                                          Real gap) const {
        Real m = volatilityType() == ShiftedLognormal ? -shift() : -QL_MAX_REAL;
        Real kl = std::max(strike-gap/2.0,m);
        Real kr = kl+gap;
        return (type==Option::Call ? 1.0 : -1.0) *
            (optionPrice(kl,type,discount)-optionPrice(kr,type,discount)) / gap;
    }

    inline Real SmileSection::density(Rate strike, Real discount, Real gap) const {
        Real m = volatilityType() == ShiftedLognormal ? -shift() : -QL_MAX_REAL;
        Real kl = std::max(strike-gap/2.0,m);
        Real kr = kl+gap;
        return (digitalOptionPrice(kl,Option::Call,discount,gap) -
                digitalOptionPrice(kr,Option::Call,discount,gap)) / gap;
    }

    inline Real SmileSection::vega(Rate strike, Real discount) const {
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute option vega");
        if (volatilityType() == ShiftedLognormal)
            return blackFormulaVolDerivative(strike,atmLevel(),
                                             sqrt(variance(strike)),
                                             exerciseTime(),discount,shift())*0.01;
        else
            QL_FAIL("vega for normal smilesection not yet implemented");
    }

    inline Real SmileSection::volatility(Rate strike, VolatilityType volatilityType,
                                  Real shift) const {
        if(volatilityType == volatilityType_ && close(shift,this->shift()))
            return volatility(strike);
        Real atm = atmLevel();
        QL_REQUIRE(atm != Null<Real>(),
                   "smile section must provide atm level to compute converted volatilties");
        Option::Type type = strike >= atm ? Option::Call : Option::Put;
        Real premium = optionPrice(strike,type);
        Real premiumAtm = optionPrice(atm,type);
        if (volatilityType == ShiftedLognormal) {
            try {
                return blackFormulaImpliedStdDev(type, strike, atm, premium,
                                                 1.0, shift) /
                       std::sqrt(exerciseTime());
            } catch(...) {
                return blackFormulaImpliedStdDevChambers(
                    type, strike, atm, premium, premiumAtm, 1.0, shift) /
                       std::sqrt(exerciseTime());
            }
        } else {
                return bachelierBlackFormulaImpliedVol(type, strike, atm,
                                                       exerciseTime(), premium);
            }
    }
}
#endif