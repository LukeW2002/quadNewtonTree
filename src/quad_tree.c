#include "../include/globals.h"

QuadTree * createQuadTree( double x, double y, double width, double height)
{
	QuadTree *qt = (QuadTree*)malloc(sizeof(QuadTree));
	LOG_DEBUG("Creating QuadTree at (%.2f, %.2f) with dimensions %.2f x %.2f", x, y, width, height);

	if (qt == NULL) {
 		LOG_ERROR("Failed to allocate memory for QuadTree");
 		return NULL;
  }

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
	bool result =  p->x >= qt->x && p->x < qt->x  + qt->width &&
			           p->y >= qt->y && p->y < qt->y  + qt->height;	
	if (!result)
	{
		LOG_WARNING("Point (%.2f, %.2f) out of bounds for QuadTree at (%.2f, %.2f) %.2f x %.2f",
                    p->x, p->y, qt->x, qt->y, qt->width, qt->height);
  }

	return result;
}

void subdivide(QuadTree *qt)
{
	LOG_DEBUG("Subdividing QuadTree at (%.2f, %.2f)", qt->x, qt->y);
	double new_width = qt->width/2;
	double new_height = qt->height/2;

	qt->children[0] = createQuadTree(qt->x, qt->y, new_width, new_height);
	qt->children[1] = createQuadTree(qt->x + new_width, qt->y, new_width, new_height);
	qt->children[2] = createQuadTree(qt->x, qt->y + new_height, new_width, new_height);
	qt->children[3] = createQuadTree(qt->x + new_width, qt->y + new_height, new_width, new_height);
}

void freeQuadTree(QuadTree* qt) 
{
	LOG_DEBUG("Freeing QuadTree at (%.2f, %.2f)", qt->x, qt->y);
    if (qt == NULL) return;
	if (qt == NULL) return;
	for (int i = 0; i < 4; i++) {
	    freeQuadTree(qt->children[i]);
	}
	free(qt);
}

void insert(QuadTree *qt, Point *p)
{
	LOG_DEBUG("Inserting point (%.2f, %.2f) into QuadTree at (%.2f, %.2f)",
              p->x, p->y, qt->x, qt->y);
	if (!isInBounds(qt, p))
	{
		LOG_WARNING("Attempted to insert out-of-bounds point (%.2f, %.2f)", p->x, p->y);
		return;
	
	}

	
	
	if (qt->point_count < _MAX_POINTS && qt->children[0] == NULL)
	{
		qt->points[qt->point_count++] = p;
		LOG_DEBUG("Inserted point into leaf node, now contains %d points", qt->point_count);
		return;
	}

	if (qt->children[0] == NULL)
	{
		subdivide(qt);
		LOG_DEBUG("Subdivided node at (%.2f, %.2f)", qt->x, qt->y);
    

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
		qt->point_count = 0;
	}

	for (int i = 0; i<4; i++)
	{
		if (isInBounds(qt->children[i], p))
		{
			insert(qt->children[i], p);
			break;
		}
	}
	qt->total_mass += p->mass;
  qt->cmX = (qt->cmX * (qt->total_mass - p->mass) + p->x * p->mass) / qt->total_mass;
  qt->cmY = (qt->cmY * (qt->total_mass - p->mass) + p->y * p->mass) / qt->total_mass;
}

void clearQuadTree(QuadTree* qt)
{
	LOG_DEBUG("Clearing QuadTree at (%.2f, %.2f)", qt->x, qt->y);
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
