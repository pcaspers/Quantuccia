/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file tqreigendecomposition.hpp
    \brief tridiag. QR eigen decomposition with explicite shift aka Wilkinson
*/

#ifndef quantlib_tqr_eigen_decomposition_hpp
#define quantlib_tqr_eigen_decomposition_hpp

#include <ql/math/array.hpp>
#include <ql/math/matrix.hpp>

#include <vector>

namespace QuantLib {

    //! tridiag. QR eigen decomposition with explicite shift aka Wilkinson
    /*! References:

        Wilkinson, J.H. and Reinsch, C. 1971, Linear Algebra, vol. II of
        Handbook for Automatic Computation (New York: Springer-Verlag)

        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery,

        \test the correctness of the result is tested by checking it
              against known good values.
    */
    class TqrEigenDecomposition {
      public:
        enum EigenVectorCalculation { WithEigenVector,
                                      WithoutEigenVector,
                                      OnlyFirstRowEigenVector };

        enum ShiftStrategy { NoShift,
                             Overrelaxation,
                             CloseEigenValue };

        TqrEigenDecomposition(const Array& diag,
                              const Array& sub,
                              EigenVectorCalculation calc = WithEigenVector,
                              ShiftStrategy strategy = CloseEigenValue);

        const Array& eigenvalues()  const { return d_; }
        const Matrix& eigenvectors() const { return ev_; }

        Size iterations() const { return iter_; }

      private:
        bool offDiagIsZero(Size k, Array& e);

        Size iter_;
        Array d_;
        Matrix ev_;
    };

    // implementation

    inline TqrEigenDecomposition::TqrEigenDecomposition(const Array& diag,
                                                 const Array& sub,
                                                 EigenVectorCalculation calc,
                                                 ShiftStrategy strategy)
    : iter_(0), d_(diag),
      ev_((calc == WithEigenVector)? d_.size() :
          (calc == WithoutEigenVector)? 0 : 1, d_.size(), 0)
    {
        Size n = diag.size();

        QL_REQUIRE(n == sub.size()+1, "Wrong dimensions");

        Array e(n, 0.0);
        std::copy(sub.begin(),sub.end(),e.begin()+1);
        Size i;
        for (i=0; i < ev_.rows(); ++i) {
            ev_[i][i] = 1.0;
        }

        for (Size k=n-1; k >=1; --k) {
            while (!offDiagIsZero(k, e)) {
                Size l = k;
                while (--l > 0 && !offDiagIsZero(l,e));
                iter_++;

                Real q = d_[l];
                if (strategy != NoShift) {
                    // calculated eigenvalue of 2x2 sub matrix of
                    // [ d_[k-1] e_[k] ]
                    // [  e_[k]  d_[k] ]
                    // which is closer to d_[k+1].
                    // FLOATING_POINT_EXCEPTION
                    const Real t1 = std::sqrt(
                                          0.25*(d_[k]*d_[k] + d_[k-1]*d_[k-1])
                                          - 0.5*d_[k-1]*d_[k] + e[k]*e[k]);
                    const Real t2 = 0.5*(d_[k]+d_[k-1]);

                    const Real lambda =
                        (std::fabs(t2+t1 - d_[k]) < std::fabs(t2-t1 - d_[k]))?
                        t2+t1 : t2-t1;

                    if (strategy == CloseEigenValue) {
                        q-=lambda;
                    } else {
                        q-=((k==n-1)? 1.25 : 1.0)*lambda;
                    }
                }

                // the QR transformation
                Real sine = 1.0;
                Real cosine = 1.0;
                Real u = 0.0;

                bool recoverUnderflow = false;
                for (Size i=l+1; i <= k && !recoverUnderflow; ++i) {
                    const Real h = cosine*e[i];
                    const Real p = sine*e[i];

                    e[i-1] = std::sqrt(p*p+q*q);
                    if (e[i-1] != 0.0) {
                        sine = p/e[i-1];
                        cosine = q/e[i-1];

                        const Real g = d_[i-1]-u;
                        const Real t = (d_[i]-g)*sine+2*cosine*h;

                        u = sine*t;
                        d_[i-1] = g + u;
                        q = cosine*t - h;

                        for (Size j=0; j < ev_.rows(); ++j) {
                            const Real tmp = ev_[j][i-1];
                            ev_[j][i-1] = sine*ev_[j][i] + cosine*tmp;
                            ev_[j][i] = cosine*ev_[j][i] - sine*tmp;
                        }
                    } else {
                        // recover from underflow
                        d_[i-1] -= u;
                        e[l] = 0.0;
                        recoverUnderflow = true;
                    }
                }

                if (!recoverUnderflow) {
                    d_[k] -= u;
                    e[k] = q;
                    e[l] = 0.0;
                }
            }
        }

        // sort (eigenvalues, eigenvectors),
        // code taken from symmetricSchureDecomposition.cpp
        std::vector<std::pair<Real, std::vector<Real> > > temp(n);
        std::vector<Real> eigenVector(ev_.rows());
        for (i=0; i<n; i++) {
            if (ev_.rows() > 0)
                std::copy(ev_.column_begin(i),
                          ev_.column_end(i), eigenVector.begin());
            temp[i] = std::make_pair(d_[i], eigenVector);
        }
        std::sort(temp.begin(), temp.end(),
                  std::greater<std::pair<Real, std::vector<Real> > >());
        // first element is positive
        for (i=0; i<n; i++) {
            d_[i] = temp[i].first;
            Real sign = 1.0;
            if (ev_.rows() > 0 && temp[i].second[0]<0.0)
                sign = -1.0;
            for (Size j=0; j<ev_.rows(); ++j) {
                ev_[j][i] = sign * temp[i].second[j];
            }
        }
    }

    // see NR for abort assumption as it is
    // not part of the original Wilkinson algorithm
    inline bool TqrEigenDecomposition::offDiagIsZero(Size k, Array& e) {
        return std::fabs(d_[k-1])+std::fabs(d_[k])
            == std::fabs(d_[k-1])+std::fabs(d_[k])+std::fabs(e[k]);
    }


}


#endif
