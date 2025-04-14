//
// Copyright (c) 2025 @nyashiki
//
// This software is licensed under the MIT license.
// For details, see the LICENSE file in the root of this repository.
//
// SPDX-License-Identifier: MIT
//

#include "ml.h"

#include "../ml/common.h"
#include "../ml/featuretype.h"
#include "../ml/featurestack.h"

#include <vector>
#include <cstddef>

using namespace nshogi;

namespace {

void mlApiMakeFeatureVector(
        float* Dest,
        const nshogi_state_t* CState,
        const nshogi_state_config_t* CStateConfig,
        const nshogi_feature_type_t* FeatureTypes,
        int NumFeatures) {
    std::vector<ml::FeatureType> Types((std::size_t)NumFeatures);
    for (int I = 0; I < NumFeatures; ++I) {
        Types[(std::size_t)I] = static_cast<ml::FeatureType>(FeatureTypes[I]);
    }

    const core::State* State = reinterpret_cast<const core::State*>(CState);
    const core::StateConfig* StateConfig = reinterpret_cast<const core::StateConfig*>(CStateConfig);

    ml::FeatureStackRuntime FSR(Types, *State, *StateConfig);
    FSR.extract<core::IterateOrder::ESWN>(Dest);
}

int mlApiMoveToIndex(const nshogi_state_t* CState, nshogi_move_t CMove) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    core::Move32 Move = core::Move32::fromValue(CMove);
    return static_cast<int>(ml::getMoveIndex(State->getSideToMove(), Move));
}

} // namespace

nshogi_ml_api_t* c_api::ml::getApi() {
    static bool Initialized = false;
    static nshogi_ml_api_t Api;

    if (!Initialized) {
        Api.makeFeatureVector = mlApiMakeFeatureVector;
        Api.moveToIndex = mlApiMoveToIndex;

        Initialized = true;
    }

    return &Api;
}
