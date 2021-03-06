/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file compositeinstrument.hpp
    \brief Composite instrument class
*/

#ifndef quantlib_composite_instrument_hpp
#define quantlib_composite_instrument_hpp

#include <ql/instrument.hpp>
#include <list>
#include <utility>

namespace QuantLib {

    //! %Composite instrument
    /*! This instrument is an aggregate of other instruments. Its NPV
        is the sum of the NPVs of its components, each possibly
        multiplied by a given factor.

        <b>Example: </b>
        \link Replication.cpp
        static replication of a down-and-out barrier option
        \endlink

        \warning Methods that drive the calculation directly (such as
                 recalculate(), freeze() and others) might not work
                 correctly.

        \ingroup instruments
    */
    class CompositeInstrument : public Instrument {
        typedef std::pair<boost::shared_ptr<Instrument>, Real> component;
        typedef std::list<component>::iterator iterator;
        typedef std::list<component>::const_iterator const_iterator;
      public:
        //! adds an instrument to the composite
        void add(const boost::shared_ptr<Instrument>& instrument,
                 Real multiplier = 1.0);
        //! shorts an instrument from the composite
        void subtract(const boost::shared_ptr<Instrument>& instrument,
                      Real multiplier = 1.0);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
      protected:
        void performCalculations() const;
        //@}
      private:
        std::list<component> components_;
    };

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 StatPro Italia srl

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


namespace QuantLib {

    inline void CompositeInstrument::add(
           const boost::shared_ptr<Instrument>& instrument, Real multiplier) {
        components_.push_back(std::make_pair(instrument,multiplier));
        registerWith(instrument);
        update();
        // When we ask for the NPV of an expired composite, the
        // components are not recalculated and thus wouldn't forward
        // later notifications according to the default behavior of
        // LazyObject instances.  This means that even if the
        // evaluation date changes so that the composite is no longer
        // expired, the instrument wouldn't be notified and thus it
        // wouldn't recalculate.  To avoid this, we override the
        // default behavior of the components.
        instrument->alwaysForwardNotifications();
    }

    inline void CompositeInstrument::subtract(
           const boost::shared_ptr<Instrument>& instrument, Real multiplier) {
        add(instrument, -multiplier);
    }

    inline bool CompositeInstrument::isExpired() const {
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i) {
            if (!i->first->isExpired())
                return false;
        }
        return true;
    }

    inline void CompositeInstrument::performCalculations() const {
        NPV_ = 0.0;
        for (const_iterator i=components_.begin(); i!=components_.end(); ++i) {
            NPV_ += i->second * i->first->NPV();
        }
    }

}


#endif