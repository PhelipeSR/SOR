#include <pthread.h>
#include "function.h"

// ============================
void *threads_trabalhadores(void *blocksize);
void process_lock();
void process_unlock();
int receive_line();
// ============================

int varredor_imagem = 0;

uchar *image;
camera c;
point urand[NRAN];
int irand[NRAN];
int num_trabalhadores;
pthread_mutex_t lock; //stuff from mutex. dont ask me.

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

	//--start and check of mutex--
	if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n Mmutex init failed. Mutex sucks\n");
        return 1;
    }

	//---malloc the image frame---
	image = (uchar *) malloc(c.view.width * c.view.height * 3 * sizeof(uchar));
	if(image == NULL) {
		fprintf(stderr,"Error. Cannot malloc image frame.\n");
		return 0;
	}

	//---just init the image frame with some data---
	initImage(&c,image);
	generateScene();
	generateRandomLightSources();

	//---create a 1D array with primary rays coordinates
	for(i=0; i<NRAN; i++) urand[i].x = (double)rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) urand[i].y = (double)rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) irand[i] = (int)(NRAN * ((double)rand() / RAND_MAX));


	//array contendo o numero maximo de threads definido ao início do programa
	pthread_t trabalhadores[num_trabalhadores];
	// Thread que vai executar o traçado de raios
	int thread_trabalhadora;

	for(int b = 0; b < num_trabalhadores; b++) {
		thread_trabalhadora = pthread_create(&trabalhadores[b], NULL, threads_trabalhadores, NULL);
		if(thread_trabalhadora){
			printf("Error criacao de threads");
		}
	}

	for(int k = 0; k < num_trabalhadores; k++) {
		//desacopla as threads que estavam em execução
		pthread_join(trabalhadores[k], NULL);
	}
	if(save_bmp("output_rt.bmp",&c,image) != 0) {
		fprintf(stderr,"Cannot write image 'output.bmp'.\n");
		return 0;
	}
	free(image);
	pthread_mutex_destroy(&lock);
	//termina a thread pai
	pthread_exit(NULL);
	return 0;
}

void *threads_trabalhadores(void *blocksize){

    long parcial = (long) blocksize;//not even using nmore
    float rcp_samples;
    int samples = 8;
    int j, s;

    rcp_samples = 1.0 / (float)samples;
    int i;

	do{

		process_lock();
		i = receive_line();
		process_unlock();
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
    }while(i < WID);
    printf("thread's mission accomplished\n");
}

int receive_line(){

	//acabaram as linhas pra fazer varredura. tchau.
	if(varredor_imagem > WID){
		return (WID);
	}
	int varredor_imagem_atual = varredor_imagem;
	varredor_imagem++;

    return varredor_imagem_atual;
}

void process_lock(void) {
	int ret;
	ret = pthread_mutex_lock(&lock);
	if (ret != 0)
		exit(EXIT_FAILURE);
}

void process_unlock(void) {
	int ret;
    ret = pthread_mutex_unlock(&lock);
	if (ret != 0)
		exit(EXIT_FAILURE);
}