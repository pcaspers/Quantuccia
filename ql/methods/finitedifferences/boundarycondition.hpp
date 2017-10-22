/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file boundarycondition.hpp
    \brief boundary conditions for differential operators
*/

#ifndef quantlib_boundary_condition_hpp
#define quantlib_boundary_condition_hpp

#include <ql/utilities/null.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>

namespace QuantLib {

    //! Abstract boundary condition class for finite difference problems
    /*! \ingroup findiff */
    template <class Operator>
    class BoundaryCondition {
      public:
        // types and enumerations
        typedef Operator operator_type;
        typedef typename Operator::array_type array_type;
        //! \todo Generalize for n-dimensional conditions
        enum Side { None, Upper, Lower };
        // destructor
        virtual ~BoundaryCondition() {}
        // interface
        /*! This method modifies an operator \f$ L \f$ before it is
            applied to an array \f$ u \f$ so that \f$ v = Lu \f$ will
            satisfy the given condition. */
        virtual void applyBeforeApplying(operator_type&) const = 0;
        /*! This method modifies an array \f$ u \f$ so that it satisfies
            the given condition. */
        virtual void applyAfterApplying(array_type&) const = 0;
        /*! This method modifies an operator \f$ L \f$ before the linear
            system \f$ Lu' = u \f$ is solved so that \f$ u' \f$ will
            satisfy the given condition. */
        virtual void applyBeforeSolving(operator_type&,
                                        array_type& rhs) const = 0;
        /*! This method modifies an array \f$ u \f$ so that it satisfies
            the given condition. */
        virtual void applyAfterSolving(array_type&) const = 0;
        /*! This method sets the current time for time-dependent
            boundary conditions. */
        virtual void setTime(Time t) = 0;
    };

    // Time-independent boundary conditions for tridiagonal operators

    //! Neumann boundary condition (i.e., constant derivative)
    /*! \warning The value passed must not be the value of the derivative.
                 Instead, it must be comprehensive of the grid step
                 between the first two points--i.e., it must be the
                 difference between f[0] and f[1].
        \todo generalize to time-dependent conditions.

        \ingroup findiff
    */
    class NeumannBC : public BoundaryCondition<TridiagonalOperator> {
      public:
        NeumannBC(Real value, Side side);
        // interface
        void applyBeforeApplying(TridiagonalOperator&) const;
        void applyAfterApplying(Array&) const;
        void applyBeforeSolving(TridiagonalOperator&, Array& rhs) const;
        void applyAfterSolving(Array&) const;
        void setTime(Time) {}
      private:
        Real value_;
        Side side_;
    };

    //! Neumann boundary condition (i.e., constant value)
    /*! \todo generalize to time-dependent conditions.

        \ingroup findiff
    */
    class DirichletBC : public BoundaryCondition<TridiagonalOperator> {
      public:
        DirichletBC(Real value, Side side);
        // interface
        void applyBeforeApplying(TridiagonalOperator&) const;
        void applyAfterApplying(Array&) const;
        void applyBeforeSolving(TridiagonalOperator&, Array& rhs) const;
        void applyAfterSolving(Array&) const;
        void setTime(Time) {}
      private:
        Real value_;
        Side side_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

    NeumannBC::NeumannBC(Real value, NeumannBC::Side side)
    : value_(value), side_(side) {}

    void NeumannBC::applyBeforeApplying(TridiagonalOperator& L) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(-1.0,1.0);
            break;
          case Upper:
            L.setLastRow(-1.0,1.0);
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void NeumannBC::applyAfterApplying(Array& u) const {
        switch (side_) {
          case Lower:
            u[0] = u[1] - value_;
            break;
          case Upper:
            u[u.size()-1] = u[u.size()-2] + value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void NeumannBC::applyBeforeSolving(TridiagonalOperator& L,
                                       Array& rhs) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(-1.0,1.0);
            rhs[0] = value_;
            break;
          case Upper:
            L.setLastRow(-1.0,1.0);
            rhs[rhs.size()-1] = value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void NeumannBC::applyAfterSolving(Array&) const {}



    DirichletBC::DirichletBC(Real value, DirichletBC::Side side)
    : value_(value), side_(side) {}

    void DirichletBC::applyBeforeApplying(TridiagonalOperator& L) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(1.0,0.0);
            break;
          case Upper:
            L.setLastRow(0.0,1.0);
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void DirichletBC::applyAfterApplying(Array& u) const {
        switch (side_) {
          case Lower:
            u[0] = value_;
            break;
          case Upper:
            u[u.size()-1] = value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void DirichletBC::applyBeforeSolving(TridiagonalOperator& L,
                                         Array& rhs) const {
        switch (side_) {
          case Lower:
            L.setFirstRow(1.0,0.0);
            rhs[0] = value_;
            break;
          case Upper:
            L.setLastRow(0.0,1.0);
            rhs[rhs.size()-1] = value_;
            break;
          default:
            QL_FAIL("unknown side for Neumann boundary condition");
        }
    }

    void DirichletBC::applyAfterSolving(Array&) const {}

}


#endif
