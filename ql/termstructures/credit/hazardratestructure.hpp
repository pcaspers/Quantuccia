/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file hazardratestructure.hpp
    \brief hazard-rate term structure
*/

#ifndef quantlib_hazard_rate_structure_hpp
#define quantlib_hazard_rate_structure_hpp

#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    //! Hazard-rate term structure
    /*! This abstract class acts as an adapter to
        DefaultProbabilityTermStructure allowing the programmer to implement
        only the <tt>hazardRateImpl(Time)</tt> method in derived classes.

        Survival/default probabilities and default densities are calculated
        from hazard rates.

        Hazard rates are defined with annual frequency and continuous
        compounding.

        \ingroup defaultprobabilitytermstructures
    */
    class HazardRateStructure : public DefaultProbabilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        HazardRateStructure(
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        HazardRateStructure(
            const Date& referenceDate,
            const Calendar& cal = Calendar(),
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        HazardRateStructure(
            Natural settlementDays,
            const Calendar& cal,
            const DayCounter& dayCounter = DayCounter(),
            const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
            const std::vector<Date>& jumpDates = std::vector<Date>());
        //@}
      protected:
        /*! \name Calculations

            This method must be implemented in derived classes to
            perform the actual calculations. When it is called,
            range check has already been performed; therefore, it
            must assume that extrapolation is required.
        */
        //@{
        //! hazard rate calculation
        virtual Real hazardRateImpl(Time) const = 0;
        //@}

        //! \name DefaultProbabilityTermStructure implementation
        //@{
        /*! survival probability calculation
            implemented in terms of the hazard rate \f$ h(t) \f$ as
            \f[
            S(t) = \exp\left( - \int_0^t h(\tau) d\tau \right).
            \f]

            \warning This default implementation uses numerical integration,
                     which might be inefficient and inaccurate.
                     Derived classes should override it if a more efficient
                     implementation is available.
        */
        Probability survivalProbabilityImpl(Time) const;
        //! default density calculation
        Real defaultDensityImpl(Time) const;
        //@}
    };

    // inline definitions

    inline Real HazardRateStructure::defaultDensityImpl(Time t) const {
        return hazardRateImpl(t)*survivalProbabilityImpl(t);
    }

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

#include <ql/math/integrals/gaussianquadratures.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <boost/bind.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

using namespace boost;

namespace QuantLib {

    namespace {

        template <class F>
        struct remapper {
            F f;
            Time T;
            remapper(const F& f, Time T) : f(f), T(T) {}
            // This remaps [-1,1] to [0,T]. No differential included.
            Real operator()(Real x) const {
                const Real arg = (x+1.0)*T/2.0;
                return f(arg);
            }
        };

        template <class F>
        remapper<F> remap(const F& f, Time T) {
            return remapper<F>(f,T);
        }

    }

    inline HazardRateStructure::HazardRateStructure(
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(dc, jumps, jumpDates) {}

    inline HazardRateStructure::HazardRateStructure(
                                    const Date& refDate,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(refDate, cal, dc, jumps, jumpDates) {}

    inline HazardRateStructure::HazardRateStructure(
                                    Natural settlDays,
                                    const Calendar& cal,
                                    const DayCounter& dc,
                                    const std::vector<Handle<Quote> >& jumps,
                                    const std::vector<Date>& jumpDates)
    : DefaultProbabilityTermStructure(settlDays, cal, dc, jumps, jumpDates) {}

    inline Probability HazardRateStructure::survivalProbabilityImpl(Time t) const {
        static GaussChebyshevIntegration integral(48);
        // this stores the address of the method to integrate (so that
        // we don't have to insert its full expression inside the
        // integral below--it's long enough already)
        Real (HazardRateStructure::*f)(Time) const =
            &HazardRateStructure::hazardRateImpl;
        // the Gauss-Chebyshev quadratures integrate over [-1,1],
        // hence the remapping (and the Jacobian term t/2)
        return std::exp(-integral(remap(bind(f,this,_1), t)) * t/2.0);
    }

}

#endif
