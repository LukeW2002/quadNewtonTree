#include "../include/globals.h"
#include "../include/point.h"
#include "../include/input.h"
#include "../include/physics.h"
#include "../include/quad_tree.h"
#include "../include/render.h"
#include "../include/sdl_setup.h"



int main(int argc, char* args[])
{
	init_logger("Debug.log");
	set_log_level(DEBUG);

	LOG_INFO("Starting Program");
	
	if (!initSDL() || !initGlew() || !initOpenGL())
	{
		LOG_ERROR("SDL, INITGLEW,INITOPENGL FAILED");
		cleanup();
		return 1;
	}

	LOG_INFO("Initialization successful");
//	double size = fmax(_WINDOW_WIDTH, _WINDOW_HEIGHT) * _SCALE_FACTOR;
	LOG_DEBUG("Creating Root QuadTree");
	QuadTree* root = createQuadTree(0,0, (double) _RELATIVE_WIDTH, (double)_RELATIVE_HEIGHT);
	LOG_DEBUG("Created root QuadTree: (%.2f, %.2f) %.2f x %.2f", 0.0, 0.0, _RELATIVE_WIDTH, _RELATIVE_HEIGHT);

	Point** allPoints = malloc((2 + _AMOUNT_OF_BALLS) * sizeof(Point*));
	if (!allPoints) {
  	LOG_ERROR("Failed to allocate memory for allPoints");
  	cleanup();
  	close_logger();
  	return 1;
   }
	memset(allPoints, 0, (2 + _AMOUNT_OF_BALLS) * sizeof(Point*));
	int pointCount = 0;


	Point *centralBody = (Point*)malloc(sizeof(Point));
	createPoint(centralBody,(double) _RELATIVE_WIDTH/2, (double)_RELATIVE_HEIGHT/2, 0, 0, 1e10, 0, 0, 0, 0, 0, 0);
	insert(root, centralBody);
	allPoints[pointCount++] = centralBody;
	LOG_DEBUG("Created central body 1 at (%.2f, %.2f)", centralBody->x, centralBody->y);

	//Point *centralBody2 = (Point*)malloc(sizeof(Point));
	//createPoint(centralBody2,(double) _RELATIVE_WIDTH/2 + 30, (double)_RELATIVE_HEIGHT/2 - 20, 0, 0, 1e10, 0, 0, 0, 0, 0, 0);
	//insert(root, centralBody2);
	//allPoints[pointCount++] = centralBody2;
//	LOG_DEBUG("Created central body 2 at (%.2f, %.2f)", centralBody2->x, centralBody2->y);
	
	

	for (int i = 0; i < _AMOUNT_OF_BALLS; ++i) {
		double angle = ((double)rand() / RAND_MAX) * M_PI;
		double radius = ((double)rand() / RAND_MAX) * _RELATIVE_HEIGHT*_RELATIVE_WIDTH * _SCALE_FACTOR /2;
		double speed = sqrt(_G*centralBody->mass/radius);

		Point* referenceBody = (i % 2 == 0) ? centralBody : centralBody;
		Point *p = (Point*)malloc(sizeof(Point));
		double rfx = referenceBody->x + radius*cos(angle);
		double rfy = referenceBody->y + radius*sin(angle);
		double vfx = -speed*sin(angle);
		double vfy = speed*cos(angle);
		double mass = (rand() % 100 + 1)*1e3;
		createPoint(p,rfx ,
									 rfy,
									 vfx,
									 vfy,
									 mass,
										0, 0, 0, 0, 0, 0);
		insert(root, p);
		allPoints[pointCount++] = p;
		LOG_DEBUG("Created point %d at (%.2f, %.2f)", i, p->x, p->y);
	}

  LOG_INFO("Created %d points in total", pointCount);


	SDL_Event e;
	bool quit = false;
	int frameCount = 0;
	while(!quit)
	{
		LOG_DEBUG("Starting frame %d", frameCount);
		while(SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				LOG_INFO("Quit event received");
				quit = true;
			} 
			else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
			{
				handleKeyPress(&e.key);
			}
		} 
		clearQuadTree(root);

		for (int i = 0; i < pointCount; ++i) {
			insert(root, allPoints[i]);
		}
		
		#pragma omp parallel for schedule(guided)
		for (int i = 0; i < _CALCS_PER_FRAME; i++)
		{
			for (int i = 0; i < pointCount; ++i) {
				calcForce(allPoints[i], root, 0.5);
			}
			updatePos(root);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		drawQuadTree(root);
		SDL_GL_SwapWindow(window);
		checkOpenGLState();
		log_opengl_error(__FILE__, __LINE__, __func__);

    frameCount++;

	}
	LOG_INFO("EXIT PROGRAM");
	close_logger();

	printf("EXIT\n");
	freeQuadTree(root);
  for (int i = 0; i < pointCount; ++i) {
        freePoint(allPoints[i]);
  }
  free(allPoints);
	cleanup();
	return 0;
}
