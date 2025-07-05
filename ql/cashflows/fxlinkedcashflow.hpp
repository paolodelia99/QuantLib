/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

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


/*! \file ql/cashflows/fxlinkedcashflow.hpp
    \brief An FX linked cashflow

        \ingroup cashflows
*/

#ifndef quantlib_fx_linked_cashflow_hpp
#define quantlib_fx_linked_cashflow_hpp

#include <ql/cashflow.hpp>
#include <ql/handle.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/quote.hpp>
#include <ql/time/date.hpp>
#include <ql/indexes/fxindex.hpp>

namespace QuantLib {

//! Base class for FX Linked cashflows
class FXLinked {
public:
    FXLinked(const Date& fixingDate, Real foreignAmount, ext::shared_ptr<FxIndex> fxIndex);
    virtual ~FXLinked() {}
    Date fxFixingDate() const { return fxFixingDate_; }
    Real foreignAmount() const { return foreignAmount_; }
    const ext::shared_ptr<FxIndex>& fxIndex() const { return fxIndex_; }
    Real fxRate() const;

    virtual ext::shared_ptr<FXLinked> clone(ext::shared_ptr<FxIndex> fxIndex) = 0;

protected:
    Date fxFixingDate_;
    Real foreignAmount_;
    ext::shared_ptr<FxIndex> fxIndex_;
};

class AverageFXLinked {
public:
    // if inverted = true, the arithmetic averaging is done over the inverted fixings and the reciprocal of the result
    // is taken to compute the rate
    AverageFXLinked(const std::vector<Date>& fixingDates, Real foreignAmount, ext::shared_ptr<FxIndex> fxIndex,
                    const bool inverted = false);
    virtual ~AverageFXLinked() {}
    const std::vector<Date>& fxFixingDates() const { return fxFixingDates_; }
    Real foreignAmount() const { return foreignAmount_; }
    const ext::shared_ptr<FxIndex>& fxIndex() const { return fxIndex_; }
    Real fxRate() const;

    virtual ext::shared_ptr<AverageFXLinked> clone(ext::shared_ptr<FxIndex> fxIndex) = 0;

protected:
    std::vector<Date> fxFixingDates_;
    Real foreignAmount_;
    ext::shared_ptr<FxIndex> fxIndex_;
    bool inverted_ = false;
};

//! FX Linked cash-flow
/*!
 * Cashflow of Domestic currency where the amount is fx linked
 * to some fixed foreign amount.
 *
 * For example: a JPY flow based off 1M USD, if the USDJPY FX rate
 * is 123.45 then the JPY amount is 123.45 M JPY.
 *
 * FXLinkedCashFlow checks the FX fixing date against the eval date
 *
 * For future fixings (date > eval) this class calculates the FX Fwd
 * rate (using the provided FX Spot rate and FOR and DOM yield curves)
 *
 * For todays fixing (date = eval) this class converts the foreign
 * amount using the provided FX Spot rate.
 *
 * For previous fixings (date < eval) this class checks the QuantLib
 * IndexManager to get the FX fixing at which the foreign rate should be
 * converted at. The name of the index is a parameter to the constructor.
 *
 * This is not a lazy object.

     \ingroup cashflows
 */
class FXLinkedCashFlow : public CashFlow, public FXLinked {
public:
    FXLinkedCashFlow(const Date& cashFlowDate, const Date& fixingDate, Real foreignAmount,
                     ext::shared_ptr<FxIndex> fxIndex);

    //! \name CashFlow interface
    //@{
    Date date() const override { return cashFlowDate_; }
    Real amount() const override {
        calculate();
        return amount_;
    }
    //@}

    //@}
    //! \name LazyObject interface
    //@{
    void performCalculations() const override { amount_ = foreignAmount() * fxRate(); }
    //@}

    //! \name Visitability
    //@{
    void accept(AcyclicVisitor&) override;
    //@}

    //! \name FXLinked interface
    //@{
    ext::shared_ptr<FXLinked> clone(ext::shared_ptr<FxIndex> fxIndex) override;
    //@}

private:
    Date cashFlowDate_;
    mutable Real amount_;
};

// inline definitions

inline void FXLinkedCashFlow::accept(AcyclicVisitor& v) {
    Visitor<FXLinkedCashFlow>* v1 = dynamic_cast<Visitor<FXLinkedCashFlow>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        CashFlow::accept(v);
}

//! Average FX Linked cash-flow
/*!
 * Cashflow of Domestic currency where the amount is fx linked
 * to some fixed foreign amount.
 *
 * Difference to the FX Linked cash-flow: The FX rate is an 
 * arithmetic average across observation dates.
 *
 * This is not a lazy object.

 \ingroup cashflows
 */
class AverageFXLinkedCashFlow : public CashFlow, public AverageFXLinked {
public:
    AverageFXLinkedCashFlow(const Date& cashFlowDate, const std::vector<Date>& fixingDates, Real foreignAmount,
                            ext::shared_ptr<FxIndex> fxIndex, const bool inverted = false);

    //! \name CashFlow interface
    //@{
    Date date() const override { return cashFlowDate_; }
    Real amount() const override {
        calculate();
        return amount_;
    }
    //@}

    //! \name Visitability
    //@{
    void accept(AcyclicVisitor&) override;
    //@}

    //@}
    //! \name LazyObject interface
    //@{
    void performCalculations() const override { amount_ = foreignAmount() * fxRate(); }
    //@}

    //! \name FXLinked interface
    //@{
    ext::shared_ptr<AverageFXLinked> clone(ext::shared_ptr<FxIndex> fxIndex) override;
    //@}

    // get single fixing dates and values
    std::map<Date, Real> fixings() const;

private:
    Date cashFlowDate_;
    mutable Real amount_;
};

inline void AverageFXLinkedCashFlow::accept(AcyclicVisitor& v) {
    Visitor<AverageFXLinkedCashFlow>* v1 = dynamic_cast<Visitor<AverageFXLinkedCashFlow>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        CashFlow::accept(v);
}
} // namespace QuantLib

#endif
