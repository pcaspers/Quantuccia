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

/*! \file japan.hpp
    \brief Japanese calendar
*/

#ifndef quantlib_japanese_calendar_hpp
#define quantlib_japanese_calendar_hpp

#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! Japanese calendar
    /*! Holidays:
        <ul>
        <li>Saturdays</li>
        <li>Sundays</li>
        <li>New Year's Day, January 1st</li>
        <li>Bank Holiday, January 2nd</li>
        <li>Bank Holiday, January 3rd</li>
        <li>Coming of Age Day, 2nd Monday in January</li>
        <li>National Foundation Day, February 11th</li>
        <li>Vernal Equinox</li>
        <li>Greenery Day, April 29th</li>
        <li>Constitution Memorial Day, May 3rd</li>
        <li>Holiday for a Nation, May 4th</li>
        <li>Children's Day, May 5th</li>
        <li>Marine Day, 3rd Monday in July</li>
        <li>Mountain Day, August 11th (from 2016 onwards)</li>
        <li>Respect for the Aged Day, 3rd Monday in September</li>
        <li>Autumnal Equinox</li>
        <li>Health and Sports Day, 2nd Monday in October</li>
        <li>National Culture Day, November 3rd</li>
        <li>Labor Thanksgiving Day, November 23rd</li>
        <li>Emperor's Birthday, December 23rd</li>
        <li>Bank Holiday, December 31st</li>
        <li>a few one-shot holidays</li>
        </ul>
        Holidays falling on a Sunday are observed on the Monday following
        except for the bank holidays associated with the new year.

        \ingroup calendars
    */
    class Japan : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            std::string name() const { return "Japan"; }
            bool isWeekend(Weekday) const;
            bool isBusinessDay(const Date&) const;
        };
      public:
        Japan();
    };

    inline Japan::Japan() {
        // all calendar instances share the same implementation instance
        static boost::shared_ptr<Calendar::Impl> impl(new Japan::Impl);
        impl_ = impl;
    }

    inline bool Japan::Impl::isWeekend(Weekday w) const {
        return w == Saturday || w == Sunday;
    }

    inline bool Japan::Impl::isBusinessDay(const Date& date) const {
        Weekday w = date.weekday();
        Day d = date.dayOfMonth();
        Month m = date.month();
        Year y = date.year();
        // equinox calculation
        const Time exact_vernal_equinox_time = 20.69115;
        const Time exact_autumnal_equinox_time = 23.09;
        const Time diff_per_year = 0.242194;
        const Time moving_amount = (y-2000)*diff_per_year;
        Integer number_of_leap_years = (y-2000)/4+(y-2000)/100-(y-2000)/400;
        Day ve =    // vernal equinox day
            Day(exact_vernal_equinox_time
                + moving_amount - number_of_leap_years);
        Day ae =    // autumnal equinox day
            Day(exact_autumnal_equinox_time
                + moving_amount - number_of_leap_years);
        // checks
        if (isWeekend(w)
            // New Year's Day
            || (d == 1  && m == January)
            // Bank Holiday
            || (d == 2  && m == January)
            // Bank Holiday
            || (d == 3  && m == January)
            // Coming of Age Day (2nd Monday in January),
            // was January 15th until 2000
            || (w == Monday && (d >= 8 && d <= 14) && m == January
                && y >= 2000)
            || ((d == 15 || (d == 16 && w == Monday)) && m == January
                && y < 2000)
            // National Foundation Day
            || ((d == 11 || (d == 12 && w == Monday)) && m == February)
            // Vernal Equinox
            || ((d == ve || (d == ve+1 && w == Monday)) && m == March)
            // Greenery Day
            || ((d == 29 || (d == 30 && w == Monday)) && m == April)
            // Constitution Memorial Day
            || (d == 3  && m == May)
            // Holiday for a Nation
            || (d == 4  && m == May)
            // Children's Day
            || (d == 5  && m == May)
            // any of the three above observed later if on Saturday or Sunday
            || (d == 6 && m == May
                && (w == Monday || w == Tuesday || w == Wednesday))
            // Marine Day (3rd Monday in July),
            // was July 20th until 2003, not a holiday before 1996
            || (w == Monday && (d >= 15 && d <= 21) && m == July
                && y >= 2003)
            || ((d == 20 || (d == 21 && w == Monday)) && m == July
                && y >= 1996 && y < 2003)
            // Mountain Day (from 2016)
            || ((d == 11 || (d == 12 && w == Monday)) && m == August
                && y >= 2016)
            // Respect for the Aged Day (3rd Monday in September),
            // was September 15th until 2003
            || (w == Monday && (d >= 15 && d <= 21) && m == September
                && y >= 2003)
            || ((d == 15 || (d == 16 && w == Monday)) && m == September
                && y < 2003)
            // If a single day falls between Respect for the Aged Day
            // and the Autumnal Equinox, it is holiday
            || (w == Tuesday && d+1 == ae && d >= 16 && d <= 22
                && m == September && y >= 2003)
            // Autumnal Equinox
            || ((d == ae || (d == ae+1 && w == Monday)) && m == September)
            // Health and Sports Day (2nd Monday in October),
            // was October 10th until 2000
            || (w == Monday && (d >= 8 && d <= 14) && m == October
                && y >= 2000)
            || ((d == 10 || (d == 11 && w == Monday)) && m == October
                && y < 2000)
            // National Culture Day
            || ((d == 3  || (d == 4 && w == Monday)) && m == November)
            // Labor Thanksgiving Day
            || ((d == 23 || (d == 24 && w == Monday)) && m == November)
            // Emperor's Birthday
            || ((d == 23 || (d == 24 && w == Monday)) && m == December
                && y >= 1989)
            // Bank Holiday
            || (d == 31 && m == December)
            // one-shot holidays
            // Marriage of Prince Akihito
            || (d == 10 && m == April && y == 1959)
            // Rites of Imperial Funeral
            || (d == 24 && m == February && y == 1989)
            // Enthronement Ceremony
            || (d == 12 && m == November && y == 1990)
            // Marriage of Prince Naruhito
            || (d == 9 && m == June && y == 1993))
            return false;
        return true;
    }

}


#endif
