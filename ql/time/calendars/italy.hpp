/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
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

/*! \file italy.hpp
    \brief Italian calendars
*/

#ifndef quantlib_italy_calendar_hpp
#define quantlib_italy_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Italian calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Epiphany, January 6th</li>
        <li>Easter Monday</li>
        <li>Liberation Day, April 25th</li>
        <li>Labour Day, May 1st</li>
        <li>Republic Day, June 2nd (since 2000)</li>
        <li>Assumption, August 15th</li>
        <li>All Saint's Day, November 1st</li>
        <li>Immaculate Conception Day, December 8th</li>
        <li>Christmas Day, December 25th</li>
        <li>St. Stephen's Day, December 26th</li>
        </ul>

        Holidays for the stock exchange (data from http://www.borsaitalia.it):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Assumption, August 15th</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>St. Stephen, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars

        \test the correctness of the returned results is tested against a
              list of known holidays.
    */
    class Italy : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Italian settlement"; }
            bool isBusinessDay(const Date&) const;
        };
        class ExchangeImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Milan stock exchange"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        //! Italian calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      Exchange        //!< Milan stock-exchange calendar
        };
        Italy(Market market = Settlement);
    };
	
	Italy::Italy(Italy::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                                   new Italy::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> exchangeImpl(
                                                   new Italy::ExchangeImpl);
        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case Exchange:
            impl_ = exchangeImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

	bool Italy::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Epiphany
            || (d == 6 && m == January)
            // Easter Monday
            || (dd == em)
            // Liberation Day
            || (d == 25 && m == April)
            // Labour Day
            || (d == 1 && m == May)
            // Republic Day
            || (d == 2 && m == June && y >= 2000)
            // Assumption
            || (d == 15 && m == August)
            // All Saints' Day
            || (d == 1 && m == November)
            // Immaculate Conception
            || (d == 8 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // St. Stephen
            || (d == 26 && m == December)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false;
        return true;
    }
	
	bool Italy::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Assumption
            || (d == 15 && m == August)
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // St. Stephen
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }
	
}




#endif
