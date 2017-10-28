/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2007, 2008, 2009, 2010, 2011 Ferdinando Ametrano
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

/*! \file cashflows.hpp
    \brief Cash-flow analysis functions
*/

#ifndef quantlib_cashflows_hpp
#define quantlib_cashflows_hpp

#include <ql/cashflows/duration.hpp>
#include <ql/cashflow.hpp>
#include <ql/interestrate.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    class YieldTermStructure;

    //! %cashflow-analysis functions
    /*! \todo add tests */
    class CashFlows {
      private:
        CashFlows();
        CashFlows(const CashFlows&);

        class IrrFinder : public std::unary_function<Rate, Real> {
          public:
            IrrFinder(const Leg& leg,
                      Real npv,
                      const DayCounter& dayCounter,
                      Compounding comp,
                      Frequency freq,
                      bool includeSettlementDateFlows,
                      Date settlementDate,
                      Date npvDate);

            Real operator()(Rate y) const;
            Real derivative(Rate y) const;
          private:
            void checkSign() const;

            const Leg& leg_;
            Real npv_;
            DayCounter dayCounter_;
            Compounding compounding_;
            Frequency frequency_;
            bool includeSettlementDateFlows_;
            Date settlementDate_, npvDate_;
        };
      public:
        //! \name Date functions
        //@{
        static Date startDate(const Leg& leg);
        static Date maturityDate(const Leg& leg);
        static bool isExpired(const Leg& leg,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date());
        //@}

        //! \name CashFlow functions
        //@{
        //! the last cashflow paying before or at the given date
        static Leg::const_reverse_iterator
        previousCashFlow(const Leg& leg,
                         bool includeSettlementDateFlows,
                         Date settlementDate = Date());
        //! the first cashflow paying after the given date
        static Leg::const_iterator
        nextCashFlow(const Leg& leg,
                     bool includeSettlementDateFlows,
                     Date settlementDate = Date());
        static Date
        previousCashFlowDate(const Leg& leg,
                             bool includeSettlementDateFlows,
                             Date settlementDate = Date());
        static Date
        nextCashFlowDate(const Leg& leg,
                         bool includeSettlementDateFlows,
                         Date settlementDate = Date());
        static Real
        previousCashFlowAmount(const Leg& leg,
                               bool includeSettlementDateFlows,
                               Date settlementDate = Date());
        static Real
        nextCashFlowAmount(const Leg& leg,
                           bool includeSettlementDateFlows,
                           Date settlementDate = Date());
        //@}

        //! \name Coupon inspectors
        //@{
        static Rate
        previousCouponRate(const Leg& leg,
                           bool includeSettlementDateFlows,
                           Date settlementDate = Date());
        static Rate
        nextCouponRate(const Leg& leg,
                       bool includeSettlementDateFlows,
                       Date settlementDate = Date());

        static Real
        nominal(const Leg& leg,
                bool includeSettlementDateFlows,
                Date settlDate = Date());
        static Date
        accrualStartDate(const Leg& leg,
                         bool includeSettlementDateFlows,
                         Date settlDate = Date());
        static Date
        accrualEndDate(const Leg& leg,
                       bool includeSettlementDateFlows,
                       Date settlementDate = Date());
        static Date
        referencePeriodStart(const Leg& leg,
                             bool includeSettlementDateFlows,
                             Date settlDate = Date());
        static Date
        referencePeriodEnd(const Leg& leg,
                           bool includeSettlementDateFlows,
                           Date settlDate = Date());
        static Time
        accrualPeriod(const Leg& leg,
                      bool includeSettlementDateFlows,
                      Date settlementDate = Date());
        static Date::serial_type
        accrualDays(const Leg& leg,
                    bool includeSettlementDateFlows,
                    Date settlementDate = Date());
        static Time
        accruedPeriod(const Leg& leg,
                      bool includeSettlementDateFlows,
                      Date settlementDate = Date());
        static Date::serial_type
        accruedDays(const Leg& leg,
                    bool includeSettlementDateFlows,
                    Date settlementDate = Date());
        static Real
        accruedAmount(const Leg& leg,
                      bool includeSettlementDateFlows,
                      Date settlementDate = Date());
        //@}

        //! \name YieldTermStructure functions
        //@{
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given term structure.
        */
        static Real npv(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given term structure.
        */
        static Real bps(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());

        //@{
        //! NPV and BPS of the cash flows.
        /*! The NPV and BPS of the cash flows calculated
            together for performance reason
        */
        static void npvbps(const Leg& leg,
                           const YieldTermStructure& discountCurve,
                           bool includeSettlementDateFlows,
                           Date settlementDate,
                           Date npvDate,
                           Real& npv,
                           Real& bps);

        //! At-the-money rate of the cash flows.
        /*! The result is the fixed rate for which a fixed rate cash flow
            vector, equivalent to the input vector, has the required NPV
            according to the given term structure. If the required NPV is
            not given, the input cash flow vector's NPV is used instead.
        */
        static Rate atmRate(const Leg& leg,
                            const YieldTermStructure& discountCurve,
                            bool includeSettlementDateFlows,
                            Date settlementDate = Date(),
                            Date npvDate = Date(),
                            Real npv = Null<Real>());
        //@}

        //! \name Yield (a.k.a. Internal Rate of Return, i.e. IRR) functions
        /*! The IRR is the interest rate at which the NPV of the cash
            flows equals the dirty price.
        */
        //@{
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the given constant interest rate.  The result
            is affected by the choice of the interest-rate compounding
            and the relative frequency and day counter.
        */
        static Real npv(const Leg& leg,
                        const InterestRate& yield,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        static Real npv(const Leg& leg,
                        Rate yield,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! Basis-point sensitivity of the cash flows.
        /*! The result is the change in NPV due to a uniform
            1-basis-point change in the rate paid by the cash
            flows. The change for each coupon is discounted according
            to the given constant interest rate.  The result is
            affected by the choice of the interest-rate compounding
            and the relative frequency and day counter.
        */
        static Real bps(const Leg& leg,
                        const InterestRate& yield,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        static Real bps(const Leg& leg,
                        Rate yield,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! Implied internal rate of return.
        /*! The function verifies
            the theoretical existance of an IRR and numerically
            establishes the IRR to the desired precision.
        */
        static Rate yield(const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          bool includeSettlementDateFlows,
                          Date settlementDate = Date(),
                          Date npvDate = Date(),
                          Real accuracy = 1.0e-10,
                          Size maxIterations = 100,
                          Rate guess = 0.05);

        template <typename Solver>
        static Rate yield(Solver solver,
                          const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          bool includeSettlementDateFlows,
                          Date settlementDate = Date(),
                          Date npvDate = Date(),
                          Real accuracy = 1.0e-10,
                          Rate guess = 0.05) {
            IrrFinder objFunction(leg, npv, dayCounter, compounding,
                                  frequency, includeSettlementDateFlows,
                                  settlementDate, npvDate);
            return solver.solve(objFunction, accuracy, guess, guess/10.0);
        }

        //! Cash-flow duration.
        /*! The simple duration of a string of cash flows is defined as
            \f[
            D_{\mathrm{simple}} = \frac{\sum t_i c_i B(t_i)}{\sum c_i B(t_i)}
            \f]
            where \f$ c_i \f$ is the amount of the \f$ i \f$-th cash
            flow, \f$ t_i \f$ is its payment time, and \f$ B(t_i) \f$
            is the corresponding discount according to the passed yield.

            The modified duration is defined as
            \f[
            D_{\mathrm{modified}} = -\frac{1}{P} \frac{\partial P}{\partial y}
            \f]
            where \f$ P \f$ is the present value of the cash flows
            according to the given IRR \f$ y \f$.

            The Macaulay duration is defined for a compounded IRR as
            \f[
            D_{\mathrm{Macaulay}} = \left( 1 + \frac{y}{N} \right)
                                    D_{\mathrm{modified}}
            \f]
            where \f$ y \f$ is the IRR and \f$ N \f$ is the number of
            cash flows per year.
        */
        static Time duration(const Leg& leg,
                             const InterestRate& yield,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate = Date(),
                             Date npvDate = Date());
        static Time duration(const Leg& leg,
                             Rate yield,
                             const DayCounter& dayCounter,
                             Compounding compounding,
                             Frequency frequency,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate = Date(),
                             Date npvDate = Date());

        //! Cash-flow convexity
        /*! The convexity of a string of cash flows is defined as
            \f[
            C = \frac{1}{P} \frac{\partial^2 P}{\partial y^2}
            \f]
            where \f$ P \f$ is the present value of the cash flows
            according to the given IRR \f$ y \f$.
        */
        static Real convexity(const Leg& leg,
                              const InterestRate& yield,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date());
        static Real convexity(const Leg& leg,
                              Rate yield,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date());

        //! Basis-point value
        /*! Obtained by setting dy = 0.0001 in the 2nd-order Taylor
            series expansion.
        */
        static Real basisPointValue(const Leg& leg,
                                    const InterestRate& yield,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate = Date(),
                                    Date npvDate = Date());
        static Real basisPointValue(const Leg& leg,
                                    Rate yield,
                                    const DayCounter& dayCounter,
                                    Compounding compounding,
                                    Frequency frequency,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate = Date(),
                                    Date npvDate = Date());

        //! Yield value of a basis point
        /*! The yield value of a one basis point change in price is
            the derivative of the yield with respect to the price
            multiplied by 0.01
        */
        static Real yieldValueBasisPoint(const Leg& leg,
                                         const InterestRate& yield,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate = Date(),
                                         Date npvDate = Date());
        static Real yieldValueBasisPoint(const Leg& leg,
                                         Rate yield,
                                         const DayCounter& dayCounter,
                                         Compounding compounding,
                                         Frequency frequency,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate = Date(),
                                         Date npvDate = Date());
        //@}

        //! \name Z-spread functions
        /*! For details on z-spread refer to:
            "Credit Spreads Explained", Lehman Brothers European Fixed
            Income Research - March 2004, D. O'Kane
        */
        //@{
        //! NPV of the cash flows.
        /*! The NPV is the sum of the cash flows, each discounted
            according to the z-spreaded term structure.  The result
            is affected by the choice of the z-spread compounding
            and the relative frequency and day counter.
        */
        static Real npv(const Leg& leg,
                        const boost::shared_ptr<YieldTermStructure>& discount,
                        Spread zSpread,
                        const DayCounter& dayCounter,
                        Compounding compounding,
                        Frequency frequency,
                        bool includeSettlementDateFlows,
                        Date settlementDate = Date(),
                        Date npvDate = Date());
        //! implied Z-spread.
        static Spread zSpread(const Leg& leg,
                              Real npv,
                              const boost::shared_ptr<YieldTermStructure>&,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date(),
                              Real accuracy = 1.0e-10,
                              Size maxIterations = 100,
                              Rate guess = 0.0);
        //! deprecated implied Z-spread.
        static Spread zSpread(const Leg& leg,
                              const boost::shared_ptr<YieldTermStructure>& d,
                              Real npv,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate = Date(),
                              Date npvDate = Date(),
                              Real accuracy = 1.0e-10,
                              Size maxIterations = 100,
                              Rate guess = 0.0) {
            return zSpread(leg, npv, d, dayCounter, compounding, frequency,
                           includeSettlementDateFlows, settlementDate, npvDate,
                           accuracy, maxIterations, guess);
        }
        //@}

    };

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 StatPro Italia srl
 Copyright (C) 2005 Charles Whitmore
 Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ferdinando Ametrano
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

#include <ql/cashflows/coupon.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>

using boost::shared_ptr;
using boost::dynamic_pointer_cast;

namespace QuantLib {

    // Date inspectors

    inline Date CashFlows::startDate(const Leg& leg) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        Date d = Date::maxDate();
        for (Size i=0; i<leg.size(); ++i) {
            shared_ptr<Coupon> c = dynamic_pointer_cast<Coupon>(leg[i]);
            if (c)
                d = std::min(d, c->accrualStartDate());
            else
                d = std::min(d, leg[i]->date());
        }
        return d;
    }

    inline Date CashFlows::maturityDate(const Leg& leg) {
        QL_REQUIRE(!leg.empty(), "empty leg");

        Date d = Date::minDate();
        for (Size i=0; i<leg.size(); ++i) {
            shared_ptr<Coupon> c = dynamic_pointer_cast<Coupon>(leg[i]);
            if (c)
                d = std::max(d, c->accrualEndDate());
            else
                d = std::max(d, leg[i]->date());
        }
        return d;
    }

    inline bool CashFlows::isExpired(const Leg& leg,
                              bool includeSettlementDateFlows,
                              Date settlementDate)
    {
        if (leg.empty())
            return true;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        for (Size i=leg.size(); i>0; --i)
            if (!leg[i-1]->hasOccurred(settlementDate,
                                       includeSettlementDateFlows))
                return false;
        return true;
    }

    inline Leg::const_reverse_iterator
    CashFlows::previousCashFlow(const Leg& leg,
                                bool includeSettlementDateFlows,
                                Date settlementDate) {
        if (leg.empty())
            return leg.rend();

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_reverse_iterator i;
        for (i = leg.rbegin(); i<leg.rend(); ++i) {
            if ( (*i)->hasOccurred(settlementDate, includeSettlementDateFlows) )
                return i;
        }
        return leg.rend();
    }

    inline Leg::const_iterator
    CashFlows::nextCashFlow(const Leg& leg,
                            bool includeSettlementDateFlows,
                            Date settlementDate) {
        if (leg.empty())
            return leg.end();

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_iterator i;
        for (i = leg.begin(); i<leg.end(); ++i) {
            if ( ! (*i)->hasOccurred(settlementDate, includeSettlementDateFlows) )
                return i;
        }
        return leg.end();
    }

    inline Date CashFlows::previousCashFlowDate(const Leg& leg,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.rend())
            return Date();

        return (*cf)->date();
    }

    inline Date CashFlows::nextCashFlowDate(const Leg& leg,
                                     bool includeSettlementDateFlows,
                                     Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.end())
            return Date();

        return (*cf)->date();
    }

    inline Real CashFlows::::previousCashFlowAmount(const Leg& leg,
                                           bool includeSettlementDateFlows,
                                           Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.rend())
            return Real();

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.rend() && (*cf)->date()==paymentDate; ++cf)
            result += (*cf)->amount();
        return result;
    }

    inline Real CashFlows::::nextCashFlowAmount(const Leg& leg,
                                       bool includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);

        if (cf==leg.end())
            return Real();

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf)
            result += (*cf)->amount();
        return result;
    }

    // Coupon utility functions
    namespace {

        template<typename Iter>
        Rate aggregateRate(const Leg& leg,
                           Iter first,
                           Iter last) {
            if (first==last) return 0.0;

            Date paymentDate = (*first)->date();
            bool firstCouponFound = false;
            Real nominal = 0.0;
            Time accrualPeriod = 0.0;
            DayCounter dc;
            Rate result = 0.0;
            for (; first<last && (*first)->date()==paymentDate; ++first) {
                shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*first);
                if (cp) {
                    if (firstCouponFound) {
                        QL_REQUIRE(nominal       == cp->nominal() &&
                                   accrualPeriod == cp->accrualPeriod() &&
                                   dc            == cp->dayCounter(),
                                   "cannot aggregate two different coupons on "
                                   << paymentDate);
                    } else {
                        firstCouponFound = true;
                        nominal = cp->nominal();
                        accrualPeriod = cp->accrualPeriod();
                        dc = cp->dayCounter();
                    }
                    result += cp->rate();
                }
            }
            QL_ENSURE(firstCouponFound,
                      "no coupon paid at cashflow date " << paymentDate);
            return result;
        }

    } // anonymous namespace ends here

    inline Rate CashFlows::previousCouponRate(const Leg& leg,
                                       bool includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_reverse_iterator cf;
        cf = previousCashFlow(leg, includeSettlementDateFlows, settlementDate);

        return aggregateRate<Leg::const_reverse_iterator>(leg, cf, leg.rend());
    }

    inline Rate CashFlows::nextCouponRate(const Leg& leg,
                                   bool includeSettlementDateFlows,
                                   Date settlementDate) {
        Leg::const_iterator cf;
        cf = nextCashFlow(leg, includeSettlementDateFlows, settlementDate);
        return aggregateRate<Leg::const_iterator>(leg, cf, leg.end());
    }

    inline Real CashFlows::::nominal(const Leg& leg,
                            bool includeSettlementDateFlows,
                            Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->nominal();
        }
        return 0.0;
    }

    inline Date CashFlows::accrualStartDate(const Leg& leg,
                                     bool includeSettlementDateFlows,
                                     Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualStartDate();
        }
        return Date();
    }

    inline Date CashFlows::accrualEndDate(const Leg& leg,
                                   bool includeSettlementDateFlows,
                                   Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualEndDate();
        }
        return Date();
    }

    inline Date CashFlows::referencePeriodStart(const Leg& leg,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->referencePeriodStart();
        }
        return Date();
    }

    inline Date CashFlows::referencePeriodEnd(const Leg& leg,
                                       bool includeSettlementDateFlows,
                                       Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return Date();

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->referencePeriodEnd();
        }
        return Date();
    }

    inline inline Time CashFlows::accrualPeriod(const Leg& leg,
                                  bool includeSettlementDateFlows,
                                  Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualPeriod();
        }
        return 0;
    }

    inline inline Date::serial_type CashFlows::accrualDays(const Leg& leg,
                                             bool includeSettlementDateFlows,
                                             Date settlementDate) {
        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accrualDays();
        }
        return 0;
    }

    inline Time CashFlows::accruedPeriod(const Leg& leg,
                                  bool includeSettlementDateFlows,
                                  Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accruedPeriod(settlementDate);
        }
        return 0;
    }

    inline Date::serial_type CashFlows::accruedDays(const Leg& leg,
                                             bool includeSettlementDateFlows,
                                             Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0;

        Date paymentDate = (*cf)->date();
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                return cp->accruedDays(settlementDate);
        }
        return 0;
    }

    inline Real CashFlows::::accruedAmount(const Leg& leg,
                                  bool includeSettlementDateFlows,
                                  Date settlementDate) {
        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        Leg::const_iterator cf = nextCashFlow(leg,
                                              includeSettlementDateFlows,
                                              settlementDate);
        if (cf==leg.end()) return 0.0;

        Date paymentDate = (*cf)->date();
        Real result = 0.0;
        for (; cf<leg.end() && (*cf)->date()==paymentDate; ++cf) {
            shared_ptr<Coupon> cp = dynamic_pointer_cast<Coupon>(*cf);
            if (cp)
                result += cp->accruedAmount(settlementDate);
        }
        return result;
    }

    // YieldTermStructure utility functions
    namespace {

        class BPSCalculator : public AcyclicVisitor,
                              public Visitor<CashFlow>,
                              public Visitor<Coupon> {
          public:
            explicit BPSCalculator(const YieldTermStructure& discountCurve)
            : discountCurve_(discountCurve), bps_(0.0), nonSensNPV_(0.0) {}
            void visit(Coupon& c) {
                Real bps = c.nominal() *
                           c.accrualPeriod() *
                           discountCurve_.discount(c.date());
                bps_ += bps;
            }
            void visit(CashFlow& cf) {
                nonSensNPV_ += cf.amount() * 
                               discountCurve_.discount(cf.date());
            }
            Real bps() const { return bps_; }
            Real nonSensNPV() const { return nonSensNPV_; }
          private:
            const YieldTermStructure& discountCurve_;
            Real bps_, nonSensNPV_;
        };

        const Spread basisPoint_ = 1.0e-4;
    } // anonymous namespace ends here

    inline Real CashFlows::::npv(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real totalNPV = 0.0;
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows) &&
                !leg[i]->tradingExCoupon(settlementDate))
                totalNPV += leg[i]->amount() *
                            discountCurve.discount(leg[i]->date());
        }

        return totalNPV/discountCurve.discount(npvDate);
    }

    inline Real CashFlows::::bps(const Leg& leg,
                        const YieldTermStructure& discountCurve,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        BPSCalculator calc(discountCurve);
        for (Size i=0; i<leg.size(); ++i) {
            if (!leg[i]->hasOccurred(settlementDate,
                                     includeSettlementDateFlows) &&
                !leg[i]->tradingExCoupon(settlementDate))
                leg[i]->accept(calc);
        }
        return basisPoint_*calc.bps()/discountCurve.discount(npvDate);
    }

    inline void CashFlows::npvbps(const Leg& leg,
                           const YieldTermStructure& discountCurve,
                           bool includeSettlementDateFlows,
                           Date settlementDate,
                           Date npvDate,
                           Real& npv,
                           Real& bps) {

        npv = 0.0;
        if (leg.empty()) {
            bps = 0.0;
            return;
        }

        for (Size i=0; i<leg.size(); ++i) {
            CashFlow& cf = *leg[i];
            if (!cf.hasOccurred(settlementDate,
                                includeSettlementDateFlows) &&
                !cf.tradingExCoupon(settlementDate)) {
                boost::shared_ptr<Coupon> cp =
                    boost::dynamic_pointer_cast<Coupon>(leg[i]);
                Real df = discountCurve.discount(cf.date());
                npv += cf.amount() * df;
                if(cp != NULL)
                    bps += cp->nominal() * cp->accrualPeriod() * df;
            }
        }
        DiscountFactor d = discountCurve.discount(npvDate);
        npv /= d;
        bps = basisPoint_ * bps / d;
    }

    inline Rate CashFlows::atmRate(const Leg& leg,
                            const YieldTermStructure& discountCurve,
                            bool includeSettlementDateFlows,
                            Date settlementDate,
                            Date npvDate,
                            Real targetNpv) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real npv = 0.0;
        BPSCalculator calc(discountCurve);
        for (Size i=0; i<leg.size(); ++i) {
            CashFlow& cf = *leg[i];
            if (!cf.hasOccurred(settlementDate,
                                includeSettlementDateFlows) &&
                !cf.tradingExCoupon(settlementDate)) {
                npv += cf.amount() *
                       discountCurve.discount(cf.date());
                cf.accept(calc);
            }
        }

        if (targetNpv==Null<Real>())
            targetNpv = npv - calc.nonSensNPV();
        else {
            targetNpv *= discountCurve.discount(npvDate);
            targetNpv -= calc.nonSensNPV();
        }

        if (targetNpv==0.0)
            return 0.0;

        Real bps = calc.bps();
        QL_REQUIRE(bps!=0.0, "null bps: impossible atm rate");

        return targetNpv/bps;
    }

    // IRR utility functions
    namespace {

        template <class T>
        Integer sign(T x) {
            static T zero = T();
            if (x == zero)
                return 0;
            else if (x > zero)
                return 1;
            else
                return -1;
        }

        // helper fucntion used to calculate Time-To-Discount for each stage when calculating discount factor stepwisely
        Time getStepwiseDiscountTime(const boost::shared_ptr<QuantLib::CashFlow> cashFlow,
                                     const DayCounter& dc,
                                     Date npvDate,
                                     Date lastDate) {
            Date cashFlowDate = cashFlow->date();
            Date refStartDate, refEndDate;
            shared_ptr<Coupon> coupon =
                    boost::dynamic_pointer_cast<Coupon>(cashFlow);
            if (coupon) {
                refStartDate = coupon->referencePeriodStart();
                refEndDate = coupon->referencePeriodEnd();
            } else {
                if (lastDate == npvDate) {
                    // we don't have a previous coupon date,
                    // so we fake it
                    refStartDate = cashFlowDate - 1*Years;
                } else  {
                    refStartDate = lastDate;
                }
                refEndDate = cashFlowDate;
            }

            if (coupon && lastDate!=coupon->accrualStartDate()) {
                Time couponPeriod = dc.yearFraction(coupon->accrualStartDate(),
                                                cashFlowDate, refStartDate, refEndDate);
                Time accruedPeriod = dc.yearFraction(coupon->accrualStartDate(),
                                                lastDate, refStartDate, refEndDate);
                return couponPeriod - accruedPeriod;
            }
            else {
                return dc.yearFraction(lastDate, cashFlowDate,
                                       refStartDate, refEndDate);
            }
        }

        inline Real simpleDuration(const Leg& leg,
                            const InterestRate& y,
                            bool includeSettlementDateFlows,
                            Date settlementDate,
                            Date npvDate) {
            if (leg.empty())
                return 0.0;

            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

            if (npvDate == Date())
                npvDate = settlementDate;

            Real P = 0.0;
            Real dPdy = 0.0;
            Time t = 0.0;
            Date lastDate = npvDate;
            const DayCounter& dc = y.dayCounter();
            for (Size i=0; i<leg.size(); ++i) {
                if (leg[i]->hasOccurred(settlementDate,
                                        includeSettlementDateFlows))
                    continue;

                Real c = leg[i]->amount();
                if (leg[i]->tradingExCoupon(settlementDate)) {
                    c = 0.0;
                }

                t += getStepwiseDiscountTime(leg[i], dc, npvDate, lastDate);
                DiscountFactor B = y.discountFactor(t);
                P += c * B;
                dPdy += t * c * B;
                
                lastDate = leg[i]->date();
            }
            if (P == 0.0) // no cashflows
                return 0.0;
            return dPdy/P;
        }

        inline Real modifiedDuration(const Leg& leg,
                              const InterestRate& y,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
            if (leg.empty())
                return 0.0;

            if (settlementDate == Date())
                settlementDate = Settings::instance().evaluationDate();

            if (npvDate == Date())
                npvDate = settlementDate;

            Real P = 0.0;
            Time t = 0.0;
            Real dPdy = 0.0;
            Rate r = y.rate();
            Natural N = y.frequency();
            Date lastDate = npvDate;
            const DayCounter& dc = y.dayCounter();
            for (Size i=0; i<leg.size(); ++i) {
                if (leg[i]->hasOccurred(settlementDate,
                                        includeSettlementDateFlows))
                    continue;

                Real c = leg[i]->amount();
                if (leg[i]->tradingExCoupon(settlementDate)) {
                    c = 0.0;
                }

                t += getStepwiseDiscountTime(leg[i], dc, npvDate, lastDate);
                DiscountFactor B = y.discountFactor(t);
                P += c * B;
                switch (y.compounding()) {
                  case Simple:
                    dPdy -= c * B*B * t;
                    break;
                  case Compounded:
                    dPdy -= c * t * B/(1+r/N);
                    break;
                  case Continuous:
                    dPdy -= c * B * t;
                    break;
                  case SimpleThenCompounded:
                    if (t<=1.0/N)
                        dPdy -= c * B*B * t;
                    else
                        dPdy -= c * t * B/(1+r/N);
                    break;
                  case CompoundedThenSimple:
                    if (t>1.0/N)
                        dPdy -= c * B*B * t;
                    else
                        dPdy -= c * t * B/(1+r/N);
                    break;
                  default:
                    QL_FAIL("unknown compounding convention (" <<
                            Integer(y.compounding()) << ")");
                }
                lastDate = leg[i]->date();
            }

            if (P == 0.0) // no cashflows
                return 0.0;
            return -dPdy/P; // reverse derivative sign
        }

        inline Real macaulayDuration(const Leg& leg,
                              const InterestRate& y,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {

            QL_REQUIRE(y.compounding() == Compounded,
                       "compounded rate required");

            return (1.0+y.rate()/y.frequency()) *
                modifiedDuration(leg, y,
                                 includeSettlementDateFlows,
                                 settlementDate, npvDate);
        }

        struct CashFlowLater {
            bool operator()(const boost::shared_ptr<CashFlow> &c,
                            const boost::shared_ptr<CashFlow> &d) {
                return c->date() > d->date();
            }
        };

    } // anonymous namespace ends here

    inline CashFlows::IrrFinder::IrrFinder(const Leg& leg,
                                    Real npv,
                                    const DayCounter& dayCounter,
                                    Compounding comp,
                                    Frequency freq,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate)
    : leg_(leg), npv_(npv),
      dayCounter_(dayCounter), compounding_(comp), frequency_(freq),
      includeSettlementDateFlows_(includeSettlementDateFlows),
      settlementDate_(settlementDate),
      npvDate_(npvDate) {

        if (settlementDate_ == Date())
            settlementDate_ = Settings::instance().evaluationDate();

        if (npvDate_ == Date())
            npvDate_ = settlementDate_;

        checkSign();
    }

    inline Real CashFlows::::IrrFinder::operator()(Rate y) const {
        InterestRate yield(y, dayCounter_, compounding_, frequency_);
        Real NPV = CashFlows::npv(leg_, yield,
                                  includeSettlementDateFlows_,
                                  settlementDate_, npvDate_);
        return npv_ - NPV;
    }

    inline Real CashFlows::::IrrFinder::derivative(Rate y) const {
        InterestRate yield(y, dayCounter_, compounding_, frequency_);
        return modifiedDuration(leg_, yield,
                                includeSettlementDateFlows_,
                                settlementDate_, npvDate_);
    }

    inline void CashFlows::IrrFinder::checkSign() const {
        // depending on the sign of the market price, check that cash
        // flows of the opposite sign have been specified (otherwise
        // IRR is nonsensical.)

        Integer lastSign = sign(-npv_),
                signChanges = 0;
        for (Size i = 0; i < leg_.size(); ++i) {
            if (!leg_[i]->hasOccurred(settlementDate_,
                                      includeSettlementDateFlows_) &&
                !leg_[i]->tradingExCoupon(settlementDate_)) {
                Integer thisSign = sign(leg_[i]->amount());
                if (lastSign * thisSign < 0) // sign change
                    signChanges++;

                if (thisSign != 0)
                    lastSign = thisSign;
            }
        }
        QL_REQUIRE(signChanges > 0,
                   "the given cash flows cannot result in the given market "
                   "price due to their sign");

        /* The following is commented out due to the lack of a QL_WARN macro
        if (signChanges > 1) {    // Danger of non-unique solution
                                  // Check the aggregate cash flows (Norstrom)
            Real aggregateCashFlow = npv;
            signChanges = 0;
            for (Size i = 0; i < leg.size(); ++i) {
                Real nextAggregateCashFlow =
                    aggregateCashFlow + leg[i]->amount();

                if (aggregateCashFlow * nextAggregateCashFlow < 0.0)
                    signChanges++;

                aggregateCashFlow = nextAggregateCashFlow;
            }
            if (signChanges > 1)
                QL_WARN( "danger of non-unique solution");
        };
        */
    }

    inline Real CashFlows::::npv(const Leg& leg,
                        const InterestRate& y,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

#if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(std::adjacent_find(leg.begin(), leg.end(),
                                      CashFlowLater()) == leg.end(),
                   "cashflows must be sorted in ascending order w.r.t. their payment dates");
