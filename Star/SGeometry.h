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
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/algorithms/envelope.hpp>

#include <Star/SMathFwd.h>

BOOST_GEOMETRY_REGISTER_POINT_3D(Star::Vector3f, float, boost::geometry::cs::cartesian, x(), y(), z())

namespace Star {

using Point3f = Vector3f;
using Box3f = boost::geometry::model::box<Point3f>;
using Polygon3f = boost::geometry::model::polygon<Point3f, false, true>;
using Linestring3f = boost::geometry::model::linestring<Point3f>;

}
