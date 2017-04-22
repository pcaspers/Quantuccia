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

/*! \file unitedkingdom.hpp
    \brief UK calendars
*/

#ifndef quantlib_united_kindgom_calendar_hpp
#define quantlib_united_kindgom_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! United Kingdom calendars
    /*! Public holidays (data from http://www.dti.gov.uk/er/bankhol.htm):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or
            Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Holidays for the stock exchange:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or
            Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        Holidays for the metals exchange:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st (possibly moved to Monday)</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Early May Bank Holiday, first Monday of May</li>
        <li>Spring Bank Holiday, last Monday of May</li>
        <li>Summer Bank Holiday, last Monday of August</li>
        <li>Christmas Day, December 25th (possibly moved to Monday or
            Tuesday)</li>
        <li>Boxing Day, December 26th (possibly moved to Monday or
            Tuesday)</li>
        </ul>

        \ingroup calendars

        \todo add LIFFE

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class UnitedKingdom : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "UK settlement"; }
            bool isBusinessDay(const Date&) const;
        };
        class ExchangeImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "London stock exchange"; }
            bool isBusinessDay(const Date&) const;
        };
        class MetalsImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "London metals exchange"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        //! UK calendars
        enum Market { Settlement,     //!< generic settlement calendar
                      Exchange,       //!< London stock-exchange calendar
                      Metals          //|< London metals-exchange calendar
        };
        UnitedKingdom(Market market = Settlement);
    };

    // implementation

    inline UnitedKingdom::UnitedKingdom(UnitedKingdom::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                                                new UnitedKingdom::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> exchangeImpl(
                                                              new UnitedKingdom::ExchangeImpl);
        static boost::shared_ptr<Calendar::Impl> metalsImpl(
                                                            new UnitedKingdom::MetalsImpl);
        switch (market) {
        case Settlement:
            impl_ = settlementImpl;
            break;
        case Exchange:
            impl_ = exchangeImpl;
            break;
        case Metals:
            impl_ = metalsImpl;
            break;
        default:
            QL_FAIL("unknown market");
        }
    }

    inline bool UnitedKingdom::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // first Monday of May (Early May Bank Holiday)
            || (d <= 7 && w == Monday && m == May)
            // last Monday of May (Spring Bank Holiday)
            || (d >= 25 && w == Monday && m == May && y != 2002 && y != 2012)
            // last Monday of August (Summer Bank Holiday)
            || (d >= 25 && w == Monday && m == August)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // June 3rd, 2002 only (Golden Jubilee Bank Holiday)
            // June 4rd, 2002 only (special Spring Bank Holiday)
            || ((d == 3 || d == 4) && m == June && y == 2002)
            // April 29th, 2011 only (Royal Wedding Bank Holiday)
            || (d == 29 && m == April && y == 2011)
            // June 4th, 2012 only (Diamond Jubilee Bank Holiday)
            // June 5th, 2012 only (Special Spring Bank Holiday)
            || ((d == 4 || d == 5) && m == June && y == 2012)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false;
        return true;
    }


    inline bool UnitedKingdom::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // first Monday of May (Early May Bank Holiday)
            || (d <= 7 && w == Monday && m == May)
            // last Monday of May (Spring Bank Holiday)
            || (d >= 25 && w == Monday && m == May && y != 2002 && y != 2012)
            // last Monday of August (Summer Bank Holiday)
            || (d >= 25 && w == Monday && m == August)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // June 3rd, 2002 only (Golden Jubilee Bank Holiday)
            // June 4rd, 2002 only (special Spring Bank Holiday)
            || ((d == 3 || d == 4) && m == June && y == 2002)
            // April 29th, 2011 only (Royal Wedding Bank Holiday)
            || (d == 29 && m == April && y == 2011)
            // June 4th, 2012 only (Diamond Jubilee Bank Holiday)
            // June 5th, 2012 only (Special Spring Bank Holiday)
            || ((d == 4 || d == 5) && m == June && y == 2012)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false;
        return true;
    }


    inline bool UnitedKingdom::MetalsImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if (isWeekend(w)
            // New Year's Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // first Monday of May (Early May Bank Holiday)
            || (d <= 7 && w == Monday && m == May)
            // last Monday of May (Spring Bank Holiday)
            || (d >= 25 && w == Monday && m == May && y != 2002 && y != 2012)
            // last Monday of August (Summer Bank Holiday)
            || (d >= 25 && w == Monday && m == August)
            // Christmas (possibly moved to Monday or Tuesday)
            || ((d == 25 || (d == 27 && (w == Monday || w == Tuesday)))
                && m == December)
            // Boxing Day (possibly moved to Monday or Tuesday)
            || ((d == 26 || (d == 28 && (w == Monday || w == Tuesday)))
                && m == December)
            // June 3rd, 2002 only (Golden Jubilee Bank Holiday)
            // June 4rd, 2002 only (special Spring Bank Holiday)
            || ((d == 3 || d == 4) && m == June && y == 2002)
            // April 29th, 2011 only (Royal Wedding Bank Holiday)
            || (d == 29 && m == April && y == 2011)
            // June 4th, 2012 only (Diamond Jubilee Bank Holiday)
            // June 5th, 2012 only (Special Spring Bank Holiday)
            || ((d == 4 || d == 5) && m == June && y == 2012)
            // December 31st, 1999 only
            || (d == 31 && m == December && y == 1999))
            return false;
        return true;
    }

}


#endif
