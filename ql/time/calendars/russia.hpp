/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 StatPro Italia srl

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

/*! \file russia.hpp
    \brief Russian calendar
*/

#ifndef quantlib_russia_calendar_hpp
#define quantlib_russia_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Russian calendars
    /*! Public holidays (see <http://www.cbr.ru/eng/>:):
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year holidays and Christmas, January 1st to 8th</li>
        <li>Defender of the Fatherland Day, February 23rd (possibly
            moved to Monday)</li>
        <li>International Women's Day, March 8th (possibly moved to
            Monday)</li>
        <li>Labour Day, May 1st (possibly moved to Monday)</li>
        <li>Victory Day, May 9th (possibly moved to Monday)</li>
        <li>Russia Day, June 12th (possibly moved to Monday)</li>
        <li>Unity Day, November 4th (possibly moved to Monday)</li>
        </ul>

        Holidays for the Moscow Exchange (MOEX) taken from
        <http://moex.com/s726> and related pages.  These holidays are
        <em>not</em> consistent year-to-year, may or may not correlate
        to public holidays, and are only available for dates since the
        introduction of the MOEX 'brand' (a merger of the stock and
        futures markets).

        \ingroup calendars
    */
    class Russia : public Calendar {
      private:
        class SettlementImpl : public Calendar::OrthodoxImpl {
          public:
            std::string name() const { return "Russian settlement"; }
            bool isBusinessDay(const Date&) const;
        };
        class ExchangeImpl : public Calendar::OrthodoxImpl {
          public:
            std::string name() const { return "Moscow exchange"; }
            bool isBusinessDay(const Date&) const;
        };
      public:
        //! Russian calendars
        enum Market { Settlement, //!< generic settlement calendar
                      MOEX        //!< Moscow Exchange calendar
        };
        Russia(Market = Settlement);
    };
	
	inline Russia::Russia(Russia::Market market) {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> settlementImpl(
                                                  new Russia::SettlementImpl);
        static boost::shared_ptr<Calendar::Impl> exchangeImpl(
                                                    new Russia::ExchangeImpl);

        switch (market) {
          case Settlement:
            impl_ = settlementImpl;
            break;
          case MOEX:
            impl_ = exchangeImpl;
            break;
          default:
            QL_FAIL("unknown market");
        }
    }

    inline bool Russia::SettlementImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        if (isWeekend(w)
            // New Year's holidays
            || (d >= 1 && d <= 8 && m == January)
            // Defender of the Fatherland Day (possibly moved to Monday)
            || ((d == 23 || ((d == 24 || d == 25) && w == Monday)) &&
                m == February)
            // International Women's Day (possibly moved to Monday)
            || ((d == 8 || ((d == 9 || d == 10) && w == Monday)) &&
                m == March)
            // Labour Day (possibly moved to Monday)
            || ((d == 1 || ((d == 2 || d == 3) && w == Monday)) &&
                m == May)
            // Victory Day (possibly moved to Monday)
            || ((d == 9 || ((d == 10 || d == 11) && w == Monday)) &&
                m == May)
            // Russia Day (possibly moved to Monday)
            || ((d == 12 || ((d == 13 || d == 14) && w == Monday)) &&
                m == June)
            // Unity Day (possibly moved to Monday)
            || ((d == 4 || ((d == 5 || d == 6) && w == Monday)) &&
                m == November))
            return false;
        return true;
    }
	
	namespace {

        bool isWorkingWeekend(Day d, Month month, Year year) {
            switch (year) {
              case 2012:
                switch (month) {
                  case March: return d == 11;
                  case April: return d == 28;
                  case May:   return d == 5 || d == 12;
                  case June:  return d == 9;
                  default:    return false;
                }
              case 2016:
                switch (month)
                {
                case February: return d == 20;
                default: return false;
                }
              default:
                return false;
            }
        }

        bool isExtraHoliday(Day d, Month month, Year year) {
            switch (year) {
              case 2012:
                switch (month) {
                  case January: return d == 2;
                  case March:   return d == 9;
                  case April:   return d == 30;
                  case June:    return d == 11;
                  default:      return false;
                }
              case 2013:
                switch (month) {
                  case January: return d == 1 || d == 2 || d == 3
                                    || d == 4 || d == 7;
                  default:      return false;
                }
              case 2014:
                switch (month) {
                  case January: return d == 1 || d == 2 || d == 3 || d == 7;
                  default:      return false;
                }
              case 2015:
                switch (month) {
                  case January: return d == 1 || d == 2 || d == 7;
                  default:      return false;
                }
              case 2016:
                switch (month)
                {
                case January: return d == 1 || d == 7 || d == 8;
                case May:     return d == 2 || d == 3;
                case June:    return d == 13;
                case December: return d == 30;
                default:      return false;
                }
              default:
                return false;
            }
        }

    }

    inline bool Russia::ExchangeImpl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();

        // the exchange was formally established in 2011, so data are only
        // available from 2012 to present
        if (y < 2012)
            QL_FAIL("MOEX calendar for the year " << y
                    << " does not exist.");

        if (isWorkingWeekend(d,m,y))
            return true;

        // Known holidays
        if (isWeekend(w)
            // Defender of the Fatherland Day
            || (d == 23 && m == February)
            // International Women's Day (possibly moved to Monday)
            || ((d == 8 || ((d == 9 || d == 10) && w == Monday)) && m == March)
            // Labour Day
            || (d == 1 && m == May)
            // Victory Day (possibly moved to Monday)
            || ((d == 9 || ((d == 10 || d == 11) && w == Monday)) && m == May)
            // Russia Day
            || (d == 12 && m == June)
            // Unity Day (possibly moved to Monday)
            || ((d == 4 || ((d == 5 || d == 6) && w == Monday))
                && m == November)
            // New Years Eve
            || (d == 31 && m == December))
            return false;

        if (isExtraHoliday(d,m,y))
            return false;

        return true;
    }

}


#endif
