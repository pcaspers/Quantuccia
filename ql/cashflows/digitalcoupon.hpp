/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2007 Cristina Duminuco
  Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file digitalcoupon.hpp
    \brief Floating-rate coupon with digital call/put option
*/

#ifndef quantlib_digital_coupon_hpp
#define quantlib_digital_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/replication.hpp>
#include <ql/position.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    //! Digital-payoff coupon
    /*! Implementation of a floating-rate coupon with digital call/put option.
        Payoffs:
        - Coupon with cash-or-nothing Digital Call
          rate + csi * payoffRate * Heaviside(rate-strike)
        - Coupon with cash-or-nothing Digital Put
          rate + csi * payoffRate * Heaviside(strike-rate)
        where csi=+1 or csi=-1.
        - Coupon with asset-or-nothing Digital Call
          rate + csi * rate * Heaviside(rate-strike)
        - Coupon with asset-or-nothing Digital Put
          rate + csi * rate * Heaviside(strike-rate)
        where csi=+1 or csi=-1.
        The evaluation of the coupon is made using the call/put spread
        replication method.
    */
    /*! \ingroup instruments

        \test
        - the correctness of the returned value in case of Asset-or-nothing
          embedded option is tested by pricing the digital option with
          Cox-Rubinstein formula.
        - the correctness of the returned value in case of deep-in-the-money
          Asset-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value in case of deep-out-of-the-money
          Asset-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value in case of Cash-or-nothing
          embedded option is tested by pricing the digital option with
          Reiner-Rubinstein formula.
        - the correctness of the returned value in case of deep-in-the-money
          Cash-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value in case of deep-out-of-the-money
          Cash-or-nothing embedded option is tested vs the expected values of
          coupon and option.
        - the correctness of the returned value is tested checking the correctness
          of the call-put parity relation.
        - the correctness of the returned value is tested by the relationship
          between prices in case of different replication types.
    */
    class DigitalCoupon : public FloatingRateCoupon {
      public:
        //! \name Constructors
        //@{
        //! general constructor
        DigitalCoupon(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                      Rate callStrike = Null<Rate>(),
                      Position::Type callPosition = Position::Long,
                      bool isCallITMIncluded = false,
                      Rate callDigitalPayoff = Null<Rate>(),
                      Rate putStrike = Null<Rate>(),
                      Position::Type putPosition = Position::Long,
                      bool isPutITMIncluded = false,
                      Rate putDigitalPayoff = Null<Rate>(),
                      const boost::shared_ptr<DigitalReplication>& replication =
                        boost::shared_ptr<DigitalReplication>() );

        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const;
        Rate convexityAdjustment() const;
        //@}
        //@}
        //! \name Digital inspectors
        //@{
        Rate callStrike() const;
        Rate putStrike() const;
        Rate callDigitalPayoff() const;
        Rate putDigitalPayoff() const;
        bool hasPut() const { return hasPutStrike_; }
        bool hasCall() const {return hasCallStrike_; }
        bool hasCollar() const {return (hasCallStrike_ && hasPutStrike_); }
        bool isLongPut() const { return (putCsi_==1.); }
        bool isLongCall() const { return (callCsi_==1.); }
        boost::shared_ptr<FloatingRateCoupon> underlying() const { return underlying_; }
        /*! Returns the call option rate
           (multiplied by: nominal*accrualperiod*discount is the NPV of the option)
        */
        Rate callOptionRate() const;
        /*! Returns the put option rate
           (multiplied by: nominal*accrualperiod*discount is the NPV of the option)
        */
        Rate putOptionRate() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);

        void setPricer(
            const boost::shared_ptr<FloatingRateCouponPricer>& pricer) {
            if (pricer_)
                unregisterWith(pricer_);
            pricer_ = pricer;
            if (pricer_)
                registerWith(pricer_);
            update();
            underlying_->setPricer(pricer);
        }

        protected:
        //! \name Data members
        //@{
        //!
        boost::shared_ptr<FloatingRateCoupon> underlying_;
        //! strike rate for the the call option
        Rate callStrike_;
        //! strike rate for the the put option
        Rate putStrike_;
        //! multiplicative factor of call payoff
        Real callCsi_;
        //! multiplicative factor of put payoff
        Real putCsi_;
        //! inclusion flag og the call payoff if the call option ends at-the-money
        bool isCallATMIncluded_;
        //! inclusion flag og the put payoff if the put option ends at-the-money
        bool isPutATMIncluded_;
        //! digital call option type: if true, cash-or-nothing, if false asset-or-nothing
        bool isCallCashOrNothing_;
        //! digital put option type: if true, cash-or-nothing, if false asset-or-nothing
        bool isPutCashOrNothing_;
        //! digital call option payoff rate, if any
        Rate callDigitalPayoff_;
        //! digital put option payoff rate, if any
        Rate putDigitalPayoff_;
        //! the left and right gaps applied in payoff replication for call
        Real callLeftEps_, callRightEps_;
        //! the left and right gaps applied in payoff replication for puf
        Real putLeftEps_, putRightEps_;
        //!
        bool hasPutStrike_, hasCallStrike_;
        //! Type of replication
        Replication::Type replicationType_;

        //@}
      private:
        Rate callPayoff() const;
        Rate putPayoff() const;

    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/indexes/interestrateindex.hpp>

