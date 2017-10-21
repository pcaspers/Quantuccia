/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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

/*! \file gmres.hpp
    \brief generalized minimal residual method
*/

#ifndef quantlib_gmres_hpp
#define quantlib_gmres_hpp

#include <ql/math/array.hpp>
#include <boost/function.hpp>

#include <list>

namespace QuantLib {

    /*! References:
        Saad, Yousef. 1996, Iterative methods for sparse linear systems,
        http://www-users.cs.umn.edu/~saad/books.html

        Dongarra et al. 1994,
        Templates for the Solution of Linear Systems: Building Blocks
        for Iterative Methods, 2nd Edition, SIAM, Philadelphia
        http://www.netlib.org/templates/templates.pdf

        Christian Kanzow
        Numerik linearer Gleichungssysteme (German)
        Chapter 6: GMRES und verwandte Verfahren
        http://bilder.buecher.de/zusatz/12/12950/12950560_lese_1.pdf
    */

    struct GMRESResult {
        std::list<Real> errors;
        Array x;
    };

    class GMRES  {
      public:
        typedef boost::function1<Disposable<Array> , const Array& > MatrixMult;

        GMRES(const MatrixMult& A, Size maxIter, Real relTol,
                 const MatrixMult& preConditioner = MatrixMult());

        GMRESResult solve(const Array& b, const Array& x0 = Array()) const;
        GMRESResult solveWithRestart(
            Size restart, const Array& b, const Array& x0 = Array()) const;

      protected:
        GMRESResult solveImpl(const Array& b, const Array& x0) const;

        const MatrixMult A_, M_;
        const Size maxIter_;
        const Real relTol_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license0/0 iee along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file gmres.cpp
    \brief generalized minimal residual method
*/


#include <ql/math/functional.hpp>
#include <ql/math/matrixutilities/qrdecomposition.hpp>

#include <numeric>

namespace QuantLib {

    GMRES::GMRES(const GMRES::MatrixMult& A,
                 Size maxIter, Real relTol,
                 const GMRES::MatrixMult& preConditioner)
    : A_(A), M_(preConditioner),
      maxIter_(maxIter), relTol_(relTol) {

        QL_REQUIRE(maxIter_ > 0, "maxIter must be greater then zero");
    }

    GMRESResult GMRES::solve(const Array& b, const Array& x0) const {
        const GMRESResult result = solveImpl(b, x0);

        QL_REQUIRE(result.errors.back() < relTol_, "could not converge");

        return result;
    }

    GMRESResult GMRES::solveWithRestart(
        Size restart, const Array& b, const Array& x0) const {

        GMRESResult result = solveImpl(b, x0);

        std::list<Real> errors = result.errors;

        for (Size i=0; i < restart-1 && result.errors.back() >= relTol_;++i) {
            result = solveImpl(b, result.x);
            errors.insert(
                errors.end(), result.errors.begin(), result.errors.end());
        }

        QL_REQUIRE(errors.back() < relTol_, "could not converge");

        result.errors = errors;
        return result;
    }

    GMRESResult GMRES::solveImpl(const Array& b, const Array& x0) const {
        const Real bn = Norm2(b);
        if (bn == 0.0) {
            GMRESResult result = { std::list<Real>(1, 0.0), b };
            return result;
        }

        Array x = ((!x0.empty()) ? x0 : Array(b.size(), 0.0));
        Array r = b - A_(x);

        const Real g = Norm2(r);
        if (g/bn < relTol_) {
            GMRESResult result = { std::list<Real>(1, g/bn), x };
            return result;
        }

        std::vector<Array> v(1, r/g);
        std::vector<Array> h(1, Array(maxIter_, 0.0));
        std::vector<Real>  c(maxIter_+1), s(maxIter_+1), z(maxIter_+1);

        z[0] = g;

        std::list<Real> errors(1, g/bn);

        for (Size j=0; j < maxIter_ && errors.back() >= relTol_; ++j) {
            h.push_back(Array(maxIter_, 0.0));
            Array w = A_((M_)? M_(v[j]) : v[j]);

            for (Size i=0; i <= j; ++i) {
                h[i][j] = DotProduct(w, v[i]);
                w -= h[i][j] * v[i];
            }

            h[j+1][j] = Norm2(w);

            if (h[j+1][j] < QL_EPSILON*QL_EPSILON)
                break;

            v.push_back(w / h[j+1][j]);

            for (Size i=0; i < j; ++i) {
                const Real h0 = c[i]*h[i][j] + s[i]*h[i+1][j];
                const Real h1 =-s[i]*h[i][j] + c[i]*h[i+1][j];

                h[i][j]   = h0;
                h[i+1][j] = h1;
            }

            const Real nu = std::sqrt(  square<Real>()(h[j][j])
                                      + square<Real>()(h[j+1][j]));

            c[j] = h[j][j]/nu;
            s[j] = h[j+1][j]/nu;

            h[j][j]   = nu;
            h[j+1][j] = 0.0;

            z[j+1] = -s[j]*z[j];
            z[j] = c[j] * z[j];

            errors.push_back(std::fabs(z[j+1]/bn));
        }

        const Size k = v.size()-1;

        Array y(k, 0.0);
        y[k-1]=z[k-1]/h[k-1][k-1];

        for (Integer i=k-2; i >= 0; --i) {
            y[i] = (z[i] - std::inner_product(
                 h[i].begin()+i+1, h[i].begin()+k, y.begin()+i+1, 0.0))/h[i][i];
        }

        Array xm = std::inner_product(
            v.begin(), v.begin()+k, y.begin(), Array(x.size(), 0.0));

        xm = x + ((M_)? M_(xm) : xm);

        GMRESResult result = { errors, xm };
        return result;
    }

}

#endif
