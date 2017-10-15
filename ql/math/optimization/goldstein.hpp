/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Cheng Li
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

/*! \file goldstein.hpp
    \brief Goldstein and Price line-search class
*/

#ifndef quantlib_optimization_goldstein_hpp
#define quantlib_optimization_goldstein_hpp

#include <ql/math/optimization/linesearch.hpp>

namespace QuantLib {

    class GoldsteinLineSearch : public LineSearch {
      public:
        //! Default constructor
        GoldsteinLineSearch(Real eps = 1e-8,
                            Real alpha = 0.05,
                            Real beta = 0.65,
                            Real extrapolation = 1.5)
        : LineSearch(eps), alpha_(alpha), beta_(beta), extrapolation_(extrapolation) {}

        //! Perform line search
        Real operator()(Problem& P,             // Optimization problem
                        EndCriteria::Type& ecType,
                        const EndCriteria&,
                        const Real t_ini);      // initial value of line-search step

      private:
        Real alpha_, beta_;
        Real extrapolation_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Cheng Li
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
#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/comparison.hpp>

namespace QuantLib {

    Real GoldsteinLineSearch::operator()(Problem& P, 
                                         EndCriteria::Type& ecType, 
                                         const EndCriteria& endCriteria, 
                                         const Real t_ini) 
    {
        Constraint& constraint = P.constraint();
        succeed_=true;
        bool maxIter = false;
        Real t = t_ini;
        Size loopNumber = 0;

        Real q0 = P.functionValue();
        Real qp0 = P.gradientNormValue();
        
        Real tl = 0.0;
        Real tr = 0.0;

        qt_ = q0;
        qpt_ = (gradient_.empty()) ? qp0 : -DotProduct(gradient_,searchDirection_);

        // Initialize gradient
        gradient_ = Array(P.currentValue().size());
        // Compute new point
        xtd_ = P.currentValue();
        t = update(xtd_, searchDirection_, t, constraint);
        // Compute function value at the new point
        qt_ = P.value (xtd_);

        while ((qt_ - q0) < -beta_*t*qpt_ || (qt_ - q0) > -alpha_*t*qpt_) {
            if ((qt_ - q0) > -alpha_*t*qpt_)
                tr = t;
            else
                tl = t;
            ++loopNumber;
            
            // calculate the new step
            if (close_enough(tr, 0.0))
                t *= extrapolation_;
            else
                t = (tl + tr) / 2.0;

            // New point value
            xtd_ = P.currentValue();
            t = update(xtd_, searchDirection_, t, constraint);

            // Compute function value at the new point
            qt_ = P.value (xtd_);
            P.gradient (gradient_, xtd_);
            // and it squared norm
            maxIter = endCriteria.checkMaxIterations(loopNumber, ecType);

            if (maxIter)
                break;
        }

        if (maxIter)
            succeed_ = false;

        // Compute new gradient
        P.gradient(gradient_, xtd_);
        // and it squared norm
        qpt_ = DotProduct(gradient_, gradient_);

        // Return new step value
        return t;
    }

}


#endif
