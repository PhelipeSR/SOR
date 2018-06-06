#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "function.h"

point urand[NRAN];
int irand[NRAN];


int main(int argc, char *argv[]) {
	/**
	* Verifica se o usuário colocou a quantidade de processoas a serem criados
	*/
	if (argc < 2) {
		printf("Especifique o numero de processos -> $ %s <numero de processos>\n", argv[0] );
		exit(1);
	}

	/**
	* Variáveis usadas no processo
	*/
	int num_processos = strtol(argv[1], NULL, 10);
	key_t k = 0;
	int shmid = 0;
	pid_t pid = 0;

	/**
	* Verifica se a quantidade de processos é divisível pela largura
	*/
	if (WID % (num_processos) != 0) {
		printf("O numero de processos %d nao e divisivel por %d\n", num_processos,WID);
		return 0;
	}

	int i,j;
	uchar *image;
	camera c;
	point eye;
	point lookat;
	int samples;
	int s;
	float rcp_samples;// = 1.0 / (float)samples;

	//char fname[20];
	//ray * rays;
	//color cor;

	//srand ( time(NULL) );

	//---init virtual camera---
	//point eye = {10.0f,400.0f,1000.0f};
	//point eye = {0.0f,2.0f,-20.0f};
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

	k = ftok("/home/larissapires/SOR/Processos/simpleRT.c",'R');
	shmid = shmget(k, c.view.width * c.view.height * 3 * sizeof(uchar), 0644|IPC_CREAT);
	//pai associa-se a regiao compartilhada
	image = shmat(shmid, (void*)0, 0);

	if(image == NULL)
	{
		fprintf(stderr,"Error. Cannot malloc image frame.\n");
		return 0;
	}

	/*
	* Define quantas colunas cada processo vai calcular
	*/
	int colunas = WID/num_processos;

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

	for(int a = 0; a < num_processos; a++) {
		pid = fork();

		if(pid < 0) {
			fprintf(stderr,"Erro, nao foi possivel criar o processo filho\n");
			return 1;
		}
		else if(pid == 0) {
			for(i = a * colunas; i < (a+1) * colunas; i++)
			{
				for(j = 0 ; j < c.view.height ; j++)  //para cada pixel da coluna de varimento
				{
					//Determinar o raio que une o centro de projecção com o pixel
					float r, g, b;
					r = g = b = 0.0;

					for(s=0; s<samples; s++) { //Para cada objecto da cena
						//Se  o  raio  intersecta  o  objecto  e  o  ponto  de  intersecção  encontra-se mais  próximo  do  centro  de  projecção  do  que  o  ponto  de intersecção até agora encontrado
						ray rr = get_primary_ray(&c,i,j,s);
						color col = trace(c,&rr,0);
						r += col.r;
						g += col.g;
						b += col.b;
					}
					//Registar o ponto de intersecção e o objecto intersectado
					r = r * rcp_samples;
					g = g * rcp_samples;
					b = b * rcp_samples;

					//ray rr = get_primary_ray(&c, i, j, samples);
					//color clr = trace(c,&rr,0);

					//red green blue color components
					//Atribuir  ao  pixel  a  cor  do  objecto  intersectado  no  ponto  de  intersecção registado
					image[ 3* (i * c.view.height + j) + 0] = floatToIntColor(r);
					image[ 3* (i * c.view.height + j) + 1] = floatToIntColor(g);
					image[ 3* (i * c.view.height + j) + 2] = floatToIntColor(b);
				}
			}
			exit(0); //encerra o filho
		}
		else //pai
		{

		}
	}
	//encerra o pai
	while(wait(NULL) > 0);

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
