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
#include "../ml/featurestack.h"
#include "../ml/featuretype.h"

#include <cstddef>
#include <vector>

using namespace nshogi;

namespace {

void mlApiMakeFeatureVectorWithOption(float* Dest, const nshogi_state_t* CState,
                                      const nshogi_state_config_t* CStateConfig,
                                      const nshogi_feature_type_t* FeatureTypes,
                                      int NumFeatures, int ChannelsFirst) {
    std::vector<ml::FeatureType> Types((std::size_t)NumFeatures);
    for (int I = 0; I < NumFeatures; ++I) {
        Types[(std::size_t)I] = static_cast<ml::FeatureType>(FeatureTypes[I]);
    }

    const core::State* State = reinterpret_cast<const core::State*>(CState);
    const core::StateConfig* StateConfig =
        reinterpret_cast<const core::StateConfig*>(CStateConfig);

    ml::FeatureStackRuntime FSR(Types, *State, *StateConfig);

    if (ChannelsFirst == 0) {
        FSR.extract<core::IterateOrder::ESWN, false>(Dest);
    } else {
        FSR.extract<core::IterateOrder::ESWN, true>(Dest);
    }
}

void mlApiMakeFeatureVector(float* Dest, const nshogi_state_t* CState,
                            const nshogi_state_config_t* CStateConfig,
                            const nshogi_feature_type_t* FeatureTypes,
                            int NumFeatures) {
    return mlApiMakeFeatureVectorWithOption(Dest, CState, CStateConfig,
                                            FeatureTypes, NumFeatures, 1);
}

int mlApiMoveToIndexWithOption(const nshogi_state_t* CState,
                               nshogi_move_t CMove, int ChannelsFirst) {
    const core::State* State = reinterpret_cast<const core::State*>(CState);
    core::Move32 Move = core::Move32::fromValue(CMove);

    if (ChannelsFirst == 0) {
        return static_cast<int>(
            ml::getMoveIndex<false>(State->getSideToMove(), Move));
    } else {
        return static_cast<int>(
            ml::getMoveIndex<true>(State->getSideToMove(), Move));
    }
}

int mlApiMoveToIndex(const nshogi_state_t* CState, nshogi_move_t CMove) {
    return mlApiMoveToIndexWithOption(CState, CMove, 1);
}

} // namespace

nshogi_ml_api_t* c_api::ml::getApi() {
    static bool Initialized = false;
    static nshogi_ml_api_t Api;

    if (!Initialized) {
        Api.makeFeatureVector = mlApiMakeFeatureVector;
        Api.makeFeatureVectorWithOption = mlApiMakeFeatureVectorWithOption;
        Api.moveToIndex = mlApiMoveToIndex;
        Api.moveToIndexWithOption = mlApiMoveToIndexWithOption;

        Initialized = true;
    }

    return &Api;
}
