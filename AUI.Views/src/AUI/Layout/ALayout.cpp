#include "ALayout.h"

void ALayout::addView(const _<AView>& view)
{
	mViews << view;
}

void ALayout::removeView(const _<AView>& view)
{
	mViews.remove(view);
}

void ALayout::setSpacing(int spacing)
{
}
