#include "SvgImageLoader.h"

#include "SvgDrawable.h"
#include "AUI/Common/AColor.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/GL/Vao.h"
#include "AUI/IO/ByteBufferInputStream.h"
#include "AUI/Render/Render.h"
#include "AUI/Util/Triangles.h"
#include "AUI/Xml/AXml.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Util/Tokenizer.h"

#include <glm/gtx/norm.hpp>
#include "AUI/Platform/AWindow.h"

SvgImageLoader::SvgImageLoader()
{
}

bool SvgImageLoader::matches(_<AByteBuffer> buffer)
{
	char buf[8];
	buffer->get(buf, 5);
		
	return memcmp(buf, "<?xml", 5) == 0;
}


class PolygonVisitor: public IXmlEntityVisitor
{
private:
	_<SvgDrawable> mDrawable;
	AColor mColor;
	AStringVector mPoints;
	
public:
	PolygonVisitor(const _<SvgDrawable>& drawable)
		: mDrawable(drawable)
	{
	}
	virtual ~PolygonVisitor()
	{
		if (mPoints.size() > 2) {
			AVector<glm::vec2> vertices;

			glm::vec2 offset(mDrawable->mViewBox.x, mDrawable->mViewBox.y);
			glm::vec2 delta = glm::vec2(mDrawable->mViewBox.z, mDrawable->mViewBox.w) - offset;

			for (auto& point : mPoints)
			{
				auto s = point.split(',');
				if (s.size() == 2)
				{
					vertices << ((glm::vec2(s[0].toFloat(), s[1].toFloat()) - offset) / delta);
				}
			}
			Triangles::optimize(vertices);
			vertices << vertices.front();
			AVector<unsigned> indices;
			Triangles::triangulate(vertices, indices);
						
			auto vao = _new<GL::Vao>();
			vao->insert(0, vertices);
			vao->indices(indices);
			auto color = mColor;
			
			mDrawable->mDrawList << [vao, color]()
			{
				RenderHints::PushColor c;
				Render::instance().setColor(color);
				Render::instance().uploadToShader();
				vao->bind();
				vao->draw(GL_TRIANGLES);
			};
		}
	}

	void visitAttribute(const AString& name, const AString& value) override
	{
		if (name == "fill")
		{
			mColor = AColor(value);
		} else if (name == "points")
		{
			mPoints = value.split(' ');
		}
	}
	_<IXmlEntityVisitor> visitEntity(const AString& entityName) override
	{
		return nullptr;
	}
	void visitTextEntity(const AString& entity) override
	{
		
	}
};
class RectVisitor: public IXmlEntityVisitor
{
private:
	_<SvgDrawable> mDrawable;
	AColor mColor;
	glm::vec4 mPoints;
	
public:
	RectVisitor(const _<SvgDrawable>& drawable)
		: mDrawable(drawable)
	{
	}
	virtual ~RectVisitor()
	{
		auto color = mColor;

		auto vao = _new<GL::Vao>();
		glm::vec2 offset(mDrawable->mViewBox.x, mDrawable->mViewBox.y);
		glm::vec2 delta = glm::vec2(mDrawable->mViewBox.z, mDrawable->mViewBox.w) - offset;
		
		vao->insert(0, {
			(glm::vec2{mPoints.x, mPoints.y } - offset) / delta,
			(glm::vec2{mPoints.x, mPoints.y + mPoints.w } - offset) / delta,
			(glm::vec2{mPoints.x + mPoints.z, mPoints.y } - offset) / delta,
			(glm::vec2{mPoints.x + mPoints.z, mPoints.y + mPoints.w } - offset) / delta,
			});

		vao->indices({ 0, 1, 2, 2, 1, 3 });
		
		mDrawable->mDrawList << [color, vao]()
		{
			RenderHints::PushColor c;
			Render::instance().setColor(color);
			Render::instance().uploadToShader();
			vao->bind();
			vao->draw(GL_TRIANGLES);
		};
	}

	void visitAttribute(const AString& name, const AString& value) override
	{
		if (name == "fill")
		{
			mColor = AColor(value);
		} else if (name == "x")
		{
			mPoints.x = value.toFloat();
		} else if (name == "y")
		{
			mPoints.y = value.toFloat();
		} else if (name == "width")
		{
			mPoints.z = value.toFloat();
		} else if (name == "height")
		{
			mPoints.w = value.toFloat();
		}
	}
	_<IXmlEntityVisitor> visitEntity(const AString& entityName) override
	{
		return nullptr;
	}
	void visitTextEntity(const AString& entity) override
	{
		
	}
};

