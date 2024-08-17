
#include "../include/globals.h"
void freePoint(Point *p)
{
	if (p != NULL)
	{
		free(p);
	}
};

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

void createPoint( Point *p,
                 long double x, 
                 long double y, 
                 long double vx, 
                 long double vy, 
                 long double mass,
                 long double fx,
                 long double fy,
                 double r,
                 double g,
                 double b,
                 double a
                 )
{
		p->x = x;
		p->y = y;
		p->vx = vx;
		p->vy = vy;
		p->mass = mass;
		p->fx = fx;
		p->fy = fy;
		p->r = r;
		p->g = g;
		p->b = b;
		p->a = a;
}

