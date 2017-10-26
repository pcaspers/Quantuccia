/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2009 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file yieldtermstructure.hpp
    \brief Interest-rate term structure
*/

#ifndef quantlib_yield_term_structure_hpp
#define quantlib_yield_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/interestrate.hpp>
#include <ql/quote.hpp>
#include <vector>

namespace QuantLib {

    //! Interest-rate term structure
    /*! This abstract class defines the interface of concrete
        interest rate structures which will be derived from this one.

        \ingroup yieldtermstructures

        \test observability against evaluation date changes is checked.
    */
    class YieldTermStructure : public TermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        YieldTermStructure(const DayCounter& dc = DayCounter(),
                           const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
                           const std::vector<Date>& jumpDates = std::vector<Date>());
        YieldTermStructure(const Date& referenceDate,
                           const Calendar& cal = Calendar(),
                           const DayCounter& dc = DayCounter(),
                           const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
                           const std::vector<Date>& jumpDates = std::vector<Date>());
        YieldTermStructure(Natural settlementDays,
                           const Calendar& cal,
                           const DayCounter& dc = DayCounter(),
                           const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
                           const std::vector<Date>& jumpDates = std::vector<Date>());
        //@}

        /*! \name Discount factors

            These methods return the discount factor from a given date or time
            to the reference date.  In the latter case, the time is calculated
            as a fraction of year from the reference date.
        */
        //@{
        DiscountFactor discount(const Date& d,
                                bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        DiscountFactor discount(Time t,
                                bool extrapolate = false) const;
        //@}

        /*! \name Zero-yield rates

            These methods return the implied zero-yield rate for a
            given date or time.  In the former case, the time is
            calculated as a fraction of year from the reference date.
        */
        //@{
        /*! The resulting interest rate has the required daycounting
            rule.
        */
        InterestRate zeroRate(const Date& d,
                              const DayCounter& resultDayCounter,
                              Compounding comp,
                              Frequency freq = Annual,
                              bool extrapolate = false) const;

        /*! The resulting interest rate has the same day-counting rule
            used by the term structure. The same rule should be used
            for calculating the passed time t.
        */
        InterestRate zeroRate(Time t,
                              Compounding comp,
                              Frequency freq = Annual,
                              bool extrapolate = false) const;
        //@}

        /*! \name Forward rates

            These methods returns the forward interest rate between two dates
            or times.  In the former case, times are calculated as fractions
            of year from the reference date.

            If both dates (times) are equal the instantaneous forward rate is
            returned.
        */
        //@{
        /*! The resulting interest rate has the required day-counting
            rule.
        */
        InterestRate forwardRate(const Date& d1,
                                 const Date& d2,
                                 const DayCounter& resultDayCounter,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;
        /*! The resulting interest rate has the required day-counting
            rule.
            \warning dates are not adjusted for holidays
        */
        InterestRate forwardRate(const Date& d,
                                 const Period& p,
                                 const DayCounter& resultDayCounter,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;

        /*! The resulting interest rate has the same day-counting rule
            used by the term structure. The same rule should be used
            for calculating the passed times t1 and t2.
        */
        InterestRate forwardRate(Time t1,
                                 Time t2,
                                 Compounding comp,
                                 Frequency freq = Annual,
                                 bool extrapolate = false) const;
        //@}

        //! \name Jump inspectors
        //@{
        const std::vector<Date>& jumpDates() const;
        const std::vector<Time>& jumpTimes() const;
        //@}

        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        /*! \name Calculations

            This method must be implemented in derived classes to
            perform the actual calculations. When it is called,
            range check has already been performed; therefore, it
            must assume that extrapolation is required.
        */
        //@{
        //! discount factor calculation
        virtual DiscountFactor discountImpl(Time) const = 0;
        //@}
      private:
        // methods
        void setJumps();
        // data members
        std::vector<Handle<Quote> > jumps_;
        std::vector<Date> jumpDates_;
        std::vector<Time> jumpTimes_;
        Size nJumps_;
        Date latestReference_;
    };

    // inline definitions

    inline
    DiscountFactor YieldTermStructure::discount(const Date& d,
                                                bool extrapolate) const {
        return discount(timeFromReference(d), extrapolate);
    }

    inline
    InterestRate YieldTermStructure::forwardRate(const Date& d,
                                                 const Period& p,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        return forwardRate(d, d+p, dayCounter, comp, freq, extrapolate);
    }

    inline const std::vector<Date>& YieldTermStructure::jumpDates() const {
        return this->jumpDates_;
    }

    inline const std::vector<Time>& YieldTermStructure::jumpTimes() const {
        return this->jumpTimes_;
    }

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2009 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    namespace {
        // time interval used in finite differences
        const Time dt = 0.0001;
    }

    inline YieldTermStructure::YieldTermStructure(
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : TermStructure(dc), jumps_(jumps),
      jumpDates_(jumpDates), jumpTimes_(jumpDates.size()),
      nJumps_(jumps_.size()) {
        setJumps();
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    inline YieldTermStructure::YieldTermStructure(
                                    const Date& referenceDate,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : TermStructure(referenceDate, cal, dc), jumps_(jumps),
      jumpDates_(jumpDates), jumpTimes_(jumpDates.size()),
      nJumps_(jumps_.size()) {
        setJumps();
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    inline YieldTermStructure::YieldTermStructure(
                                    Natural settlementDays,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : TermStructure(settlementDays, cal, dc), jumps_(jumps),
      jumpDates_(jumpDates), jumpTimes_(jumpDates.size()),
      nJumps_(jumps_.size()) {
        setJumps();
        for (Size i=0; i<nJumps_; ++i)
            registerWith(jumps_[i]);
    }

    inline void YieldTermStructure::setJumps() {
        if (jumpDates_.empty() && !jumps_.empty()) { // turn of year dates
            jumpDates_.resize(nJumps_);
            jumpTimes_.resize(nJumps_);
            Year y = referenceDate().year();
            for (Size i=0; i<nJumps_; ++i)
                jumpDates_[i] = Date(31, December, y+i);
        } else { // fixed dats
            QL_REQUIRE(jumpDates_.size()==nJumps_,
                       "mismatch between number of jumps (" << nJumps_ <<
                       ") and jump dates (" << jumpDates_.size() << ")");
        }
        for (Size i=0; i<nJumps_; ++i)
            jumpTimes_[i] = timeFromReference(jumpDates_[i]);
        latestReference_ = referenceDate();
    }

    inline DiscountFactor YieldTermStructure::discount(Time t,
                                                bool extrapolate) const {
        checkRange(t, extrapolate);

        if (jumps_.empty())
            return discountImpl(t);

        DiscountFactor jumpEffect = 1.0;
        for (Size i=0; i<nJumps_; ++i) {
            if (jumpTimes_[i]>0 && jumpTimes_[i]<t) {
                QL_REQUIRE(jumps_[i]->isValid(),
                           "invalid " << io::ordinal(i+1) << " jump quote");
                DiscountFactor thisJump = jumps_[i]->value();
                QL_REQUIRE(thisJump > 0.0,
                           "invalid " << io::ordinal(i+1) << " jump value: " <<
                           thisJump);
                #if !defined(QL_NEGATIVE_RATES)
                QL_REQUIRE(thisJump <= 1.0,
                           "invalid " << io::ordinal(i+1) << " jump value: " <<
                           thisJump);
                #endif
                jumpEffect *= thisJump;
            }
        }
        return jumpEffect * discountImpl(t);
    }

    inline InterestRate YieldTermStructure::zeroRate(const Date& d,
                                              const DayCounter& dayCounter,
                                              Compounding comp,
                                              Frequency freq,
                                              bool extrapolate) const {
        if (d==referenceDate()) {
            Real compound = 1.0/discount(dt, extrapolate);
            // t has been calculated with a possibly different daycounter
            // but the difference should not matter for very small times
            return InterestRate::impliedRate(compound,
                                             dayCounter, comp, freq,
                                             dt);
        }
        Real compound = 1.0/discount(d, extrapolate);
        return InterestRate::impliedRate(compound,
                                         dayCounter, comp, freq,
                                         referenceDate(), d);
    }

    inline InterestRate YieldTermStructure::zeroRate(Time t,
                                              Compounding comp,
                                              Frequency freq,
                                              bool extrapolate) const {
        if (t==0.0) t = dt;
        Real compound = 1.0/discount(t, extrapolate);
        return InterestRate::impliedRate(compound,
                                         dayCounter(), comp, freq,
                                         t);
    }

    inline InterestRate YieldTermStructure::forwardRate(const Date& d1,
                                                 const Date& d2,
                                                 const DayCounter& dayCounter,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        if (d1==d2) {
            checkRange(d1, extrapolate);
            Time t1 = std::max(timeFromReference(d1) - dt/2.0, 0.0);
            Time t2 = t1 + dt;
            Real compound =
                discount(t1, true)/discount(t2, true);
            // times have been calculated with a possibly different daycounter
            // but the difference should not matter for very small times
            return InterestRate::impliedRate(compound,
                                             dayCounter, comp, freq,
                                             dt);
        }
        QL_REQUIRE(d1 < d2,  d1 << " later than " << d2);
        Real compound = discount(d1, extrapolate)/discount(d2, extrapolate);
        return InterestRate::impliedRate(compound,
                                         dayCounter, comp, freq,
                                         d1, d2);
    }

    inline InterestRate YieldTermStructure::forwardRate(Time t1,
                                                 Time t2,
                                                 Compounding comp,
                                                 Frequency freq,
                                                 bool extrapolate) const {
        Real compound;
        if (t2==t1) {
            checkRange(t1, extrapolate);
            t1 = std::max(t1 - dt/2.0, 0.0);
            t2 = t1 + dt;
            compound = discount(t1, true)/discount(t2, true);
        } else {
            QL_REQUIRE(t2>t1, "t2 (" << t2 << ") < t1 (" << t2 << ")");
            compound = discount(t1, extrapolate)/discount(t2, extrapolate);
        }
        return InterestRate::impliedRate(compound,
                                         dayCounter(), comp, freq,
                                         t2-t1);
    }

    inline void YieldTermStructure::update() {
        TermStructure::update();
        Date newReference = Date();
        try {
            newReference = referenceDate();
            if (newReference != latestReference_)
                setJumps();
        } catch (Error&) {
            if (newReference == Date()) {
                // the curve couldn't calculate the reference
                // date. Most of the times, this is because some
                // underlying handle wasn't set, so we can just absorb
                // the exception and continue; the jumps will be set
                // correctly when a valid underlying is set.
                return;
            } else {
                // something else happened during the call to
                // setJumps(), so we let the exception bubble up.
                throw;
            }
        }
    }

}

#endif