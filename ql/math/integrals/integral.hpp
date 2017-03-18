/*
 Copyright (C) 2007 François du Vignaud

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

/*! \file integral.hpp
\brief Integrators base class definition
*/

#ifndef quantlib_math_integrator_hpp
#define quantlib_math_integrator_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <boost/function.hpp>

namespace QuantLib {

    class Integrator{
      public:
        Integrator(Real absoluteAccuracy,
                   Size maxEvaluations);
        virtual ~Integrator() {}

        Real operator()(const boost::function<Real (Real)>& f,
                        Real a,
                        Real b) const;

        //! \name Modifiers
        //@{
        void setAbsoluteAccuracy(Real);
        void setMaxEvaluations(Size);
        //@}

        //! \name Inspectors
        //@{
        Real absoluteAccuracy() const;
        Size maxEvaluations() const;
        //@}

        Real absoluteError() const ;

        Size numberOfEvaluations() const;

        virtual bool integrationSuccess() const;

      protected:
        virtual Real integrate(const boost::function<Real (Real)>& f,
                               Real a,
                               Real b) const = 0;
        void setAbsoluteError(Real error) const;
        void setNumberOfEvaluations(Size evaluations) const;
        void increaseNumberOfEvaluations(Size increase) const;
      private:
        Real absoluteAccuracy_;
        mutable Real absoluteError_;
        Size maxEvaluations_;
        mutable Size evaluations_;
    };

    // implementation

    inline Integrator::Integrator(Real absoluteAccuracy,
                          Size maxEvaluations)
    : absoluteAccuracy_(absoluteAccuracy),
      maxEvaluations_(maxEvaluations) {
        QL_REQUIRE(absoluteAccuracy > QL_EPSILON,
                   std::scientific << "required tolerance (" <<
                   absoluteAccuracy << ") not allowed. It must be > " <<
                   QL_EPSILON);
    }

    inline void Integrator::setAbsoluteAccuracy(Real accuracy) {
        absoluteAccuracy_= accuracy;
    }

    inline void Integrator::setMaxEvaluations(Size maxEvaluations) {
        maxEvaluations_ = maxEvaluations;
    }

    inline Real Integrator::absoluteAccuracy() const {
        return absoluteAccuracy_;
    }

    inline Size Integrator::maxEvaluations() const {
        return maxEvaluations_;
    }

    inline Real Integrator::absoluteError() const {
        return absoluteError_;
    }

    inline void Integrator::setAbsoluteError(Real error) const {
        absoluteError_ = error;
    }

    inline Size Integrator::numberOfEvaluations() const {
        return evaluations_;
    }

    inline void Integrator::setNumberOfEvaluations(Size evaluations) const {
        evaluations_ = evaluations;
    }

    inline void Integrator::increaseNumberOfEvaluations(Size increase) const {
        evaluations_ += increase;
    }

    inline bool Integrator::integrationSuccess() const {
        return evaluations_ <= maxEvaluations_
            && absoluteError_ <= absoluteAccuracy_;
    }

    inline Real Integrator::operator()(const boost::function<Real (Real)>& f,
                                Real a,
                                Real b) const {
        evaluations_ = 0;
        if (a == b)
            return 0.0;
        if (b > a)
            return integrate(f, a, b);
        else
            return -integrate(f, b, a);
    }


}


#endif
