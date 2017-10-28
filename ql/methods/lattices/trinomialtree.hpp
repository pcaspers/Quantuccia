/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file trinomialtree.hpp
    \brief Trinomial tree class
*/

#ifndef quantlib_trinomial_tree_hpp
#define quantlib_trinomial_tree_hpp

#include <ql/methods/lattices/tree.hpp>
#include <ql/timegrid.hpp>

namespace QuantLib {
    class StochasticProcess1D;
    //! Recombining trinomial tree class
    /*! This class defines a recombining trinomial tree approximating a
        1-D stochastic process.
        \warning The diffusion term of the SDE must be independent of the
                 underlying process.

        \ingroup lattices
    */
    class TrinomialTree : public Tree<TrinomialTree> {
        class Branching;
      public:
        enum Branches { branches = 3 };
        TrinomialTree(const boost::shared_ptr<StochasticProcess1D>& process,
                      const TimeGrid& timeGrid,
                      bool isPositive = false);
        Real dx(Size i) const { return dx_[i]; }
        const TimeGrid& timeGrid() const { return timeGrid_; }

        Size size(Size i) const;
        Real underlying(Size i, Size index) const;
        Size descendant(Size i, Size index, Size branch) const;
        Real probability(Size i, Size index, Size branch) const;

      protected:
        std::vector<Branching> branchings_;
        Real x0_;
        std::vector<Real> dx_;
        TimeGrid timeGrid_;

      private:
        /* Branching scheme for a trinomial node.  Each node has three
           descendants, with the middle branch linked to the node
           which is closest to the expectation of the variable. */
        class Branching {
          public:
            Branching();
            Size descendant(Size index, Size branch) const;
            Real probability(Size index, Size branch) const;
            Size size() const;
            Integer jMin() const;
            Integer jMax() const;
            void add(Integer k, Real p1, Real p2, Real p3);
          private:
            std::vector<Integer> k_;
            std::vector<std::vector<Real> > probs_;
            Integer kMin_, jMin_, kMax_, jMax_;
        };
    };

    // inline definitions

    inline Size TrinomialTree::size(Size i) const {
        return i==0 ? 1 : branchings_[i-1].size();
    }

    inline Real TrinomialTree::underlying(Size i, Size index) const {
        if (i==0)
            return x0_;
        else
            return x0_ + (branchings_[i-1].jMin() +
                          static_cast<Real>(index))*dx(i);
    }

    inline Size TrinomialTree::descendant(Size i, Size index,
                                          Size branch) const {
        return branchings_[i].descendant(index, branch);
    }

    inline Real TrinomialTree::probability(Size i, Size j, Size b) const {
        return branchings_[i].probability(j, b);
    }

    inline TrinomialTree::Branching::Branching()
    : probs_(3), kMin_(QL_MAX_INTEGER), jMin_(QL_MAX_INTEGER),
                 kMax_(QL_MIN_INTEGER), jMax_(QL_MIN_INTEGER) {}

    inline Size TrinomialTree::Branching::descendant(Size index,
                                                     Size branch) const {
        return k_[index] - jMin_ - 1 + branch;
    }

    inline Real TrinomialTree::Branching::probability(Size index,
                                                      Size branch) const {
        return probs_[branch][index];
    }

    inline Size TrinomialTree::Branching::size() const {
        return jMax_ - jMin_ + 1;
    }

    inline Integer TrinomialTree::Branching::jMin() const {
        return jMin_;
    }

    inline Integer TrinomialTree::Branching::jMax() const {
        return jMax_;
    }

    inline void TrinomialTree::Branching::add(Integer k,
                                              Real p1, Real p2, Real p3) {
        // store
        k_.push_back(k);
        probs_[0].push_back(p1);
        probs_[1].push_back(p2);
        probs_[2].push_back(p3);
        // maintain invariants
        kMin_ = std::min(kMin_, k);
        jMin_ = kMin_ - 1;
        kMax_ = std::max(kMax_, k);
        jMax_ = kMax_ + 1;
    }

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

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

#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    inline TrinomialTree::TrinomialTree(
                        const boost::shared_ptr<StochasticProcess1D>& process,
                        const TimeGrid& timeGrid,
                        bool isPositive)
    : Tree<TrinomialTree>(timeGrid.size()), dx_(1, 0.0), timeGrid_(timeGrid) {
        x0_ = process->x0();

        Size nTimeSteps = timeGrid.size() - 1;
        Integer jMin = 0;
        Integer jMax = 0;

        for (Size i=0; i<nTimeSteps; i++) {
            Time t = timeGrid[i];
            Time dt = timeGrid.dt(i);

            //Variance must be independent of x
            Real v2 = process->variance(t, 0.0, dt);
            Volatility v = std::sqrt(v2);
            dx_.push_back(v*std::sqrt(3.0));

            Branching branching;
            for (Integer j=jMin; j<=jMax; j++) {
                Real x = x0_ + j*dx_[i];
                Real m = process->expectation(t, x, dt);
                Integer temp = Integer(std::floor((m-x0_)/dx_[i+1] + 0.5));

                if (isPositive) {
                    while (x0_+(temp-1)*dx_[i+1]<=0) {
                        temp++;
                    }
                }

                Real e = m - (x0_ + temp*dx_[i+1]);
                Real e2 = e*e;
                Real e3 = e*std::sqrt(3.0);

                Real p1 = (1.0 + e2/v2 - e3/v)/6.0;
                Real p2 = (2.0 - e2/v2)/3.0;
                Real p3 = (1.0 + e2/v2 + e3/v)/6.0;

                branching.add(temp, p1, p2, p3);
            }
            branchings_.push_back(branching);

            jMin = branching.jMin();
            jMax = branching.jMax();
        }
    }

}


#endif
