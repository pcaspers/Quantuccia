/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Mark Joshi

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


#ifndef quantlib_basis_incomplete_ordered_hpp
#define quantlib_basis_incomplete_ordered_hpp

#include <ql/math/matrix.hpp>
#include <valarray>

namespace QuantLib {

    class BasisIncompleteOrdered {
      public:
        BasisIncompleteOrdered(Size euclideanDimension);
        //! return value indicates if the vector was linearly independent
        bool addVector(const Array& newVector);
        Size basisSize() const;
        Size euclideanDimension() const;
        Matrix getBasisAsRowsInMatrix() const;
      private:
        std::vector<Array> currentBasis_;
        Size euclideanDimension_;
        Array newVector_;
    };

/*! Given a collection of vectors, w_i, find a collection of vectors x_i such that
x_i is orthogonal to w_j for i != j, and <x_i, w_i> = <w_i, w_i>

This is done by performing GramSchmidt on the other vectors and then projecting onto
the orthogonal space.

This class is tested in

    MatricesTest::testOrthogonalProjection();
*/

    class OrthogonalProjections
    {
    public:
        OrthogonalProjections(const Matrix& originalVectors,
                              Real multiplierCutOff,
                               Real tolerance  );

        const std::valarray<bool>& validVectors() const;
        const std::vector<Real>& GetVector(Size index) const;

        Size numberValidVectors() const;


    private:

        //! inputs
        Matrix originalVectors_;
        Real multiplierCutoff_;
        Size numberVectors_;
        Size numberValidVectors_;
        Size dimension_;

        //!outputs
        std::valarray<bool> validVectors_;
        std::vector<std::vector<Real> > projectedVectors_;

        //!workspace
        Matrix orthoNormalizedVectors_;


    };

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007, 2008 Mark Joshi

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

#include <algorithm>

namespace QuantLib {

  inline BasisIncompleteOrdered::BasisIncompleteOrdered(Size euclideanDimension)
        : euclideanDimension_(euclideanDimension) {}

  inline bool BasisIncompleteOrdered::addVector(const Array& newVector1) {

        QL_REQUIRE(newVector1.size() == euclideanDimension_,
            "missized vector passed to "
            "BasisIncompleteOrdered::addVector");

        newVector_ = newVector1;

        if (currentBasis_.size()==euclideanDimension_)
            return false;

        for (Size j=0; j<currentBasis_.size(); ++j) {
            Real innerProd = std::inner_product(newVector_.begin(),
                newVector_.end(),
                currentBasis_[j].begin(), 0.0);

            for (Size k=0; k<euclideanDimension_; ++k)
                newVector_[k] -=innerProd*currentBasis_[j][k];
        }

        Real norm = std::sqrt(std::inner_product(newVector_.begin(),
            newVector_.end(),
            newVector_.begin(), 0.0));

        if (norm<1e-12) // maybe this should be a tolerance
            return false;

        for (Size l=0; l<euclideanDimension_; ++l)
            newVector_[l]/=norm;

        currentBasis_.push_back(newVector_);

        return true;
    }

  inline Size BasisIncompleteOrdered::basisSize() const {
        return currentBasis_.size();
    }

  inline Size BasisIncompleteOrdered::euclideanDimension() const {
        return euclideanDimension_;
    }


  inline Matrix BasisIncompleteOrdered::getBasisAsRowsInMatrix() const {
        Matrix basis(currentBasis_.size(), euclideanDimension_);
        for (Size i=0; i<basis.rows(); ++i)
            for (Size j=0; j<basis.columns(); ++j)
                basis[i][j] = currentBasis_[i][j];

        return basis;
    }

    namespace
    {
        inline Real normSquared(const Matrix& v, Size row)
        {
            Real x=0.0;
            for (Size i=0; i < v.columns(); ++i)
                x += v[row][i]*v[row][i];

            return x;
        }


        inline Real norm(const Matrix& v, Size row)
        {
            return std::sqrt(normSquared( v,  row));
        }

