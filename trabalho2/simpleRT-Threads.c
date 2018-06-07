#include <pthread.h>
#include "function.h"

void *threads_trabalhadores(void *blocksize);
int receive_line();
int varredor_imagem = 0;

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
		printf("Especifique o numero de trabalhadores -> $ %s <numero de trabalhadores>\n", argv[0] );
		exit(1);
	}
	num_trabalhadores = strtol(argv[1], NULL, 10);
	

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
	int thread_trabalhadora; //thread que vai executar o traçado de raios
	long razao = WID/num_trabalhadores; //Cada thread irá processar 1 linha da imagem por vez

	//loop de trabalhadores, do 1 até o n escolhido
	//em cada iteracao é criado uma thread e enviado a essa thread
	//uma sessão da imagem.
	for(int b = 0; b < num_trabalhadores; b++) {
		// vai ser enviado para a subrotina da thread a quantidade de pixels
		// que vai ser processada, e a quantidade de pixels que já foi processada
		long divisao = (b+1)*razao;
							//pthread_create(worker associeted, null, function to be called, arg of function)
		thread_trabalhadora = pthread_create(&trabalhadores[b], NULL, threads_trabalhadores,(void*) divisao);
		if(thread_trabalhadora){
			printf("Deu merda na criacao de threads");
		}
	}

	for(int k = 0; k < num_trabalhadores; k++) {
		//desacopla as threads que estavam em execução
		pthread_join(trabalhadores[k], NULL);	}

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
    long i = 0; //contador do for
    long escalonador = parcial - (WID/num_trabalhadores); //representa a quantidade de pixels já foi processada, pois depende do parâmetro que foi passado do main()
    int samples = 8;
    int j, s;

    rcp_samples = 1.0 / (float)samples;
    for(i = escalonador; i < parcial ; i++)
    {
        for(j = 0 ; j < c.view.height ; j++)
        {
            float r, g, b;
            r = g = b = 0.0;

            for(s=0; s < samples; s++) {
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

//a esperança é que essa funcao retorne uma linha/coordenada quando
//uma thread pedir para que possa ser calculado o tracado de raios
int receive_line(){

	//há de haver um sinal/mutex aqui, para controlar o acesso à uma coordenada


    do{
   		printf("%d\n", varredor_imagem);
   		//return varredor_imagem;
   		varredor_imagem++; //varredor imagem é uma variavel global
    }while(varredor_imagem <= WID);

    return 0;
	
}