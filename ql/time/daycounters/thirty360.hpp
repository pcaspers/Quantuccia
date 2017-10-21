/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file thirty360.hpp
    \brief 30/360 day counters
*/

#ifndef quantlib_thirty360_day_counter_h
#define quantlib_thirty360_day_counter_h

#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! 30/360 day count convention
    /*! The 30/360 day count can be calculated according to US, European, or
        Italian conventions.

        US (NASD) convention: if the starting date is the 31st of a
        month, it becomes equal to the 30th of the same month.
        If the ending date is the 31st of a month and the starting
        date is earlier than the 30th of a month, the ending date
        becomes equal to the 1st of the next month, otherwise the
        ending date becomes equal to the 30th of the same month.
        Also known as "30/360", "360/360", or "Bond Basis"

        European convention: starting dates or ending dates that
        occur on the 31st of a month become equal to the 30th of the
        same month.
        Also known as "30E/360", or "Eurobond Basis"

        Italian convention: starting dates or ending dates that
        occur on February and are grater than 27 become equal to 30
        for computational sake.

        \ingroup daycounters
    */
    class Thirty360 : public DayCounter {
      public:
        enum Convention { USA, BondBasis,
                          European, EurobondBasis,
                          Italian };
      private:
        class US_Impl : public DayCounter::Impl {
          public:
            std::string name() const { return std::string("30/360 (Bond Basis)");}
            Date::serial_type dayCount(const Date& d1,
                                       const Date& d2) const;
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date&, 
                              const Date&) const {
                return dayCount(d1,d2)/360.0; }
        };
        class EU_Impl : public DayCounter::Impl {
          public:
            std::string name() const { return std::string("30E/360 (Eurobond Basis)");}
            Date::serial_type dayCount(const Date& d1,
                                       const Date& d2) const;
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date&,
                              const Date&) const {
                return dayCount(d1,d2)/360.0; }
        };
        class IT_Impl : public DayCounter::Impl {
          public:
            std::string name() const { return std::string("30/360 (Italian)");}
            Date::serial_type dayCount(const Date& d1, const Date& d2) const;
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date&,
                              const Date&) const {
                return dayCount(d1,d2)/360.0; }
        };
        static boost::shared_ptr<DayCounter::Impl> implementation(
                                                               Convention c);
      public:
        Thirty360(Convention c = Thirty360::BondBasis)
        : DayCounter(implementation(c)) {}
    };

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

namespace QuantLib {

    boost::shared_ptr<DayCounter::Impl>
    Thirty360::implementation(Thirty360::Convention c) {
        switch (c) {
          case USA:
          case BondBasis:
            return boost::shared_ptr<DayCounter::Impl>(new US_Impl);
          case European:
          case EurobondBasis:
            return boost::shared_ptr<DayCounter::Impl>(new EU_Impl);
          case Italian:
            return boost::shared_ptr<DayCounter::Impl>(new IT_Impl);
          default:
            QL_FAIL("unknown 30/360 convention");
        }
    }

    Date::serial_type Thirty360::US_Impl::dayCount(const Date& d1,
                                                   const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Integer mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (dd2 == 31 && dd1 < 30) { dd2 = 1; mm2++; }

        return 360*(yy2-yy1) + 30*(mm2-mm1-1) +
            std::max(Integer(0),30-dd1) + std::min(Integer(30),dd2);
    }

    Date::serial_type Thirty360::EU_Impl::dayCount(const Date& d1,
                                                   const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Month mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        return 360*(yy2-yy1) + 30*(mm2-mm1-1) +
            std::max(Integer(0),30-dd1) + std::min(Integer(30),dd2);
    }

    Date::serial_type Thirty360::IT_Impl::dayCount(const Date& d1,
                                                   const Date& d2) const {
        Day dd1 = d1.dayOfMonth(), dd2 = d2.dayOfMonth();
        Month mm1 = d1.month(), mm2 = d2.month();
        Year yy1 = d1.year(), yy2 = d2.year();

        if (mm1 == 2 && dd1 > 27) dd1 = 30;
        if (mm2 == 2 && dd2 > 27) dd2 = 30;

        return 360*(yy2-yy1) + 30*(mm2-mm1-1) +
            std::max(Integer(0),30-dd1) + std::min(Integer(30),dd2);
    }

}

#endif