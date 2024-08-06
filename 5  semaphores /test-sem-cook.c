#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* Semaforo per sincronizzare */
#define ID_READY      0   /* figli pronti: padre puo` procedere */
#define ID_GO         1   /* padre pronto: figli possono procedere */
/* Ingredienti */
#define ID_UOVO       2
#define ID_SALE       3
#define ID_POMODORO   4
#define ID_PANCETTA   5
#define ID_CIPOLLA    6
#define ID_PATATE     7
#define ID_PASTA      8
#define ID_OLIO       9
/* Strumenti */
#define ID_FORNELLO  10
#define ID_PADELLA   11
#define ID_PENTOLA   12

#define NUM_RISORSE  13

/* RISORSE INIZIALI */
/* Ingredienti */
#define INIT_PASTA        10  /* Espresso in ettogrami */
#define INIT_OLIO         20  /* Cucchiai */
#define INIT_UOVO         10  /* Numero uova */
#define INIT_SALE        100  /* Pizzichi */
#define INIT_POMODORO     20  /* Numero pomodori */
#define INIT_PANCETTA     10  /* Fette */
#define INIT_CIPOLLA      10  /* Numero cipolle */
#define INIT_PATATE       30  /* Numero patate */
/* Strumenti */
#define INIT_PADELLA       1
#define INIT_PENTOLA       1
#define INIT_FORNELLO      2

#define TIMEOUT     60        /* Tempo (secondi) per fare le ricette */

#define NUM_PROC      20
#define TEST_ERROR    if (errno) {fprintf(stderr, \
					   "%s:%d: PID=%5d: Error %d (%s)\n",\
					   __FILE__,\
					   __LINE__,\
					   getpid(),\
					   errno,\
					   strerror(errno));}

#define GESTISCI_RISORSE_AND_CHECK(sem_id, sops, num)			\
	if (semop(sem_id, sops, num) == -1) {				\
	        switch (errno) {					\
		case EIDRM:						\
			printf("PID = %d, riga:%d : semaforo rimosso mentre ero bloccato\n", \
			       getpid(), __LINE__);			\
			exit(EXIT_FAILURE);				\
		case EINVAL:						\
			printf("PID = %d, riga:%d : semaforo rimosso prima di blocco (o mai esistito)\n", \
			       getpid(), __LINE__);			\
			exit(EXIT_FAILURE);				\
		case EAGAIN:						\
			printf("PID = %d, riga:%d : mi sono stufato di aspettare\n", \
			       getpid(), __LINE__);			\
			exit(EXIT_FAILURE);				\
						\
		default:						\
			printf("PID = %d, riga:%d : altro errore\n",	\
			       getpid(), __LINE__);			\
			TEST_ERROR;					\
			exit(EXIT_FAILURE);				\
		}							\
	}


/*
 * Funzioni per preparare le ricette
 */
#define NUM_RICETTE 7
void make_pasta_bianco();
void make_carbonara();
void make_pasta_pom();
void make_uovo_fritto();
void make_frittata_pata();
void make_pata_fritte();
void make_pata_fritte_subito();
void make_verdu_lesse();


/*
 * Handler di SIGALRM/SIGINT: deve soltanto rimuovere la "dispensa" ovvero
 * tutti i semafori.
 */
void handle_signal(int signal);


/*
 * Imposta i valori iniziali dei semafori
 */
void reset_sem();

/*
 * Assegna le risorse iniziali: "la dispensa"
 */
void fill_resource();

/*
 * Stampa risorse correnti
 */
void print_resource();

/* ID dell'IPC del semaforo e` globale */
int sem_id;

