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

/*! \file fxblackvolsurface.hpp
    \brief FX Black volatility surface that incorporates an FxSmile
*/

#ifndef quantlib_fx_black_vol_surface_hpp
#define quantlib_fx_black_vol_surface_hpp

#include <ql/quotes/deltavolquote.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/fxvannavolgasmilesection.hpp>

namespace QuantLib {

//! Fx Black volatility surface
/*! This class defines and Abstract interfacees for Fx Volatility surfaces that are calculating time/strike dependent Black volatilities
*/
class FxBlackVolatilitySurface : public BlackVolatilityTermStructure {
public:
    /*! \brief Construct an FX Black volatility surface
     *
     * \param referenceDate   Reference date for the volatility surface
     * \param dates           Vector of dates for the volatility quotes
     * \param atmVols         ATM volatilities for each date
     * \param rr              Risk reversal quotes for each date
     * \param bf              Butterfly quotes for each date
     * \param dayCounter      Day counter for time conversion
     * \param cal             Calendar for date handling
     * \param fxSpot          Handle to the FX spot rate
     * \param domesticTS      Handle to the domestic yield term structure
     * \param foreignTS       Handle to the foreign yield term structure
     * \param requireMonotoneVariance If true, throws if variance is not monotone
     * \param atmType         ATM type for short tenors
     * \param deltaType       Delta type for short tenors
     * \param delta           Delta level for RR and BF quotes
     * \param switchTenor     Tenor at which to switch to long-term settings
     * \param longTermAtmType ATM type for tenors beyond switchTenor
     * \param longTermDeltaType Delta type for tenors beyond switchTenor
     */
    FxBlackVolatilitySurface(const Date& referenceDate, const std::vector<Date>& dates,
                             const std::vector<Volatility>& atmVols, const std::vector<Volatility>& rr,
                             const std::vector<Volatility>& bf, const DayCounter& dayCounter, const Calendar& cal,
                             const Handle<Quote>& fxSpot, const Handle<YieldTermStructure>& domesticTS,
                             const Handle<YieldTermStructure>& foreignTS, bool requireMonotoneVariance = true,
                             const DeltaVolQuote::AtmType atmType = DeltaVolQuote::AtmType::AtmDeltaNeutral,
                             const DeltaVolQuote::DeltaType deltaType = DeltaVolQuote::DeltaType::Spot,
                             const Real delta = 0.25, const Period& switchTenor = 0 * Days,
                             const DeltaVolQuote::AtmType longTermAtmType = DeltaVolQuote::AtmType::AtmDeltaNeutral,
                             const DeltaVolQuote::DeltaType longTermDeltaType = DeltaVolQuote::DeltaType::Spot);

    //! \name TermStructure interface
    //@{
    DayCounter dayCounter() const override { return dayCounter_; }
    Date maxDate() const override { return maxDate_; }
    //@}
    //! \name VolatilityTermStructure interface
    //@{
    Real minStrike() const override { return 0; } // we allow 0 for ATM vols
    Real maxStrike() const override { return QL_MAX_REAL; }
    //@}
    //! \name Visitability
    //@{
    virtual void accept(AcyclicVisitor&) override;
    //@}
    //! Return an FxSmile for the time t
    /*! Note the smile does not observe the spot or YTS handles, it will
     *  not update when they change
     */
    QuantLib::ext::shared_ptr<FxSmileSection> blackVolSmile(Time t) const;

protected:
    virtual Volatility blackVolImpl(Time t, Real strike) const override;

    //! this must be implemented.
    virtual QuantLib::ext::shared_ptr<FxSmileSection> blackVolSmileImpl(Real spot, Real rd, Real rf, Time t, Volatility atm,
                                                                Volatility rr, Volatility bf) const = 0;