#endif

        Real npv = 0.0;
        DiscountFactor discount = 1.0;
        Date lastDate = npvDate;
        const DayCounter& dc = y.dayCounter();
        for (Size i=0; i<leg.size(); ++i) {
            if (leg[i]->hasOccurred(settlementDate,
                                    includeSettlementDateFlows))
                continue;

            Real amount = leg[i]->amount();
            if (leg[i]->tradingExCoupon(settlementDate)) {
                amount = 0.0;
            }

            DiscountFactor b = y.discountFactor(getStepwiseDiscountTime(leg[i], dc, npvDate, lastDate));
            discount *= b;
            lastDate = leg[i]->date();

            npv += amount * discount;
        }

        return npv;
    }

    inline Real CashFlows::::npv(const Leg& leg,
                        Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        return npv(leg, InterestRate(yield, dc, comp, freq),
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    inline Real CashFlows::::bps(const Leg& leg,
                        const InterestRate& yield,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        FlatForward flatRate(settlementDate, yield.rate(), yield.dayCounter(),
                             yield.compounding(), yield.frequency());
        return bps(leg, flatRate,
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    inline Real CashFlows::::bps(const Leg& leg,
                        Rate yield,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {
        return bps(leg, InterestRate(yield, dc, comp, freq),
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    inline Rate CashFlows::yield(const Leg& leg,
                          Real npv,
                          const DayCounter& dayCounter,
                          Compounding compounding,
                          Frequency frequency,
                          bool includeSettlementDateFlows,
                          Date settlementDate,
                          Date npvDate,
                          Real accuracy,
                          Size maxIterations,
                          Rate guess) {
        NewtonSafe solver;
        solver.setMaxEvaluations(maxIterations);
        return CashFlows::yield<NewtonSafe>(solver, leg, npv, dayCounter,
                                            compounding, frequency,
                                            includeSettlementDateFlows,
                                            settlementDate, npvDate,
                                            accuracy, guess);
    }


    inline Time CashFlows::duration(const Leg& leg,
                             const InterestRate& rate,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate,
                             Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        switch (type) {
          case Duration::Simple:
            return simpleDuration(leg, rate,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
          case Duration::Modified:
            return modifiedDuration(leg, rate,
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
          case Duration::Macaulay:
            return macaulayDuration(leg, rate,
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
          default:
            QL_FAIL("unknown duration type");
        }
    }

    inline Time CashFlows::duration(const Leg& leg,
                             Rate yield,
                             const DayCounter& dc,
                             Compounding comp,
                             Frequency freq,
                             Duration::Type type,
                             bool includeSettlementDateFlows,
                             Date settlementDate,
                             Date npvDate) {
        return duration(leg, InterestRate(yield, dc, comp, freq),
                        type,
                        includeSettlementDateFlows,
                        settlementDate, npvDate);
    }

    inline Real CashFlows::::convexity(const Leg& leg,
                              const InterestRate& y,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        const DayCounter& dc = y.dayCounter();

        Real P = 0.0;
        Time t = 0.0;
        Real d2Pdy2 = 0.0;
        Rate r = y.rate();
        Natural N = y.frequency();
        Date lastDate = npvDate;
        for (Size i=0; i<leg.size(); ++i) {
            if (leg[i]->hasOccurred(settlementDate,
                                        includeSettlementDateFlows))
                continue;
            
            Real c = leg[i]->amount();
            if (leg[i]->tradingExCoupon(settlementDate)) {
                c = 0.0;
            }

            t += getStepwiseDiscountTime(leg[i], dc, npvDate, lastDate);
            DiscountFactor B = y.discountFactor(t);
            P += c * B;
            switch (y.compounding()) {
              case Simple:
                d2Pdy2 += c * 2.0*B*B*B*t*t;
                break;
              case Compounded:
                d2Pdy2 += c * B*t*(N*t+1)/(N*(1+r/N)*(1+r/N));
                break;
              case Continuous:
                d2Pdy2 += c * B*t*t;
                break;
              case SimpleThenCompounded:
                if (t<=1.0/N)
                    d2Pdy2 += c * 2.0*B*B*B*t*t;
                else
                    d2Pdy2 += c * B*t*(N*t+1)/(N*(1+r/N)*(1+r/N));
                break;
              case CompoundedThenSimple:
                if (t>1.0/N)
                    d2Pdy2 += c * 2.0*B*B*B*t*t;
                else
                    d2Pdy2 += c * B*t*(N*t+1)/(N*(1+r/N)*(1+r/N));
                break;
              default:
                QL_FAIL("unknown compounding convention (" <<
                        Integer(y.compounding()) << ")");
            }
            lastDate = leg[i]->date();
        }

        if (P == 0.0)
            // no cashflows
            return 0.0;

        return d2Pdy2/P;
    }


    inline Real CashFlows::::convexity(const Leg& leg,
                              Rate yield,
                              const DayCounter& dc,
                              Compounding comp,
                              Frequency freq,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate) {
        return convexity(leg, InterestRate(yield, dc, comp, freq),
                         includeSettlementDateFlows,
                         settlementDate, npvDate);
    }

    inline Real CashFlows::::basisPointValue(const Leg& leg,
                                    const InterestRate& y,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real npv = CashFlows::npv(leg, y,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
        Real modifiedDuration = CashFlows::duration(leg, y,
                                                    Duration::Modified,
                                                    includeSettlementDateFlows,
                                                    settlementDate, npvDate);
        Real convexity = CashFlows::convexity(leg, y,
                                              includeSettlementDateFlows,
                                              settlementDate, npvDate);
        Real delta = -modifiedDuration*npv;
        Real gamma = (convexity/100.0)*npv;

        Real shift = 0.0001;
        delta *= shift;
        gamma *= shift*shift;

        return delta + 0.5*gamma;
    }

    inline Real CashFlows::::basisPointValue(const Leg& leg,
                                    Rate yield,
                                    const DayCounter& dc,
                                    Compounding comp,
                                    Frequency freq,
                                    bool includeSettlementDateFlows,
                                    Date settlementDate,
                                    Date npvDate) {
        return basisPointValue(leg, InterestRate(yield, dc, comp, freq),
                               includeSettlementDateFlows,
                               settlementDate, npvDate);
    }

    inline Real CashFlows::::yieldValueBasisPoint(const Leg& leg,
                                         const InterestRate& y,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate,
                                         Date npvDate) {
        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real npv = CashFlows::npv(leg, y,
                                  includeSettlementDateFlows,
                                  settlementDate, npvDate);
        Real modifiedDuration = CashFlows::duration(leg, y,
                                                    Duration::Modified,
                                                    includeSettlementDateFlows,
                                                    settlementDate, npvDate);

        Real shift = 0.01;
        return (1.0/(-npv*modifiedDuration))*shift;
    }

    inline Real CashFlows::::yieldValueBasisPoint(const Leg& leg,
                                         Rate yield,
                                         const DayCounter& dc,
                                         Compounding comp,
                                         Frequency freq,
                                         bool includeSettlementDateFlows,
                                         Date settlementDate,
                                         Date npvDate) {
        return yieldValueBasisPoint(leg, InterestRate(yield, dc, comp, freq),
                                    includeSettlementDateFlows,
                                    settlementDate, npvDate);
    }

    // Z-spread utility functions
    namespace {

        class ZSpreadFinder : public std::unary_function<Rate, Real> {
          public:
            ZSpreadFinder(const Leg& leg,
                          const shared_ptr<YieldTermStructure>& discountCurve,
                          Real npv,
                          const DayCounter& dc,
                          Compounding comp,
                          Frequency freq,
                          bool includeSettlementDateFlows,
                          Date settlementDate,
                          Date npvDate)
            : leg_(leg), npv_(npv), zSpread_(new SimpleQuote(0.0)),
              curve_(Handle<YieldTermStructure>(discountCurve),
                     Handle<Quote>(zSpread_), comp, freq, dc),
              includeSettlementDateFlows_(includeSettlementDateFlows),
              settlementDate_(settlementDate),
              npvDate_(npvDate) {

                if (settlementDate_ == Date())
                    settlementDate_ = Settings::instance().evaluationDate();

                if (npvDate_ == Date())
                    npvDate_ = settlementDate_;

                // if the discount curve allows extrapolation, let's
                // the spreaded curve do too.
                curve_.enableExtrapolation(
                                  discountCurve->allowsExtrapolation());
            }
            Real operator()(Rate zSpread) const {
                zSpread_->setValue(zSpread);
                Real NPV = CashFlows::npv(leg_, curve_,
                                          includeSettlementDateFlows_,
                                          settlementDate_, npvDate_);
                return npv_ - NPV;
            }
          private:
            const Leg& leg_;
            Real npv_;
            shared_ptr<SimpleQuote> zSpread_;
            ZeroSpreadedTermStructure curve_;
            bool includeSettlementDateFlows_;
            Date settlementDate_, npvDate_;
        };

    } // anonymous namespace ends here

    inline Real CashFlows::::npv(const Leg& leg,
                        const shared_ptr<YieldTermStructure>& discountCurve,
                        Spread zSpread,
                        const DayCounter& dc,
                        Compounding comp,
                        Frequency freq,
                        bool includeSettlementDateFlows,
                        Date settlementDate,
                        Date npvDate) {

        if (leg.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Handle<YieldTermStructure> discountCurveHandle(discountCurve);
        Handle<Quote> zSpreadQuoteHandle(shared_ptr<Quote>(new
            SimpleQuote(zSpread)));

        ZeroSpreadedTermStructure spreadedCurve(discountCurveHandle,
                                                zSpreadQuoteHandle,
                                                comp, freq, dc);

        spreadedCurve.enableExtrapolation(discountCurveHandle->allowsExtrapolation());

        return npv(leg, spreadedCurve,
                   includeSettlementDateFlows,
                   settlementDate, npvDate);
    }

    inline Spread CashFlows::zSpread(const Leg& leg,
                              Real npv,
                              const shared_ptr<YieldTermStructure>& discount,
                              const DayCounter& dayCounter,
                              Compounding compounding,
                              Frequency frequency,
                              bool includeSettlementDateFlows,
                              Date settlementDate,
                              Date npvDate,
                              Real accuracy,
                              Size maxIterations,
                              Rate guess) {

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Brent solver;
        solver.setMaxEvaluations(maxIterations);
        ZSpreadFinder objFunction(leg,
                                  discount,
                                  npv,
                                  dayCounter, compounding, frequency, includeSettlementDateFlows,
                                  settlementDate, npvDate);
        Real step = 0.01;
        return solver.solve(objFunction, accuracy, guess, step);
    }

}


#endif