int main (void)
{
	struct sigaction sa;
	pid_t ret_val;
	int i, status, kids_success=0, kids_failure=0, tipo;
	struct sembuf sops;
	struct timespec now;
	sigset_t  my_mask;
	
	/* 
	 * Crea un  semaforo e  inizializza senza risorse.  Le risorse
	 * verranno  messe  soltanto  quando  tutti  i  processi  sono
	 * pronti.
	 */
	sem_id = semget(IPC_PRIVATE, NUM_RISORSE, 0600);
	TEST_ERROR;
	reset_sem(sem_id);

	/* Nessun flag per operazioni sui semafori */
	sops.sem_flg = 0;
	
	/* Create NUM_PROC processes */
	for (i=0; i<NUM_PROC; i++) {
		switch (fork()) {
 		case 0:
			/* 
			 * Informa il  padre che sono nato  e pronto a
			 * fare ottime ricette!
			 */ 
			sops.sem_num = ID_READY;
			sops.sem_op = 1;
			semop(sem_id, &sops, 1);

			/* Aspetta che il padre ci dia il via */ 
			sops.sem_num = ID_GO;
			sops.sem_op = -1;
			semop(sem_id, &sops, 1);
			/*
			 * Scelgo il  tipo di ricetta in  modo random.
			 * Meglio  pero`  usare   i  nano-secondi  per
			 * inizializzare il random  seed con srand: se
			 * usassi i  secondi e` possibile che  le fork
			 * vengano fatte tutte  nello stesso secondo e
			 * quindi  il seed  sialo stesso  per tutti  i
			 * figli.
			 */
			clock_gettime(CLOCK_REALTIME ,&now);
			tipo = now.tv_nsec % NUM_RICETTE;
			/* 
			 * Invoca la funzione. La exit avverra` dentro
			 * la funzione.
			 */
			switch (tipo) {
			case 0:
				make_pata_fritte();
			case 1:
				make_pata_fritte_subito();
			case 2:
				make_uovo_fritto();
			case 3:
			case 4:
				make_pasta_bianco();
			case 5:
				make_carbonara();
			case 6:
				make_frittata_pata();
				/*
				  make_pasta_pom();
				  make_verdu_lesse();
				*/
			} /* switch tipo ricetta */
			break;
		default:
			break;
		} /* switch della fork() */
	} /* end for creazione figli */

	/* 
	 * Il padre aspetta che tutti  siano pronti prima di dargli le
	 * risorse "ingrediente":  soltanto dopo che tutti  i NUM_PROC
	 * figli avranno eseguito la loro semop che incrementa di 1 la
	 * risorsa, si potranno consumare NUM_PROC risorse
	 */
	sops.sem_num = ID_READY;
	sops.sem_op = -NUM_PROC;
	semop(sem_id, &sops, 1);
	
	/* 
	 * Assegna le risorse e attendi che i figli abbiano terminato
	 * correttamente preparando la ricetta oppure che non ce
	 * l'abbiano fatta. Fra TIMEOUT secondi, pero`, si smette.
	 */
	sa.sa_handler = handle_signal;
	sa.sa_flags = 0; 	/* No special behaviour */
	sigemptyset(&my_mask);
	sa.sa_mask = my_mask;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	alarm(TIMEOUT);
	fill_resource(sem_id);
	print_resource();
	/* 
	 * Facciamo partire i cuochi
	 */
	sops.sem_num = ID_GO;
	sops.sem_op = NUM_PROC;
	semop(sem_id, &sops, 1);
	
	while (1) {
		ret_val = wait(&status);
		if (ret_val == -1) {
			if (errno == ECHILD) {
				/* Figli finiti: possiamo uscire dal ciclo */
				break;
			}
			if (errno == EINTR) {
				/* 
				 * Sbloccati da un segnale: dobbiamo
				 * fare di nuovo la wait
				 */
				continue;
			}
			/* Altri errori: da gestire */
			TEST_ERROR;
			exit(EXIT_FAILURE);
		}
		/* La wait non ha fallito */
		if (WIFEXITED(status)) {
			switch (WEXITSTATUS(status)) {
			case EXIT_SUCCESS:
				kids_success++;
				break;
			case EXIT_FAILURE:
				kids_failure++;
				break;
			default:
				/* 
				 * Non dovrebbe succedere: i figli non
				 * fanno altri tipi di exit
				 */
				TEST_ERROR;
			}
		} else {
			/* 
			 * Non dovrebbe succedere:  i figli dovrebbero
			 * sempre terminare con una exit(...)
			 */
			TEST_ERROR;
		}
	} /* end ciclo di wait con while(1) */
	printf("Numero ricette completate con successo\n%d\n", kids_success);
	printf("Numero ricette non completate \n%d\n", kids_failure);
	exit(EXIT_SUCCESS);
}


void handle_signal(int signal)
{
	/* Ricevuto SIGALRM o SIGINT: chiudere la dispensa */
	print_resource();
	semctl(sem_id, 0 /* ignored */, IPC_RMID);
}


