#include "../include/globals.h"

float* g_instanceData = NULL;
int g_dataSize = 0;
int g_capacity = 0;
float* g_gridLines = NULL;
int g_gridSize = 0;
int g_gridCapacity = 0;

static int frame_counter = 0;
#define LOG_FREQUENCY 100  // Log every 100 frames

// Conditional compilation macro
#ifdef DEBUG_LOGGING
#define PERFORMANCE_LOG(level, ...) if (frame_counter % LOG_FREQUENCY == 0) { LOG_ ## level(__VA_ARGS__); }
#else
#define PERFORMANCE_LOG(level, ...)
#endif



void collectGridLines(QuadTree *qt, float **lines, int *size, int *capacity)
{
	PERFORMANCE_LOG("Collecting grid lines for QuadTree at (%.2f, %.2f)", qt->x, qt->y);
	if (*size + 16 > *capacity)
	{
		*capacity = (*capacity == 0) ? 1024 : *capacity * 2;
		*lines = realloc(*lines, *capacity * sizeof(float));
		if (*lines == NULL) {
  		LOG_ERROR("Failed to reallocate memory for grid lines");
  		return;
    }
		PERFORMANCE_LOG("Reallocated grid lines buffer to capacity %d", *capacity);
	}

	 double scale = fmax((double)_RELATIVE_WIDTH/2,(double) _RELATIVE_HEIGHT/2);
	 double stretch = 2;
	 double left = (qt->x / scale)*stretch -1 ;
	 double right = ((qt->x + qt->width) / scale)*stretch- 1 ;
	 double top = (qt->y / scale)*stretch - 1;
	 double bottom = ((qt->y + qt->height) / scale)*stretch - 1;
	
	(*lines)[(*size)++] = left;  (*lines)[(*size)++] = top;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = top;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = top;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = left;  (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = left;  (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = left;  (*lines)[(*size)++] = top;
	PERFORMANCE_LOG("Added grid lines for QuadTree: (%.2f, %.2f) to (%.2f, %.2f)", left, top, right, bottom);

	if (qt->children[0] != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			collectGridLines(qt->children[i], lines, size, capacity);
		}
	}
}

void updateInstanceData(QuadTree *qt, float **instanceData, int *dataSize, int *capacity)
{
	PERFORMANCE_LOG("Updating instance data for QuadTree at (%.2f, %.2f)", qt->x, qt->y);
	if (qt->point_count + *dataSize > *capacity)
	{
		*capacity = (*capacity == 0) ? 1024 : *capacity * 2;
		*instanceData = realloc(*instanceData, *capacity * 2 * sizeof(float));
		if (*instanceData == NULL) {
   		LOG_ERROR("Failed to reallocate memory for instance data");
   		return;
    }
		PERFORMANCE_LOG("Reallocated instance data buffer to capacity %d", *capacity);
	}

	for (int i = 0; i <qt->point_count; i++)
	{
		(*instanceData)[(*dataSize)++] = (qt->points[i]->x / _RELATIVE_WIDTH) * 2 - 1;
		(*instanceData)[(*dataSize)++] = (qt->points[i]->y / _RELATIVE_HEIGHT) * 2 - 1;
		PERFORMANCE_LOG("Added point data: (%.2f, %.2f)", (*instanceData)[(*dataSize)-2], (*instanceData)[(*dataSize)-1]);
	}

	if (qt->children[0] != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			updateInstanceData(qt->children[i], instanceData, dataSize, capacity);
		}
	}
}

void collectQuadTreeData(QuadTree *qt)
{
	PERFORMANCE_LOG("Collecting QuadTree data");
	updateInstanceData(qt, &g_instanceData, &g_dataSize, &g_capacity);

	if(show_grid)
	{
		collectGridLines(qt, &g_gridLines, &g_gridSize, &g_gridCapacity);
	}

	if (qt->children[0] != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			collectQuadTreeData(qt->children[i]);
		}
	}
	PERFORMANCE_LOG("Collected data for %d points and %d grid lines", g_dataSize/2, g_gridSize/2);
	
}

void renderQuadTreeData()
{
	 PERFORMANCE_LOG("Rendering QuadTree data");

	glUseProgram(shaderProgram);
	glBindVertexArray(pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, g_dataSize * sizeof(float), g_instanceData, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glVertexAttribDivisor(1, 1);

	glDrawArraysInstanced(GL_POINTS, 0, 1, g_dataSize / 2);

	glDrawArraysInstanced(GL_POINTS, 0, 1, g_dataSize / 2);

	PERFORMANCE_LOG("Drew %d points", g_dataSize / 2);

	if (show_grid)
	{
		glBindVertexArray(gridVAO);
		glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
		glBufferData(GL_ARRAY_BUFFER, g_gridSize * sizeof(float), g_gridLines, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2* sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glUseProgram(shaderProgram);
		glDrawArrays(GL_LINES, 0, g_gridSize / 2);
    PERFORMANCE_LOG("Drew %d grid lines", g_gridSize / 2);
	}
	printf("Rendering %d points\n", g_dataSize / 2);
	printf("First point data: (%f, %f)\n", g_instanceData[0], g_instanceData[1]);
	glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindVertexArray(0);
	glBindVertexArray(0);
}

void checkOpenGLState() {
    GLint vao, arrayBuffer, elementArrayBuffer, program;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBuffer);
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    PERFORMANCE_LOG("OpenGL State: VAO: %d, Array Buffer: %d, Element Array Buffer: %d, Shader Program: %d",
              vao, arrayBuffer, elementArrayBuffer, program);    
}

void drawQuadTree(QuadTree *qt)
{
	PERFORMANCE_LOG("Drawing QuadTree: (%.2f, %.2f) %.2f x %.2f", qt->x, qt->y, qt->width, qt->height);


	g_dataSize = 0;
	g_gridSize = 0;

	collectQuadTreeData(qt);
	renderQuadTreeData();

  PERFORMANCE_LOG("Finished drawing QuadTree");
}
