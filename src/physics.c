#include "../include/globals.h"

void calcForce(Point *p, QuadTree *qt, double theta)
{
	LOG_DEBUG("Calculating force for point at (%.2f, %.2f)", p->x, p->y);
	if (qt == NULL)
	{
		LOG_WARNING("Null QuadTree in calcForce");
		return;
	}
	if (qt->point_count == 0 && qt->children[0] == NULL)
	{
		LOG_DEBUG("Empty QuadTree in calcForce");
		return;
	}

	double dx = qt->cmX - p->x;
	double dy = qt->cmY - p->y;

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
			LOG_DEBUG("Applied force (%.2f, %.2f) to point", f * dx, f * dy);

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
	LOG_DEBUG("Updating positions in QuadTree");
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

			float maxDim = fmax(_RELATIVE_WIDTH, _RELATIVE_HEIGHT);
			if (p->x <= 0 || p->x >= maxDim )
			{
				p->vx = -p->vx;
				p->x = p->x <= 0 ? 0 : maxDim;
			  LOG_DEBUG("Point bounced off x boundary: (%.2f, %.2f)", p->x, p->y);
			}
			if (p->y <= 0 || p->y >= maxDim )
			{
				p->vy = -p->vy;
				p->y = p->y <= 0 ? 0 : maxDim;
			  LOG_DEBUG("Point bounced off y boundary: (%.2f, %.2f)", p->x, p->y);
			}
			p->fx = 0;
			p->fy = 0;
		}
	}
}
