/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers

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

/*! \file optionletstripper.hpp
    \brief optionlet (caplet/floorlet) volatility stripper
*/

#ifndef quantlib_optionletstripper_hpp
#define quantlib_optionletstripper_hpp

#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>
#include <ql/termstructures/volatility/capfloor/capfloortermvolsurface.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    class IborIndex;

    /*! StrippedOptionletBase specialization. It's up to derived
        classes to implement LazyObject::performCalculations
    */
    class OptionletStripper : public StrippedOptionletBase {
      public:
        //! \name StrippedOptionletBase interface
        //@{
        const std::vector<Rate>& optionletStrikes(Size i) const;
        const std::vector<Volatility>& optionletVolatilities(Size i) const;

        const std::vector<Date>& optionletFixingDates() const;
        const std::vector<Time>& optionletFixingTimes() const;
        Size optionletMaturities() const;

        const std::vector<Rate>& atmOptionletRates() const;

        DayCounter dayCounter() const;
        Calendar calendar() const;
        Natural settlementDays() const;
        BusinessDayConvention businessDayConvention() const;
        //@}

        const std::vector<Period>& optionletFixingTenors() const;
        const std::vector<Date>& optionletPaymentDates() const;
        const std::vector<Time>& optionletAccrualPeriods() const;
        boost::shared_ptr<CapFloorTermVolSurface> termVolSurface() const;
        boost::shared_ptr<IborIndex> iborIndex() const;
        Real displacement() const;
        VolatilityType volatilityType() const;

      protected:
        OptionletStripper(const boost::shared_ptr< CapFloorTermVolSurface > &,
                          const boost::shared_ptr< IborIndex > &iborIndex_,
                          const Handle< YieldTermStructure > &discount =
                              Handle< YieldTermStructure >(),
                          const VolatilityType type = ShiftedLognormal,
                          const Real displacement = 0.0);
        boost::shared_ptr<CapFloorTermVolSurface> termVolSurface_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Handle<YieldTermStructure> discount_;
        Size nStrikes_;
        Size nOptionletTenors_;

        mutable std::vector<std::vector<Rate> > optionletStrikes_;
        mutable std::vector<std::vector<Volatility> > optionletVolatilities_;

        mutable std::vector<Time> optionletTimes_;
        mutable std::vector<Date> optionletDates_;
        std::vector<Period> optionletTenors_;
        mutable std::vector<Rate> atmOptionletRate_;
        mutable std::vector<Date> optionletPaymentDates_;
        mutable std::vector<Time> optionletAccrualPeriods_;

        std::vector<Period> capFloorLengths_;
        const VolatilityType volatilityType_;
        const Real displacement_;
    };

}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers

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

#include <ql/indexes/iborindex.hpp>

using std::vector;

namespace QuantLib {

inline OptionletStripper::OptionletStripper(
    const boost::shared_ptr< CapFloorTermVolSurface > &termVolSurface,
    const boost::shared_ptr< IborIndex > &iborIndex,
    const Handle< YieldTermStructure > &discount, const VolatilityType type,
    const Real displacement)
    : termVolSurface_(termVolSurface), iborIndex_(iborIndex),
      discount_(discount), nStrikes_(termVolSurface->strikes().size()),
      volatilityType_(type), displacement_(displacement) {

        if (volatilityType_ == Normal) {
            QL_REQUIRE(displacement_ == 0.0,
                       "non-null displacement is not allowed with Normal model");
        }

        registerWith(termVolSurface);
        registerWith(iborIndex_);
        registerWith(discount_);
        registerWith(Settings::instance().evaluationDate());

        Period indexTenor = iborIndex_->tenor();
        Period maxCapFloorTenor = termVolSurface->optionTenors().back();

        // optionlet tenors and capFloor lengths
        optionletTenors_.push_back(indexTenor);
        capFloorLengths_.push_back(optionletTenors_.back()+indexTenor);
        QL_REQUIRE(maxCapFloorTenor>=capFloorLengths_.back(),
                   "too short (" << maxCapFloorTenor <<
                   ") capfloor term vol termVolSurface");
        Period nextCapFloorLength = capFloorLengths_.back()+indexTenor;
        while (nextCapFloorLength<=maxCapFloorTenor) {
            optionletTenors_.push_back(capFloorLengths_.back());
            capFloorLengths_.push_back(nextCapFloorLength);
            nextCapFloorLength += indexTenor;
        }
        nOptionletTenors_ = optionletTenors_.size();
        
        optionletVolatilities_ =
            vector<vector<Volatility> >(nOptionletTenors_, 
                                        vector<Volatility>(nStrikes_));
        optionletStrikes_ = vector<vector<Rate> >(nOptionletTenors_,
                                                  termVolSurface->strikes());
        optionletDates_ = vector<Date>(nOptionletTenors_);
        optionletTimes_ = vector<Time>(nOptionletTenors_);
        atmOptionletRate_ = vector<Rate>(nOptionletTenors_);
        optionletPaymentDates_ = vector<Date>(nOptionletTenors_);
        optionletAccrualPeriods_ = vector<Time>(nOptionletTenors_);
    }

    inline const vector<Rate>& OptionletStripper::optionletStrikes(Size i) const {
        calculate();
        QL_REQUIRE(i<optionletStrikes_.size(),
                   "index (" << i <<
                   ") must be less than optionletStrikes size (" <<
                   optionletStrikes_.size() << ")");
        return optionletStrikes_[i];
    }   

    const vector<Volatility>&
    inline OptionletStripper::optionletVolatilities(Size i) const {
        calculate();
        QL_REQUIRE(i<optionletVolatilities_.size(),
                   "index (" << i <<
                   ") must be less than optionletVolatilities size (" <<
                   optionletVolatilities_.size() << ")");
        return optionletVolatilities_[i];
    }   

    inline const vector<Period>& OptionletStripper::optionletFixingTenors() const {
        return optionletTenors_;
    }

    inline const vector<Date>& OptionletStripper::optionletFixingDates() const {
        calculate();
        return optionletDates_;
    }
      
    inline const vector<Time>& OptionletStripper::optionletFixingTimes() const {
        calculate();
        return optionletTimes_;
    }
     
    inline Size OptionletStripper::optionletMaturities() const {
        return optionletTenors_.size();
    }

    inline const vector<Date>& OptionletStripper::optionletPaymentDates() const {
        calculate();
        return optionletPaymentDates_;
    }  

    inline const vector<Time>& OptionletStripper::optionletAccrualPeriods() const {
        calculate();
        return optionletAccrualPeriods_;
    }

    inline const vector<Rate>& OptionletStripper::atmOptionletRates() const {
        calculate();
        return atmOptionletRate_;
    }
    

    inline DayCounter OptionletStripper::dayCounter() const {
        return termVolSurface_->dayCounter();
    }

    inline Calendar OptionletStripper::calendar() const {
        return termVolSurface_->calendar();
    }

    inline Natural OptionletStripper::settlementDays() const {
        return termVolSurface_->settlementDays();
    }

    inline BusinessDayConvention OptionletStripper::businessDayConvention() const {
        return termVolSurface_->businessDayConvention();
    }

    inline boost::shared_ptr<CapFloorTermVolSurface>
    OptionletStripper::termVolSurface() const {
        return termVolSurface_;
    }

    inline boost::shared_ptr<IborIndex> OptionletStripper::iborIndex() const {
        return iborIndex_;
    }

    inline Real OptionletStripper::displacement() const {
        return displacement_;
    }

    inline VolatilityType OptionletStripper::volatilityType() const {
        return volatilityType_;
    }

}


#endif
