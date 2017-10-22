/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005, 2007 StatPro Italia srl
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file model.hpp
    \brief Abstract interest rate model class
*/

#ifndef quantlib_interest_rate_model_hpp
#define quantlib_interest_rate_model_hpp

#include <ql/option.hpp>
#include <ql/methods/lattices/lattice.hpp>
#include <ql/models/parameter.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/math/optimization/endcriteria.hpp>

namespace QuantLib {

    class OptimizationMethod;

    //! Affine model class
    /*! Base class for analytically tractable models.

        \ingroup shortrate
    */
    class AffineModel : public virtual Observable {
      public:
        //! Implied discount curve
        virtual DiscountFactor discount(Time t) const = 0;

        virtual Real discountBond(Time now,
                                  Time maturity,
                                  Array factors) const = 0;

        virtual Real discountBondOption(Option::Type type,
                                        Real strike,
                                        Time maturity,
                                        Time bondMaturity) const = 0;

        virtual Real discountBondOption(Option::Type type,
                                        Real strike,
                                        Time maturity,
                                        Time bondStart,
                                        Time bondMaturity) const;
    };


    //! Term-structure consistent model class
    /*! This is a base class for models that can reprice exactly
        any discount bond.

        \ingroup shortrate
    */
    class TermStructureConsistentModel : public virtual Observable {
      public:
        TermStructureConsistentModel(
                              const Handle<YieldTermStructure>& termStructure)
        : termStructure_(termStructure) {}
        const Handle<YieldTermStructure>& termStructure() const {
            return termStructure_;
        }
      private:
        Handle<YieldTermStructure> termStructure_;
    };


    //! Calibrated model class
    class CalibratedModel : public virtual Observer, public virtual Observable {
      public:
        CalibratedModel(Size nArguments);

        void update() {
            generateArguments();
            notifyObservers();
        }

        //! Calibrate to a set of market instruments (usually caps/swaptions)
        /*! An additional constraint can be passed which must be
            satisfied in addition to the constraints of the model.
        */
        virtual void calibrate(
                const std::vector<boost::shared_ptr<CalibrationHelper> >&,
                OptimizationMethod& method,
                const EndCriteria& endCriteria,
                const Constraint& constraint = Constraint(),
                const std::vector<Real>& weights = std::vector<Real>(),
                const std::vector<bool>& fixParameters = std::vector<bool>());

        Real value(const Array& params,
                   const std::vector<boost::shared_ptr<CalibrationHelper> >&);

        const boost::shared_ptr<Constraint>& constraint() const;

        //! Returns end criteria result
        EndCriteria::Type endCriteria() const { return shortRateEndCriteria_; }

        //! Returns the problem values
        const Array& problemValues() const { return problemValues_; }

        //! Returns array of arguments on which calibration is done
        Disposable<Array> params() const;

        virtual void setParams(const Array& params);
        Integer functionEvaluation() const { return functionEvaluation_; }

      protected:
        virtual void generateArguments() {}
        std::vector<Parameter> arguments_;
        boost::shared_ptr<Constraint> constraint_;
        EndCriteria::Type shortRateEndCriteria_;
        Array problemValues_;
        Integer functionEvaluation_;

      private:
        //! Constraint imposed on arguments
        class PrivateConstraint;
        //! Calibration cost function class
        class CalibrationFunction;
        friend class CalibrationFunction;
    };

    //! Abstract short-rate model class
    /*! \ingroup shortrate */
    class ShortRateModel : public CalibratedModel {
      public:
        ShortRateModel(Size nArguments);
        virtual boost::shared_ptr<Lattice> tree(const TimeGrid&) const = 0;
    };


    // inline definitions


    inline Real AffineModel::discountBondOption(Option::Type type,
                                                Real strike,
                                                Time maturity,
                                                Time,
                                                Time bondMaturity) const {
        return discountBondOption(type, strike, maturity, bondMaturity);
    }

    inline const boost::shared_ptr<Constraint>&
    CalibratedModel::constraint() const {
        return constraint_;
    }

    class CalibratedModel::PrivateConstraint : public Constraint {
      private:
        class Impl :  public Constraint::Impl {
          public:
            Impl(const std::vector<Parameter>& arguments)
            : arguments_(arguments) {}

            bool test(const Array& params) const {
                Size k=0;
                for (Size i=0; i<arguments_.size(); i++) {
                    Size size = arguments_[i].size();
                    Array testParams(size);
                    for (Size j=0; j<size; j++, k++)
                        testParams[j] = params[k];
                    if (!arguments_[i].testParams(testParams))
                        return false;
                }
                return true;
            }

            Array upperBound(const Array &params) const {
                Size k = 0, k2 = 0;
                Size totalSize = 0;
                for (Size i = 0; i < arguments_.size(); i++) {
                    totalSize += arguments_[i].size();
                }
                Array result(totalSize);
                for (Size i = 0; i < arguments_.size(); i++) {
                    Size size = arguments_[i].size();
                    Array partialParams(size);
                    for (Size j = 0; j < size; j++, k++)
                        partialParams[j] = params[k];
                    Array tmpBound =
                        arguments_[i].constraint().upperBound(partialParams);
                    for (Size j = 0; j < size; j++, k2++)
                        result[k2] = tmpBound[j];
                }
                return result;
            }

