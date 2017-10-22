/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009 StatPro Italia srl
 Copyright (C) 2006, 2011 Ferdinando Ametrano

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

/*! \file interestrateindex.hpp
    \brief base class for interest rate indexes
*/

#ifndef quantlib_interestrateindex_hpp
#define quantlib_interestrateindex_hpp

#include <ql/index.hpp>
#include <ql/time/calendar.hpp>
#include <ql/currency.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    //! base class for interest rate indexes
    /*! \todo add methods returning InterestRate */
    class InterestRateIndex : public Index,
                              public Observer {
      public:
        InterestRateIndex(const std::string& familyName,
                          const Period& tenor,
                          Natural settlementDays,
                          const Currency& currency,
                          const Calendar& fixingCalendar,
                          const DayCounter& dayCounter);
        //! \name Index interface
        //@{
        std::string name() const;
        Calendar fixingCalendar() const;
        bool isValidFixingDate(const Date& fixingDate) const;
        Rate fixing(const Date& fixingDate,
                    bool forecastTodaysFixing = false) const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        std::string familyName() const { return familyName_; }
        Period tenor() const { return tenor_; }
        Natural fixingDays() const { return fixingDays_; }
        Date fixingDate(const Date& valueDate) const;
        const Currency& currency() const { return currency_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        //@}
        /*! \name Date calculations

            These method can be overridden to implement particular
            conventions (e.g. EurLibor)

            @{
        */
        virtual Date valueDate(const Date& fixingDate) const;
        virtual Date maturityDate(const Date& valueDate) const = 0;
        //@}
        //! \name Fixing calculations
        //@{
        //! It can be overridden to implement particular conventions
        virtual Rate forecastFixing(const Date& fixingDate) const = 0;
        virtual Rate pastFixing(const Date& fixingDate) const;
        // @}
      protected:
        std::string familyName_;
        Period tenor_;
        Natural fixingDays_;
        Currency currency_;
        DayCounter dayCounter_;
        std::string name_;
      private:
        Calendar fixingCalendar_;
    };


    // inline definitions

    inline std::string InterestRateIndex::name() const {
        return name_;
    }

    inline Calendar InterestRateIndex::fixingCalendar() const {
        return fixingCalendar_;
    }

    inline bool InterestRateIndex::isValidFixingDate(const Date& d) const {
        return fixingCalendar().isBusinessDay(d);
    }

    inline void InterestRateIndex::update() {
        notifyObservers();
    }

    inline Date InterestRateIndex::fixingDate(const Date& valueDate) const {
        Date fixingDate = fixingCalendar().advance(valueDate,
            -static_cast<Integer>(fixingDays_), Days);
        return fixingDate;
    }

    inline Date InterestRateIndex::valueDate(const Date& fixingDate) const {
        QL_REQUIRE(isValidFixingDate(fixingDate),
                   fixingDate << " is not a valid fixing date");
        return fixingCalendar().advance(fixingDate, fixingDays_, Days);
    }

    inline Rate InterestRateIndex::pastFixing(const Date& fixingDate) const {
        QL_REQUIRE(isValidFixingDate(fixingDate),
                   fixingDate << " is not a valid fixing date");
        return timeSeries()[fixingDate];
    }

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2011 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

#include <ql/settings.hpp>

#include <sstream>

namespace QuantLib {

    InterestRateIndex::InterestRateIndex(const std::string& familyName,
                                         const Period& tenor,
                                         Natural fixingDays,
                                         const Currency& currency,
                                         const Calendar& fixingCalendar,
                                         const DayCounter& dayCounter)
    : familyName_(familyName), tenor_(tenor), fixingDays_(fixingDays),
      currency_(currency), dayCounter_(dayCounter),
      fixingCalendar_(fixingCalendar) {
        tenor_.normalize();

        std::ostringstream out;
        out << familyName_;
        if (tenor_ == 1*Days) {
            if (fixingDays_==0)
                out << "ON";
            else if (fixingDays_==1)
                out << "TN";
            else if (fixingDays_==2)
                out << "SN";
            else
                out << io::short_period(tenor_);
        } else {
            out << io::short_period(tenor_);
        }
        out << " " << dayCounter_.name();
        name_ = out.str();

        registerWith(Settings::instance().evaluationDate());
        registerWith(IndexManager::instance().notifier(name()));
    }

    Rate InterestRateIndex::fixing(const Date& fixingDate,
                                   bool forecastTodaysFixing) const {

        QL_REQUIRE(isValidFixingDate(fixingDate),
                   "Fixing date " << fixingDate << " is not valid");

        Date today = Settings::instance().evaluationDate();

        if (fixingDate>today ||
            (fixingDate==today && forecastTodaysFixing))
            return forecastFixing(fixingDate);

        if (fixingDate<today ||
            Settings::instance().enforcesTodaysHistoricFixings()) {
            // must have been fixed
            // do not catch exceptions
            Rate result = pastFixing(fixingDate);
            QL_REQUIRE(result != Null<Real>(),
                       "Missing " << name() << " fixing for " << fixingDate);
            return result;
        }

        try {
            // might have been fixed
            Rate result = pastFixing(fixingDate);
            if (result!=Null<Real>())
                return result;
            else
                ;   // fall through and forecast
        } catch (Error&) {
                ;   // fall through and forecast
        }
        return forecastFixing(fixingDate);
    }

}

#endif