void reset_sem()
{
	semctl(sem_id, ID_READY, SETVAL, 0);
	semctl(sem_id, ID_GO, SETVAL, 0);
	semctl(sem_id, ID_PASTA, SETVAL, 0);
	semctl(sem_id, ID_OLIO, SETVAL, 0);
	semctl(sem_id, ID_UOVO, SETVAL, 0);
	semctl(sem_id, ID_SALE, SETVAL, 0);
	semctl(sem_id, ID_POMODORO, SETVAL, 0);
	semctl(sem_id, ID_PANCETTA, SETVAL, 0);
	semctl(sem_id, ID_CIPOLLA, SETVAL, 0);
	semctl(sem_id, ID_PATATE, SETVAL, 0);
	semctl(sem_id, ID_PADELLA, SETVAL, 0);
	semctl(sem_id, ID_PENTOLA, SETVAL, 0);
	semctl(sem_id, ID_FORNELLO, SETVAL, 0);
}

void fill_resource()
{
	struct sembuf sops[NUM_RISORSE];

	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	sops[0].sem_num = ID_OLIO;
	sops[0].sem_op = INIT_OLIO;
	sops[1].sem_num = ID_UOVO;
	sops[1].sem_op = INIT_UOVO;
	sops[2].sem_num = ID_SALE;
	sops[2].sem_op = INIT_SALE;
	sops[3].sem_num = ID_POMODORO;
	sops[3].sem_op = INIT_POMODORO;
	sops[4].sem_num = ID_PANCETTA;
	sops[4].sem_op = INIT_PANCETTA;
	sops[5].sem_num = ID_CIPOLLA;
	sops[5].sem_op = INIT_CIPOLLA;
	sops[6].sem_num = ID_PATATE;
	sops[6].sem_op = INIT_PATATE;
	sops[7].sem_num = ID_PASTA;
	sops[7].sem_op = INIT_PASTA;
	sops[8].sem_num = ID_PENTOLA;
	sops[8].sem_op = INIT_PENTOLA;
	sops[9].sem_num = ID_FORNELLO;
	sops[9].sem_op = INIT_FORNELLO;
	sops[10].sem_num = ID_PADELLA;
	sops[10].sem_op = INIT_PADELLA;
	semop(sem_id, sops, 11);
}

void print_resource()
{
	unsigned short sem_vals[NUM_RISORSE], i;

	semctl(sem_id, 0 /* ignored */, GETALL, sem_vals);
	printf("READY    GO  OLIO  UOVO  SALE  POMO PANCE  CIPO  PATA PASTA PENTO FUOCO  PADE\n");
	for (i=0; i<NUM_RISORSE; i++) {
		printf("%5d ", semctl(sem_id, i, GETVAL));
	}
	printf("\n");
}


void make_pasta_bianco()
{
	struct sembuf sops[2];
	
	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	/* Bollire l'acqua */
	sops[0].sem_num = ID_PENTOLA;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: pasta in bianco: INIZIO\n", getpid());
	sleep(4); /* Tempo per bollire l'acqua */
	/* Cuocere la pasta */
	sops[0].sem_num = ID_PASTA;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_SALE;
	sops[1].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: pasta in bianco: butto la pasta\n", getpid());
	sleep(10); /* Tempo per cuocere la pasta */
	/* Finito con pentola e fornello */
	sops[0].sem_num = ID_PENTOLA;
	sops[0].sem_op = 1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = 1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	/* Aggiungi un filo d'olio */
	sops[0].sem_num = ID_OLIO;
	sops[0].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 1);
	printf("PID = %d: pasta in bianco: FINE\n", getpid());
	exit(EXIT_SUCCESS);
}

