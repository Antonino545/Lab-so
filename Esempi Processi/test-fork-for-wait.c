/*
* Se viene definito DO_WAIT allora ogni processo attende la
 * terminazione dei propri processi figlio
 *
 * #define DO_WAIT
 */

#define NUM_ITER  3

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef DO_WAIT
#include <sys/types.h>
#include <sys/wait.h>
#endif

int main() {
	int i, num_kids = 0;
	unsigned int my_pid, my_ppid, value;

	/*
	 * Rimuovere il buffering di stdout per capire meglio cosa sta succedendo
	 */
	setvbuf(stdout, NULL, _IONBF, 0);

	/* Quanti processi vengono generati? */
	for (i=0; i<NUM_ITER; i++) {
		// Crea un nuovo processo
		value = fork();
		if (value) {
			// Se value è diverso da zero, siamo nel processo padre
			num_kids++;
		} else {
			// Se value è zero, siamo nel processo figlio
			num_kids = 0;
		}
		// Ottiene il PID del processo corrente
		my_pid = getpid();
		// Ottiene il PID del processo padre
		my_ppid = getppid();
		/* Quante linee vengono stampate? */
		printf("PID=%6d:  PPID=%6d,  i=%d, fork_value=%d\n",
			   my_pid, my_ppid, i, value);
	}
#ifndef DO_WAIT
	/*
	 * Non aspettare i processi figli: il genitore può uscire prima dei figli
	 */
	printf("PID=%6d: Sto terminando. I miei %d processi figli possono essere vivi o no\n",
		   my_pid, num_kids);
#else
	/*
	 * Tutti i processi aspettano la terminazione di tutti i processi figli.
	 */
	while(wait(NULL) != -1);
	printf("PID=%6d: Io e tutti i miei %d processi figli siamo terminati\n",
		   my_pid, num_kids);
#endif
}
