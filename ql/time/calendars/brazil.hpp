/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Piter Dias
 Copyright (C) 2007 Richard Gomes

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

/*! \file brazil.hpp
    \brief Brazilian calendar
*/

#ifndef quantlib_brazilian_calendar_hpp
#define quantlib_brazilian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Brazilian calendar
    /*! Banking holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Tiradentes's Day, April 21th</li>
        <li>Labour Day, May 1st</li>
        <li>Independence Day, September 7th</li>
        <li>Nossa Sra. Aparecida Day, October 12th</li>
        <li>All Souls Day, November 2nd</li>
        <li>Republic Day, November 15th</li>
        <li>Christmas, December 25th</li>
        <li>Passion of Christ</li>
        <li>Carnival</li>
        <li>Corpus Christi</li>
        </ul>

        Holidays for the Bovespa stock exchange
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Sao Paulo City Day, January 25th</li>
        <li>Tiradentes's Day, April 21th</li>
        <li>Labour Day, May 1st</li>
        <li>Revolution Day, July 9th</li>
        <li>Independence Day, September 7th</li>
        <li>Nossa Sra. Aparecida Day, October 12th</li>
        <li>All Souls Day, November 2nd</li>
        <li>Republic Day, November 15th</li>
        <li>Black Consciousness Day, November 20th (since 2007)</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Passion of Christ</li>
        <li>Carnival</li>
        <li>Corpus Christi</li>
        <li>the last business day of the year</li>
        </ul>

        \ingroup calendars

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class Brazil : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Brazil"; }
            bool isBusinessDay(const Date&) const;
        };
        class ExchangeImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "BOVESPA"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        //! Brazilian calendars
        enum Market { Settlement,            //!< generic settlement calendar
                      Exchange               //!< BOVESPA calendar
        };
        Brazil(Market market = Settlement);
    };
	
	inline Brazil::Brazil(Brazil::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                                  new Brazil::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> exchangeImpl(
                                                    new Brazil::ExchangeImpl);
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
	
	inline bool Brazil::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Tiradentes Day
            || (d == 21 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Independence Day
            || (d == 7 && m == September)
            // Nossa Sra. Aparecida Day
            || (d == 12 && m == October)
            // All Souls Day
            || (d == 2 && m == November)
            // Republic Day
            || (d == 15 && m == November)
            // Christmas
            || (d == 25 && m == December)
            // Passion of Christ
            || (dd == em-3)
            // Carnival
            || (dd == em-49 || dd == em-48)
            // Corpus Christi
            || (dd == em+59)
            )
            return false;
        return true;
    }
	
	inline bool Brazil::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        Day dd = date.dayOfYear();
        Day em = easterMonday(y);

        if (isWeekend(w)
            // New Year's Day
            || (d == 1 && m == January)
            // Sao Paulo City Day
            || (d == 25 && m == January)
            // Tiradentes Day
            || (d == 21 && m == April)
            // Labor Day
            || (d == 1 && m == May)
            // Revolution Day
            || (d == 9 && m == July)
            // Independence Day
            || (d == 7 && m == September)
            // Nossa Sra. Aparecida Day
            || (d == 12 && m == October)
            // All Souls Day
            || (d == 2 && m == November)
            // Republic Day
            || (d == 15 && m == November)
            // Black Consciousness Day
            || (d == 20 && m == November && y >= 2007)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Passion of Christ
            || (dd == em-3)
            // Carnival
            || (dd == em-49 || dd == em-48)
            // Corpus Christi
            || (dd == em+59)
            // last business day of the year
            || (m == December && (d == 31 || (d >= 29 && w == Friday)))
            )
            return false;
        return true;
    }

}


#endif

