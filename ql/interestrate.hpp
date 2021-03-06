/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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

/*! \file interestrate.hpp
    \brief Instrument rate class
*/

#ifndef quantlib_interest_rate_hpp
#define quantlib_interest_rate_hpp

#include <ql/compounding.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    //! Concrete interest rate class
    /*! This class encapsulate the interest rate compounding algebra.
        It manages day-counting conventions, compounding conventions,
        conversion between different conventions, discount/compound factor
        calculations, and implied/equivalent rate calculations.

        \test Converted rates are checked against known good results
    */
    class InterestRate {
      public:
        //! \name constructors
        //@{
        //! Default constructor returning a null interest rate.
        InterestRate();
        //! Standard constructor
        InterestRate(Rate r,
                     const DayCounter& dc,
                     Compounding comp,
                     Frequency freq);
        //@}
        //! \name conversions
        //@{
        operator Rate() const { return r_; }
        //@}
        //! \name inspectors
        //@{
        Rate rate() const { return r_; }
        const DayCounter& dayCounter() const { return dc_; }
        Compounding compounding() const { return comp_; }
        Frequency frequency() const {
            return freqMakesSense_ ? Frequency(Integer(freq_)) : NoFrequency;
        }
        //@}

        //! \name discount/compound factor calculations
        //@{
        //! discount factor implied by the rate compounded at time t.
        /*! \warning Time must be measured using InterestRate's own
                     day counter.
        */
        DiscountFactor discountFactor(Time t) const {
            return 1.0/compoundFactor(t);
        }

        //! discount factor implied by the rate compounded between two dates
        DiscountFactor discountFactor(const Date& d1,
                                      const Date& d2,
                                      const Date& refStart = Date(),
                                      const Date& refEnd = Date()) const {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t = dc_.yearFraction(d1, d2, refStart, refEnd);
            return discountFactor(t);
        }

        //! compound factor implied by the rate compounded at time t.
        /*! returns the compound (a.k.a capitalization) factor
            implied by the rate compounded at time t.

            \warning Time must be measured using InterestRate's own
                     day counter.
        */
        Real compoundFactor(Time t) const;

        //! compound factor implied by the rate compounded between two dates
        /*! returns the compound (a.k.a capitalization) factor
            implied by the rate compounded between two dates.
        */
        Real compoundFactor(const Date& d1,
                            const Date& d2,
                            const Date& refStart = Date(),
                            const Date& refEnd = Date()) const {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t = dc_.yearFraction(d1, d2, refStart, refEnd);
            return compoundFactor(t);
        }
        //@}

        //! \name implied rate calculations
        //@{

        //! implied interest rate for a given compound factor at a given time.
        /*! The resulting InterestRate has the day-counter provided as input.

            \warning Time must be measured using the day-counter provided
                     as input.
        */
        static InterestRate impliedRate(Real compound,
                                        const DayCounter& resultDC,
                                        Compounding comp,
                                        Frequency freq,
                                        Time t);

        //! implied rate for a given compound factor between two dates.
        /*! The resulting rate is calculated taking the required
            day-counting rule into account.
        */
        static InterestRate impliedRate(Real compound,
                                        const DayCounter& resultDC,
                                        Compounding comp,
                                        Frequency freq,
                                        const Date& d1,
                                        const Date& d2,
                                        const Date& refStart = Date(),
                                        const Date& refEnd = Date()) {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t = resultDC.yearFraction(d1, d2, refStart, refEnd);
            return impliedRate(compound, resultDC, comp, freq, t);
        }
        //@}

        //! \name equivalent rate calculations
        //@{

        //! equivalent interest rate for a compounding period t.
        /*! The resulting InterestRate shares the same implicit
            day-counting rule of the original InterestRate instance.

            \warning Time must be measured using the InterestRate's
                     own day counter.
        */
        InterestRate equivalentRate(Compounding comp,
                                    Frequency freq,
                                    Time t) const {
            return impliedRate(compoundFactor(t), dc_, comp, freq, t);
        }

        //! equivalent rate for a compounding period between two dates
        /*! The resulting rate is calculated taking the required
            day-counting rule into account.
        */
        InterestRate equivalentRate(const DayCounter& resultDC,
                                    Compounding comp,
                                    Frequency freq,
                                    Date d1,
                                    Date d2,
                                    const Date& refStart = Date(),
                                    const Date& refEnd = Date()) const {
            QL_REQUIRE(d2>=d1,
                       "d1 (" << d1 << ") "
                       "later than d2 (" << d2 << ")");
            Time t1 = dc_.yearFraction(d1, d2, refStart, refEnd);
            Time t2 = resultDC.yearFraction(d1, d2, refStart, refEnd);
            return impliedRate(compoundFactor(t1), resultDC, comp, freq, t2);
        }
        //@}
      private:
        Rate r_;
        DayCounter dc_;
        Compounding comp_;
        bool freqMakesSense_;
        Real freq_;
    };

    /*! \relates InterestRate */
    std::ostream& operator<<(std::ostream&,
                             const InterestRate&);

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano

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
#include <sstream>
#include <iomanip>

