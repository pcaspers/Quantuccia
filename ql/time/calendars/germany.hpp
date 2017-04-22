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

/*! \file germany.hpp
    \brief German calendars
*/

#ifndef quantlib_germany_calendar_hpp
#define quantlib_germany_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! German calendars
    /*! Public holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Ascension Thursday</li>
        <li>Whit Monday</li>
        <li>Corpus Christi</li>
        <li>Labour Day, May 1st</li>
        <li>National Day, October 3rd</li>
        <li>Christmas Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Boxing Day, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Frankfurt Stock exchange
        (data from http://deutsche-boerse.com/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Xetra exchange
        (data from http://deutsche-boerse.com/):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Eurex exchange
        (data from http://www.eurexchange.com/index.html):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        Holidays for the Euwax exchange
        (data from http://www.boerse-stuttgart.de):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Good Friday</li>
        <li>Easter Monday</li>
        <li>Labour Day, May 1st</li>
        <li>Whit Monday</li>
        <li>Christmas' Eve, December 24th</li>
        <li>Christmas, December 25th</li>
        <li>Christmas Holiday, December 26th</li>
        <li>New Year's Eve, December 31st</li>
        </ul>

        \ingroup calendars

        \test the correctness of the returned results is tested
              against a list of known holidays.
    */
    class Germany : public Calendar {
      private:
        class SettlementImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "German settlement"; }
            bool isBusinessDay(const Date&) const;
        };
        class FrankfurtStockExchangeImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Frankfurt stock exchange"; }
            bool isBusinessDay(const Date&) const;
        };
        class XetraImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Xetra"; }
            bool isBusinessDay(const Date&) const;
        };
        class EurexImpl : public Calendar::WesternImpl {
          public:
            std::string name() const { return "Eurex"; }
            bool isBusinessDay(const Date&) const;
        };
        class EuwaxImpl : public Calendar::WesternImpl {
        public:
            std::string name() const { return "Euwax"; }
            bool isBusinessDay(const Date&) const;
        };

      public:
        //! German calendars
        enum Market { Settlement,             //!< generic settlement calendar
                      FrankfurtStockExchange, //!< Frankfurt stock-exchange
                      Xetra,                  //!< Xetra
                      Eurex,                  //!< Eurex
                      Euwax                   //!< Euwax
        };
        Germany(Market market = FrankfurtStockExchange);
    };

    inline Germany::Germany(Germany::Market market) {
        // all calendar instances on the same market share the same
        // implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
            new Germany::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> frankfurtStockExchangeImpl(
            new Germany::FrankfurtStockExchangeImpl);
        static boost::shared_ptr<Calendar::Impl> xetraImpl(
            new Germany::XetraImpl);
        static boost::shared_ptr<Calendar::Impl> eurexImpl(
            new Germany::EurexImpl);
        static boost::shared_ptr<Calendar::Impl> euwaxImpl(
            new Germany::EuwaxImpl);

        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case FrankfurtStockExchange:
            impl_ = frankfurtStockExchangeImpl;
            break;
          case Xetra:
            impl_ = xetraImpl;
            break;
          case Eurex:
            impl_ = eurexImpl;
            break;
          case Euwax:
            impl_ = euwaxImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }


    inline bool Germany::SettlementImpl::isBusinessDay(const Date& date) const {
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
            // Ascension Thursday
            || (dd == em+38)
            // Whit Monday
            || (dd == em+49)
            // Corpus Christi
            || (dd == em+59)
            // Labour Day
            || (d == 1 && m == May)
            // National Day
            || (d == 3 && m == October)
            // Christmas Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Boxing Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }

    inline bool Germany::FrankfurtStockExchangeImpl::isBusinessDay(
      const Date& date) const {
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
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }

    inline bool Germany::XetraImpl::isBusinessDay(const Date& date) const {
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
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }

    inline bool Germany::EurexImpl::isBusinessDay(const Date& date) const {
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
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }

    inline bool Germany::EuwaxImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth(), dd = date.dayOfYear();
        Month m = date.month();
        Year y = date.year();
        Day em = easterMonday(y);
        if ((w == Saturday || w == Sunday)
            // New Year's Day
            || (d == 1 && m == January)
            // Good Friday
            || (dd == em-3)
            // Easter Monday
            || (dd == em)
            // Labour Day
            || (d == 1 && m == May)
            // Whit Monday
            || (dd == em+49)
            // Christmas' Eve
            || (d == 24 && m == December)
            // Christmas
            || (d == 25 && m == December)
            // Christmas Day
            || (d == 26 && m == December)
            // New Year's Eve
            || (d == 31 && m == December))
            return false;
        return true;
    }

}


#endif
