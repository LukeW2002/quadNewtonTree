#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>
#include <math.h>

#define _MAX_POINTS 10
#define _TOTAL_MEMORY_POINTS 10000
#define _WINDOW_WIDTH 800
#define _WINDOW_HEIGHT 600
#define _G 0.1
#define _SCALE_FACTOR 1
#define _TIME_STEP 0.0001
#define _AMOUNT_OF_BALLS 1600
#define _SOFTENING 4

#define _RELATIVE_WIDTH _WINDOW_WIDTH * _SCALE_FACTOR
#define _RELATIVE_HEIGHT _WINDOW_HEIGHT * _SCALE_FACTOR  
#define _CALCS_PER_FRAME 10

#define GL_GLEXT_PROTOTYPES 1
#define GL_SILENCE_DEPRECATION

typedef struct Point {
	double x, y;
	double vx, vy;
	double mass;
	double fx, fy;
	double r, g, b, a;
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
bool show_grid = false;
SDL_GLContext glContext = NULL;
GLuint shaderProgram, vao, vbo;
GLuint instanceVBO;
GLuint pointVAO, gridVAO;
GLuint gridVBO;

const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\0";

bool initSDL()
{

	if (SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		printf("SDL COULD NOT INIT:\t%s\n",SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("Quad Tree", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _WINDOW_WIDTH, _WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		printf("WINDOW CANT BE MADE:\t%s\n", SDL_GetError());
		return false;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (glContext == NULL)
	{
		printf("OpenGl Could not be created! SDL ERROR:\t%s\n", SDL_GetError());
		return false;
	}
	return true;
}

bool initGlew()
{
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		printf("Error INIT GLEW! SDL ERROR:\t%s\n", SDL_GetError());
		return false;
	}
	return true;
}

GLuint compileShader(const char* source, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("Shader complication error:\t%s\n", infoLog);
		return 0;
	}
	return shader;
}


bool initOpenGL()
{
	GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
	
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ShaderProgram complication error:\t%s\n", infoLog);
		return false;
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &instanceVBO);

	glGenVertexArrays(1, &pointVAO);
	glGenVertexArrays(1, &gridVAO);

	glBindVertexArray(pointVAO);
	float point[] = {0.0f, 0.0f};
	glPointSize(5.0f);
	glGenBuffers(1, &vbo);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1,1);

	glBindVertexArray(gridVAO);
	glGenBuffers(1, &gridVBO);
	
	glBindVertexArray(0);

	return true;
}

void updateInstanceData(QuadTree *qt, float **instanceData, int *dataSize, int *capacity)
{
	if (qt->point_count + *dataSize > *capacity)
	{
		*capacity = (*capacity == 0) ? 1024 : *capacity * 2;
		*instanceData = realloc(*instanceData, *capacity * 2 * sizeof(float));
	}

	for (int i = 0; i <qt->point_count; i++)
	{
		(*instanceData)[(*dataSize)++] = (qt->points[i]->x / _RELATIVE_WIDTH) * 2 - 1;
		(*instanceData)[(*dataSize)++] = (qt->points[i]->y / _RELATIVE_WIDTH) * 2 - 1;
	}

	if (qt->children[0] != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			updateInstanceData(qt->children[i], instanceData, dataSize, capacity);
		}
	}
}
	

void cleanup()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(shaderProgram);
	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


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

void handleKeyPress(SDL_KeyboardEvent *event)
{
	if (event->type == SDL_KEYDOWN)
	{
		switch (event->keysym.sym)
		{
			case SDLK_SPACE:
				show_grid = !show_grid;
				break;
		}
	}
}

void collectGridLines(QuadTree *qt, float **lines, int *size, int *capacity)
{
	if (*size + 16 > *capacity)
	{
		*capacity = (*capacity == 0) ? 1024 : *capacity * 2;
		*lines = realloc(*lines, *capacity * sizeof(float));
	}
	float left = (qt->x / _RELATIVE_WIDTH)* 2 -1;
	float right = ((qt->x + qt->width) / _RELATIVE_WIDTH)* 2 -1;
	float top = (qt->y / _RELATIVE_WIDTH)* 2 -1;
	float bottom = ((qt->y + qt->height) / _RELATIVE_WIDTH)* 2 -1;
	
	(*lines)[(*size)++] = left;  (*lines)[(*size)++] = top;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = top;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = top;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = right; (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = left;  (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = left;  (*lines)[(*size)++] = bottom;
  (*lines)[(*size)++] = left;  (*lines)[(*size)++] = top;

	if (qt->children[0] != NULL)
	{
		for (int i = 0; i < 4; i++)
		{
			collectGridLines(qt->children[i], lines, size, capacity);
		}
	}
}

float* g_instanceData = NULL;
int g_dataSize = 0;
int g_capacity = 0;
float* g_gridLines = NULL;
int g_gridSize = 0;
int g_gridCapacity = 0;

void collectQuadTreeData(QuadTree *qt)
{
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
	
}

void renderQuadTreeData()
{
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, g_dataSize * sizeof(float), g_instanceData, GL_DYNAMIC_DRAW);

	glBindVertexArray(pointVAO);
	glUseProgram(shaderProgram);
	glDrawArraysInstanced(GL_POINTS, 0, 1, g_dataSize / 2);

	if (show_grid)
	{
		glBindVertexArray(gridVAO);
		glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
		glBufferData(GL_ARRAY_BUFFER, g_gridSize * sizeof(float), g_gridLines, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2* sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glUseProgram(shaderProgram);
		glDrawArrays(GL_LINES, 0, g_gridSize / 2);
	}
	glBindVertexArray(0);
}

void drawQuadTree(QuadTree *qt)
{
	g_dataSize = 0;
	g_gridSize = 0;

	collectQuadTreeData(qt);
	renderQuadTreeData();
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


int main(int argc, char* args[])
{
	if (!initSDL() || !initGlew() || !initOpenGL())
	{
		cleanup();
		return 1;
	}

	QuadTree* root = createQuadTree(0,0, _WINDOW_WIDTH * _SCALE_FACTOR, _WINDOW_HEIGHT*_SCALE_FACTOR);
	Point** allPoints = malloc(_AMOUNT_OF_BALLS * sizeof(Point*));
	int pointCount = 0;

	Point *centralBody = (Point*)malloc(sizeof(Point));
	centralBody->x = ( _WINDOW_WIDTH) * _SCALE_FACTOR /2 ;
	centralBody->y = ( _WINDOW_HEIGHT) * _SCALE_FACTOR /2;
	centralBody->vx = 1000;
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
			else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
			{
				handleKeyPress(&e.key);
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



    printf("QuadTree structure before drawing:\n");
    printQuadTreeInfo(root, 0);
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		drawQuadTree(root);
		SDL_GL_SwapWindow(window);

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
	cleanup();
	return 0;
}