        inline Real innerProduct(const Matrix& v, Size row1, const Matrix& w, Size row2)
        {

            Real x=0.0;
            for (Size i=0; i < v.columns(); ++i)
                x += v[row1][i]*w[row2][i];

            return x;
        }

    }



  inline OrthogonalProjections::OrthogonalProjections(const Matrix& originalVectors,
                                                 Real multiplierCutoff,
                                                 Real tolerance)
    : originalVectors_(originalVectors),
      multiplierCutoff_(multiplierCutoff),
      numberVectors_(originalVectors.rows()),
      dimension_(originalVectors.columns()),
      validVectors_(true,originalVectors.rows()), // opposite way round from vector constructor
      orthoNormalizedVectors_(originalVectors.rows(),
                              originalVectors.columns())
    {
        std::vector<Real> currentVector(dimension_);
        for (Size j=0; j < numberVectors_; ++j)
        {

            if (validVectors_[j])
            {
                for (Size k=0; k< numberVectors_; ++k) // create an orthormal basis not containing j
                {
                    for (Size m=0; m < dimension_; ++m)
                        orthoNormalizedVectors_[k][m] = originalVectors_[k][m];

                    if ( k !=j && validVectors_[k])
                    {


                        for (Size l=0; l < k; ++l)
                        {
                            if (validVectors_[l] && l !=j)
                            {
                                Real dotProduct = innerProduct(orthoNormalizedVectors_, k, orthoNormalizedVectors_,l);
                                for (Size n=0; n < dimension_; ++n)
                                    orthoNormalizedVectors_[k][n] -=  dotProduct*orthoNormalizedVectors_[l][n];
                            }

                        }

                        Real normBeforeScaling= norm(orthoNormalizedVectors_,k);

                        if (normBeforeScaling < tolerance)
                        {
                            validVectors_[k] = false;
                        }
                        else
                        {
                            Real normBeforeScalingRecip = 1.0/normBeforeScaling;
                            for (Size m=0; m < dimension_; ++m)
                                orthoNormalizedVectors_[k][m] *= normBeforeScalingRecip;

                        } // end of else (norm < tolerance)

                    } // end of if k !=j && validVectors_[k])

                }// end of  for (Size k=0; k< numberVectors_; ++k)

                // we now have an o.n. basis for everything except  j

                Real prevNormSquared = normSquared(originalVectors_, j);


                for (Size r=0; r < numberVectors_; ++r)
                    if (validVectors_[r] && r != j)
                    {
                        Real dotProduct = innerProduct(orthoNormalizedVectors_, j, orthoNormalizedVectors_,r);

                        for (Size s=0; s < dimension_; ++s)
                           orthoNormalizedVectors_[j][s] -= dotProduct*orthoNormalizedVectors_[r][s];

                    }

               Real projectionOnOriginalDirection = innerProduct(originalVectors_,j,orthoNormalizedVectors_,j);
               Real sizeMultiplier = prevNormSquared/projectionOnOriginalDirection;

               if (std::fabs(sizeMultiplier) < multiplierCutoff_)
               {
                    for (Size t=0; t < dimension_; ++t)
                        currentVector[t] = orthoNormalizedVectors_[j][t]*sizeMultiplier;

               }
               else
                   validVectors_[j] = false;


            } // end of  if (validVectors_[j])

            projectedVectors_.push_back(currentVector);


        } //end of j loop

        numberValidVectors_ =0;
        for (Size i=0; i < numberVectors_; ++i)
            numberValidVectors_ +=  validVectors_[i] ? 1 : 0;


    } // end of constructor

  inline const std::valarray<bool>& OrthogonalProjections::validVectors() const
    {
        return validVectors_;

    }

  inline const std::vector<Real>& OrthogonalProjections::GetVector(Size index) const
    {
        return projectedVectors_[index];
    }


  inline Size OrthogonalProjections::numberValidVectors() const
  {
        return numberValidVectors_;
  }




}


#endif