void make_carbonara()
{
	struct sembuf sops[5];
	int status;

	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	/* Due processi: figlio prepara pasta, padre condimento */
	if (!fork()) {
		/* Figlio: cuoce la pasta */
		sops[0].sem_num = ID_PENTOLA;
		sops[0].sem_op = -1;
		sops[1].sem_num = ID_FORNELLO;
		sops[1].sem_op = -1;
		GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
		printf("PID = %d: carbonara: metto l'acqua per %d\n",
		       getpid(), getppid());
		sleep(4); /* Tempo per bollire l'acqua */
		/* Cuocere la pasta */
		sops[0].sem_num = ID_PASTA;
		sops[0].sem_op = -1;
		sops[1].sem_num = ID_SALE;
		sops[1].sem_op = -1;
		GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
		printf("PID = %d: carbonara: butto pasta per %d\n",
		       getpid(), getppid());
		sleep(10); /* Tempo per cuocere la pasta */
		/* Finito con pentola e fornello */
		sops[0].sem_num = ID_PENTOLA;
		sops[0].sem_op = 1;
		sops[1].sem_num = ID_FORNELLO;
		sops[1].sem_op = 1;
		GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
		printf("PID = %d: carbonara: pasta pronta per %d\n",
		       getpid(), getppid());
		exit(EXIT_SUCCESS);
	}
	/* Padre: prepara condimento e salta la pasta in padella */
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = -1;
	sops[2].sem_num = ID_OLIO;
	sops[2].sem_op = -1;
	sops[3].sem_num = ID_CIPOLLA;
	sops[3].sem_op = -1;
	sops[4].sem_num = ID_PANCETTA;
	sops[4].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 5);
	printf("PID = %d: carbonara: friggo cipolla e pancetta\n", getpid());
	sleep(2);
	wait(&status);
	if (!WIFEXITED(status)) {
		exit(EXIT_FAILURE);
	}
	if (WEXITSTATUS(status) != EXIT_SUCCESS) {
		exit(EXIT_FAILURE);
	}
	sops[0].sem_num = ID_UOVO;
	sops[0].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 1);
	printf("PID = %d: carbonara: aggiungo pasta e uovo\n", getpid());
	sleep(1);
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = 1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = 1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: carbonara: FINE\n", getpid());
	exit(EXIT_SUCCESS);
}

void make_frittata_pata()
{
	struct sembuf sops[5];
	
	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = -1;
	sops[2].sem_num = ID_OLIO;
	sops[2].sem_op = -1;
	sops[3].sem_num = ID_CIPOLLA;
	sops[3].sem_op = -1;
	sops[4].sem_num = ID_PATATE;
	sops[4].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 5);
	printf("PID = %d: frittata patate e cipolla: INIZIO\n", getpid());
	sleep(3);
	sops[0].sem_num = ID_UOVO;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_SALE;
	sops[1].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: frittata patate e cipolla: aggiungo uova\n",
	       getpid());
	sleep(2);
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = 1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = 1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: frittata patate e cipolla: FINE\n", getpid());
	exit(EXIT_SUCCESS);
}

void make_uovo_fritto()
{
	struct sembuf sops[3];
	
	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = -1;
	sops[2].sem_num = ID_OLIO;
	sops[2].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 3);
	printf("PID = %d: uovo fritto: INIZIO\n", getpid());
	sleep(1);
	sops[0].sem_num = ID_UOVO;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_SALE;
	sops[1].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: uovo fritto: aggiungo uova\n",
	       getpid());
	sleep(2);
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = 1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = 1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: uovo fritto: FINE\n", getpid());
	exit(EXIT_SUCCESS);
}

void make_pata_fritte()
{
	struct sembuf sops[3];
	
	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = -1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = -1;
	sops[2].sem_num = ID_OLIO;
	sops[2].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 3);
	printf("PID = %d: patate fritte: INIZIO\n", getpid());
	sleep(1);
	sops[0].sem_num = ID_PATATE;
	sops[0].sem_op = -1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 1);
	printf("PID = %d: patate fritte: butto le patate\n", getpid());
	sleep(3);
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = 1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = 1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: patate fritte: FINE\n", getpid());
	exit(EXIT_SUCCESS);
}

void make_pata_fritte_subito()
{
	struct sembuf sops[3];
	
	/* Setting all sem_flg to zero in one shot */;
	bzero(sops,sizeof(sops));
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = -1;
	sops[0].sem_flg = IPC_NOWAIT;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = -1;
	sops[1].sem_flg = IPC_NOWAIT;
	sops[2].sem_num = ID_OLIO;
	sops[2].sem_op = -1;
	sops[2].sem_flg = IPC_NOWAIT;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 3);
	printf("PID = %d: patate fritte subito: INIZIO\n", getpid());
	sleep(1);
	sops[0].sem_num = ID_PATATE;
	sops[0].sem_op = -1;
	/* sops[0].sem_flg gia` settato a IPC_NOWAIT */
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 1);
	printf("PID = %d: patate fritte subito: butto le patate\n", getpid());
	sleep(3);
	sops[0].sem_num = ID_PADELLA;
	sops[0].sem_op = 1;
	sops[1].sem_num = ID_FORNELLO;
	sops[1].sem_op = 1;
	GESTISCI_RISORSE_AND_CHECK(sem_id, sops, 2);
	printf("PID = %d: patate fritte subito: FINE\n", getpid());
	exit(EXIT_SUCCESS);
}


/*
void make_pasta_pom();
void make_verdu_lesse();
*/
