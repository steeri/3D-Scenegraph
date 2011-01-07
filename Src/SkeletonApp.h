#pragma once

#include "Application/SimpleApplication.h"

/**
 * \brief Very simple Application which does nothing but use our framework to create a Window
 */
class SkeletonApp : public SimpleApplication {
	private:

	public:
		SkeletonApp();
		void Render(NodeVisitor* RenderVisitor, Light* l);
		void Init( Light* l, MeshFileLoader* MeshLoader);
};
