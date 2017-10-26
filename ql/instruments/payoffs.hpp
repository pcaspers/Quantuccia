/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2006, 2008 StatPro Italia srl
 Copyright (C) 2006 Chiara Fornarola

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

/*! \file payoffs.hpp
    \brief Payoffs for various options
*/

#ifndef quantlib_payoffs_hpp
#define quantlib_payoffs_hpp

#include <ql/option.hpp>
#include <ql/payoff.hpp>

namespace QuantLib {

    //! Dummy %payoff class
    class NullPayoff : public Payoff {
      public:
        //! \name Payoff interface
        //@{
        std::string name() const;
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    //! Intermediate class for put/call payoffs
    class TypePayoff : public Payoff {
      public:
        Option::Type optionType() const { return type_; };
        //! \name Payoff interface
        //@{
        std::string description() const;
        //@}
      protected:
        TypePayoff(Option::Type type) : type_(type) {}
        Option::Type type_;
    };

    ////! Intermediate class for payoffs based on a fixed strike
    //class StrikedPayoff : public Payoff {
    //  public:
    //    StrikedPayoff(Real strike) : strike_(strike) {}
    //    Real strike() const { return strike_; };
    //    //! \name Payoff interface
    //    //@{
    //    std::string description() const;
    //    //@}
    //  protected:
    //    Real strike_;
    //};

    //! %Payoff based on a floating strike
    class FloatingTypePayoff : public TypePayoff {
      public:
        FloatingTypePayoff(Option::Type type) : TypePayoff(type) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "FloatingType";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Intermediate class for payoffs based on a fixed strike
    class StrikedTypePayoff : public TypePayoff
                              //, public StrikedPayoff
    {
      public:
        //! \name Payoff interface
        //@{
        std::string description() const;
        //@}
        Real strike() const { return strike_; };
      protected:
        StrikedTypePayoff(Option::Type type,
                          Real strike)
        : TypePayoff(type), strike_(strike) {}
        Real strike_;
    };

    //! Plain-vanilla payoff
    class PlainVanillaPayoff : public StrikedTypePayoff {
      public:
        PlainVanillaPayoff(Option::Type type,
                           Real strike)
        : StrikedTypePayoff(type, strike) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "Vanilla";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! %Payoff with strike expressed as percentage
    class PercentageStrikePayoff : public StrikedTypePayoff {
      public:
        PercentageStrikePayoff(Option::Type type,
                               Real moneyness)
        : StrikedTypePayoff(type, moneyness) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "PercentageStrike";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    /*! Definitions of Binary path-independent payoffs used below,
        can be found in M. Rubinstein, E. Reiner:"Unscrambling The Binary Code", Risk, Vol.4 no.9,1991.
        (see: http://www.in-the-money.com/artandpap/Binary%20Options.doc)
    */

    //! Binary asset-or-nothing payoff
    class AssetOrNothingPayoff : public StrikedTypePayoff {
      public:
        AssetOrNothingPayoff(Option::Type type,
                             Real strike)
        : StrikedTypePayoff(type, strike) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "AssetOrNothing";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Binary cash-or-nothing payoff
    class CashOrNothingPayoff : public StrikedTypePayoff {
      public:
        CashOrNothingPayoff(Option::Type type,
                            Real strike,
                            Real cashPayoff)
        : StrikedTypePayoff(type, strike), cashPayoff_(cashPayoff) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "CashOrNothing";}
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real cashPayoff() const { return cashPayoff_;}
      protected:
        Real cashPayoff_;
    };

    //! Binary gap payoff
    /*! This payoff is equivalent to being a) long a PlainVanillaPayoff at
        the first strike (same Call/Put type) and b) short a
        CashOrNothingPayoff at the first strike (same Call/Put type) with
        cash payoff equal to the difference between the second and the first
        strike.
        \warning this payoff can be negative depending on the strikes
    */
    class GapPayoff : public StrikedTypePayoff {
      public:
        GapPayoff(Option::Type type,
                  Real strike,
                  Real secondStrike) // a.k.a. payoff strike
        : StrikedTypePayoff(type, strike), secondStrike_(secondStrike) {}
        //! \name Payoff interface
        //@{
        std::string name() const { return "Gap";}
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real secondStrike() const { return secondStrike_;}
      protected:
        Real secondStrike_;
    };

    //! Binary supershare and superfund payoffs

    //! Binary superfund payoff
    /*! Superfund sometimes also called "supershare", which can lead to ambiguity; within QuantLib
        the terms supershare and superfund are used consistently according to the definitions in
        Bloomberg OVX function's help pages.
    */
    /*! This payoff is equivalent to being (1/lowerstrike) a) long (short) an AssetOrNothing
        Call (Put) at the lower strike and b) short (long) an AssetOrNothing
        Call (Put) at the higher strike
    */
    class SuperFundPayoff : public StrikedTypePayoff {
      public:
        SuperFundPayoff(Real strike,
                        Real secondStrike)
        : StrikedTypePayoff(Option::Call, strike),
          secondStrike_(secondStrike) {
            QL_REQUIRE(strike>0.0,
                       "strike (" <<  strike << ") must be "
                       "positive");
            QL_REQUIRE(secondStrike>strike,
                       "second strike (" <<  secondStrike << ") must be "
                       "higher than first strike (" << strike << ")");
        }
        //! \name Payoff interface
        //@{
        std::string name() const { return "SuperFund";}
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real secondStrike() const { return secondStrike_;}
      protected:
        Real secondStrike_;
    };

