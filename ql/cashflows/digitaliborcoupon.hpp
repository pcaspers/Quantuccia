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

/*! \file digitaliborcoupon.hpp
    \brief Ibor-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_ibor_coupon_hpp
#define quantlib_digital_ibor_coupon_hpp

#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Ibor rate coupon with digital digital call/put option
    class DigitalIborCoupon : public DigitalCoupon {
      public:
        DigitalIborCoupon(
            const boost::shared_ptr<IborCoupon> &underlying,
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
    class DigitalIborLeg {
      public:
        DigitalIborLeg(const Schedule& schedule,
                       const boost::shared_ptr<IborIndex>& index);
        DigitalIborLeg& withNotionals(Real notional);
        DigitalIborLeg& withNotionals(const std::vector<Real>& notionals);
        DigitalIborLeg& withPaymentDayCounter(const DayCounter&);
        DigitalIborLeg& withPaymentAdjustment(BusinessDayConvention);
        DigitalIborLeg& withFixingDays(Natural fixingDays);
        DigitalIborLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        DigitalIborLeg& withGearings(Real gearing);
        DigitalIborLeg& withGearings(const std::vector<Real>& gearings);
        DigitalIborLeg& withSpreads(Spread spread);
        DigitalIborLeg& withSpreads(const std::vector<Spread>& spreads);
        DigitalIborLeg& inArrears(bool flag = true);
        DigitalIborLeg& withCallStrikes(Rate strike);
        DigitalIborLeg& withCallStrikes(const std::vector<Rate>& strikes);
        DigitalIborLeg& withLongCallOption(Position::Type);
        DigitalIborLeg& withCallATM(bool flag = true);
        DigitalIborLeg& withCallPayoffs(Rate payoff);
        DigitalIborLeg& withCallPayoffs(const std::vector<Rate>& payoffs);
        DigitalIborLeg& withPutStrikes(Rate strike);
        DigitalIborLeg& withPutStrikes(const std::vector<Rate>& strikes);
        DigitalIborLeg& withLongPutOption(Position::Type);
        DigitalIborLeg& withPutATM(bool flag = true);
        DigitalIborLeg& withPutPayoffs(Rate payoff);
        DigitalIborLeg& withPutPayoffs(const std::vector<Rate>& payoffs);
        DigitalIborLeg &withReplication(
            const boost::shared_ptr<DigitalReplication> &replication =
                boost::shared_ptr<DigitalReplication>(new DigitalReplication));
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<IborIndex> index_;
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

  inline DigitalIborCoupon::DigitalIborCoupon(
                      const boost::shared_ptr<IborCoupon>& underlying,
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

  inline void DigitalIborCoupon::accept(AcyclicVisitor& v) {
        typedef DigitalCoupon super;
        Visitor<DigitalIborCoupon>* v1 =
            dynamic_cast<Visitor<DigitalIborCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }



  inline DigitalIborLeg::DigitalIborLeg(const Schedule& schedule,
                                   const boost::shared_ptr<IborIndex>& index)
    : schedule_(schedule), index_(index),
      paymentAdjustment_(Following), inArrears_(false),
      longCallOption_(Position::Long), callATM_(false),
      longPutOption_(Position::Long), putATM_(false) {}

  inline DigitalIborLeg& DigitalIborLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPaymentDayCounter(
                                               const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1,fixingDays);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withFixingDays(
                                     const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1,gearing);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withGearings(
                                          const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1,spread);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withSpreads(
                                         const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withCallStrikes(Rate strike) {
        callStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withCallStrikes(
                                           const std::vector<Rate>& strikes) {
        callStrikes_ = strikes;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withLongCallOption(Position::Type type) {
        longCallOption_ = type;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withCallATM(bool flag) {
        callATM_ = flag;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withCallPayoffs(Rate payoff) {
        callPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withCallPayoffs(
                                           const std::vector<Rate>& payoffs) {
        callPayoffs_ = payoffs;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPutStrikes(Rate strike) {
        putStrikes_ = std::vector<Rate>(1,strike);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPutStrikes(
                                           const std::vector<Rate>& strikes) {
        putStrikes_ = strikes;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withLongPutOption(Position::Type type) {
        longPutOption_ = type;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPutATM(bool flag) {
        putATM_ = flag;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPutPayoffs(Rate payoff) {
        putPayoffs_ = std::vector<Rate>(1,payoff);
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withPutPayoffs(
                                           const std::vector<Rate>& payoffs) {
        putPayoffs_ = payoffs;
        return *this;
    }

  inline DigitalIborLeg& DigitalIborLeg::withReplication(
                   const boost::shared_ptr<DigitalReplication>& replication) {
        replication_ = replication;
        return *this;
    }

  inline DigitalIborLeg::operator Leg() const {
        return FloatingDigitalLeg<IborIndex, IborCoupon, DigitalIborCoupon>(
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
