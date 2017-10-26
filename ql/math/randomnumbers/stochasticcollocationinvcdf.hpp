/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2016 Klaus Spanderen

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

/*! \file stochasticcollocationinvcdf.hpp
    Stochastic collocation inverse cumulative distribution function
*/

#ifndef quantlib_stochastic_collation_inv_cdf_hpp
#define quantlib_stochastic_collation_inv_cdf_hpp

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>

#include <boost/function.hpp>
#include <functional>

namespace QuantLib {
    //! Stochastic collocation inverse cumulative distribution function

    /*! References:
        L.A. Grzelak, J.A.S. Witteveen, M.Suárez-Taboada, C.W. Oosterlee,
        The Stochastic Collocation Monte Carlo Sampler: Highly efficient
        sampling from “expensive” distributions
        http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2529691
     */

    class StochasticCollocationInvCDF : public std::unary_function<Real,Real> {
      public:
        StochasticCollocationInvCDF(
            const boost::function<Real(Real)>& invCDF,
            Size lagrangeOrder,
            Real pMax = Null<Real>(),
            Real pMin = Null<Real>());

        Real value(Real x) const;
        Real operator()(Real u) const;

      private:
        const Array x_;
        const Volatility sigma_;
        const Array y_;
        const LagrangeInterpolation interpl_;
    };
}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2016 Klaus Spanderen

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

/*! \file stochasticcollationcdf.cpp
*/

#include <ql/mathconstants.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>

namespace QuantLib {

    namespace {
        Disposable<Array> g(Real sigma, const Array& x,
                            const boost::function<Real(Real)>& invCDF) {

            Array y(x.size());
            const CumulativeNormalDistribution normalCDF;

            for (Size i=0, n=x.size(); i < n; ++i) {
                y[i] = invCDF(normalCDF(x[i]/sigma));
            }

            return y;
        }
    }

    inline StochasticCollocationInvCDF::StochasticCollocationInvCDF(
        const boost::function<Real(Real)>& invCDF,
        Size lagrangeOrder, Real pMax, Real pMin)
    : x_(M_SQRT2*GaussHermiteIntegration(lagrangeOrder).x()),
      sigma_( (pMax != Null<Real>())
              ? x_.back() / InverseCumulativeNormal()(pMax)
              : (pMin != Null<Real>())
                  ? x_.front() / InverseCumulativeNormal()(pMin)
                  : 1.0),
      y_(g(sigma_, x_, invCDF)),
      interpl_(x_.begin(), x_.end(), y_.begin()) {
    }

    inline Real StochasticCollocationInvCDF::value(Real x) const {
        return interpl_(x*sigma_, true);
    }
    inline Real StochasticCollocationInvCDF::operator()(Real u) const {
        return value(InverseCumulativeNormal()(u));
    }
}


#endif