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

/*! \file floatingratefxlinkednotionalcoupon.hpp
    \brief Coupon paying a Libor-type index but with an FX linked notional
*/

#ifndef quantlib_floating_rate_fx_linked_notional_coupon_hpp
#define quantlib_floating_rate_fx_linked_notional_coupon_hpp

#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/cashflows/fxlinkedcashflow.hpp>

namespace QuantLib {

//! %Coupon paying a Libor-type index on an fx-linked nominal
class FloatingRateFXLinkedNotionalCoupon : public FloatingRateCoupon, public FXLinked {
public:
    //! FloatingRateFXLinkedNotionalCoupon
    FloatingRateFXLinkedNotionalCoupon(const Date& fxFixingDate, Real foreignAmount, ext::shared_ptr<FxIndex> fxIndex,
                                       const ext::shared_ptr<FloatingRateCoupon>& underlying)
        : FloatingRateCoupon(underlying->date(), Null<Real>(), underlying->accrualStartDate(),
                             underlying->accrualEndDate(), underlying->fixingDays(), underlying->fixingDate(), underlying->index(),
                             underlying->gearing(), underlying->spread(), underlying->referencePeriodStart(),
                             underlying->referencePeriodEnd(), underlying->dayCounter(), underlying->isInArrears()),
          FXLinked(fxFixingDate, foreignAmount, fxIndex), underlying_(underlying) {
        fixingDays_ = underlying->fixingDays() == Null<Natural>()
                          ? (underlying->index() ? underlying->index()->fixingDays() : 0)
                                                                  : underlying->fixingDays();
        registerWith(FXLinked::fxIndex());
        registerWith(underlying_);
    }

    //! \name FXLinked interface
    //@{
    ext::shared_ptr<FXLinked> clone(ext::shared_ptr<FxIndex> fxIndex) override;
    //@}

    //! \name Obverver interface
    //@{
    void deepUpdate() override {
        update();
        underlying_->deepUpdate();
    }
    //@}

    //! \name LazyObject interface
    //@{
    void performCalculations() const override { rate_ = underlying_->rate(); }
    void alwaysForwardNotifications() override {
	LazyObject::alwaysForwardNotifications();
        underlying_->alwaysForwardNotifications();
    }
    //@}
    //! \name Coupon interface
    //@{
    Rate nominal() const override { return foreignAmount() * fxRate(); }
    //@}

    //! \name FloatingRateCoupon interface
    //@{
    Rate indexFixing() const override { return underlying_->indexFixing(); } // might be overwritten in underlying
    void setPricer(const ext::shared_ptr<FloatingRateCouponPricer>& p) override { underlying_->setPricer(p); }
    //@}

    //! \name Visitability
    //@{
    void accept(AcyclicVisitor&) override;
    //@}

    //! more inspectors
    ext::shared_ptr<FloatingRateCoupon> underlying() const { return underlying_; }

private:
    const ext::shared_ptr<FloatingRateCoupon> underlying_;
};

// inline definitions
inline void FloatingRateFXLinkedNotionalCoupon::accept(AcyclicVisitor& v) {
    Visitor<FloatingRateFXLinkedNotionalCoupon>* v1 = dynamic_cast<Visitor<FloatingRateFXLinkedNotionalCoupon>*>(&v);
    if (v1 != 0)
        v1->visit(*this);
    else
        FloatingRateCoupon::accept(v);
}

inline ext::shared_ptr<FXLinked> FloatingRateFXLinkedNotionalCoupon::clone(ext::shared_ptr<FxIndex> fxIndex) {
    return ext::make_shared<FloatingRateFXLinkedNotionalCoupon>(fxFixingDate(), foreignAmount(), fxIndex,
                                                                  underlying());
}

} // namespace QuantLib

#endif