    //! Binary supershare payoff
    class SuperSharePayoff : public StrikedTypePayoff {
      public:
        SuperSharePayoff(Real strike,
                         Real secondStrike,
                         Real cashPayoff)
        : StrikedTypePayoff(Option::Call, strike),
          secondStrike_(secondStrike),
          cashPayoff_(cashPayoff){
              QL_REQUIRE(secondStrike>strike,
              "second strike (" <<  secondStrike << ") must be "
              "higher than first strike (" << strike << ")");}

        //! \name Payoff interface
        //@{
        std::string name() const { return "SuperShare";}
        std::string description() const;
        Real operator()(Real price) const;
        virtual void accept(AcyclicVisitor&);
        //@}
        Real secondStrike() const { return secondStrike_;}
        Real cashPayoff() const { return cashPayoff_;}
      protected:
        Real secondStrike_;
        Real cashPayoff_;
    };
}


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Warren Chou
 Copyright (C) 2006, 2008 StatPro Italia srl
 Copyright (C) 2006 Chiara Fornarola

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

    inline std::string NullPayoff::name() const {
        return "Null";
    }

    inline std::string NullPayoff::description() const {
        return name();
    }

    inline Real NullPayoff::operator()(Real) const {
        QL_FAIL("dummy payoff given");
    }

    inline void NullPayoff::accept(AcyclicVisitor& v) {
        Visitor<NullPayoff>* v1 =
            dynamic_cast<Visitor<NullPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }



    inline std::string TypePayoff::description() const {
        std::ostringstream result;
        result << name() << " " << optionType();
        return result.str();
    }

    //std::string StrikedPayoff::description() const {
    //    std::ostringstream result;
    //    result << ", " << strike() << " strike";
    //    return result.str();
    //}

    inline Real FloatingTypePayoff::operator()(Real) const {
        QL_FAIL("floating payoff not handled");
    }

    inline std::string StrikedTypePayoff::description() const {
        std::ostringstream result;
        result << TypePayoff::description() << ", " <<
                  strike() << " strike";
        return result.str();
    }

    inline void FloatingTypePayoff::accept(AcyclicVisitor& v) {
        Visitor<FloatingTypePayoff>* v1 =
            dynamic_cast<Visitor<FloatingTypePayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    inline Real PlainVanillaPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return std::max<Real>(price-strike_,0.0);
          case Option::Put:
            return std::max<Real>(strike_-price,0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    inline void PlainVanillaPayoff::accept(AcyclicVisitor& v) {
        Visitor<PlainVanillaPayoff>* v1 =
            dynamic_cast<Visitor<PlainVanillaPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    inline Real PercentageStrikePayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return price*std::max<Real>(Real(1.0)-strike_,0.0);
          case Option::Put:
            return price*std::max<Real>(strike_-Real(1.0),0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    inline void PercentageStrikePayoff::accept(AcyclicVisitor& v) {
        Visitor<PercentageStrikePayoff>* v1 =
            dynamic_cast<Visitor<PercentageStrikePayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    inline Real AssetOrNothingPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? price : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    inline void AssetOrNothingPayoff::accept(AcyclicVisitor& v) {
        Visitor<AssetOrNothingPayoff>* v1 =
            dynamic_cast<Visitor<AssetOrNothingPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    inline std::string CashOrNothingPayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << cashPayoff() << " cash payoff";
        return result.str();
    }

    inline Real CashOrNothingPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ > 0.0 ? cashPayoff_ : 0.0);
          case Option::Put:
            return (strike_-price > 0.0 ? cashPayoff_ : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    inline void CashOrNothingPayoff::accept(AcyclicVisitor& v) {
        Visitor<CashOrNothingPayoff>* v1 =
            dynamic_cast<Visitor<CashOrNothingPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);}

    inline std::string GapPayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << secondStrike() << " strike payoff";
        return result.str();
    }

    inline Real GapPayoff::operator()(Real price) const {
        switch (type_) {
          case Option::Call:
            return (price-strike_ >= 0.0 ? price-secondStrike_ : 0.0);
          case Option::Put:
            return (strike_-price >= 0.0 ? secondStrike_-price : 0.0);
          default:
            QL_FAIL("unknown/illegal option type");
        }
    }

    inline void GapPayoff::accept(AcyclicVisitor& v) {
        Visitor<GapPayoff>* v1 =
            dynamic_cast<Visitor<GapPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

    inline Real SuperFundPayoff::operator()(Real price) const {
        return (price>=strike_ && price<secondStrike_) ? price/strike_ : 0.0;
    }

    inline void SuperFundPayoff::accept(AcyclicVisitor& v) {
        Visitor<SuperFundPayoff>* v1 =
            dynamic_cast<Visitor<SuperFundPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }
    inline std::string SuperSharePayoff::description() const {
        std::ostringstream result;
        result << StrikedTypePayoff::description() << ", " << secondStrike() << " second strike"<< ", " << cashPayoff() << " amount";
        return result.str();
    }

    inline Real SuperSharePayoff::operator()(Real price) const {
        return (price>=strike_ && price<secondStrike_) ? cashPayoff_ : 0.0;
    }

    inline void SuperSharePayoff::accept(AcyclicVisitor& v) {
        Visitor<SuperSharePayoff>* v1 =
            dynamic_cast<Visitor<SuperSharePayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Payoff::accept(v);
    }

}

#endif
