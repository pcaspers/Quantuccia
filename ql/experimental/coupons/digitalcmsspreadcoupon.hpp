/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file digitalcmsspreadcoupon.hpp
    \brief Cms-spread-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_cmsspread_coupon_hpp
#define quantlib_digital_cmsspread_coupon_hpp

#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/experimental/coupons/cmsspreadcoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Cms-spread-rate coupon with digital digital call/put option
    class DigitalCmsSpreadCoupon : public DigitalCoupon {
      public:
        DigitalCmsSpreadCoupon(
            const boost::shared_ptr<CmsSpreadCoupon> &underlying,
            Rate callStrike = Null<Rate>(),
            Position::Type callPosition = Position::Long,
            bool isCallATMIncluded = false,
            Rate callDigitalPayoff = Null<Rate>(),
            Rate putStrike = Null<Rate>(),
            Position::Type putPosition = Position::Long,
            bool isPutATMIncluded = false,
            Rate putDigitalPayoff = Null<Rate>(),
            const boost::shared_ptr<DigitalReplication> &replication =
                boost::shared_ptr<DigitalReplication>(new DigitalReplication));

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    //! helper class building a sequence of digital ibor-rate coupons
    class DigitalCmsSpreadLeg {
      public:
        DigitalCmsSpreadLeg(const Schedule& schedule,
                      const boost::shared_ptr<SwapSpreadIndex>& index);
        DigitalCmsSpreadLeg& withNotionals(Real notional);
        DigitalCmsSpreadLeg& withNotionals(const std::vector<Real>& notionals);
        DigitalCmsSpreadLeg& withPaymentDayCounter(const DayCounter&);
        DigitalCmsSpreadLeg& withPaymentAdjustment(BusinessDayConvention);
        DigitalCmsSpreadLeg& withFixingDays(Natural fixingDays);
        DigitalCmsSpreadLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        DigitalCmsSpreadLeg& withGearings(Real gearing);
        DigitalCmsSpreadLeg& withGearings(const std::vector<Real>& gearings);
        DigitalCmsSpreadLeg& withSpreads(Spread spread);
        DigitalCmsSpreadLeg& withSpreads(const std::vector<Spread>& spreads);
        DigitalCmsSpreadLeg& inArrears(bool flag = true);
        DigitalCmsSpreadLeg& withCallStrikes(Rate strike);
        DigitalCmsSpreadLeg& withCallStrikes(const std::vector<Rate>& strikes);
        DigitalCmsSpreadLeg& withLongCallOption(Position::Type);
        DigitalCmsSpreadLeg& withCallATM(bool flag = true);
        DigitalCmsSpreadLeg& withCallPayoffs(Rate payoff);
        DigitalCmsSpreadLeg& withCallPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsSpreadLeg& withPutStrikes(Rate strike);
        DigitalCmsSpreadLeg& withPutStrikes(const std::vector<Rate>& strikes);
        DigitalCmsSpreadLeg& withLongPutOption(Position::Type);
        DigitalCmsSpreadLeg& withPutATM(bool flag = true);
        DigitalCmsSpreadLeg& withPutPayoffs(Rate payoff);
        DigitalCmsSpreadLeg& withPutPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsSpreadLeg& withReplication(
            const boost::shared_ptr<DigitalReplication> &replication =
                boost::shared_ptr<DigitalReplication>(new DigitalReplication));

        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<SwapSpreadIndex> index_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        bool inArrears_;
        std::vector<Rate> callStrikes_, callPayoffs_;
        Position::Type longCallOption_;
        bool callATM_;
        std::vector<Rate> putStrikes_, putPayoffs_;
        Position::Type longPutOption_;
        bool putATM_;
        boost::shared_ptr<DigitalReplication> replication_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/cashflows/cashflowvectors.hpp>

namespace QuantLib {

  inline DigitalCmsSpreadCoupon::DigitalCmsSpreadCoupon(
                      const boost::shared_ptr<CmsSpreadCoupon>& underlying,
                      Rate callStrike,
                      Position::Type callPosition,
                      bool isCallATMIncluded,
                      Rate callDigitalPayoff,
                      Rate putStrike,
                      Position::Type putPosition,
                      bool isPutATMIncluded,
                      Rate putDigitalPayoff,
                      const boost::shared_ptr<DigitalReplication>& replication)
    : DigitalCoupon(underlying, callStrike, callPosition, isCallATMIncluded,
                    callDigitalPayoff, putStrike, putPosition,
                    isPutATMIncluded, putDigitalPayoff, replication) {}

  inline void DigitalCmsSpreadCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        Visitor<DigitalCmsSpreadCoupon>* v1 =
            dynamic_cast<Visitor<DigitalCmsSpreadCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }



  inline DigitalCmsSpreadLeg::DigitalCmsSpreadLeg(const Schedule& schedule,
                                 const boost::shared_ptr<SwapSpreadIndex>& index)
    : schedule_(schedule), index_(index),
      paymentAdjustment_(Following), inArrears_(false),
      longCallOption_(Position::Long), callATM_(false),
      longPutOption_(Position::Long), putATM_(false) {}

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallStrikes(Rate strike) {
        callStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallStrikes(
                                           const std::vector<Rate>& strikes) {
        callStrikes_ = strikes;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withLongCallOption(Position::Type type) {
        longCallOption_ = type;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallATM(bool flag) {
        callATM_ = flag;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallPayoffs(Rate payoff) {
        callPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withCallPayoffs(
                                           const std::vector<Rate>& payoffs) {
        callPayoffs_ = payoffs;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutStrikes(Rate strike) {
        putStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutStrikes(
                                           const std::vector<Rate>& strikes) {
        putStrikes_ = strikes;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withLongPutOption(Position::Type type) {
        longPutOption_ = type;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutATM(bool flag) {
        putATM_ = flag;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutPayoffs(Rate payoff) {
        putPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withPutPayoffs(
                                           const std::vector<Rate>& payoffs) {
        putPayoffs_ = payoffs;
        return *this;
    }

  inline DigitalCmsSpreadLeg& DigitalCmsSpreadLeg::withReplication(
                   const boost::shared_ptr<DigitalReplication>& replication) {
        replication_ = replication;
        return *this;
    }

  inline DigitalCmsSpreadLeg::operator Leg() const {
        return FloatingDigitalLeg<SwapSpreadIndex, CmsSpreadCoupon, DigitalCmsSpreadCoupon>(
                            schedule_, notionals_, index_, paymentDayCounter_,
                            paymentAdjustment_, fixingDays_,
                            gearings_, spreads_, inArrears_,
                            callStrikes_, longCallOption_,
                            callATM_, callPayoffs_,
                            putStrikes_, longPutOption_,
                            putATM_, putPayoffs_,
                            replication_);
    }

}


#endif