    std::vector<Time> times_;
    DayCounter dayCounter_;
    Handle<Quote> fxSpot_;
    Handle<YieldTermStructure> domesticTS_;
    Handle<YieldTermStructure> foreignTS_;
    BlackVarianceCurve atmCurve_;
    std::vector<Volatility> rr_;
    std::vector<Volatility> bf_;
    DeltaVolQuote::AtmType atmType_;
    DeltaVolQuote::DeltaType deltaType_;
    Real delta_;
    Period switchTenor_;
    DeltaVolQuote::AtmType longTermAtmType_;
    DeltaVolQuote::DeltaType longTermDeltaType_;
    Interpolation rrCurve_;
    Interpolation bfCurve_;
    Date maxDate_;
};

// inline definitions
inline void FxBlackVolatilitySurface::accept(AcyclicVisitor& v) {
    Visitor<FxBlackVolatilitySurface>* v1 = dynamic_cast<Visitor<FxBlackVolatilitySurface>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        BlackVolatilityTermStructure::accept(v);
}

//! Fx Black vanna volga volatility surface
/*! This class calculates time/strike dependent Black volatilities
     using the Vanna Volga method

     This surface extends FxBlackVolatilitySurface by constructing the smile
     using the Vanna Volga interpolation method, which ensures that ATM,
     risk reversal and butterfly quotes are reproduced exactly.

     \param firstApprox If true, use the first approximation for FxVannaVolgaSmile volatilities
*/
class FxBlackVannaVolgaVolatilitySurface : public FxBlackVolatilitySurface {
public:
    /*! \brief Construct an FX Black Vanna Volga volatility surface
     *
     * \param refDate              Reference date for the volatility surface
     * \param dates               Vector of dates for the volatility quotes
     * \param atmVols             ATM volatilities for each date
     * \param rr                  Risk reversal quotes for each date
     * \param bf                  Butterfly quotes for each date
     * \param dc                  Day counter for time conversion
     * \param cal                 Calendar for date handling
     * \param fx                  Handle to the FX spot rate
     * \param dom                 Handle to the domestic yield term structure
     * \param fore                Handle to the foreign yield term structure
     * \param requireMonotoneVariance If true, throws if variance is not monotone
     * \param firstApprox         Use first-order approximation
     * \param atmType            ATM type for short tenors
     * \param deltaType          Delta type for short tenors
     * \param delta              Delta level for RR and BF quotes
     * \param switchTenor        Tenor at which to switch to long-term settings
     * \param longTermAtmType    ATM type for tenors beyond switchTenor
     * \param longTermDeltaType  Delta type for tenors beyond switchTenor
     */
    FxBlackVannaVolgaVolatilitySurface(
        const Date& refDate, const std::vector<Date>& dates, const std::vector<Volatility>& atmVols,
        const std::vector<Volatility>& rr, const std::vector<Volatility>& bf, const DayCounter& dc, const Calendar& cal,
        const Handle<Quote>& fx, const Handle<YieldTermStructure>& dom, const Handle<YieldTermStructure>& fore,
        bool requireMonotoneVariance = true, const bool firstApprox = false,
        const DeltaVolQuote::AtmType atmType = DeltaVolQuote::AtmType::AtmDeltaNeutral,
        const DeltaVolQuote::DeltaType deltaType = DeltaVolQuote::DeltaType::Spot, const Real delta = 0.25,
        const Period& switchTenor = 0 * Days,
        const DeltaVolQuote::AtmType longTermAtmType = DeltaVolQuote::AtmType::AtmDeltaNeutral,
        const DeltaVolQuote::DeltaType longTermDeltaType = DeltaVolQuote::DeltaType::Spot)
        : FxBlackVolatilitySurface(refDate, dates, atmVols, rr, bf, dc, cal, fx, dom, fore, requireMonotoneVariance,
                                   atmType, deltaType, delta, switchTenor, longTermAtmType, longTermDeltaType),
          firstApprox_(firstApprox) {}

protected:
    bool firstApprox_;
    virtual QuantLib::ext::shared_ptr<FxSmileSection> blackVolSmileImpl(Real spot, Real rd, Real rf, Time t, Volatility atm,
                                                                Volatility rr, Volatility bf) const override;
};
} // namespace QuantLib

#endif
