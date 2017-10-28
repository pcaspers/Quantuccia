/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2006, 2011 Ferdinando Ametrano
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file swap.hpp
    \brief Interest rate swap
*/

#ifndef quantlib_swap_hpp
#define quantlib_swap_hpp

#include <ql/instrument.hpp>
#include <ql/cashflow.hpp>

namespace QuantLib {

    //! Interest rate swap
    /*! The cash flows belonging to the first leg are paid;
        the ones belonging to the second leg are received.

        \ingroup instruments
    */
    class Swap : public Instrument {
      public:
        class arguments;
        class results;
        class engine;
        //! \name Constructors
        //@{
        /*! The cash flows belonging to the first leg are paid;
            the ones belonging to the second leg are received.
        */
        Swap(const Leg& firstLeg,
             const Leg& secondLeg);
        /*! Multi leg constructor. */
        Swap(const std::vector<Leg>& legs,
             const std::vector<bool>& payer);
        //@}
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        void setupArguments(PricingEngine::arguments*) const;
        void fetchResults(const PricingEngine::results*) const;
        //@}
        //! \name Additional interface
        //@{
        Date startDate() const;
        Date maturityDate() const;
        Real legBPS(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg# " << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(legBPS_[j] != Null<Real>(), "result not available");
            return legBPS_[j];
        }
        Real legNPV(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(legNPV_[j] != Null<Real>(), "result not available");
            return legNPV_[j];
        }
        DiscountFactor startDiscounts(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(startDiscounts_[j] != Null<Real>(), "result not available");
            return startDiscounts_[j];
        }
        DiscountFactor endDiscounts(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            calculate();
            QL_REQUIRE(endDiscounts_[j] != Null<Real>(), "result not available");
            return endDiscounts_[j];
        }
        DiscountFactor npvDateDiscount() const {
            calculate();
            QL_REQUIRE(npvDateDiscount_ != Null<Real>(), "result not available");
            return npvDateDiscount_;
        }
        const Leg& leg(Size j) const {
            QL_REQUIRE(j<legs_.size(), "leg #" << j << " doesn't exist!");
            return legs_[j];
        }
        //@}
      protected:
        //! \name Constructors
        //@{
        /*! This constructor can be used by derived classes that will
            build their legs themselves.
        */
        Swap(Size legs);
        //@}
        //! \name Instrument interface
        //@{
        void setupExpired() const;
        //@}
        // data members
        std::vector<Leg> legs_;
        std::vector<Real> payer_;
        mutable std::vector<Real> legNPV_;
        mutable std::vector<Real> legBPS_;
        mutable std::vector<DiscountFactor> startDiscounts_, endDiscounts_;
        mutable DiscountFactor npvDateDiscount_;
    };


    class Swap::arguments : public virtual PricingEngine::arguments {
      public:
        std::vector<Leg> legs;
        std::vector<Real> payer;
        void validate() const;
    };

    class Swap::results : public Instrument::results {
      public:
        std::vector<Real> legNPV;
        std::vector<Real> legBPS;
        std::vector<DiscountFactor> startDiscounts, endDiscounts;
        DiscountFactor npvDateDiscount;
        void reset();
    };

    class Swap::engine : public GenericEngine<Swap::arguments,
                                              Swap::results> {};

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2011 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007, 2008 StatPro Italia srl

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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    inline Swap::Swap(const Leg& firstLeg,
               const Leg& secondLeg)
    : legs_(2), payer_(2),
      legNPV_(2, 0.0), legBPS_(2, 0.0),
      startDiscounts_(2, 0.0), endDiscounts_(2, 0.0),
      npvDateDiscount_(0.0) {
        legs_[0] = firstLeg;
        legs_[1] = secondLeg;
        payer_[0] = -1.0;
        payer_[1] =  1.0;
        for (Leg::iterator i = legs_[0].begin(); i!= legs_[0].end(); ++i)
            registerWith(*i);
        for (Leg::iterator i = legs_[1].begin(); i!= legs_[1].end(); ++i)
            registerWith(*i);
    }

