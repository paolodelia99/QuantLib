/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Quaternion Risk Management Ltd

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fxvannavolgasmilesection.hpp
    \brief FX smile section assuming a strike/volatility space using vanna volga method
*/

#ifndef quantlib_fx_vanna_volga_smile_section_hpp
#define quantlib_fx_vanna_volga_smile_section_hpp

#include <ql/experimental/barrieroption/vannavolgainterpolation.hpp>
#include <ql/pricingengines/blackdeltacalculator.hpp>
#include <ql/termstructures/volatility/equityfx/fxsmilesection.hpp>

namespace QuantLib {

/*! Vanna Volga Smile section
 *
 *  Consistent Pricing of FX Options
 *  Castagna & Mercurio (2006)
 *  http://papers.ssrn.com/sol3/papers.cfm?abstract_id=873788
 */
class VannaVolgaSmileSection : public FxSmileSection {
public:
    //! \name Constructors
    //@{
    /*! \brief Construct a Vanna Volga smile section
     *
     * \param spot       Spot FX rate
     * \param rd         Domestic risk-free rate
     * \param rf         Foreign risk-free rate
     * \param t          Time to maturity
     * \param atmVol     ATM volatility
     * \param rr         Risk reversal (call vol - put vol)
     * \param bf         Butterfly (call vol + put vol - 2 * ATM vol)
     * \param firstApprox Use first approximation when calculating the volatility
     * \param atmType    ATM type
     * \param deltaType  Delta type
     * \param delta      Delta level for RR and BF strikes
     */
    VannaVolgaSmileSection(Real spot, Real rd, Real rf, Time t, Volatility atmVol, Volatility rr, Volatility bf,
                           bool firstApprox = false,
                           const DeltaVolQuote::AtmType& atmType = DeltaVolQuote::AtmType::AtmDeltaNeutral,
                           const DeltaVolQuote::DeltaType& deltaType = DeltaVolQuote::DeltaType::Spot,
                           const Real delta = 0.25);
    //@}

    //! \name Inspectors
    //@{
    //! ATM strike
    Real k_atm() const { return k_atm_; }
    //! Call strike at the given delta
    Real k_c() const { return k_c_; }
    //! Put strike at the given delta
    Real k_p() const { return k_p_; }
    //! ATM volatility
    Volatility vol_atm() const { return atmVol_; }
    //! Call volatility at the given delta
    Volatility vol_c() const { return vol_c_; }
    //! Put volatility at the given delta
    Volatility vol_p() const { return vol_p_; }
    //@}

    //! \name FxSmileSection interface
    //@{
    Volatility volatility(Real strike) const override;
    //}@

private:
    Real d1(Real x) const;
    Real d2(Real x) const;

    Real k_atm_, k_c_, k_p_;
    Volatility atmVol_, rr_, bf_;
    Volatility vol_c_, vol_p_;
    bool firstApprox_;
};

}

#endif
