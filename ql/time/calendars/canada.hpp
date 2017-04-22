/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file canada.hpp
    \brief Canadian calendar
*/

#ifndef quantlib_canadian_calendar_hpp
#define quantlib_canadian_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Canadian calendar
    /*! Banking holidays
        (data from <http://www.bankofcanada.ca/en/about/holiday.html>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Family Day, third Monday of February (since 2008)</li>
        <li>Good Friday</li>
        <li>Victoria Day, the Monday on or preceding May 24th</li>
        <li>Canada Day, July 1st (possibly moved to Monday)</li>
        <li>Provincial Holiday, first Monday of August</li>
        <li>Labour Day, first Monday of September</li>
        <li>Thanksgiving Day, second Monday of October</li>
        <li>Remembrance Day, November 11th (possibly moved to Monday)</li>
        <li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Holidays for the Toronto stock exchange
        (data from <http://www.tsx.com/en/about_tsx/market_hours.html>):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Family Day, third Monday of February (since 2008)</li>
        <li>Good Friday</li>
        <li>Victoria Day, the Monday on or preceding May 24th</li>
        <li>Canada Day, July 1st (possibly moved to Monday)</li>
        <li>Provincial Holiday, first Monday of August</li>
        <li>Labour Day, first Monday of September</li>
        <li>Thanksgiving Day, second Monday of October</li>
        <li>Christmas, December 25th (possibly moved to Monday or Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        \ingroup calendars
    */
    class Canada : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Canada"; }
            bool isBusinessDay(const Date&) const;
        };
        class TsxImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "TSX"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        enum Market { Settlement,       //!< generic settlement calendar
                      TSX               //!< Toronto stock exchange calendar
        };
        Canada(Market market = Settlement);
    };

    inline Canada::Canada(Canada::Market market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                                  new Canada::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> tsxImpl(new Canada::TsxImpl);
        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case TSX:
            impl_ = tsxImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    inline bool Canada::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // Family Day (third Monday in February, since 2008)
            || ((d >= 15 && d <= 21) && w == Monday && m == February
                && y >= 2008)
            // Good Friday
            || (dd == em-3)
            // The Monday on or preceding 24 May (Victoria Day)
            || (d > 17 && d <= 24 && w == Monday && m == May)
            // July 1st, possibly moved to Monday (Canada Day)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) && m==July)
            // first Monday of August (Provincial Holiday)
            || (d <= 7 && w == Monday && m == August)
            // first Monday of September (Labor Day)
            || (d <= 7 && w == Monday && m == September)
            // second Monday of October (Thanksgiving Day)
            || (d > 7 && d <= 14 && w == Monday && m == October)
            // November 11th (possibly moved to Monday)
            || ((d == 11 || ((d == 12 || d == 13) && w == Monday))
                && m == November)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            )
            return false;
        return true;
    }

    inline bool Canada::TsxImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || (d == 2 && w == Monday)) && m == January)
            // Family Day (third Monday in February, since 2008)
            || ((d >= 15 && d <= 21) && w == Monday && m == February
                && y >= 2008)
            // Good Friday
            || (dd == em-3)
            // The Monday on or preceding 24 May (Victoria Day)
            || (d > 17 && d <= 24 && w == Monday && m == May)
            // July 1st, possibly moved to Monday (Canada Day)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) && m==July)
            // first Monday of August (Provincial Holiday)
            || (d <= 7 && w == Monday && m == August)
            // first Monday of September (Labor Day)
            || (d <= 7 && w == Monday && m == September)
            // second Monday of October (Thanksgiving Day)
            || (d > 7 && d <= 14 && w == Monday && m == October)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            )
            return false;
        return true;
    }

}


#endif
