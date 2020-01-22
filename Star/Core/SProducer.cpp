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

#include "SProducer.h"
#include "SManagerPrivate.h"

namespace Star::Core {

Producer::Producer() = default;
Producer::~Producer() = default;

void Producer::deliver(const Resource& resource, void* pointer, bool async) const {
    if (async) {
        Manager::instance().async_created(resource, pointer);
    } else {
        Manager::instance().sync_created(resource, pointer);
    }
}

void Producer::registerProducer(const ResourceType& tag) {
    Manager::instance().registerProducer(tag, this);
}

}
