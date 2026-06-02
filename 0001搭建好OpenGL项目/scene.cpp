#include "scene.h"
#include "oglcontext.h"
#include "matrix4.h"
#include "quaternion.h"
#include <stdio.h>
#include <stdint.h>
#include "utils.h"
#include "RenderPass.h"
#include "fullscreenquad.h"
#define _4MB 4194304
static int sCanvasWidth, sCanvasHeight;
void Init(int inCanvasWidth, int inCanvasHeight) {
	sCanvasWidth = inCanvasWidth;
	sCanvasHeight = inCanvasHeight;
}
void RenderOneFrame(float inFrameTimeInSecond) {
	SCOPED_EVENT("Scene");
	glViewport(0,0,sCanvasWidth, sCanvasHeight);
	glScissor(0,0,sCanvasWidth, sCanvasHeight);
	glClearColor(0.1f, 0.4f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}