    inline Swap::Swap(const std::vector<Leg>& legs,
               const std::vector<bool>& payer)
    : legs_(legs), payer_(legs.size(), 1.0),
      legNPV_(legs.size(), 0.0), legBPS_(legs.size(), 0.0),
      startDiscounts_(legs.size(), 0.0), endDiscounts_(legs.size(), 0.0),
      npvDateDiscount_(0.0) {
        QL_REQUIRE(payer.size()==legs_.size(),
                   "size mismatch between payer (" << payer.size() <<
                   ") and legs (" << legs_.size() << ")");
        for (Size j=0; j<legs_.size(); ++j) {
            if (payer[j]) payer_[j]=-1.0;
            for (Leg::iterator i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                registerWith(*i);
        }
    }

    inline Swap::Swap(Size legs)
    : legs_(legs), payer_(legs),
      legNPV_(legs, 0.0), legBPS_(legs, 0.0),
      startDiscounts_(legs, 0.0), endDiscounts_(legs, 0.0),
      npvDateDiscount_(0.0) {}


    inline bool Swap::isExpired() const {
        for (Size j=0; j<legs_.size(); ++j) {
            Leg::const_iterator i; 
            for (i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                if (!(*i)->hasOccurred())
                    return false;
        }
        return true;
    }

   inline  void Swap::setupExpired() const {
        Instrument::setupExpired();
        std::fill(legBPS_.begin(), legBPS_.end(), 0.0);
        std::fill(legNPV_.begin(), legNPV_.end(), 0.0);
        std::fill(startDiscounts_.begin(), startDiscounts_.end(), 0.0);
        std::fill(endDiscounts_.begin(), endDiscounts_.end(), 0.0);
        npvDateDiscount_ = 0.0;
    }

    inline void Swap::setupArguments(PricingEngine::arguments* args) const {
        Swap::arguments* arguments = dynamic_cast<Swap::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->legs = legs_;
        arguments->payer = payer_;
    }

    inline void Swap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const Swap::results* results = dynamic_cast<const Swap::results*>(r);
        QL_REQUIRE(results != 0, "wrong result type");

        if (!results->legNPV.empty()) {
            QL_REQUIRE(results->legNPV.size() == legNPV_.size(),
                       "wrong number of leg NPV returned");
            legNPV_ = results->legNPV;
        } else {
            std::fill(legNPV_.begin(), legNPV_.end(), Null<Real>());
        }

        if (!results->legBPS.empty()) {
            QL_REQUIRE(results->legBPS.size() == legBPS_.size(),
                       "wrong number of leg BPS returned");
            legBPS_ = results->legBPS;
        } else {
            std::fill(legBPS_.begin(), legBPS_.end(), Null<Real>());
        }

        if (!results->startDiscounts.empty()) {
            QL_REQUIRE(results->startDiscounts.size() == startDiscounts_.size(),
                       "wrong number of leg start discounts returned");
            startDiscounts_ = results->startDiscounts;
        } else {
            std::fill(startDiscounts_.begin(), startDiscounts_.end(),
                                                    Null<DiscountFactor>());
        }

        if (!results->endDiscounts.empty()) {
            QL_REQUIRE(results->endDiscounts.size() == endDiscounts_.size(),
                       "wrong number of leg end discounts returned");
            endDiscounts_ = results->endDiscounts;
        } else {
            std::fill(endDiscounts_.begin(), endDiscounts_.end(),
                                                    Null<DiscountFactor>());
        }

        if (results->npvDateDiscount != Null<DiscountFactor>()) {
            npvDateDiscount_ = results->npvDateDiscount;
        } else {
            npvDateDiscount_ = Null<DiscountFactor>();
        }
    }


    inline Date Swap::startDate() const {
        QL_REQUIRE(!legs_.empty(), "no legs given");
        Date d = CashFlows::startDate(legs_[0]);
        for (Size j=1; j<legs_.size(); ++j)
            d = std::min(d, CashFlows::startDate(legs_[j]));
        return d;
    }

    inline Date Swap::maturityDate() const {
        QL_REQUIRE(!legs_.empty(), "no legs given");
        Date d = CashFlows::maturityDate(legs_[0]);
        for (Size j=1; j<legs_.size(); ++j)
            d = std::max(d, CashFlows::maturityDate(legs_[j]));
        return d;
    }


    inline void Swap::arguments::validate() const {
        QL_REQUIRE(legs.size() == payer.size(),
                   "number of legs and multipliers differ");
    }

    inline void Swap::results::reset() {
        Instrument::results::reset();
        legNPV.clear();
        legBPS.clear();
        startDiscounts.clear();
        endDiscounts.clear();
        npvDateDiscount = Null<DiscountFactor>();
    }

}

#endif
