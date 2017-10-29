/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Peter Caspers
 Copyright (C) 2015 Michael von den Driesch

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

/*! \file strippedoptionlet.hpp
*/

#ifndef quantlib_strippedoptionlet_hpp
#define quantlib_strippedoptionlet_hpp

#include <ql/termstructures/volatility/optionlet/strippedoptionletbase.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    /*! Helper class to wrap in a StrippedOptionletBase object a matrix of
        exogenously calculated optionlet (i.e. caplet/floorlet) volatilities
        (a.k.a. forward-forward volatilities).
    */
    class StrippedOptionlet : public StrippedOptionletBase {
      public:
        StrippedOptionlet(Natural settlementDays, const Calendar &calendar,
                          BusinessDayConvention bdc,
                          const boost::shared_ptr< IborIndex > &iborIndex,
                          const std::vector< Date > &optionletDates,
                          const std::vector< Rate > &strikes,
                          const std::vector< std::vector< Handle< Quote > > > &,
                          const DayCounter &dc,
                          VolatilityType type = ShiftedLognormal,
                          Real displacement = 0.0);
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
        VolatilityType volatilityType() const;
        Real displacement() const;

      private:
        void checkInputs() const;
        void registerWithMarketData();
        void performCalculations() const;

        Calendar calendar_;
        Natural settlementDays_;
        BusinessDayConvention businessDayConvention_;
        DayCounter dc_;
        boost::shared_ptr<IborIndex> iborIndex_;
        VolatilityType type_;
        Real displacement_;

        Size nOptionletDates_;
        std::vector<Date> optionletDates_;
        std::vector<Time> optionletTimes_;
        mutable std::vector<Rate> optionletAtmRates_;
        std::vector<std::vector<Rate> > optionletStrikes_;
        Size nStrikes_;

        std::vector<std::vector<Handle<Quote> > > optionletVolQuotes_;
        mutable std::vector<std::vector<Volatility> > optionletVolatilities_;
    };

}

/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Ferdinando Ametrano
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

#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/utilities/dataformatters.hpp>


namespace QuantLib {

inline StrippedOptionlet::StrippedOptionlet(
    Natural settlementDays, const Calendar &calendar, BusinessDayConvention bdc,
    const boost::shared_ptr< IborIndex > &iborIndex,
    const std::vector< Date > &optionletDates, const std::vector< Rate > &strikes,
    const std::vector< std::vector< Handle< Quote > > > &v, const DayCounter &dc,
    VolatilityType type, Real displacement)
    : calendar_(calendar), settlementDays_(settlementDays),
      businessDayConvention_(bdc), dc_(dc), iborIndex_(iborIndex), type_(type),
      displacement_(displacement), nOptionletDates_(optionletDates.size()),
      optionletDates_(optionletDates), optionletTimes_(nOptionletDates_),
      optionletAtmRates_(nOptionletDates_),
      optionletStrikes_(nOptionletDates_, strikes), nStrikes_(strikes.size()),
      optionletVolQuotes_(v),
      optionletVolatilities_(nOptionletDates_, std::vector< Volatility >(nStrikes_))

{
        checkInputs();
        registerWith(Settings::instance().evaluationDate());
        registerWithMarketData();

        Date refDate = calendar.advance(Settings::instance().evaluationDate(),
                                        settlementDays, Days);

        for (Size i=0; i<nOptionletDates_; ++i)
            optionletTimes_[i] = dc_.yearFraction(refDate, optionletDates_[i]);
    }

    inline void StrippedOptionlet::checkInputs() const {

        QL_REQUIRE(!optionletDates_.empty(), "empty optionlet tenor vector");
        QL_REQUIRE(nOptionletDates_==optionletVolQuotes_.size(),
                   "mismatch between number of option tenors (" <<
                   nOptionletDates_ << ") and number of volatility rows (" <<
                   optionletVolQuotes_.size() << ")");
        QL_REQUIRE(optionletDates_[0]>Settings::instance().evaluationDate(),
                   "first option date (" << optionletDates_[0] << ") is in the past");
        for (Size i=1; i<nOptionletDates_; ++i)
            QL_REQUIRE(optionletDates_[i]>optionletDates_[i-1],
                       "non increasing option dates: " << io::ordinal(i) <<
                       " is " << optionletDates_[i-1] << ", " <<
                       io::ordinal(i+1) << " is " << optionletDates_[i]);

        QL_REQUIRE(nStrikes_==optionletVolQuotes_[0].size(),
                   "mismatch between strikes(" << optionletStrikes_[0].size() <<
                   ") and vol columns (" << optionletVolQuotes_[0].size() << ")");
        for (Size j=1; j<nStrikes_; ++j)
            QL_REQUIRE(optionletStrikes_[0][j-1]<optionletStrikes_[0][j],
                       "non increasing strikes: " << io::ordinal(j) <<
                       " is " << io::rate(optionletStrikes_[0][j-1]) << ", " <<
                       io::ordinal(j+1) << " is " << io::rate(optionletStrikes_[0][j]));
    }

    inline void StrippedOptionlet::registerWithMarketData()
    {
        for (Size i=0; i<nOptionletDates_; ++i)
            for (Size j=0; j<nStrikes_; ++j)
                registerWith(optionletVolQuotes_[i][j]);
    }

    inline void StrippedOptionlet::performCalculations() const {
        for (Size i=0; i<nOptionletDates_; ++i)
          for (Size j=0; j<nStrikes_; ++j)
            optionletVolatilities_[i][j] = optionletVolQuotes_[i][j]->value();
    }

    inline const std::vector<Rate>& StrippedOptionlet::optionletStrikes(Size i) const{
        QL_REQUIRE(i<optionletStrikes_.size(),
                   "index (" << i <<
                   ") must be less than optionletStrikes size (" <<
                   optionletStrikes_.size() << ")");
        return optionletStrikes_[i];
    }

    inline const std::vector<Volatility>&
    StrippedOptionlet::optionletVolatilities(Size i) const{
        calculate();
        QL_REQUIRE(i<optionletVolatilities_.size(),
                   "index (" << i <<
                   ") must be less than optionletVolatilities size (" <<
                   optionletVolatilities_.size() << ")");
        return optionletVolatilities_[i];
    }

    inline const std::vector<Date>& StrippedOptionlet::optionletFixingDates() const {
        calculate();
        return optionletDates_;
    }

    inline const std::vector<Time>& StrippedOptionlet::optionletFixingTimes() const {
        calculate();
        return optionletTimes_;
    }

    inline Size StrippedOptionlet::optionletMaturities() const {
        return nOptionletDates_;
    }

    inline const std::vector<Time>& StrippedOptionlet::atmOptionletRates() const {
        calculate();
        for (Size i=0; i<nOptionletDates_; ++i)
            optionletAtmRates_[i] = iborIndex_->fixing(optionletDates_[i], true);
        return optionletAtmRates_;
    }

    inline DayCounter StrippedOptionlet::dayCounter() const {
        return dc_;
    }

    inline Calendar StrippedOptionlet::calendar() const {
        return calendar_;
    }

    inline Natural StrippedOptionlet::settlementDays() const {
        return settlementDays_;
    }

    inline BusinessDayConvention StrippedOptionlet::businessDayConvention() const {
        return businessDayConvention_;
    }

    inline VolatilityType StrippedOptionlet::volatilityType() const {
        return type_;
    }

    inline Real StrippedOptionlet::displacement() const {
        return displacement_;
    }

}


#endif
