#pragma once

#include <glm/glm.hpp>
#include "AUI/Common/AVector.h"
#include "AUI/Common/SharedPtr.h"

namespace Util {
	typedef int dim;
	class Rect {
	public:
		dim x, y, width, height;
		Rect(dim x, dim y, dim width, dim height);
		Rect() = default;
		bool hasPoint(dim x, dim y) const;
		bool collidesWith(const Rect& rect);
	};
	class TexturePacker_Lol {
	protected:
		dim side;
		AVector<Rect> mRects;
		bool noCollision(const Rect& r);
		bool check(Rect& res, Rect r);
		bool allocateRect(Rect& r, dim width, dim height);
	};

	template<class T>
	class TexturePacker : public TexturePacker_Lol {
	private:
		AVector<_<glm::vec4>> uvs;
	protected:
		_<glm::vec4> insert(_<T> data, dim width, dim height)
		{
			if (side == 0)
				resize(data, 64);
			Rect r(0, 0, 0, 0);
			while (!allocateRect(r, width, height))
			{
				resize(data, side * 2);
			}
			this->onInsert(data, r.x, r.y);
			_<glm::vec4> f = _new<glm::vec4>(float(r.x) / float(side), float(r.y) / float(side), float(r.x + r.width) / float(side), float(r.y + r.height) / float(side));
			uvs.push_back(f);
			return f;
		}

		virtual void onResize(_<T> data, dim side) = 0;
		virtual void onInsert(_<T> data, const dim& x, const dim& y) = 0;

	public:
		TexturePacker()
		{
			side = 0;
		}
		virtual ~TexturePacker() {

		}

		virtual _<glm::vec4> insert(_<T> data) = 0;

		void resize(_<T> data, dim size)
		{
			if (side) {
				float k = float(size) / side;
				for (auto& i : uvs) {
					i->x /= k;
					i->y /= k;
					i->z /= k;
					i->a /= k;
				}
			}
			side = size;
			this->onResize(data, size);
		}
	};
}

