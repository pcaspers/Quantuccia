/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

/*! \file iborcoupon.hpp
    \brief Coupon paying a Libor-type index
*/

#ifndef quantlib_ibor_coupon_hpp
#define quantlib_ibor_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! %Coupon paying a Libor-type index
    class IborCoupon : public FloatingRateCoupon {
      public:
        IborCoupon(const Date& paymentDate,
                   Real nominal,
                   const Date& startDate,
                   const Date& endDate,
                   Natural fixingDays,
                   const boost::shared_ptr<IborIndex>& index,
                   Real gearing = 1.0,
                   Spread spread = 0.0,
                   const Date& refPeriodStart = Date(),
                   const Date& refPeriodEnd = Date(),
                   const DayCounter& dayCounter = DayCounter(),
                   bool isInArrears = false);
        //! \name Inspectors
        //@{
        const boost::shared_ptr<IborIndex>& iborIndex() const {
            return iborIndex_;
        }
        //! this is dependent on QL_USE_INDEXED_COUPON
        const Date& fixingEndDate() const { return fixingEndDate_; }
        //@}
        //! \name FloatingRateCoupon interface
        //@{
        //! Implemented in order to manage the case of par coupon
        Rate indexFixing() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<IborIndex> iborIndex_;
        Date fixingDate_, fixingValueDate_, fixingEndDate_;
        Time spanningTime_;
    };


    //! helper class building a sequence of capped/floored ibor-rate coupons
    class IborLeg {
      public:
        IborLeg(const Schedule& schedule,
                const boost::shared_ptr<IborIndex>& index);
        IborLeg& withNotionals(Real notional);
        IborLeg& withNotionals(const std::vector<Real>& notionals);
        IborLeg& withPaymentDayCounter(const DayCounter&);
        IborLeg& withPaymentAdjustment(BusinessDayConvention);
        IborLeg& withPaymentLag(Natural lag);
        IborLeg& withPaymentCalendar(const Calendar&);
        IborLeg& withFixingDays(Natural fixingDays);
        IborLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        IborLeg& withGearings(Real gearing);
        IborLeg& withGearings(const std::vector<Real>& gearings);
        IborLeg& withSpreads(Spread spread);
        IborLeg& withSpreads(const std::vector<Spread>& spreads);
        IborLeg& withCaps(Rate cap);
        IborLeg& withCaps(const std::vector<Rate>& caps);
        IborLeg& withFloors(Rate floor);
        IborLeg& withFloors(const std::vector<Rate>& floors);
        IborLeg& inArrears(bool flag = true);
        IborLeg& withZeroPayments(bool flag = true);
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<IborIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        Natural paymentLag_;
        Calendar paymentCalendar_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_, zeroPayments_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2010, 2011 Ferdinando Ametrano
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

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

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

using boost::shared_ptr;

namespace QuantLib {

    inline IborCoupon::IborCoupon(const Date& paymentDate,
                           Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           Natural fixingDays,
                           const shared_ptr<IborIndex>& iborIndex,
                           Real gearing,
                           Spread spread,
                           const Date& refPeriodStart,
                           const Date& refPeriodEnd,
                           const DayCounter& dayCounter,
                           bool isInArrears)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, iborIndex, gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears),
      iborIndex_(iborIndex) {

        fixingDate_ = fixingDate();

        const Calendar& fixingCalendar = index_->fixingCalendar();
        Natural indexFixingDays = index_->fixingDays();

        fixingValueDate_ = fixingCalendar.advance(
            fixingDate_, indexFixingDays, Days);

        #ifdef QL_USE_INDEXED_COUPON
        fixingEndDate_ = index_->maturityDate(fixingValueDate_);
        #else
        if (isInArrears_)
            fixingEndDate_ = index_->maturityDate(fixingValueDate_);
        else { // par coupon approximation
            Date nextFixingDate = fixingCalendar.advance(
                accrualEndDate_, -static_cast<Integer>(fixingDays_), Days);
            fixingEndDate_ = fixingCalendar.advance(
                nextFixingDate, indexFixingDays, Days);
        }
        #endif

        const DayCounter& dc = index_->dayCounter();
        spanningTime_ = dc.yearFraction(fixingValueDate_,
                                        fixingEndDate_);
        QL_REQUIRE(spanningTime_>0.0,
                   "\n cannot calculate forward rate between " <<
                   fixingValueDate_ << " and " << fixingEndDate_ <<
                   ":\n non positive time (" << spanningTime_ <<
                   ") using " << dc.name() << " daycounter");
    }

    inline Rate IborCoupon::indexFixing() const {

        /* instead of just returning index_->fixing(fixingValueDate_)
           its logic is duplicated here using a specialized iborIndex
           forecastFixing overload which
           1) allows to save date/time recalculations, and
           2) takes into account par coupon needs
        */
        Date today = Settings::instance().evaluationDate();

        if (fixingDate_>today)
            return iborIndex_->forecastFixing(fixingValueDate_,
                                              fixingEndDate_,
                                              spanningTime_);

        if (fixingDate_<today ||
            Settings::instance().enforcesTodaysHistoricFixings()) {
            // do not catch exceptions
            Rate result = index_->pastFixing(fixingDate_);
            QL_REQUIRE(result != Null<Real>(),
                       "Missing " << index_->name() << " fixing for " << fixingDate_);
            return result;
        }

        try {
            Rate result = index_->pastFixing(fixingDate_);
            if (result!=Null<Real>())
                return result;
            else
                ;   // fall through and forecast
        } catch (Error&) {
                ;   // fall through and forecast
        }
        return iborIndex_->forecastFixing(fixingValueDate_,
                                          fixingEndDate_,
                                          spanningTime_);
    }

    inline void IborCoupon::accept(AcyclicVisitor& v) {
        Visitor<IborCoupon>* v1 =
            dynamic_cast<Visitor<IborCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }



    inline IborLeg::IborLeg(const Schedule& schedule,
                     const shared_ptr<IborIndex>& index)
    : schedule_(schedule), index_(index),
      paymentAdjustment_(Following),
      paymentLag_(0), paymentCalendar_(Calendar()),
      inArrears_(false), zeroPayments_(false) {}

    inline IborLeg& IborLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    inline IborLeg& IborLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    inline IborLeg& IborLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

    inline IborLeg& IborLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    inline IborLeg& IborLeg::withPaymentLag(Natural lag) {
        paymentLag_ = lag;
        return *this;
    }

    inline IborLeg& IborLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    inline IborLeg& IborLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

    inline IborLeg& IborLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

    inline IborLeg& IborLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

    inline IborLeg& IborLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

    inline IborLeg& IborLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

    inline IborLeg& IborLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

    inline IborLeg& IborLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1,cap);
        return *this;
    }

    inline IborLeg& IborLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

    inline IborLeg& IborLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1,floor);
        return *this;
    }

    inline IborLeg& IborLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

    inline IborLeg& IborLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

    inline IborLeg& IborLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

}


#endif
