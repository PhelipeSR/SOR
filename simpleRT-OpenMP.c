#include "function.h"

point urand[NRAN];
int irand[NRAN];

int main(int argc, char ** argv) {
	int i,j;
	uchar *image;
	camera c;
	point eye;
	point lookat;
	int samples;
	int s;
	float rcp_samples;

	eye.x = 0.0f;
	eye.y = 2.0f;
	eye.z = -20.0f;

	//point lookat = {0.5f,0.0f,0.0f};
	lookat.x = 0.5f;
	lookat.y = 0.0f;
	lookat.z = 0.0f;

	initCamera(&c,eye,lookat,WID,HEI);
	setupCamera(&c);

	//---malloc the image frame---
	image = (uchar *) malloc(c.view.width * c.view.height * 3 * sizeof(uchar));
	if(image == NULL)
	{
		fprintf(stderr,"Error. Cannot malloc image frame.\n");
		return 0;
	}

	//---just init the image frame with some data---
	initImage(&c,image);

	//---insert random N_SPHERES into the 'data' array
	//generateRandomSpheres();
	generateScene();

	//---insert random N_LIGHTS into the 'lights' array
	generateRandomLightSources();

	//---create a 1D array with primary rays coordinates
	//rays = generatePrimaryRays(&c);

	for(i=0; i<NRAN; i++) urand[i].x = (double)rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) urand[i].y = (double)rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) irand[i] = (int)(NRAN * ((double)rand() / RAND_MAX));

	//---ray tracing loop---

	samples = 8;
	s = 0;
	rcp_samples = 1.0 / (float)samples;

	#pragma omp parallel shared(image) private(i, j, s)
	{
		#pragma omp for
		for(i = 0 ; i < c.view.width ; i++)
		{
			for(j = 0 ; j < c.view.height ; j++)
			{
				float r, g, b;
				r = g = b = 0.0;

				for(s=0; s<samples; s++) {
					ray rr = get_primary_ray(&c,i,j,s);
					color col = trace(c,&rr,0);
					r += col.r;
					g += col.g;
					b += col.b;
				}

				r = r * rcp_samples;
				g = g * rcp_samples;
				b = b * rcp_samples;

				//ray rr = get_primary_ray(&c, i, j, samples);
				//color clr = trace(c,&rr,0);

				//red green blue color components
				image[ 3* (i * c.view.height + j) + 0] = floatToIntColor(r);
				image[ 3* (i * c.view.height + j) + 1] = floatToIntColor(g);
				image[ 3* (i * c.view.height + j) + 2] = floatToIntColor(b);
			}
		}
	}

	//printPrimaryRays(rays,c.view.width*c.view.height); //for testing only

	if(save_bmp("output_rt.bmp",&c,image) != 0)
	{
		fprintf(stderr,"Cannot write image 'output.bmp'.\n");
		return 0;
	}

	//---freeing data---
	//free(rays);
	free(image);

	//---exit---
	return 0;
}