class PathVisitor: public IXmlEntityVisitor
{
private:
	_<SvgDrawable> mDrawable;
	AColor mColor;
	AString mPathData;
	
public:
	PathVisitor(const _<SvgDrawable>& drawable)
		: mDrawable(drawable)
	{
	}
	virtual ~PathVisitor()
	{
		if (!mPathData.empty())
		{
			AVector<glm::vec2> vertices;
			try {				
				Tokenizer t(mPathData);

				glm::vec2 penPos;
				unsigned pathBeginIndex = 0;
				
				auto pushPos = [&]()
				{
					if (vertices.empty() || glm::distance2(vertices.back(), penPos) > 0.0001f)
					{
						vertices << penPos;
					}
				};
				
				for (;;) {
					switch (t.readChar())
					{
					// ������������� � �����
					case 'M': 
						penPos = t.readVec2();
						break;
					case 'm':
						penPos += t.readVec2();
						break;

					// ������������� � ����� � ���������� �����
					case 'L':
						pushPos();
						penPos = t.readVec2();
						pushPos();
						break;
					case 'l':
						pushPos();
						penPos += t.readVec2();
						pushPos();
						break;
						
					// ������������� � ����� � ���������� ����� (�����������)
					case 'V':
						pushPos();
						penPos.y = t.readFloat();
						pushPos();
						break;
					case 'v':
						pushPos();
						penPos.y += t.readFloat();
						pushPos();
						break;
						
					// ������������� � ����� � ���������� ����� (�������������)
					case 'H':
						pushPos();
						penPos.x = t.readFloat();
						pushPos();
						break;
					case 'h':
						pushPos();
						penPos.x += t.readFloat();
						pushPos();
						break;

					case 'z':
					case 'Z':
						//vertices << vertices.at(pathBeginIndex);
						pathBeginIndex = vertices.size();
						break;
					}
				}
			} catch (...) {}
			if (!vertices.empty())
			{
				auto offset = glm::vec2(mDrawable->mViewBox.x, mDrawable->mViewBox.y);
				auto delta = glm::vec2(mDrawable->mViewBox.z, mDrawable->mViewBox.w) - offset;
				for (auto& v : vertices)
				{
					v = v / delta + offset;
				}
				
				AVector<unsigned> indices;
				Triangles::optimize(vertices);
				Triangles::triangulate(vertices, indices);
				auto vao = _new<GL::Vao>();
				vao->insert(0, vertices);
				vao->indices(indices);
				auto color = mColor;

				mDrawable->mDrawList << [vao, color]()
				{
					RenderHints::PushColor c;
					Render::instance().setColor(color);
					Render::instance().uploadToShader();
					vao->bind();
					vao->draw(GL_TRIANGLES);
				};
			}
		}
	}

	void visitAttribute(const AString& name, const AString& value) override
	{
		if (name == "fill")
		{
			mColor = AColor(value);
		} else if (name == "d")
		{
			mPathData = value;
		}
	}
	_<IXmlEntityVisitor> visitEntity(const AString& entityName) override
	{
		return nullptr;
	}
	void visitTextEntity(const AString& entity) override
	{
		
	}
};

class SvgEntityVisitor: public IXmlEntityVisitor
{
	_<SvgDrawable> mDrawable;
	
public:
	SvgEntityVisitor(const _<SvgDrawable>& drawable)
		: mDrawable(drawable)
	{
	}

	void visitAttribute(const AString& name, const AString& value) override
	{
		if (name == "viewBox")
		{
			auto viewBox = value.split(' ');
			if (viewBox.size() == 4)
			{
				mDrawable->mViewBox = {
					viewBox[0].toFloat(),
					viewBox[1].toFloat(),
					viewBox[2].toFloat(),
					viewBox[3].toFloat(),
				};
			}
		}
	}
	
	_<IXmlEntityVisitor> visitEntity(const AString& entityName) override
	{
		if (entityName == "polygon")
		{
			return _new<PolygonVisitor>(mDrawable);
		}
		if (entityName == "path")
		{
			return _new<PathVisitor>(mDrawable);
		}
		if (entityName == "rect")
		{
			return _new<RectVisitor>(mDrawable);
		}
		return nullptr;
	}
	void visitTextEntity(const AString& entity) override {}
};

class SvgDocumentVisitor : public IXmlDocumentVisitor
{
private:
	_<SvgDrawable> mDrawable;
	
public:
	SvgDocumentVisitor(const _<SvgDrawable>& drawable)
		: mDrawable(drawable)
	{
	}

	_<IXmlHeaderVisitor> visitHeader() override
	{
		return nullptr;
	}
	_<IXmlEntityVisitor> visitEntity(const AString& entityName) override
	{
		if (entityName == "svg")
		{
			return _new<SvgEntityVisitor>(mDrawable);
		}
		return nullptr;
	}
};


_<IDrawable> SvgImageLoader::getDrawable(_<AByteBuffer> buffer)
{
	auto context = AWindow::current()->acquireTemporaryRenderingContext();
	auto drawable = _new<SvgDrawable>();
	AXml::read(_new<ByteBufferInputStream>(buffer), _new<SvgDocumentVisitor>(drawable));
	
	return drawable;
}

_<AImage> SvgImageLoader::getRasterImage(_<AByteBuffer> buffer) {
    return nullptr;
}