            Array lowerBound(const Array &params) const {
                Size k = 0, k2 = 0;
                Size totalSize = 0;
                for (Size i = 0; i < arguments_.size(); i++) {
                    totalSize += arguments_[i].size();
                }
                Array result(totalSize);
                for (Size i = 0; i < arguments_.size(); i++) {
                    Size size = arguments_[i].size();
                    Array partialParams(size);
                    for (Size j = 0; j < size; j++, k++)
                        partialParams[j] = params[k];
                    Array tmpBound =
                        arguments_[i].constraint().lowerBound(partialParams);
                    for (Size j = 0; j < size; j++, k2++)
                        result[k2] = tmpBound[j];
                }
                return result;
            }

          private:
            const std::vector<Parameter>& arguments_;
        };
      public:
        PrivateConstraint(const std::vector<Parameter>& arguments)
        : Constraint(boost::shared_ptr<Constraint::Impl>(
                                   new PrivateConstraint::Impl(arguments))) {}
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2013, 2015 Peter Caspers

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

#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/projection.hpp>
#include <ql/math/optimization/projectedconstraint.hpp>

#include <ql/utilities/null_deleter.hpp>

using std::vector;

namespace QuantLib {

    CalibratedModel::CalibratedModel(Size nArguments)
    : arguments_(nArguments),
      constraint_(new PrivateConstraint(arguments_)),
      shortRateEndCriteria_(EndCriteria::None) {}

    class CalibratedModel::CalibrationFunction : public CostFunction {
      public:
        CalibrationFunction(CalibratedModel* model,
                            const vector<boost::shared_ptr<CalibrationHelper> >& h,
                            const vector<Real>& weights,
                            const Projection& projection)
            : model_(model, null_deleter()), instruments_(h),
              weights_(weights), projection_(projection) { }

        virtual ~CalibrationFunction() {}

        virtual Real value(const Array& params) const {
            model_->setParams(projection_.include(params));
            Real value = 0.0;
            for (Size i=0; i<instruments_.size(); i++) {
                Real diff = instruments_[i]->calibrationError();
                value += diff*diff*weights_[i];
            }
            return std::sqrt(value);
        }

        virtual Disposable<Array> values(const Array& params) const {
            model_->setParams(projection_.include(params));
            Array values(instruments_.size());
            for (Size i=0; i<instruments_.size(); i++) {
                values[i] = instruments_[i]->calibrationError()
                           *std::sqrt(weights_[i]);
            }
            return values;
        }

        virtual Real finiteDifferenceEpsilon() const { return 1e-6; }

      private:
        boost::shared_ptr<CalibratedModel> model_;
        const vector<boost::shared_ptr<CalibrationHelper> >& instruments_;
        vector<Real> weights_;
        const Projection projection_;
    };

    void CalibratedModel::calibrate(
                    const vector<boost::shared_ptr<CalibrationHelper> >& instruments,
                    OptimizationMethod& method,
                    const EndCriteria& endCriteria,
                    const Constraint& additionalConstraint,
                    const vector<Real>& weights,
                    const vector<bool>& fixParameters) {

        QL_REQUIRE(weights.empty() || weights.size() == instruments.size(),
                   "mismatch between number of instruments (" <<
                   instruments.size() << ") and weights(" <<
                   weights.size() << ")");

        Constraint c;
        if (additionalConstraint.empty())
            c = *constraint_;
        else
            c = CompositeConstraint(*constraint_,additionalConstraint);
        vector<Real> w =
            weights.empty() ? vector<Real>(instruments.size(), 1.0): weights;

        Array prms = params();
        vector<bool> all(prms.size(), false);
        Projection proj(prms,fixParameters.size()>0 ? fixParameters : all);
        CalibrationFunction f(this,instruments,w,proj);
        ProjectedConstraint pc(c,proj);
        Problem prob(f, pc, proj.project(prms));
        shortRateEndCriteria_ = method.minimize(prob, endCriteria);
        Array result(prob.currentValue());
        setParams(proj.include(result));
        problemValues_ = prob.values(result);
        functionEvaluation_ = prob.functionEvaluation();

        notifyObservers();
    }

    Real CalibratedModel::value(
                const Array& params,
                const vector<boost::shared_ptr<CalibrationHelper> >& instruments) {
        vector<Real> w = vector<Real>(instruments.size(), 1.0);
        Projection p(params);
        CalibrationFunction f(this, instruments, w, p);
        return f.value(params);
    }

    Disposable<Array> CalibratedModel::params() const {
        Size size = 0, i;
        for (i=0; i<arguments_.size(); i++)
            size += arguments_[i].size();
        Array params(size);
        Size k = 0;
        for (i=0; i<arguments_.size(); i++) {
            for (Size j=0; j<arguments_[i].size(); j++, k++) {
                params[k] = arguments_[i].params()[j];
            }
        }
        return params;
    }

    void CalibratedModel::setParams(const Array& params) {
        Array::const_iterator p = params.begin();
        for (Size i=0; i<arguments_.size(); ++i) {
            for (Size j=0; j<arguments_[i].size(); ++j, ++p) {
                QL_REQUIRE(p!=params.end(),"parameter array too small");
                arguments_[i].setParam(j, *p);
            }
        }
        QL_REQUIRE(p==params.end(),"parameter array too big!");
        generateArguments();
        notifyObservers();
    }

    ShortRateModel::ShortRateModel(Size nArguments)
    : CalibratedModel(nArguments) {}

}


#endif
