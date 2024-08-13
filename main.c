#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>

#define _MAX_POINTS 1
#define _TOTAL_MEMORY_POINTS 10000
#define _WINDOW_WIDTH 800
#define _WINDOW_HEIGHT 600
#define _G 0.1
#define _SCALE_FACTOR 1
#define _TIME_STEP 0.0001
#define _AMOUNT_OF_BALLS 400
#define _SOFTENING 4

#define _RELATIVE_WIDTH _WINDOW_WIDTH * _SCALE_FACTOR
#define _RELATIVE_HEIGHT _WINDOW_HEIGHT * _SCALE_FACTOR  
#define _CALCS_PER_FRAME 10

typedef struct Point {
	double x, y;
	double vx, vy;
	double mass;
	double fx, fy;
} Point;

typedef struct QuadTree {
	double x, y, width, height;
	struct QuadTree* children[4];
	Point* points[_TOTAL_MEMORY_POINTS];
	int point_count;

	double total_mass;
	double cmX, cmY;
} QuadTree;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

void printQuadTreeInfo(QuadTree *qt, int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
    printf("QuadTree at (%f, %f), width: %f, height: %f, points: %d\n", 
           qt->x/_SCALE_FACTOR, qt->y/_SCALE_FACTOR, qt->width/_SCALE_FACTOR, qt->height/_SCALE_FACTOR, qt->point_count);
    
    for (int i = 0; i < qt->point_count; i++) {
        for (int j = 0; j < depth + 1; j++) printf("  ");
        printf("Point at (%f, %f)\n", qt->points[i]->x/_SCALE_FACTOR, qt->points[i]->y/_SCALE_FACTOR);
        for (int j = 0; j < depth + 2; j++) printf("  ");
        printf("Has velocity of (%f, %f)\n", qt->points[i]->vx/_SCALE_FACTOR, qt->points[i]->vy/_SCALE_FACTOR);
        for (int j = 0; j < depth + 2; j++) printf("  ");
        printf("Has mass of %f\n", qt->points[i]->mass/_SCALE_FACTOR);
        for (int j = 0; j < depth + 2; j++) printf("  ");
        printf("Experiencing force of (%f, %f)\n", qt->points[i]->fx, qt->points[i]->fy);
    }
    
    if (qt->children[0] != NULL) {
        for (int i = 0; i < 4; i++) {
            printQuadTreeInfo(qt->children[i], depth + 1);
        }
    }
}
void drawQuadTree(QuadTree *qt)
{
	SDL_SetRenderDrawColor(renderer, 255,255,255,255);
	SDL_Rect rect = {(int)qt->x / _SCALE_FACTOR, 
									 (int)qt->y / _SCALE_FACTOR, 
									 (int)qt->width / _SCALE_FACTOR,
									 (int)qt->height/ _SCALE_FACTOR};
	SDL_RenderDrawRect(renderer, &rect);

	for (int i = 0; i < qt->point_count; i++)
	{
		SDL_SetRenderDrawColor(renderer, 255,0,0,255);
		SDL_Rect point_rect = {((int)qt->points[i]->x /_SCALE_FACTOR) - 2, ((int)qt->points[i]->y / _SCALE_FACTOR) - 2, 4, 4};
		SDL_RenderFillRect(renderer, &point_rect);
		//printf("Drawing point at (%f, %f)\n", qt->points[i]->x, qt->points[i]->y);
	}

	if (qt->children[0] != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			drawQuadTree(qt->children[i]);
		}
	}

}

QuadTree * createQuadTree( double x, double y, double width, double height)
{
	QuadTree *qt = (QuadTree*)malloc(sizeof(QuadTree));
	qt->x = x;
	qt->y = y;
	qt->width = width;
	qt->height = height;
	for (int i = 0; i < 4; i++)
	{
		qt->children[i] = NULL;
	}
	qt->point_count = 0;
	qt->total_mass = 0;
	qt->cmX = 0;
	qt->cmY = 0;
	return qt;
};

bool isInBounds(QuadTree *qt, Point *p)
{
	return p->x >= qt->x && p->x < qt->x  + qt->width &&
			   p->y >= qt->y && p->y < qt->y  + qt->height;	
}

void subdivide(QuadTree *qt)
{
	double new_width = qt->width/2;
	double new_height = qt->height/2;

	qt->children[0] = createQuadTree(qt->x, qt->y, new_width, new_height);
	qt->children[1] = createQuadTree(qt->x + new_width, qt->y, new_width, new_height);
	qt->children[2] = createQuadTree(qt->x, qt->y + new_height, new_width, new_height);
	qt->children[3] = createQuadTree(qt->x + new_width, qt->y + new_height, new_width, new_height);
}


void insert(QuadTree *qt, Point *p)
{
	if (!isInBounds(qt, p))
	{
		return;
	
	}

	qt->total_mass += p->mass;
  qt->cmX = (qt->cmX * (qt->total_mass - p->mass) + p->x * p->mass) / qt->total_mass;
  qt->cmY = (qt->cmY * (qt->total_mass - p->mass) + p->y * p->mass) / qt->total_mass;
	
	
	if (qt->point_count < _MAX_POINTS && qt->children[0] == NULL)
	{
		qt->points[qt->point_count++] = p;
		return;
	}

	if (qt->children[0] == NULL)
	{
		subdivide(qt);

		for (int i = 0; i < qt->point_count; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (isInBounds(qt->children[j], qt->points[i]))
				{
					insert(qt->children[j], qt->points[i]);
					break;
				}
			}
		}
	}

	for (int i = 0; i<4; i++)
	{
		if (isInBounds(qt->children[i], p))
		{
			insert(qt->children[i], p);
			break;
		}
	}

}

