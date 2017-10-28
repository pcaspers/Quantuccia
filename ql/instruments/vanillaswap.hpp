/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2006, 2008 Ferdinando Ametrano

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

/*! \file vanillaswap.hpp
    \brief Simple fixed-rate vs Libor swap
*/

#ifndef quantlib_vanilla_swap_hpp
#define quantlib_vanilla_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    class IborIndex;

    //! Plain-vanilla swap: fix vs floating leg
    /*! \ingroup instruments

        If no payment convention is passed, the convention of the
        floating-rate schedule is used.

        \warning if <tt>Settings::includeReferenceDateCashFlows()</tt>
                 is set to <tt>true</tt>, payments occurring at the
                 settlement date of the swap might be included in the
                 NPV and therefore affect the fair-rate and
                 fair-spread calculation. This might not be what you
                 want.

        \test
        - the correctness of the returned value is tested by checking
          that the price of a swap paying the fair fixed rate is null.
        - the correctness of the returned value is tested by checking
          that the price of a swap receiving the fair floating-rate
          spread is null.
        - the correctness of the returned value is tested by checking
          that the price of a swap decreases with the paid fixed rate.
        - the correctness of the returned value is tested by checking
          that the price of a swap increases with the received
          floating-rate spread.
        - the correctness of the returned value is tested by checking
          it against a known good value.
    */
    class VanillaSwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        class arguments;
        class results;
        class engine;
        VanillaSwap(
            Type type,
            Real nominal,
            const Schedule& fixedSchedule,
            Rate fixedRate,
            const DayCounter& fixedDayCount,
            const Schedule& floatSchedule,
            const boost::shared_ptr<IborIndex>& iborIndex,
            Spread spread,
            const DayCounter& floatingDayCount,
            boost::optional<BusinessDayConvention> paymentConvention =
                                                                 boost::none);
        //! \name Inspectors
        //@{
        Type type() const;
        Real nominal() const;

        const Schedule& fixedSchedule() const;
        Rate fixedRate() const;
        const DayCounter& fixedDayCount() const;

        const Schedule& floatingSchedule() const;
        const boost::shared_ptr<IborIndex>& iborIndex() const;
        Spread spread() const;
        const DayCounter& floatingDayCount() const;

        BusinessDayConvention paymentConvention() const;

        const Leg& fixedLeg() const;
        const Leg& floatingLeg() const;
        //@}

        //! \name Results
        //@{
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Rate fairRate() const;

        Real floatingLegBPS() const;
        Real floatingLegNPV() const;
        Spread fairSpread() const;
        //@}
        // other
        void setupArguments(PricingEngine::arguments* args) const;
        void fetchResults(const PricingEngine::results*) const;
      private:
        void setupExpired() const;
        Type type_;
        Real nominal_;
        Schedule fixedSchedule_;
        Rate fixedRate_;
        DayCounter fixedDayCount_;
        Schedule floatingSchedule_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Spread spread_;
        DayCounter floatingDayCount_;
        BusinessDayConvention paymentConvention_;
        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
    };


    //! %Arguments for simple swap calculation
    class VanillaSwap::arguments : public Swap::arguments {
      public:
        arguments() : type(Receiver),
                      nominal(Null<Real>()) {}
        Type type;
        Real nominal;

        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;

        std::vector<Real> fixedCoupons;
        std::vector<Spread> floatingSpreads;
        std::vector<Real> floatingCoupons;
        void validate() const;
    };

    //! %Results from simple swap calculation
    class VanillaSwap::results : public Swap::results {
      public:
        Rate fairRate;
        Spread fairSpread;
        void reset();
    };

    class VanillaSwap::engine : public GenericEngine<VanillaSwap::arguments,
                                                     VanillaSwap::results> {};


    // inline definitions

    inline VanillaSwap::Type VanillaSwap::type() const {
        return type_;
    }

    inline Real VanillaSwap::nominal() const {
        return nominal_;
    }

    inline const Schedule& VanillaSwap::fixedSchedule() const {
        return fixedSchedule_;
    }

    inline Rate VanillaSwap::fixedRate() const {
        return fixedRate_;
    }

    inline const DayCounter& VanillaSwap::fixedDayCount() const {
        return fixedDayCount_;
    }

    inline const Schedule& VanillaSwap::floatingSchedule() const {
        return floatingSchedule_;
    }

    inline const boost::shared_ptr<IborIndex>& VanillaSwap::iborIndex() const {
        return iborIndex_;
    }

    inline Spread VanillaSwap::spread() const {
        return spread_;
    }

    inline const DayCounter& VanillaSwap::floatingDayCount() const {
        return floatingDayCount_;
    }

    inline BusinessDayConvention VanillaSwap::paymentConvention() const {
        return paymentConvention_;
    }

    inline const Leg& VanillaSwap::fixedLeg() const {
        return legs_[0];
    }

    inline const Leg& VanillaSwap::floatingLeg() const {
        return legs_[1];
    }

    std::ostream& operator<<(std::ostream& out,
                             VanillaSwap::Type t);

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2007 Ferdinando Ametrano

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

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    inline VanillaSwap::VanillaSwap(
                     Type type,
                     Real nominal,
                     const Schedule& fixedSchedule,
                     Rate fixedRate,
                     const DayCounter& fixedDayCount,
                     const Schedule& floatSchedule,
                     const boost::shared_ptr<IborIndex>& iborIndex,
                     Spread spread,
                     const DayCounter& floatingDayCount,
                     boost::optional<BusinessDayConvention> paymentConvention)
    : Swap(2), type_(type), nominal_(nominal),
      fixedSchedule_(fixedSchedule), fixedRate_(fixedRate),
      fixedDayCount_(fixedDayCount),
      floatingSchedule_(floatSchedule), iborIndex_(iborIndex), spread_(spread),
      floatingDayCount_(floatingDayCount) {

        if (paymentConvention)
            paymentConvention_ = *paymentConvention;
        else
            paymentConvention_ = floatingSchedule_.businessDayConvention();

        legs_[0] = FixedRateLeg(fixedSchedule_)
            .withNotionals(nominal_)
            .withCouponRates(fixedRate_, fixedDayCount_)
            .withPaymentAdjustment(paymentConvention_);

        legs_[1] = IborLeg(floatingSchedule_, iborIndex_)
            .withNotionals(nominal_)
            .withPaymentDayCounter(floatingDayCount_)
            .withPaymentAdjustment(paymentConvention_)
            .withSpreads(spread_);
        for (Leg::const_iterator i = legs_[1].begin(); i < legs_[1].end(); ++i)
            registerWith(*i);

        switch (type_) {
          case Payer:
            payer_[0] = -1.0;
            payer_[1] = +1.0;
            break;
          case Receiver:
            payer_[0] = +1.0;
            payer_[1] = -1.0;
            break;
          default:
            QL_FAIL("Unknown vanilla-swap type");
        }
    }

    inline void VanillaSwap::setupArguments(PricingEngine::arguments* args) const {

        Swap::setupArguments(args);

        VanillaSwap::arguments* arguments =
            dynamic_cast<VanillaSwap::arguments*>(args);

        if (!arguments)  // it's a swap engine...
            return;

        arguments->type = type_;
        arguments->nominal = nominal_;

        const Leg& fixedCoupons = fixedLeg();

        arguments->fixedResetDates = arguments->fixedPayDates =
            std::vector<Date>(fixedCoupons.size());
        arguments->fixedCoupons = std::vector<Real>(fixedCoupons.size());

        for (Size i=0; i<fixedCoupons.size(); ++i) {
            boost::shared_ptr<FixedRateCoupon> coupon =
                boost::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            arguments->fixedPayDates[i] = coupon->date();
            arguments->fixedResetDates[i] = coupon->accrualStartDate();
            arguments->fixedCoupons[i] = coupon->amount();
        }

        const Leg& floatingCoupons = floatingLeg();

        arguments->floatingResetDates = arguments->floatingPayDates =
            arguments->floatingFixingDates =
            std::vector<Date>(floatingCoupons.size());
        arguments->floatingAccrualTimes =
            std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            std::vector<Spread>(floatingCoupons.size());
        arguments->floatingCoupons = std::vector<Real>(floatingCoupons.size());
        for (Size i=0; i<floatingCoupons.size(); ++i) {
            boost::shared_ptr<IborCoupon> coupon =
                boost::dynamic_pointer_cast<IborCoupon>(floatingCoupons[i]);

            arguments->floatingResetDates[i] = coupon->accrualStartDate();
            arguments->floatingPayDates[i] = coupon->date();

            arguments->floatingFixingDates[i] = coupon->fixingDate();
            arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
            arguments->floatingSpreads[i] = coupon->spread();
            try {
                arguments->floatingCoupons[i] = coupon->amount();
            } catch (Error&) {
                arguments->floatingCoupons[i] = Null<Real>();
            }
        }
    }

    inline Rate VanillaSwap::fairRate() const {
        calculate();
        QL_REQUIRE(fairRate_ != Null<Rate>(), "result not available");
        return fairRate_;
    }

    inline Spread VanillaSwap::fairSpread() const {
        calculate();
        QL_REQUIRE(fairSpread_ != Null<Spread>(), "result not available");
        return fairSpread_;
    }

    inline Real VanillaSwap::fixedLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[0] != Null<Real>(), "result not available");
        return legBPS_[0];
    }

    inline Real VanillaSwap::floatingLegBPS() const {
        calculate();
        QL_REQUIRE(legBPS_[1] != Null<Real>(), "result not available");
        return legBPS_[1];
    }

    inline Real VanillaSwap::fixedLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[0] != Null<Real>(), "result not available");
        return legNPV_[0];
    }

    inline Real VanillaSwap::floatingLegNPV() const {
        calculate();
        QL_REQUIRE(legNPV_[1] != Null<Real>(), "result not available");
        return legNPV_[1];
    }

    inline void VanillaSwap::setupExpired() const {
        Swap::setupExpired();
        legBPS_[0] = legBPS_[1] = 0.0;
        fairRate_ = Null<Rate>();
        fairSpread_ = Null<Spread>();
    }

    inline void VanillaSwap::fetchResults(const PricingEngine::results* r) const {
        static const Spread basisPoint = 1.0e-4;

        Swap::fetchResults(r);

        const VanillaSwap::results* results =
            dynamic_cast<const VanillaSwap::results*>(r);
        if (results) { // might be a swap engine, so no error is thrown
            fairRate_ = results->fairRate;
            fairSpread_ = results->fairSpread;
        } else {
            fairRate_ = Null<Rate>();
            fairSpread_ = Null<Spread>();
        }

        if (fairRate_ == Null<Rate>()) {
            // calculate it from other results
            if (legBPS_[0] != Null<Real>())
                fairRate_ = fixedRate_ - NPV_/(legBPS_[0]/basisPoint);
        }
        if (fairSpread_ == Null<Spread>()) {
            // ditto
            if (legBPS_[1] != Null<Real>())
                fairSpread_ = spread_ - NPV_/(legBPS_[1]/basisPoint);
        }
    }

    inline void VanillaSwap::arguments::validate() const {
        Swap::arguments::validate();
        QL_REQUIRE(nominal != Null<Real>(), "nominal null or not set");
        QL_REQUIRE(fixedResetDates.size() == fixedPayDates.size(),
                   "number of fixed start dates different from "
                   "number of fixed payment dates");
        QL_REQUIRE(fixedPayDates.size() == fixedCoupons.size(),
                   "number of fixed payment dates different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(floatingResetDates.size() == floatingPayDates.size(),
                   "number of floating start dates different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingFixingDates.size() == floatingPayDates.size(),
                   "number of floating fixing dates different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingAccrualTimes.size() == floatingPayDates.size(),
                   "number of floating accrual Times different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingSpreads.size() == floatingPayDates.size(),
                   "number of floating spreads different from "
                   "number of floating payment dates");
        QL_REQUIRE(floatingPayDates.size() == floatingCoupons.size(),
                   "number of floating payment dates different from "
                   "number of floating coupon amounts");
    }

    inline void VanillaSwap::results::reset() {
        Swap::results::reset();
        fairRate = Null<Rate>();
        fairSpread = Null<Spread>();
    }

    inline std::ostream& operator<<(std::ostream& out,
                             VanillaSwap::Type t) {
        switch (t) {
          case VanillaSwap::Payer:
            return out << "Payer";
          case VanillaSwap::Receiver:
            return out << "Receiver";
          default:
            QL_FAIL("unknown VanillaSwap::Type(" << Integer(t) << ")");
        }
    }

}


#endif
