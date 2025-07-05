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

#include <ql/indexes/indexmanager.hpp>
#include <ql/cashflows/fxlinkedcashflow.hpp>

namespace QuantLib {

FXLinked::FXLinked(const Date& fxFixingDate, Real foreignAmount, ext::shared_ptr<FxIndex> fxIndex)
    : fxFixingDate_(fxFixingDate), foreignAmount_(foreignAmount), fxIndex_(fxIndex) {}

AverageFXLinked::AverageFXLinked(const std::vector<Date>& fxFixingDates, Real foreignAmount,
                                 ext::shared_ptr<FxIndex> fxIndex, const bool inverted)
    : fxFixingDates_(fxFixingDates), foreignAmount_(foreignAmount), fxIndex_(fxIndex), inverted_(inverted) {}

Real FXLinked::fxRate() const {
    return fxIndex_->fixing(fxFixingDate_);
}

Real AverageFXLinked::fxRate() const {
    Real fx = 0;
    for (auto const& d: fxFixingDates_)
        fx += inverted_ ? 1.0 / fxIndex_->fixing(d) : fxIndex_->fixing(d);
    fx /= fxFixingDates_.size();
    return inverted_ ? 1.0 / fx : fx;
}

FXLinkedCashFlow::FXLinkedCashFlow(const Date& cashFlowDate, const Date& fxFixingDate, Real foreignAmount,
                                   ext::shared_ptr<FxIndex> fxIndex)
    : FXLinked(fxFixingDate, foreignAmount, fxIndex), cashFlowDate_(cashFlowDate) {
    registerWith(FXLinked::fxIndex());
}

AverageFXLinkedCashFlow::AverageFXLinkedCashFlow(const Date& cashFlowDate, const std::vector<Date>& fxFixingDates,
                                                 Real foreignAmount, ext::shared_ptr<FxIndex> fxIndex,
                                                 const bool inverted)
    : AverageFXLinked(fxFixingDates, foreignAmount, fxIndex, inverted), cashFlowDate_(cashFlowDate) {
    registerWith(AverageFXLinked::fxIndex());
}

ext::shared_ptr<FXLinked> FXLinkedCashFlow::clone(ext::shared_ptr<FxIndex> fxIndex) {
    return ext::make_shared<FXLinkedCashFlow>(date(), fxFixingDate(), foreignAmount(), fxIndex);
}

ext::shared_ptr<AverageFXLinked> AverageFXLinkedCashFlow::clone(ext::shared_ptr<FxIndex> fxIndex) {
    return ext::make_shared<AverageFXLinkedCashFlow>(date(), fxFixingDates(), foreignAmount(), fxIndex);
}

std::map<Date, Real> AverageFXLinkedCashFlow::fixings() const {
    std::map<Date, Real> result;
    for (auto const& d : fxFixingDates_)
        result[d] = inverted_ ? 1.0 / fxIndex_->fixing(d) : fxIndex_->fixing(d);
    return result;
}

} // namespace QuantLib