void freePoint(Point *p)
{
	if (p != NULL)
	{
		free(p);
	}
};


void freeQuadTree(QuadTree* qt) 
{
	if (qt == NULL) return;
	for (int i = 0; i < 4; i++) {
	    freeQuadTree(qt->children[i]);
	}
	free(qt);
}

void clearQuadTree(QuadTree* qt)
{
	if (qt == NULL) return;
		for (int i = 0; i < 4; i++) {
		    if (qt->children[i] != NULL) {
  	    clearQuadTree(qt->children[i]);
  	    free(qt->children[i]);
  	    qt->children[i] = NULL;
  	}
	}
	qt->point_count = 0;
	qt->total_mass = 0;
	qt->cmX = 0;
	qt->cmY = 0;
}

void freeAllPoints(QuadTree* root) {
    if (root == NULL) return;

    for (int i = 0; i < root->point_count; i++) {
        freePoint(root->points[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (root->children[i] != NULL) {
            freeAllPoints(root->children[i]);
        }
    }
}

void calcForce(Point *p, QuadTree *qt, double theta)
{
	if (qt == NULL)
	{
		return;
	}
	if (qt->point_count == 0 && qt->children[0] == NULL)
	{
		return;
	}

	double dx= qt->cmX - p->x;
	double dy= qt->cmY - p->y;

	double distance = sqrt(dx * dx + dy * dy + _SOFTENING * _SOFTENING);
	if (((qt->width / distance < theta) || qt->children[0] == NULL))
	{
		if (distance > 0 && qt->point_count > 0)
		{
			double f = _G*p->mass * qt->total_mass / (distance * distance * distance);
			#pragma omp atomic
			p-> fx += f * dx;
			#pragma omp atomic
			p-> fy += f * dy;

		}
	}
	else if (qt->children[0] != NULL)
	{
		for ( int i = 0; i < 4; i++)
		{
			calcForce(p, qt->children[i], theta);
		}
	}
}

void updatePos(QuadTree *qt)
{
	if (qt->children[0] != NULL)
	{
		for (int i = 0; i< 4; i++)
		{
			updatePos(qt->children[i]);
		}
	}
	else
	{
		for (int i = 0; i< qt->point_count; i++)
		{
			Point *p = qt->points[i];
			p->vx += (p->fx /p->mass) *_TIME_STEP;
			p->vy += (p->fy /p->mass) *_TIME_STEP;
			p->x += p->vx *_TIME_STEP;
			p->y += p->vy *_TIME_STEP; 		
			if (p->x <= 0 || p->x >= _RELATIVE_WIDTH )
			{
				p->vx = -p->vx;
				p->x = p->x <= 0 ? 0 : _RELATIVE_WIDTH;
			}
			if (p->y <= 0 || p->y >= _RELATIVE_HEIGHT )
			{
				p->vy = -p->vy;
				p->y = p->y <= 0 ? 0 : _RELATIVE_HEIGHT;
			}
			p->fx = 0;
			p->fy = 0;
		}
	}
}


int main()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		printf("SDL COULD NOT INIT:\t%s\n",SDL_GetError());
	}

	window = SDL_CreateWindow("Quad Tree", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _WINDOW_WIDTH, _WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("WINDOW CANT BE MADE:\t%s\n", SDL_GetError());
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(renderer == NULL)
	{
		printf("RENDERER FAILED:\t%s:\n",SDL_GetError());
	}

	QuadTree* root = createQuadTree(0,0, _WINDOW_WIDTH * _SCALE_FACTOR, _WINDOW_HEIGHT*_SCALE_FACTOR);
	Point** allPoints = malloc(_AMOUNT_OF_BALLS * sizeof(Point*));
	int pointCount = 0;

	Point *centralBody = (Point*)malloc(sizeof(Point));
	centralBody->x = ( _WINDOW_WIDTH) * _SCALE_FACTOR /2 ;
	centralBody->y = ( _WINDOW_HEIGHT) * _SCALE_FACTOR /2;
	centralBody->vx = 0;
	centralBody->vy = 0;
	centralBody->mass = 1e10;
	centralBody->fx = 0;
	centralBody->fy = 0;
	insert(root, centralBody);
	allPoints[pointCount++] = centralBody;
	

	for (int i = 0; i < _AMOUNT_OF_BALLS; ++i) {
		Point *p = (Point*)malloc(sizeof(Point));
		double angle = ((double)rand() / RAND_MAX) * M_PI;
		double radius = ((double)rand() / RAND_MAX) * _WINDOW_WIDTH * _SCALE_FACTOR /2;
		p->x = centralBody->x + radius*cos(angle);
		p->y = centralBody->y + radius*sin(angle);

		double speed = sqrt(_G*centralBody->mass/radius);
		p->vx = -speed * sin(angle);
		p->vy = speed * cos(angle);

		p->mass = (rand() % 100 + 1)*1e3;
		p->fx = 0;
		p->fy = 0;

		insert(root, p);
		allPoints[pointCount++] = p;
	}
	printQuadTreeInfo(root, 0);

	bool quit = false;
	SDL_Event e;

	while(!quit)
	{
		while(SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			} 
		} 

		clearQuadTree(root);
    printf("QuadTree clearing and reinserting:\n");
    printQuadTreeInfo(root, 0);
		printf("After clear: %d points\n", root->point_count);

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

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

    printf("QuadTree structure before drawing:\n");
    printQuadTreeInfo(root, 0);
		


		drawQuadTree(root);

		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		//static int iterations = 0;
		//printf("ITERATIONS:\t%i\n", iterations);
    //if (++iterations > 3) break;
	}

	printf("EXIT\n");
	freeQuadTree(root);
  for (int i = 0; i < pointCount; ++i) {
        freePoint(allPoints[i]);
  }
  free(allPoints);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
