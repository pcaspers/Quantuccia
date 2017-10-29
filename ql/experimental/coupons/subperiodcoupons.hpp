/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2008 Toyin Akin

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

/*! \file subperiodcoupons.hpp
    \brief averaging coupons
*/

#ifndef quantlib_sub_period_coupons_hpp
#define quantlib_sub_period_coupons_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/time/schedule.hpp>
#include <vector>

namespace QuantLib {

    class IborIndex;
    class AveragingRatePricer;

    class SubPeriodsCoupon: public FloatingRateCoupon {
      public:
          // The index object passed in has a tenor significantly less than the
          // start/end dates.
          // Thus endDate-startDate may equal 3M
          // The Tenor used within the index object should be 1M for
          // averaging/compounding across three coupons within the
          // coupon period.
          SubPeriodsCoupon(
                const Date& paymentDate,
                Real nominal,
                const boost::shared_ptr<IborIndex>& index,
                const Date& startDate,
                const Date& endDate,
                Natural fixingDays,
                const DayCounter& dayCounter,
                Real gearing,
                Rate couponSpread, // Spread added to the computed
                                   // averaging/compounding rate.
                Rate rateSpread, // Spread to be added onto each
                                 // fixing within the
                                 // averaging/compounding calculation
                const Date& refPeriodStart,
                const Date& refPeriodEnd);

        Spread rateSpread() const { return rateSpread_; }

        Real startTime() const { return startTime_; }
        Real endTime() const { return endTime_; }
        Size observations() const { return observations_; }
        const std::vector<Date>& observationDates() const {
            return observationDates_;
        }
        const std::vector<Real>& observationTimes() const {
            return observationTimes_;
        }

        const boost::shared_ptr<Schedule> observationsSchedule() const {
            return observationsSchedule_;
        }

        Real priceWithoutOptionality(
                       const Handle<YieldTermStructure>& discountCurve) const;
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:

        Real startTime_;                               // S
        Real endTime_;                                 // T

        boost::shared_ptr<Schedule> observationsSchedule_;
        std::vector<Date> observationDates_;
        std::vector<Real> observationTimes_;

        Size observations_;
        Rate rateSpread_;
    };

    class SubPeriodsPricer: public FloatingRateCouponPricer {
      public:
        virtual Rate swapletRate() const;
        virtual Real capletPrice(Rate effectiveCap) const;
        virtual Rate capletRate(Rate effectiveCap) const;
        virtual Real floorletPrice(Rate effectiveFloor) const;
        virtual Rate floorletRate(Rate effectiveFloor) const;
        void initialize(const FloatingRateCoupon& coupon);

      protected:
        const SubPeriodsCoupon* coupon_;
        Real startTime_;
        Real endTime_;
        Real accrualFactor_;
        std::vector<Real> observationTimes_;
        std::vector<Real> observationCvg_;
        std::vector<Real> initialValues_;

        std::vector<Date> observationIndexStartDates_;
        std::vector<Date> observationIndexEndDates_;

        Size observations_;
        Real discount_;
        Real gearing_;
        Spread spread_;
        Real spreadLegValue_;

    };

    class AveragingRatePricer: public SubPeriodsPricer {
      public:
        Real swapletPrice() const;
    };

    class CompoundingRatePricer: public SubPeriodsPricer {
      public:
        Real swapletPrice() const;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Toyin Akin

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
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <cmath>

namespace QuantLib {

  inline SubPeriodsCoupon::SubPeriodsCoupon(
                                    const Date& paymentDate,
                                    Real nominal,
                                    const boost::shared_ptr<IborIndex>& index,
                                    const Date& startDate,
                                    const Date& endDate,
                                    Natural fixingDays,
                                    const DayCounter& dayCounter,
                                    Real gearing,
                                    Rate couponSpread,
                                    Rate rateSpread,
                                    const Date& refPeriodStart,
                                    const Date& refPeriodEnd)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, couponSpread,
                         refPeriodStart, refPeriodEnd, dayCounter),
      rateSpread_(rateSpread) {
        const Handle<YieldTermStructure>& rateCurve =
            index->forwardingTermStructure();
        const Date& referenceDate = rateCurve->referenceDate();

        observationsSchedule_ = boost::shared_ptr<Schedule>(new
            Schedule(startDate, endDate,
                     index->tenor(),
                     NullCalendar(),
                     Unadjusted,
                     Unadjusted,
                     DateGeneration::Forward,
                     false));

        observationDates_ = observationsSchedule_->dates();
        observationDates_.pop_back();                       //remove end date
        observations_ = observationDates_.size();

        startTime_ = dayCounter.yearFraction(referenceDate, startDate);
        endTime_ = dayCounter.yearFraction(referenceDate, endDate);

        for (Size i=0; i<observations_; ++i) {
            observationTimes_.push_back(
                dayCounter.yearFraction(referenceDate, observationDates_[i]));
        }
     }

