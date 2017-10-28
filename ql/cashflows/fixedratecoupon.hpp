/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl
 Copyright (C) 2007 Piter Dias
 Copyright (C) 2010 Ferdinando Ametrano
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

/*! \file fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate
*/

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include <ql/cashflows/coupon.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/interestrate.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! %Coupon paying a fixed interest rate
    class FixedRateCoupon : public Coupon {
      public:
        //! \name constructors
        //@{
        FixedRateCoupon(const Date& paymentDate,
                        Real nominal,
                        Rate rate,
                        const DayCounter& dayCounter,
                        const Date& accrualStartDate,
                        const Date& accrualEndDate,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date(),
                        const Date& exCouponDate = Date());
        FixedRateCoupon(const Date& paymentDate,
                        Real nominal,
                        const InterestRate& interestRate,
                        const Date& accrualStartDate,
                        const Date& accrualEndDate,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date(),
                        const Date& exCouponDate = Date());
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const { return rate_; }
        InterestRate interestRate() const { return rate_; }
        DayCounter dayCounter() const { return rate_.dayCounter(); }
        Real accruedAmount(const Date&) const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        InterestRate rate_;
    };



    //! helper class building a sequence of fixed rate coupons
    class FixedRateLeg {
      public:
        FixedRateLeg(const Schedule& schedule);
        FixedRateLeg& withNotionals(Real);
        FixedRateLeg& withNotionals(const std::vector<Real>&);
        FixedRateLeg& withCouponRates(Rate,
                                      const DayCounter& paymentDayCounter,
                                      Compounding comp = Simple,
                                      Frequency freq = Annual);
        FixedRateLeg& withCouponRates(const std::vector<Rate>&,
                                      const DayCounter& paymentDayCounter,
                                      Compounding comp = Simple,
                                      Frequency freq = Annual);
        FixedRateLeg& withCouponRates(const InterestRate&);
        FixedRateLeg& withCouponRates(const std::vector<InterestRate>&);
        FixedRateLeg& withPaymentAdjustment(BusinessDayConvention);
        FixedRateLeg& withFirstPeriodDayCounter(const DayCounter&);
        FixedRateLeg& withLastPeriodDayCounter(const DayCounter&);
        FixedRateLeg& withPaymentCalendar(const Calendar&);
        FixedRateLeg& withPaymentLag(Natural lag);
        FixedRateLeg& withExCouponPeriod(const Period&,
                                         const Calendar&,
                                         BusinessDayConvention,
                                         bool endOfMonth = false);
        operator Leg() const;
      private:
        Schedule schedule_;
        std::vector<Real> notionals_;
        std::vector<InterestRate> couponRates_;
        DayCounter firstPeriodDC_ , lastPeriodDC_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentAdjustment_;
        Natural paymentLag_;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_;
        bool exCouponEndOfMonth_;
    };

    inline void FixedRateCoupon::accept(AcyclicVisitor& v) {
        Visitor<FixedRateCoupon>* v1 =
            dynamic_cast<Visitor<FixedRateCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl
 Copyright (C) 2007 Piter Dias
 Copyright (C) 2010 Ferdinando Ametrano
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

using boost::shared_ptr;
using std::vector;

namespace QuantLib {


    inline FixedRateCoupon::FixedRateCoupon(const Date& paymentDate,
                                     Real nominal,
                                     Rate rate,
                                     const DayCounter& dayCounter,
                                     const Date& accrualStartDate,
                                     const Date& accrualEndDate,
                                     const Date& refPeriodStart,
                                     const Date& refPeriodEnd,
                                     const Date& exCouponDate)
    : Coupon(paymentDate, nominal, accrualStartDate, accrualEndDate,
             refPeriodStart, refPeriodEnd, exCouponDate),
      rate_(InterestRate(rate, dayCounter, Simple, Annual)) {}

    inline FixedRateCoupon::FixedRateCoupon(const Date& paymentDate,
                                     Real nominal,
                                     const InterestRate& interestRate,
                                     const Date& accrualStartDate,
                                     const Date& accrualEndDate,
                                     const Date& refPeriodStart,
                                     const Date& refPeriodEnd,
                                     const Date& exCouponDate)
    : Coupon(paymentDate, nominal, accrualStartDate, accrualEndDate,
             refPeriodStart, refPeriodEnd, exCouponDate),
      rate_(interestRate) {}

    inline Real FixedRateCoupon::amount() const {
        return nominal()*(rate_.compoundFactor(accrualStartDate_,
                                               accrualEndDate_,
                                               refPeriodStart_,
                                               refPeriodEnd_) - 1.0);
    }

    inline Real FixedRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else if (tradingExCoupon(d)) {
            return -nominal()*(rate_.compoundFactor(d,
                                                    accrualEndDate_,
                                                    refPeriodStart_,
                                                    refPeriodEnd_) - 1.0);
        } else {
            return nominal()*(rate_.compoundFactor(accrualStartDate_,
                                                   std::min(d,accrualEndDate_),
                                                   refPeriodStart_,
                                                   refPeriodEnd_) - 1.0);
        }
    }


    inline FixedRateLeg::FixedRateLeg(const Schedule& schedule)
    : schedule_(schedule), paymentCalendar_(schedule.calendar()),
      paymentAdjustment_(Following), paymentLag_(0) {}

    inline FixedRateLeg& FixedRateLeg::withNotionals(Real notional) {
        notionals_ = vector<Real>(1,notional);
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withNotionals(const vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withCouponRates(Rate rate,
                                                const DayCounter& dc,
                                                Compounding comp,
                                                Frequency freq) {
        couponRates_.resize(1);
        couponRates_[0] = InterestRate(rate, dc, comp, freq);
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withCouponRates(const InterestRate& i) {
        couponRates_.resize(1);
        couponRates_[0] = i;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withCouponRates(const vector<Rate>& rates,
                                                const DayCounter& dc,
                                                Compounding comp,
                                                Frequency freq) {
        couponRates_.resize(rates.size());
        for (Size i=0; i<rates.size(); ++i)
            couponRates_[i] = InterestRate(rates[i], dc, comp, freq);
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withCouponRates(
                                const vector<InterestRate>& interestRates) {
        couponRates_ = interestRates;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withFirstPeriodDayCounter(
                                            const DayCounter& dayCounter) {
        firstPeriodDC_ = dayCounter;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withLastPeriodDayCounter(
                                               const DayCounter& dayCounter) {
        lastPeriodDC_ = dayCounter;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withPaymentCalendar(const Calendar& cal) {
        paymentCalendar_ = cal;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withPaymentLag(Natural lag) {
        paymentLag_ = lag;
        return *this;
    }

    inline FixedRateLeg& FixedRateLeg::withExCouponPeriod(
                                const Period& period,
                                const Calendar& cal,
                                BusinessDayConvention convention,
                                bool endOfMonth) {
        exCouponPeriod_ = period;
        exCouponCalendar_ = cal;
        exCouponAdjustment_ = convention;
        exCouponEndOfMonth_ = endOfMonth;
        return *this;
    }

    inline FixedRateLeg::operator Leg() const {

        QL_REQUIRE(!couponRates_.empty(), "no coupon rates given");
        QL_REQUIRE(!notionals_.empty(), "no notional given");

        Leg leg;
        leg.reserve(schedule_.size()-1);

        // first period might be short or long
        Date start = schedule_.date(0), end = schedule_.date(1);
        Date paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);
        Date exCouponDate;
        InterestRate rate = couponRates_[0];
        Real nominal = notionals_[0];

        if (exCouponPeriod_ != Period())
        {
            exCouponDate = exCouponCalendar_.advance(paymentDate,
                                                     -exCouponPeriod_,
                                                     exCouponAdjustment_,
                                                     exCouponEndOfMonth_);
        }

        if (schedule_.isRegular(1)) {
            QL_REQUIRE(firstPeriodDC_.empty() ||
                       firstPeriodDC_ == rate.dayCounter(),
                       "regular first coupon "
                       "does not allow a first-period day count");
            shared_ptr<CashFlow> temp(new
                FixedRateCoupon(paymentDate, nominal, rate,
                                start, end, start, end, exCouponDate));
            leg.push_back(temp);
        } else {
            Date ref = schedule_.calendar().advance(
                                            end,
                                            -schedule_.tenor(),
                                            schedule_.businessDayConvention(),
                                            schedule_.endOfMonth());
            InterestRate r(rate.rate(),
                           firstPeriodDC_.empty() ? rate.dayCounter()
                                                  : firstPeriodDC_,
                           rate.compounding(), rate.frequency());
            leg.push_back(shared_ptr<CashFlow>(new
                FixedRateCoupon(paymentDate, nominal, r,
                                start, end, ref, end, exCouponDate)));
        }
        // regular periods
        for (Size i=2; i<schedule_.size()-1; ++i) {
            start = end; end = schedule_.date(i);
            Date paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);
            if (exCouponPeriod_ != Period())
            {
                exCouponDate = exCouponCalendar_.advance(paymentDate,
                                                         -exCouponPeriod_,
                                                         exCouponAdjustment_,
                                                         exCouponEndOfMonth_);
            }
            if ((i-1) < couponRates_.size())
                rate = couponRates_[i-1];
            else
                rate = couponRates_.back();
            if ((i-1) < notionals_.size())
                nominal = notionals_[i-1];
            else
                nominal = notionals_.back();
            leg.push_back(shared_ptr<CashFlow>(new
                FixedRateCoupon(paymentDate, nominal, rate,
                                start, end, start, end, exCouponDate)));
        }
        if (schedule_.size() > 2) {
            // last period might be short or long
            Size N = schedule_.size();
            start = end; end = schedule_.date(N-1);
            Date paymentDate = paymentCalendar_.advance(end, paymentLag_, Days, paymentAdjustment_);
            if (exCouponPeriod_ != Period())
            {
                exCouponDate = exCouponCalendar_.advance(paymentDate,
                                                         -exCouponPeriod_,
                                                         exCouponAdjustment_,
                                                         exCouponEndOfMonth_);
            }
            if ((N-2) < couponRates_.size())
                rate = couponRates_[N-2];
            else
                rate = couponRates_.back();
            if ((N-2) < notionals_.size())
                nominal = notionals_[N-2];
            else
                nominal = notionals_.back();
            InterestRate r( rate.rate(), lastPeriodDC_.empty() ?
                rate.dayCounter() :
                lastPeriodDC_ , rate.compounding(), rate.frequency() );
            if (schedule_.isRegular(N-1)) {
                leg.push_back(shared_ptr<CashFlow>(new
                    FixedRateCoupon(paymentDate, nominal, r,
                                    start, end, start, end, exCouponDate)));
            } else {
                Date ref = schedule_.calendar().advance(
                                            start,
                                            schedule_.tenor(),
                                            schedule_.businessDayConvention(),
                                            schedule_.endOfMonth());
                leg.push_back(shared_ptr<CashFlow>(new
                    FixedRateCoupon(paymentDate, nominal, r,
                                    start, end, start, ref, exCouponDate)));
            }
        }
        return leg;
    }

}


#endif
