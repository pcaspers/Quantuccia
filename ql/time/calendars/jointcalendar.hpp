/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file jointcalendar.hpp
    \brief Joint calendar
*/

#ifndef quantlib_joint_calendar_h
#define quantlib_joint_calendar_h

#include <ql/time/calendar.hpp>
#include <ql/errors.hpp>
#include <sstream>

namespace QuantLib {

    //! rules for joining calendars
    enum JointCalendarRule { JoinHolidays,    /*!< A date is a holiday
                                                   for the joint calendar
                                                   if it is a holiday
                                                   for any of the given
                                                   calendars */
                             JoinBusinessDays /*!< A date is a business day
                                                   for the joint calendar
                                                   if it is a business day
                                                   for any of the given
                                                   calendars */
    };

    //! Joint calendar
    /*! Depending on the chosen rule, this calendar has a set of
        business days given by either the union or the intersection
        of the sets of business days of the given calendars.

        \ingroup calendars

        \test the correctness of the returned results is tested by
              reproducing the calculations.
    */
    class JointCalendar : public Calendar {
      private:
        class Impl : public Calendar::Impl {
          public:
            Impl(const Calendar&, const Calendar&,
                 JointCalendarRule);
            Impl(const Calendar&, const Calendar&,
                 const Calendar&, JointCalendarRule);
            Impl(const Calendar&, const Calendar&,
                 const Calendar&, const Calendar&,
                 JointCalendarRule);
            std::string name() const;
            bool isWeekend(Weekday) const;
            bool isBusinessDay(const Date&) const;
          private:
            JointCalendarRule rule_;
            std::vector<Calendar> calendars_;
        };
      public:
        JointCalendar(const Calendar&, const Calendar&,
                      JointCalendarRule = JoinHolidays);
        JointCalendar(const Calendar&, const Calendar&,
                      const Calendar&,
                      JointCalendarRule = JoinHolidays);
        JointCalendar(const Calendar&, const Calendar&,
                      const Calendar&, const Calendar&,
                      JointCalendarRule = JoinHolidays);
    };

    // implementation

    inline JointCalendar::Impl::Impl(const Calendar& c1,
                              const Calendar& c2,
                              JointCalendarRule r)
    : rule_(r), calendars_(2) {
        calendars_[0] = c1;
        calendars_[1] = c2;
    }

    inline JointCalendar::Impl::Impl(const Calendar& c1,
                              const Calendar& c2,
                              const Calendar& c3,
                              JointCalendarRule r)
    : rule_(r), calendars_(3) {
        calendars_[0] = c1;
        calendars_[1] = c2;
        calendars_[2] = c3;
    }

    inline JointCalendar::Impl::Impl(const Calendar& c1,
                              const Calendar& c2,
                              const Calendar& c3,
                              const Calendar& c4,
                              JointCalendarRule r)
    : rule_(r), calendars_(4) {
        calendars_[0] = c1;
        calendars_[1] = c2;
        calendars_[2] = c3;
        calendars_[3] = c4;
    }

    inline std::string JointCalendar::Impl::name() const {
        std::ostringstream out;
        switch (rule_) {
          case JoinHolidays:
            out << "JoinHolidays(";
            break;
          case JoinBusinessDays:
            out << "JoinBusinessDays(";
            break;
          default:
            QL_FAIL("unknown joint calendar rule");
        }
        out << calendars_.front().name();
        std::vector<Calendar>::const_iterator i;
        for (i=calendars_.begin()+1; i!=calendars_.end(); ++i)
            out << ", " << i->name();
        out << ")";
        return out.str();
    }

    inline bool JointCalendar::Impl::isWeekend(Weekday w) const {
        std::vector<Calendar>::const_iterator i;
        switch (rule_) {
          case JoinHolidays:
            for (i=calendars_.begin(); i!=calendars_.end(); ++i) {
                if (i->isWeekend(w))
                    return true;
            }
            return false;
          case JoinBusinessDays:
            for (i=calendars_.begin(); i!=calendars_.end(); ++i) {
                if (!i->isWeekend(w))
                    return false;
            }
            return true;
          default:
            QL_FAIL("unknown joint calendar rule");
        }
    }

    inline bool JointCalendar::Impl::isBusinessDay(const Date& date) const {
        std::vector<Calendar>::const_iterator i;
        switch (rule_) {
          case JoinHolidays:
            for (i=calendars_.begin(); i!=calendars_.end(); ++i) {
                if (i->isHoliday(date))
                    return false;
            }
            return true;
          case JoinBusinessDays:
            for (i=calendars_.begin(); i!=calendars_.end(); ++i) {
                if (i->isBusinessDay(date))
                    return true;
            }
            return false;
          default:
            QL_FAIL("unknown joint calendar rule");
        }
    }


    inline JointCalendar::JointCalendar(const Calendar& c1,
                                 const Calendar& c2,
                                 JointCalendarRule r) {
        impl_ = boost::shared_ptr<Calendar::Impl>(
                                            new JointCalendar::Impl(c1,c2,r));
    }

    inline JointCalendar::JointCalendar(const Calendar& c1,
                                 const Calendar& c2,
                                 const Calendar& c3,
                                 JointCalendarRule r) {
        impl_ = boost::shared_ptr<Calendar::Impl>(
                                         new JointCalendar::Impl(c1,c2,c3,r));
    }

    inline JointCalendar::JointCalendar(const Calendar& c1,
                                 const Calendar& c2,
                                 const Calendar& c3,
                                 const Calendar& c4,
                                 JointCalendarRule r) {
        impl_ = boost::shared_ptr<Calendar::Impl>(
                                      new JointCalendar::Impl(c1,c2,c3,c4,r));
    }


}


#endif
