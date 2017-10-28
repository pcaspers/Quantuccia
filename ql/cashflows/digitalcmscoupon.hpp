/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti
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

/*! \file digitalcmscoupon.hpp
    \brief Cms-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_cms_coupon_hpp
#define quantlib_digital_cms_coupon_hpp

#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Cms-rate coupon with digital digital call/put option
    class DigitalCmsCoupon : public DigitalCoupon {
      public:
        DigitalCmsCoupon(
            const boost::shared_ptr<CmsCoupon> &underlying,
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
    class DigitalCmsLeg {
      public:
        DigitalCmsLeg(const Schedule& schedule,
                      const boost::shared_ptr<SwapIndex>& index);
        DigitalCmsLeg& withNotionals(Real notional);
        DigitalCmsLeg& withNotionals(const std::vector<Real>& notionals);
        DigitalCmsLeg& withPaymentDayCounter(const DayCounter&);
        DigitalCmsLeg& withPaymentAdjustment(BusinessDayConvention);
        DigitalCmsLeg& withFixingDays(Natural fixingDays);
        DigitalCmsLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        DigitalCmsLeg& withGearings(Real gearing);
        DigitalCmsLeg& withGearings(const std::vector<Real>& gearings);
        DigitalCmsLeg& withSpreads(Spread spread);
        DigitalCmsLeg& withSpreads(const std::vector<Spread>& spreads);
        DigitalCmsLeg& inArrears(bool flag = true);
        DigitalCmsLeg& withCallStrikes(Rate strike);
        DigitalCmsLeg& withCallStrikes(const std::vector<Rate>& strikes);
        DigitalCmsLeg& withLongCallOption(Position::Type);
        DigitalCmsLeg& withCallATM(bool flag = true);
        DigitalCmsLeg& withCallPayoffs(Rate payoff);
        DigitalCmsLeg& withCallPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsLeg& withPutStrikes(Rate strike);
        DigitalCmsLeg& withPutStrikes(const std::vector<Rate>& strikes);
        DigitalCmsLeg& withLongPutOption(Position::Type);
        DigitalCmsLeg& withPutATM(bool flag = true);
        DigitalCmsLeg& withPutPayoffs(Rate payoff);
        DigitalCmsLeg& withPutPayoffs(const std::vector<Rate>& payoffs);
        DigitalCmsLeg& withReplication(
            const boost::shared_ptr<DigitalReplication> &replication =
                boost::shared_ptr<DigitalReplication>(new DigitalReplication));
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<SwapIndex> index_;
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
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti
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

#include <ql/cashflows/cashflowvectors.hpp>

namespace QuantLib {

  inline DigitalCmsCoupon::DigitalCmsCoupon(
                      const boost::shared_ptr<CmsCoupon>& underlying,
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

  inline void DigitalCmsCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        Visitor<DigitalCmsCoupon>* v1 =
            dynamic_cast<Visitor<DigitalCmsCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }



  inline DigitalCmsLeg::DigitalCmsLeg(const Schedule& schedule,
                                 const boost::shared_ptr<SwapIndex>& index)
    : schedule_(schedule), index_(index),
      paymentAdjustment_(Following), inArrears_(false),
      longCallOption_(Position::Long), callATM_(false),
      longPutOption_(Position::Long), putATM_(false) {}

  inline DigitalCmsLeg& DigitalCmsLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withCallStrikes(Rate strike) {
        callStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withCallStrikes(
                                           const std::vector<Rate>& strikes) {
        callStrikes_ = strikes;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withLongCallOption(Position::Type type) {
        longCallOption_ = type;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withCallATM(bool flag) {
        callATM_ = flag;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withCallPayoffs(Rate payoff) {
        callPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withCallPayoffs(
                                           const std::vector<Rate>& payoffs) {
        callPayoffs_ = payoffs;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPutStrikes(Rate strike) {
        putStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPutStrikes(
                                           const std::vector<Rate>& strikes) {
        putStrikes_ = strikes;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withLongPutOption(Position::Type type) {
        longPutOption_ = type;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPutATM(bool flag) {
        putATM_ = flag;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPutPayoffs(Rate payoff) {
        putPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withPutPayoffs(
                                           const std::vector<Rate>& payoffs) {
        putPayoffs_ = payoffs;
        return *this;
    }

  inline DigitalCmsLeg& DigitalCmsLeg::withReplication(
                   const boost::shared_ptr<DigitalReplication>& replication) {
        replication_ = replication;
        return *this;
    }

  inline DigitalCmsLeg::operator Leg() const {
        return FloatingDigitalLeg<SwapIndex, CmsCoupon, DigitalCmsCoupon>(
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
