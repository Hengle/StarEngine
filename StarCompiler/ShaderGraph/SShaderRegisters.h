// Copyright (C) 2019-2020 star.engine at outlook dot com
//
// This file is part of StarEngine
//
// StarEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// StarEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with StarEngine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <StarCompiler/ShaderGraph/SShaderTypes.h>

namespace Star::Graphics::Render::Shader {

class ShaderRegister {
public:
    uint32_t get(RootAccessEnum stage, DescriptorType type, uint32_t space);
    uint32_t increase(RootAccessEnum stage, DescriptorType type, uint32_t space, uint32_t count = 1);
    void reserveAll(DescriptorType type, uint32_t space, uint32_t count = 1);
private:
    using SpaceMap = boost::container::flat_map<uint32_t, uint32_t>;
    using TypeMap = boost::container::flat_map<DescriptorType, SpaceMap>;
    boost::container::flat_map<RootAccessEnum, TypeMap> mSlots;
};

}
