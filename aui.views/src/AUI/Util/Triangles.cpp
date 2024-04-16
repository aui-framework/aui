// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "Triangles.h"
#include <glm/gtx/norm.hpp>



// -------------------------------------------------------------------------------
/** Compute the signed area of a triangle.
 *  The function accepts an unconstrained template parameter for use with
 *  both aiVector3D and aiVector2D, but generally ignores the third coordinate.*/
template <typename T>
inline double GetArea2D(const T& v1, const T& v2, const T& v3)
{
	return 0.5 * (v1.x * ((double)v3.y - v2.y) + v2.x * ((double)v1.y - v3.y) + v3.x * ((double)v2.y - v1.y));
}

// -------------------------------------------------------------------------------
/** Test if a given point p2 is on the left side of the line formed by p0-p1.
 *  The function accepts an unconstrained template parameter for use with
 *  both aiVector3D and glm::vec2, but generally ignores the third coordinate.*/
template <typename T>
inline bool OnLeftSideOfLine2D(const T& p0, const T& p1, const T& p2)
{
	return GetArea2D(p0, p2, p1) < 0;
}

// -------------------------------------------------------------------------------
/** Test if a given point is inside a given triangle in R2.
 * The function accepts an unconstrained template parameter for use with
 *  both aiVector3D and glm::vec2, but generally ignores the third coordinate.*/
template <typename T>
inline bool PointInTriangle2D(const T& p0, const T& p1, const T& p2, const T& pp)
{
	// Point in triangle test using baryzentric coordinates
	const glm::vec2 v0 = p1 - p0;
	const glm::vec2 v1 = p2 - p0;
	const glm::vec2 v2 = pp - p0;

	double dot00 = glm::dot(v0, v0);
	double dot01 = glm::dot(v0, v1);
	double dot02 = glm::dot(v0, v2);
	double dot11 = glm::dot(v1, v1);
	double dot12 = glm::dot(v1, v2);

	const double invDenom = 1 / (glm::dot(dot00, dot11) - glm::dot(dot01, dot01));
	dot11 = (glm::dot(dot11, dot02) - glm::dot(dot01, dot12)) * invDenom;
	dot00 = (glm::dot(dot00, dot12) - glm::dot(dot01, dot02)) * invDenom;

	return (dot11 > 0) && (dot00 > 0) && (dot11 + dot00 < 1);
}
void Triangles::triangulate(const AVector<glm::vec2>& vertices, AVector<unsigned>& indices)
{
	// https://github.com/assimp/assimp/blob/master/code/PostProcessing/TriangulateProcess.cpp
	// A polygon with more than 3 vertices can be either concave or convex.
	// Usually everything we're getting is convex and we could easily
	// triangulate by tri-fanning. However, LightWave is probably the only
	// modeling suite to make extensive use of highly concave, monster polygons ...
	// so we need to apply the full 'ear cutting' algorithm to get it right.

	int num = (int)vertices.size(), ear = 0, tmp, prev = num - 1, next = 0, max = num;

	std::vector<bool> done;
	done.resize(0x1000);

	//
	// FIXME: currently this is the slow O(kn) variant with a worst case
	// complexity of O(n^2) (I think). Can be done in O(n).
	while (num > 3)
	{
		// Find the next ear of the polygon
		int num_found = 0;
		for (ear = next;; prev = ear, ear = next)
		{
			// break after we looped two times without a positive match
			for (next = ear + 1; done[(next >= max ? next = 0 : next)]; ++next);
			if (next < ear)
			{
				if (++num_found == 2)
				{
					break;
				}
			}
			const glm::vec2 *pnt1 = &vertices[ear],
			                 *pnt0 = &vertices[prev],
			                 *pnt2 = &vertices[next];

			// Must be a convex point. Assuming ccw winding, it must be on the right of the line between p-1 and p+1.
			if (OnLeftSideOfLine2D(*pnt0, *pnt2, *pnt1))
			{
				continue;
			}

			// and no other point may be contained in this triangle
			for (tmp = 0; tmp < max; ++tmp)
			{
				// We need to compare the actual values because it's possible that multiple indexes in
				// the polygon are referring to the same position. concave_polygon.obj is a sample
				//
				// FIXME: Use 'epsiloned' comparisons instead? Due to numeric inaccuracies in
				// PointInTriangle() I'm guessing that it's actually possible to construct
				// input data that would cause us to end up with no ears. The problem is,
				// which epsilon? If we chose a too large value, we'd get wrong results
				const glm::vec2& vtmp = vertices[tmp];
				if (vtmp != *pnt1 && vtmp != *pnt2 && vtmp != *pnt0 && PointInTriangle2D(*pnt0, *pnt1, *pnt2, vtmp))
				{
					break;
				}
			}
			if (tmp != max)
			{
				continue;
			}

			// this vertex is an ear
			break;
		}
		if (num_found == 2)
		{
			// Due to the 'two ear theorem', every simple polygon with more than three points must
			// have 2 'ears'. Here's definitely something wrong ... but we don't give up yet.
			//

			// Instead we're continuing with the standard tri-fanning algorithm which we'd
			// use if we had only convex polygons. That's life.
			
			num = 0;
			break;

			/*curOut -= (max-num); // undo all previous work
			for (tmp = 0; tmp < max-2; ++tmp) {
				aiFace& nface = *curOut++;
				nface.mNumIndices = 3;
				if (!nface.mIndices)
					nface.mIndices = new unsigned int[3];
				nface.mIndices[0] = 0;
				nface.mIndices[1] = tmp+1;
				nface.mIndices[2] = tmp+2;
			}
			num = 0;
			break;*/
		}

		// setup indices for the new triangle ...
		indices << prev;
		indices << ear;
		indices << next;

		// exclude the ear from most further processing
		done[ear] = true;
		--num;
	}
	if (num > 0)
	{
		// We have three indices forming the last 'ear' remaining. Collect them.

		for (tmp = 0; done[tmp]; ++tmp)
			indices << tmp;

		for (++tmp; done[tmp]; ++tmp)
			indices << tmp;

		for (++tmp; done[tmp]; ++tmp)
			indices << tmp;
	}
}

void Triangles::optimize(AVector<glm::vec2>& vertices)
{
	for (auto i = vertices.begin(); i != vertices.end();)
	{
		bool erase = false;
		for (auto j = vertices.begin(); j != vertices.end(); ++j)
		{
			if (i != j && glm::distance2(*i, *j) < 0.000001f)
			{
				erase = true;
				break;
			}
		}
		if (erase)
		{
			i = vertices.erase(i);
		} else
		{
			++i;
		}
	}
}
