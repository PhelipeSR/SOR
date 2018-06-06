#include <pthread.h>
#include "function.h"

void *threads_trabalhadores(void *blocksize);

uchar *image;
camera c;
point urand[NRAN];
int irand[NRAN];
int num_trabalhadores;

int main(int argc, char *argv[]) {
	/**
	* Verifica se o usuário colocou a quantidade de processoas a serem criados
	*/
	if (argc < 2) {
		printf("Especifique numero de trabalhadores -> $ %s <numero de trabalhadores>\n", argv[0]);
		exit(1);
	}
	num_trabalhadores = strtol(argv[1], NULL, 10);
	/**
	* Verifica se a quantidade de processos é divisível pela largura
	*/
	if (WID % (num_trabalhadores) != 0) {
		printf("O numero de processos %d nao e divisivel por %d\n", num_trabalhadores,WID);
		return 0;
	}
	int i;
	point eye;
	point lookat;
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
	if(image == NULL) {
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


	//array contendo o numero maximo de threads definido ao início do programa
	pthread_t trabalhadores[num_trabalhadores];
	//thread que vai executar o traçado de raios
	int thread_trabalhadora;
	//Quantidade de pixels que cada thread deve processar
	long colunas = WID/num_trabalhadores;

	for(int b = 0; b < num_trabalhadores; b++) {
		// vai ser enviado para a subrotina da thread a quantidade de pixels
		// que vai ser processada, e a quantidade de pixels que já foi processada
		long divisao = (b+1)*colunas;
							//pthread_create(id_trabalhador, null, function to be executed, argument passed to it
		thread_trabalhadora = pthread_create(&trabalhadores[b], NULL, threads_trabalhadores, (void*) divisao);
		if(thread_trabalhadora){ //retorna 0 em erro de criacao
			printf("Erro");
		}
	}

	//desacopla as threads que estavam em execução
	for(int k = 0; k < num_trabalhadores; k++) {
	  //pthread_join(thread 2b waited, std null)
		pthread_join(trabalhadores[k], NULL);
	}

	if(save_bmp("output_rt.bmp",&c,image) != 0) {
		fprintf(stderr,"Cannot write image 'output.bmp'.\n");
		return 0;
	}

	free(image);

	//termina a thread pai
	pthread_exit(NULL);
	//---exit---
	return 0;
}

void *threads_trabalhadores(void *blocksize){

    long parcial = (long) blocksize;
    float rcp_samples;
    int samples;
    rcp_samples = 1.0 / (float)samples;
    samples = 8;

    // representa a quantidade de pixels já foi processada,
    // pois depende do parâmetro que foi passado do main()
    long escalonador = parcial - (WID/num_trabalhadores);
    for(int i = escalonador; i < parcial ; i++) {
        for(int j = 0 ; j < c.view.height ; j++) {
            float r, g, b;
            r = g = b = 0.0;

            for(int s=0; s < samples; s++) {
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