namespace QuantLib {

  inline DigitalCoupon::DigitalCoupon(const boost::shared_ptr<FloatingRateCoupon>& underlying,
                  Rate callStrike,
                  Position::Type callPosition,
                  bool isCallATMIncluded,
                  Rate callDigitalPayoff,
                  Rate putStrike,
                  Position::Type putPosition,
                  bool isPutATMIncluded,
                  Rate putDigitalPayoff,
                  const boost::shared_ptr<DigitalReplication>& replication)
    : FloatingRateCoupon(underlying->date(),
                         underlying->nominal(),
                         underlying->accrualStartDate(),
                         underlying->accrualEndDate(),
                         underlying->fixingDays(),
                         underlying->index(),
                         underlying->gearing(),
                         underlying->spread(),
                         underlying->referencePeriodStart(),
                         underlying->referencePeriodEnd(),
                         underlying->dayCounter(),
                         underlying->isInArrears()),
      underlying_(underlying), callCsi_(0.), putCsi_(0.),
      isCallATMIncluded_(isCallATMIncluded), isPutATMIncluded_(isPutATMIncluded),
      isCallCashOrNothing_(false), isPutCashOrNothing_(false),
      callLeftEps_(replication->gap()/2.), callRightEps_(replication->gap()/2.),
      putLeftEps_(replication->gap()/2.), putRightEps_(replication->gap()/2.),
      hasPutStrike_(false), hasCallStrike_(false),
      replicationType_(replication->replicationType()) {

        QL_REQUIRE(replication->gap()>0.0, "Non positive epsilon not allowed");

        if (putStrike == Null<Rate>()) {
            QL_REQUIRE(putDigitalPayoff == Null<Rate>(),
            "Put Cash rate non allowed if put strike is null");
        }
        if (callStrike == Null<Rate>()) {
            QL_REQUIRE(callDigitalPayoff == Null<Rate>(),
            "Call Cash rate non allowed if call strike is null");
        }
        if (callStrike != Null<Rate>()) {
            QL_REQUIRE(callStrike >= 0., "negative call strike not allowed");
            hasCallStrike_ = true;
            callStrike_ = callStrike;
            QL_REQUIRE(callStrike_>=replication->gap()/2., "call strike < eps/2");
            switch (callPosition) {
                case Position::Long :
                    callCsi_ = 1.0;
                    break;
                case Position::Short :
                    callCsi_ = -1.0;
                    break;
                default:
                    QL_FAIL("unsupported position type");
            }
            if (callDigitalPayoff != Null<Rate>()){
                callDigitalPayoff_ = callDigitalPayoff;
                isCallCashOrNothing_ = true;
            }
        }
        if (putStrike != Null<Rate>()){
            QL_REQUIRE(putStrike >= 0., "negative put strike not allowed");
            hasPutStrike_ = true;
            putStrike_ = putStrike;
            switch (putPosition) {
                case Position::Long :
                    putCsi_ = 1.0;
                    break;
                case Position::Short :
                    putCsi_ = -1.0;
                    break;
                default:
                    QL_FAIL("unsupported position type");
            }
            if (putDigitalPayoff != Null<Rate>()){
                putDigitalPayoff_ = putDigitalPayoff;
                isPutCashOrNothing_ = true;
            }
        }

        switch (replicationType_) {
          case Replication::Central :
            // do nothing
            break;
          case Replication::Sub :
            if (hasCallStrike_) {
                switch (callPosition) {
                    case Position::Long :
                        callLeftEps_ = 0.;
                        callRightEps_ = replication->gap();
                        break;
                    case Position::Short :
                        callLeftEps_ = replication->gap();
                        callRightEps_ = 0.;
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            if (hasPutStrike_) {
                switch (putPosition) {
                    case Position::Long :
                        putLeftEps_ = replication->gap();
                        putRightEps_ = 0.;
                        break;
                    case Position::Short :
                        putLeftEps_ = 0.;
                        putRightEps_ = replication->gap();
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            break;
          case Replication::Super :
            if (hasCallStrike_) {
                switch (callPosition) {
                    case Position::Long :
                        callLeftEps_ = replication->gap();
                        callRightEps_ = 0.;
                        break;
                    case Position::Short :
                        callLeftEps_ = 0.;
                        callRightEps_ = replication->gap();
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            if (hasPutStrike_) {
                switch (putPosition) {
                    case Position::Long :
                        putLeftEps_ = 0.;
                        putRightEps_ = replication->gap();
                        break;
                    case Position::Short :
                        putLeftEps_ = replication->gap();
                        putRightEps_ = 0.;
                        break;
                    default:
                        QL_FAIL("unsupported position type");
                }
            }
            break;
          default:
            QL_FAIL("unsupported replication type");
        }

        registerWith(underlying);
    }


  inline Rate DigitalCoupon::callOptionRate() const {

        Rate callOptionRate = Rate(0.);
        if(hasCallStrike_) {
            // Step function
            callOptionRate = isCallCashOrNothing_ ? callDigitalPayoff_ : callStrike_;
            CappedFlooredCoupon next(underlying_, callStrike_ + callRightEps_);
            CappedFlooredCoupon previous(underlying_, callStrike_ - callLeftEps_);
            callOptionRate *= (next.rate() - previous.rate())
                            / (callLeftEps_ + callRightEps_);
            if (!isCallCashOrNothing_) {
                // Call
                CappedFlooredCoupon atStrike(underlying_, callStrike_);
                Rate call = underlying_->rate() - atStrike.rate();
                // Sum up
                callOptionRate += call;
            }
        }
        return callOptionRate;
    }

  inline Rate DigitalCoupon::putOptionRate() const {

        Rate putOptionRate = Rate(0.);
        if(hasPutStrike_) {
            // Step function
            putOptionRate = isPutCashOrNothing_ ? putDigitalPayoff_ : putStrike_;
            CappedFlooredCoupon next(underlying_, Null<Rate>(), putStrike_ + putRightEps_);
            CappedFlooredCoupon previous(underlying_, Null<Rate>(), putStrike_ - putLeftEps_);
            putOptionRate *= (next.rate() - previous.rate())
                           / (putLeftEps_ + putRightEps_);
            if (!isPutCashOrNothing_) {
                // Put
                CappedFlooredCoupon atStrike(underlying_, Null<Rate>(), putStrike_);
                Rate put = - underlying_->rate() + atStrike.rate();
                // Sum up
                putOptionRate -= put;
            }
        }
        return putOptionRate;
    }

  inline Rate DigitalCoupon::rate() const {

        QL_REQUIRE(underlying_->pricer(), "pricer not set");

        Date fixingDate = underlying_->fixingDate();
        Date today = Settings::instance().evaluationDate();
        bool enforceTodaysHistoricFixings =
            Settings::instance().enforcesTodaysHistoricFixings();
        Rate underlyingRate = underlying_->rate();
        if (fixingDate < today ||
            ((fixingDate == today) && enforceTodaysHistoricFixings)) {
            // must have been fixed
            return underlyingRate + callCsi_ * callPayoff() + putCsi_  * putPayoff();
        }
        if (fixingDate == today) {
            // might have been fixed
            Rate pastFixing =
                IndexManager::instance().getHistory((underlying_->index())->name())[fixingDate];
            if (pastFixing != Null<Real>()) {
                return underlyingRate + callCsi_ * callPayoff() + putCsi_  * putPayoff();
            } else
                return underlyingRate + callCsi_ * callOptionRate() + putCsi_ * putOptionRate();
        }
        return underlyingRate + callCsi_ * callOptionRate() + putCsi_ * putOptionRate();
    }

  inline Rate DigitalCoupon::convexityAdjustment() const {
        return underlying_->convexityAdjustment();
    }

  inline Rate DigitalCoupon::callStrike() const {
        if (hasCall())
            return callStrike_;
        else
            return Null<Rate>();
   }

  inline Rate DigitalCoupon::putStrike() const {
        if (hasPut())
            return putStrike_;
        else
            return Null<Rate>();
    }

  inline Rate DigitalCoupon::callDigitalPayoff() const {
        if (isCallCashOrNothing_)
            return callDigitalPayoff_;
        else
            return Null<Rate>();
    }

  inline Rate DigitalCoupon::putDigitalPayoff() const {
        if (isPutCashOrNothing_)
            return putDigitalPayoff_;
        else
            return Null<Rate>();
    }

  inline void DigitalCoupon::update() {
        notifyObservers();
    }

  inline void DigitalCoupon::accept(AcyclicVisitor& v) {
        typedef FloatingRateCoupon super;
        Visitor<DigitalCoupon>* v1 =
            dynamic_cast<Visitor<DigitalCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            super::accept(v);
    }

  inline Rate DigitalCoupon::callPayoff() const {
        // to use only if index has fixed
        Rate payoff(0.);
        if(hasCallStrike_) {
            Rate underlyingRate = underlying_->rate();
            if ( (underlyingRate - callStrike_) > 1.e-16 ) {
                payoff = isCallCashOrNothing_ ? callDigitalPayoff_ : underlyingRate;
            } else {
                if (isCallATMIncluded_) {
                    if ( std::abs(callStrike_ - underlyingRate) <= 1.e-16 )
                        payoff = isCallCashOrNothing_ ? callDigitalPayoff_ : underlyingRate;
                }
            }
        }
        return payoff;
    }

  inline Rate DigitalCoupon::putPayoff() const {
        // to use only if index has fixed
        Rate payoff(0.);
        if(hasPutStrike_) {
            Rate underlyingRate = underlying_->rate();
            if ( (putStrike_ - underlyingRate) > 1.e-16 ) {
                payoff = isPutCashOrNothing_ ? putDigitalPayoff_ : underlyingRate;
            } else {
                // putStrike_ <= underlyingRate
                if (isPutATMIncluded_) {
                    if ( std::abs(putStrike_ - underlyingRate) <= 1.e-16 )
                        payoff = isPutCashOrNothing_ ? putDigitalPayoff_ : underlyingRate;
                }
            }
        }
        return payoff;
    }

}


#endif
