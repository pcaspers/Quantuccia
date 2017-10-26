/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2005 Joseph Wang

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

/*! \file event.hpp
    \brief Base class for events associated with a given date
*/

#ifndef quantlib_event_hpp
#define quantlib_event_hpp

#include <ql/time/date.hpp>
#include <ql/patterns/observable.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    class AcyclicVisitor;

    //! Base class for event
    /*! This class acts as a base class for the actual
        event implementations.
    */
    class Event : public Observable {
      public:
        virtual ~Event() {}
        //! \name Event interface
        //@{
        //! returns the date at which the event occurs
        virtual Date date() const = 0;

        //! returns true if an event has already occurred before a date
        /*! If includeRefDate is true, then an event has not occurred if its
            date is the same as the refDate, i.e. this method returns false if
            the event date is the same as the refDate.
        */
        virtual bool hasOccurred(
                    const Date& refDate = Date(),
                    boost::optional<bool> includeRefDate = boost::none) const;
        //@}

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    namespace detail {

        // used to create an Event instance.
        // to be replaced with specific events as soon as we find out which.
        class simple_event : public Event {
          public:
            explicit simple_event(const Date& date) : date_(date) {}
            Date date() const { return date_; }
          private:
            Date date_;
        };

    }

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2005 Joseph Wang

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

#include <ql/patterns/visitor.hpp>
#include <ql/settings.hpp>

namespace QuantLib {

    bool Event::hasOccurred(const Date& d, // refDate
                            boost::optional<bool> includeRefDate) const {
        Date refDate =
            d != Date() ? d : Settings::instance().evaluationDate();
        bool includeRefDateEvent =
            includeRefDate ? *includeRefDate :
                           Settings::instance().includeReferenceDateEvents();
        if (includeRefDateEvent)
            return date() < refDate;
        else
            return date() <= refDate;
    }

    void Event::accept(AcyclicVisitor& v) {
        Visitor<Event>* v1 = dynamic_cast<Visitor<Event>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not an event visitor");
    }

}

#endif