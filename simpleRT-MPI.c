#include <mpi.h>
#include "function.h"

point urand[NRAN];
int irand[NRAN];

int main(int argc, char *argv[]) {
	int i,j;
	uchar *imagem_pai;
	uchar *imagem_filho;
	camera c;
	point eye;
	point lookat;
	int samples;
	int s;
	float rcp_samples;// = 1.0 / (float)samples;

	/**
	* Variáveis usadas no MPI
	*/
	int num_trabalhadores, rank, len, mpi_status;
	char hostname[MPI_MAX_PROCESSOR_NAME];

	/*
	* Inicia o MPI
	*/
	mpi_status = MPI_Init (&argc, &argv);
	if (mpi_status != MPI_SUCCESS) {
		printf( "Erro ao iniciar trabalhador %d de %d\n", rank, num_trabalhadores );
		MPI_Abort(MPI_COMM_WORLD,mpi_status);
		return 0;
	}

	MPI_Comm_size(MPI_COMM_WORLD, &num_trabalhadores);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(hostname, &len);

	/*
	* verifica se (número de trabalhadores - 1) é divisível pola largura da imagem
	*/
	if (WID % (num_trabalhadores-1) != 0) {
		printf("O numero de trabalhdores %d nao e divisivel por %d\n", (num_trabalhadores - 1),WID);
		MPI_Finalize();
		return 0;
	}

	eye.x = 0.0f;
	eye.y = 2.0f;
	eye.z = -20.0f;

	//point lookat = {0.5f,0.0f,0.0f};
	lookat.x = 0.5f;
	lookat.y = 0.0f;
	lookat.z = 0.0f;

	initCamera(&c,eye,lookat,WID,HEI);
	setupCamera(&c);

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

	/*
	* Define quantas colunas cada trabalhador vai calcular
	*/
	int colunas = WID/(num_trabalhadores - 1);

	/*
	* Trabalhador PAI
	*/
	if (rank == 0) {

		// Aloca espaço na memória para a imagem completa
		imagem_pai = (uchar *) malloc(c.view.width * c.view.height * 3 * sizeof(uchar));
		if(imagem_pai == NULL){
			fprintf(stderr,"Error. Cannot malloc image frame.\n");
			MPI_Finalize();
			return 0;
		}

		for (int i = 1; i < num_trabalhadores; i++){
			MPI_Recv(&imagem_pai[(3*c.view.height*colunas)*(i-1)], (colunas * c.view.height * 3 *
				sizeof(uchar)), MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		if(save_bmp("output_rt.bmp",&c,imagem_pai) != 0) {
			fprintf(stderr,"Cannot write image 'output.bmp'.\n");
			return 0;
		}
		free(imagem_pai);
	}
	/*
	* Trabalhadores FILHOS
	*/
	else{
		// Aloca espaço na memória para a imagem que cada filho vai trabalhar
		imagem_filho = (uchar *) malloc(colunas * c.view.height * 3 * sizeof(uchar));
		if(imagem_filho == NULL){
			fprintf(stderr,"Error. Cannot malloc image frame.\n");
			MPI_Finalize();
			return 0;
		}

		// Calcula os limites inferiores e superiores
		int inf = (rank * colunas) - colunas;
        int sup = (rank * colunas);

		for(i = inf ; i < sup ; i++)
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
				imagem_filho[ 3* ((i - inf) * c.view.height + j) + 0] = floatToIntColor(r);
				imagem_filho[ 3* ((i - inf) * c.view.height + j) + 1] = floatToIntColor(g);
				imagem_filho[ 3* ((i - inf) * c.view.height + j) + 2] = floatToIntColor(b);
			}
		}
		MPI_Send(imagem_filho, (colunas * c.view.height * 3 * sizeof(uchar)),
				MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD);
		free(imagem_filho);
	}

	MPI_Finalize();
}
