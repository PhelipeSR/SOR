#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>
#include "function.h"

// ============================
void process_lock();
void process_unlock();
int receive_line();
// ============================

point urand[NRAN];
int irand[NRAN];

// ============================
int num_trabalhadores;
int *varredor_imagem;
pthread_mutex_t lock;
// ============================


int main(int argc, char *argv[]) {
	/**
	* Verifica se o usuário colocou a quantidade de processoas a serem criados
	*/
	if (argc < 2) {
		printf("Especifique o numero de processos -> $ %s <numero de processos>\n", argv[0] );
		exit(1);
	}

	int num_processos = strtol(argv[1], NULL, 10);
	key_t k = 0;
	key_t k1 = 0;
	int shmid = 0;
	int shmid1 = 0;
	pid_t pid = 0;

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

	lookat.x = 0.5f;
	lookat.y = 0.0f;
	lookat.z = 0.0f;

	initCamera(&c,eye,lookat,WID,HEI);
	setupCamera(&c);

	//================================================
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init failed. mutex sucks\n");
		return 1;
	}
	//================================================

	//====================================================================================
	// ---malloc the image frame---
	k = ftok("/tmp",'R');
	shmid = shmget(k, c.view.width * c.view.height * 3 * sizeof(uchar), 0644|IPC_CREAT);
	image = shmat(shmid, (void*)0, 0);
	initImage(&c,image);
	if(image == NULL) {
		fprintf(stderr,"Error. Cannot malloc image frame.\n");
		return 0;
	}
	//---malloc the varredor_imagem--
	k1 = ftok("/tmp",'L');
	shmid1 = shmget(k1, sizeof(int), 0644|IPC_CREAT);
	varredor_imagem = shmat(shmid1, (void*)0, 0);

	*varredor_imagem = 0;
	//====================================================================================

	//---just init the image frame with some data---
	generateScene();
	generateRandomLightSources();

	for(i=0; i<NRAN; i++) urand[i].x = (double)rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) urand[i].y = (double)rand() / RAND_MAX - 0.5;
	for(i=0; i<NRAN; i++) irand[i] = (int)(NRAN * ((double)rand() / RAND_MAX));

	samples = 8;
	rcp_samples = 1.0 / (float)samples;


	for(int a = 0; a < num_processos; a++) {
		pid = fork();
		if(pid < 0) {
			fprintf(stderr,"Erro, nao foi possivel criar o processo filho\n");
			return 1;
		}
		else if(pid == 0) {
			do {
				process_lock();
				i = receive_line();
				process_unlock();
				for(j = 0 ; j < c.view.height ; j++) {
					float r, g, b;
					r = g = b = 0.0;

					for(int s=0; s<samples; s++) {

						ray rr = get_primary_ray(&c,i,j,s);
						color col = trace(c,&rr,0);
						r += col.r;
						g += col.g;
						b += col.b;
					}

					r = r * rcp_samples;
					g = g * rcp_samples;
					b = b * rcp_samples;

					image[ 3* (i * c.view.height + j) + 0] = floatToIntColor(r);
					image[ 3* (i * c.view.height + j) + 1] = floatToIntColor(g);
					image[ 3* (i * c.view.height + j) + 2] = floatToIntColor(b);
				}
			}while(i < WID);
			//encerra o filho
			exit(0);
		}
		else{
		}
	}
	//encerra o pai
	while(wait(NULL) > 0);

	if(save_bmp("output_rt.bmp",&c,image) != 0) {
		fprintf(stderr,"Cannot write image 'output.bmp'.\n");
		return 0;
	}

	pthread_mutex_destroy(&lock);

	return 0;
}

int receive_line(){

	//acabaram as linhas pra fazer varredura. tchau.
	if(*varredor_imagem > WID){
		return (WID);
	}
	int varredor_imagem_atual = *varredor_imagem;
	(*varredor_imagem)++;

    return varredor_imagem_atual;
}

void process_lock(void) {
	int ret;
	ret = pthread_mutex_lock(&lock);
	if (ret != 0)
		_exit(EXIT_FAILURE);
}

void process_unlock(void) {
	int ret;
    ret = pthread_mutex_unlock(&lock);
	if (ret != 0)
		_exit(EXIT_FAILURE);
}
