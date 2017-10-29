/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file cmscoupon.hpp
    \brief CMS coupon
*/

#ifndef quantlib_cms_coupon_hpp
#define quantlib_cms_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class SwapIndex;

    //! CMS coupon class
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class CmsCoupon : public FloatingRateCoupon {
      public:
        CmsCoupon(const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const boost::shared_ptr<SwapIndex>& index,
                  Real gearing = 1.0,
                  Spread spread = 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = DayCounter(),
                  bool isInArrears = false);
        //! \name Inspectors
        //@{
        const boost::shared_ptr<SwapIndex>& swapIndex() const {
            return swapIndex_;
        }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        boost::shared_ptr<SwapIndex> swapIndex_;
    };


    //! helper class building a sequence of capped/floored cms-rate coupons
    class CmsLeg {
      public:
        CmsLeg(const Schedule& schedule,
               const boost::shared_ptr<SwapIndex>& swapIndex);
        CmsLeg& withNotionals(Real notional);
        CmsLeg& withNotionals(const std::vector<Real>& notionals);
        CmsLeg& withPaymentDayCounter(const DayCounter&);
        CmsLeg& withPaymentAdjustment(BusinessDayConvention);
        CmsLeg& withFixingDays(Natural fixingDays);
        CmsLeg& withFixingDays(const std::vector<Natural>& fixingDays);
        CmsLeg& withGearings(Real gearing);
        CmsLeg& withGearings(const std::vector<Real>& gearings);
        CmsLeg& withSpreads(Spread spread);
        CmsLeg& withSpreads(const std::vector<Spread>& spreads);
        CmsLeg& withCaps(Rate cap);
        CmsLeg& withCaps(const std::vector<Rate>& caps);
        CmsLeg& withFloors(Rate floor);
        CmsLeg& withFloors(const std::vector<Rate>& floors);
        CmsLeg& inArrears(bool flag = true);
        CmsLeg& withZeroPayments(bool flag = true);
        operator Leg() const;
      private:
        Schedule schedule_;
        boost::shared_ptr<SwapIndex> swapIndex_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        bool inArrears_, zeroPayments_;
    };

}


/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006, 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

  inline CmsCoupon::CmsCoupon(const Date& paymentDate,
                         Real nominal,
                         const Date& startDate,
                         const Date& endDate,
                         Natural fixingDays,
                         const boost::shared_ptr<SwapIndex>& swapIndex,
                         Real gearing,
                         Spread spread,
                         const Date& refPeriodStart,
                         const Date& refPeriodEnd,
                         const DayCounter& dayCounter,
                         bool isInArrears)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, swapIndex, gearing, spread,
                         refPeriodStart, refPeriodEnd,
                         dayCounter, isInArrears),
      swapIndex_(swapIndex) {}

  inline void CmsCoupon::accept(AcyclicVisitor& v) {
        Visitor<CmsCoupon>* v1 = dynamic_cast<Visitor<CmsCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }



  inline CmsLeg::CmsLeg(const Schedule& schedule,
                   const boost::shared_ptr<SwapIndex>& swapIndex)
    : schedule_(schedule), swapIndex_(swapIndex),
      paymentAdjustment_(Following),
      inArrears_(false), zeroPayments_(false) {}

  inline CmsLeg& CmsLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1, notional);
        return *this;
    }

  inline CmsLeg& CmsLeg::withNotionals(const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

  inline CmsLeg& CmsLeg::withPaymentDayCounter(const DayCounter& dayCounter) {
        paymentDayCounter_ = dayCounter;
        return *this;
    }

  inline CmsLeg& CmsLeg::withPaymentAdjustment(BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

  inline CmsLeg& CmsLeg::withFixingDays(Natural fixingDays) {
        fixingDays_ = std::vector<Natural>(1, fixingDays);
        return *this;
    }

  inline CmsLeg& CmsLeg::withFixingDays(const std::vector<Natural>& fixingDays) {
        fixingDays_ = fixingDays;
        return *this;
    }

  inline CmsLeg& CmsLeg::withGearings(Real gearing) {
        gearings_ = std::vector<Real>(1, gearing);
        return *this;
    }

  inline CmsLeg& CmsLeg::withGearings(const std::vector<Real>& gearings) {
        gearings_ = gearings;
        return *this;
    }

  inline CmsLeg& CmsLeg::withSpreads(Spread spread) {
        spreads_ = std::vector<Spread>(1, spread);
        return *this;
    }

  inline CmsLeg& CmsLeg::withSpreads(const std::vector<Spread>& spreads) {
        spreads_ = spreads;
        return *this;
    }

  inline CmsLeg& CmsLeg::withCaps(Rate cap) {
        caps_ = std::vector<Rate>(1, cap);
        return *this;
    }

  inline CmsLeg& CmsLeg::withCaps(const std::vector<Rate>& caps) {
        caps_ = caps;
        return *this;
    }

  inline CmsLeg& CmsLeg::withFloors(Rate floor) {
        floors_ = std::vector<Rate>(1, floor);
        return *this;
    }

  inline CmsLeg& CmsLeg::withFloors(const std::vector<Rate>& floors) {
        floors_ = floors;
        return *this;
    }

  inline CmsLeg& CmsLeg::inArrears(bool flag) {
        inArrears_ = flag;
        return *this;
    }

  inline CmsLeg& CmsLeg::withZeroPayments(bool flag) {
        zeroPayments_ = flag;
        return *this;
    }

}


#endif