  inline void SubPeriodsCoupon::accept(AcyclicVisitor& v) {
        Visitor<SubPeriodsCoupon>* v1 =
            dynamic_cast<Visitor<SubPeriodsCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }


  inline void SubPeriodsPricer::initialize(const FloatingRateCoupon& coupon) {
        coupon_ =  dynamic_cast<const SubPeriodsCoupon*>(&coupon);
        QL_REQUIRE(coupon_, "sub-periods coupon required");
        gearing_ = coupon_->gearing();
        spread_ = coupon_->spread();

        Date paymentDate = coupon_->date();

        boost::shared_ptr<IborIndex> index =
            boost::dynamic_pointer_cast<IborIndex>(coupon_->index());
        const Handle<YieldTermStructure>& rateCurve =
            index->forwardingTermStructure();
        discount_ = rateCurve->discount(paymentDate);
        accrualFactor_ = coupon_->accrualPeriod();
        spreadLegValue_ = spread_ * accrualFactor_* discount_;

        startTime_ = coupon_->startTime();
        endTime_ = coupon_->endTime();
        observationTimes_ = coupon_->observationTimes();
        observations_ = coupon_->observations();

        const std::vector<Date>& observationDates =
            coupon_->observationsSchedule()->dates();

        QL_REQUIRE(observationDates.size()==observations_+2,
                   "incompatible size of initialValues vector");

        initialValues_ = std::vector<Real>(observationDates.size(),0.);

        observationCvg_ = std::vector<Real>(observationDates.size(),0.);

        observationIndexStartDates_ =
            std::vector<Date>(observationDates.size());
        observationIndexEndDates_ =
            std::vector<Date>(observationDates.size());

        Calendar calendar = index->fixingCalendar();

        for(Size i=0; i<observationDates.size(); i++) {
            Date fixingDate = calendar.advance(
                                 observationDates[i],
                                 -static_cast<Integer>(coupon_->fixingDays()),
                                 Days);

            initialValues_[i] =
                index->fixing(fixingDate) + coupon_->rateSpread();

            Date fixingValueDate = index->valueDate(fixingDate);
            Date endValueDate = index->maturityDate(fixingValueDate);

            observationIndexStartDates_[i] = fixingValueDate;
            observationIndexEndDates_[i] = endValueDate;

            observationCvg_[i] =
                index->dayCounter().yearFraction(fixingValueDate, endValueDate);
        }
    }

  inline Real SubPeriodsPricer::swapletRate() const {
        return swapletPrice()/(accrualFactor_*discount_);
    }

  inline Real SubPeriodsPricer::capletPrice(Rate) const {
        QL_FAIL("SubPeriodsPricer::capletPrice not implemented");
    }

  inline Rate SubPeriodsPricer::capletRate(Rate) const {
        QL_FAIL("SubPeriodsPricer::capletRate not implemented");
    }

  inline Real SubPeriodsPricer::floorletPrice(Rate) const {
        QL_FAIL("SubPeriodsPricer::floorletPrice not implemented");
    }

  inline Rate SubPeriodsPricer::floorletRate(Rate) const {
        QL_FAIL("SubPeriodsPricer::floorletRate not implemented");
    }

  inline Real AveragingRatePricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Size nCount = initialValues_.size();
        Real dAvgRate = 0.0, dTotalCvg = 0.0, dTotalPayment = 0.0;
        for (Size i=0; i<nCount; i++) {
            dTotalPayment += initialValues_[i] * observationCvg_[i];
            dTotalCvg += observationCvg_[i];
        }

        dAvgRate =  dTotalPayment/dTotalCvg;

        Real swapletPrice = dAvgRate*coupon_->accrualPeriod()*discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

  inline Real CompoundingRatePricer::swapletPrice() const {
        // past or future fixing is managed in InterestRateIndex::fixing()

        Real dNotional = 1.0;

        Size nCount = initialValues_.size();
        Real dCompRate = 0.0, dTotalCvg = 0.0, dTotalPayment = 0.0;
        for (Size i=0; i<nCount; i++) {
            dTotalPayment = initialValues_[i] * observationCvg_[i]*dNotional;
            dNotional += dTotalPayment;
            dTotalCvg += observationCvg_[i];
        }

        dCompRate = (dNotional - 1.0)/dTotalCvg;

        Real swapletPrice = dCompRate*coupon_->accrualPeriod()*discount_;
        return gearing_ * swapletPrice + spreadLegValue_;
    }

}



#endif
