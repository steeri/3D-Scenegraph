#pragma once

// Frameworks
#include <glog/logging.h>
#include "../Include/GL/glew.h"
#include <GL/glut.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include "../Include/GLShaderManager.h"
#include "../Include/GLBatch.h"

#include "InputManager.h"

/**
 * \brief This class initializes OpenGL with Glut and provides some utility methods and lifecycle methods which are useful for all kind of applications. It doesn't draw anything or handle any keys.
 */
class BaseApplication {
	private:
		void RenderBase();
		void SetupOpenGL();
		void CheckOpenGLError();

	protected:
		/** \brief InputManager which we use for this application. */
		InputManager Input;
		/** \brief ShaderManager which we use for this application */
		GLShaderManager ShaderManager;
		/** \brief Batch which we use for this application */
		GLBatch Batch;
		/** \brief always holds the actual number of frames we processed */
		int FrameCounter;
		/** \brief Points to the most recently created instance of this class. This is necessary so that the Static Callbacks for Resize and Render are able to call the methods of the active instance */
		static BaseApplication *Instance;

	public:
		BaseApplication(string Title, int WindowWidth, int WindowHeight);
		void Start();
		virtual int GetFrameCounter();
		/** \brief This function is called only once before the first update happens to initialize everything we want */
		virtual void Init() = 0;
		/** \brief This function is called always before Render. You can do calculations for the next frame in this function */
		virtual void Update() = 0;
		/** \brief This function is called everytime we draw a frame. You need to draw everything you want to be on the next frame */
		virtual void Render() = 0;
		static void RenderS();
		/** \brief This function is called whenever we change the size of the window (which includes one call when it is created the first time) */
		virtual void Resize(int NewWidth, int NewHeight) = 0;
		static void ResizeS(int NewWidth, int NewHeight);
};