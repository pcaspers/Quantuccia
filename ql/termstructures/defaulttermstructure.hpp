/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file defaulttermstructure.hpp
    \brief default-probability term structure
*/

#ifndef quantlib_default_term_structure_hpp
#define quantlib_default_term_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Default probability term structure
    /*! This abstract class defines the interface of concrete
        credit structures which will be derived from this one.

        \ingroup defaultprobabilitytermstructures
    */
    class DefaultProbabilityTermStructure : public TermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        DefaultProbabilityTermStructure(
            const DayCounter& dc = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        DefaultProbabilityTermStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dc = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        DefaultProbabilityTermStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dc = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        //@}

        /*! \name Survival probabilities

            These methods return the survival probability from the reference
            date until a given date or time.  In the latter case, the time
            is calculated as a fraction of year from the reference date.
        */
        //@{
        Probability survivalProbability(const Date& d,
                                        bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        Probability survivalProbability(Time t,
                                        bool extrapolate = false) const;
        //@}

        /*! \name Default probabilities

            These methods return the default probability from the reference
            date until a given date or time.  In the latter case, the time
            is calculated as a fraction of year from the reference date.
        */
        //@{
        Probability defaultProbability(const Date& d,
                                       bool extrapolate = false) const;
        /*! The same day-counting rule used by the term structure
            should be used for calculating the passed time t.
        */
        Probability defaultProbability(Time t,
                                       bool extrapolate = false) const;
        //! probability of default between two given dates
        Probability defaultProbability(const Date&,
                                       const Date&,
                                       bool extrapolate = false) const;
        //! probability of default between two given times
        Probability defaultProbability(Time,
                                       Time,
                                       bool extrapo = false) const;
        //@}

        /*! \name Default densities

            These methods return the default density at a given date or time.
            In the latter case, the time is calculated as a fraction of year
            from the reference date.
        */
        //@{
        Real defaultDensity(const Date& d,
                            bool extrapolate = false) const;
        Real defaultDensity(Time t,
                            bool extrapolate = false) const;
        //@}

        /*! \name Hazard rates

            These methods returns the hazard rate at a given date or time.
            In the latter case, the time is calculated as a fraction of year
            from the reference date.
            
            Hazard rates are defined with annual frequency and continuous
            compounding.
        */

        //@{
        Rate hazardRate(const Date& d,
                        bool extrapolate = false) const;
        Rate hazardRate(Time t,
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
            These methods must be implemented in derived classes to
            perform the actual calculations. When they are called,
            range check has already been performed; therefore, they
            must assume that extrapolation is required.
        */
        //@{
        //! survival probability calculation
        virtual Probability survivalProbabilityImpl(Time) const = 0;
        //! default density calculation
        virtual Real defaultDensityImpl(Time) const = 0;
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
    Probability DefaultProbabilityTermStructure::survivalProbability(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return survivalProbability(timeFromReference(d), extrapolate);
    }

    inline
    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return 1.0 - survivalProbability(d, extrapolate);
    }

    inline
    Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     Time t,
                                                     bool extrapolate) const {
        return 1.0 - survivalProbability(t, extrapolate);
    }

    inline
    Real DefaultProbabilityTermStructure::defaultDensity(
                                                     const Date& d,
                                                     bool extrapolate) const {
        return defaultDensity(timeFromReference(d), extrapolate);
    }

    inline
    Real DefaultProbabilityTermStructure::defaultDensity(
                                                     Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);
        return defaultDensityImpl(t);
    }

    inline
    Rate DefaultProbabilityTermStructure::hazardRate(const Date& d,
                                                     bool extrapolate) const {
        return hazardRate(timeFromReference(d), extrapolate);
    }

    inline
    Rate DefaultProbabilityTermStructure::hazardRate(Time t,
                                                     bool extrapolate) const {
        Probability S = survivalProbability(t, extrapolate);
        return S == 0.0 ? 0.0 : defaultDensity(t, extrapolate)/S;
    }

    inline
    const std::vector<Date>&
    DefaultProbabilityTermStructure::jumpDates() const {
        return this->jumpDates_;
    }

    inline
    const std::vector<Time>&
    DefaultProbabilityTermStructure::jumpTimes() const {
        return this->jumpTimes_;
    }

    inline void DefaultProbabilityTermStructure::update() {
        TermStructure::update();
        if (referenceDate() != latestReference_)
            setJumps();
    }

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

    inline DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
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

    inline DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
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

    inline DefaultProbabilityTermStructure::DefaultProbabilityTermStructure(
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

    inline void DefaultProbabilityTermStructure::setJumps() {
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

    inline Probability DefaultProbabilityTermStructure::survivalProbability(
                                                     Time t,
                                                     bool extrapolate) const {
        checkRange(t, extrapolate);

        if (!jumps_.empty()) {
            Probability jumpEffect = 1.0;
            for (Size i=0; i<nJumps_ && jumpTimes_[i]<t; ++i) {
                QL_REQUIRE(jumps_[i]->isValid(),
                           "invalid " << io::ordinal(i+1) << " jump quote");
                DiscountFactor thisJump = jumps_[i]->value();
                QL_REQUIRE(thisJump > 0.0 && thisJump <= 1.0,
                           "invalid " << io::ordinal(i+1) << " jump value: " <<
                           thisJump);
                jumpEffect *= thisJump;
            }
            return jumpEffect * survivalProbabilityImpl(t);
        }

        return survivalProbabilityImpl(t);
    }

    inline Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     const Date& d1,
                                                     const Date& d2,
                                                     bool extrapolate) const {
        QL_REQUIRE(d1 <= d2,
                   "initial date (" << d1 << ") "
                   "later than final date (" << d2 << ")");
        Probability p1 = d1 < referenceDate() ? 0.0 :
                                           defaultProbability(d1,extrapolate),
                    p2 = defaultProbability(d2,extrapolate);
        return p2 - p1;
    }

    inline Probability DefaultProbabilityTermStructure::defaultProbability(
                                                     Time t1,
                                                     Time t2,
                                                     bool extrapolate) const {
        QL_REQUIRE(t1 <= t2,
                   "initial time (" << t1 << ") "
                   "later than final time (" << t2 << ")");
        Probability p1 = t1 < 0.0 ? 0.0 : defaultProbability(t1,extrapolate),
                    p2 = defaultProbability(t2,extrapolate);
        return p2 - p1;
    }

}


#endif