namespace QuantLib {

    // constructors

    inline InterestRate::InterestRate()
    : r_(Null<Real>()) {}

    inline InterestRate::InterestRate(Rate r,
                               const DayCounter& dc,
                               Compounding comp,
                               Frequency freq)
    : r_(r), dc_(dc), comp_(comp), freqMakesSense_(false) {

        if (comp_==Compounded || comp_==SimpleThenCompounded || comp_==CompoundedThenSimple) {
            freqMakesSense_ = true;
            QL_REQUIRE(freq!=Once && freq!=NoFrequency,
                       "frequency not allowed for this interest rate");
            freq_ = Real(freq);
        }
    }

    inline Real InterestRate::compoundFactor(Time t) const {

        QL_REQUIRE(t>=0.0, "negative time (" << t << ") not allowed");
        QL_REQUIRE(r_ != Null<Rate>(), "null interest rate");
        switch (comp_) {
          case Simple:
            return 1.0 + r_*t;
          case Compounded:
            return std::pow(1.0+r_/freq_, freq_*t);
          case Continuous:
            return std::exp(r_*t);
          case SimpleThenCompounded:
            if (t<=1.0/Real(freq_))
                return 1.0 + r_*t;
            else
                return std::pow(1.0+r_/freq_, freq_*t);
          case CompoundedThenSimple:
            if (t>1.0/Real(freq_))
                return 1.0 + r_*t;
            else
                return std::pow(1.0+r_/freq_, freq_*t);
          default:
            QL_FAIL("unknown compounding convention");
        }
    }

    inline InterestRate InterestRate::impliedRate(Real compound,
                                           const DayCounter& resultDC,
                                           Compounding comp,
                                           Frequency freq,
                                           Time t) {

        QL_REQUIRE(compound>0.0, "positive compound factor required");

        Rate r;
        if (compound==1.0) {
            QL_REQUIRE(t>=0.0, "non negative time (" << t << ") required");
            r = 0.0;
        } else {
            QL_REQUIRE(t>0.0, "positive time (" << t << ") required");
            switch (comp) {
              case Simple:
                r = (compound - 1.0)/t;
                break;
              case Compounded:
                r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
                break;
              case Continuous:
                r = std::log(compound)/t;
                break;
              case SimpleThenCompounded:
                if (t<=1.0/Real(freq))
                    r = (compound - 1.0)/t;
                else
                    r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
                break;
              case CompoundedThenSimple:
                if (t>1.0/Real(freq))
                    r = (compound - 1.0)/t;
                else
                    r = (std::pow(compound, 1.0/(Real(freq)*t))-1.0)*Real(freq);
                break;
              default:
                QL_FAIL("unknown compounding convention ("
                        << Integer(comp) << ")");
            }
        }
        return InterestRate(r, resultDC, comp, freq);
    }


    inline std::ostream& operator<<(std::ostream& out, const InterestRate& ir) {
        if (ir.rate() == Null<Rate>())
            return out << "null interest rate";

        out << io::rate(ir.rate()) << " " << ir.dayCounter().name() << " ";
        switch (ir.compounding()) {
          case Simple:
            out << "simple compounding";
            break;
          case Compounded:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << ir.frequency() <<" compounding";
            }
            break;
          case Continuous:
            out << "continuous compounding";
            break;
          case SimpleThenCompounded:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << "simple compounding up to "
                    << Integer(12/ir.frequency()) << " months, then "
                    << ir.frequency() << " compounding";
            }
            break;
          case CompoundedThenSimple:
            switch (ir.frequency()) {
              case NoFrequency:
              case Once:
                QL_FAIL(ir.frequency() << " frequency not allowed "
                        "for this interest rate");
              default:
                out << "compounding up to "
                    << Integer(12/ir.frequency()) << " months, then "
                    << ir.frequency() << " simple compounding";
            }
            break;
          default:
            QL_FAIL("unknown compounding convention ("
                    << Integer(ir.compounding()) << ")");
        }
        return out;
    }

}

#endif