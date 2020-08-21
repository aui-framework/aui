#include "RenderHints.h"
#include "AUI/GL/gl.h"
#include "Render.h"

RenderHints::PushAntialiasing::PushAntialiasing()
{	
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

RenderHints::PushAntialiasing::~PushAntialiasing()
{
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);
}

RenderHints::PushMatrix::PushMatrix()
{
	mStored = Render::instance().getTransform();
}

RenderHints::PushMatrix::~PushMatrix()
{
	Render::instance().setTransformForced(mStored);
}

RenderHints::PushColor::PushColor()
{
	mStored = Render::instance().getColor();
}

RenderHints::PushColor::~PushColor()
{
	Render::instance().setColorForced(mStored);
}
