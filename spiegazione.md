
# Spiegazione Lezione

## Comandi Docker

1. **Avviare un container Docker:**
   ```bash
    docker run -it --name lab-esempi -v lab-esempi:/data -v "/Users/antonino/Desktop/Lab-so":/new-lab ubuntu-so
   ```

2. **Verificare i container in esecuzione:**
   ```bash
   docker ps
   ```

3. **Accedere a un container in esecuzione:**
   ```bash
   docker exec -it lab-so bash
   ```

## Processi in Unix

### Comandi Utili

- `ps`: serve per guardare i processi in esecuzione.
- `top`: serve per vedere i processi in esecuzione in tempo reale.
- `kill -9 <PID>`: serve per terminare il processo con PID specificato (ad esempio `kill -9 1234`).

  > Nota: "Kill non ammazza necessariamente un processo, ma manda un segnale al processo."

### Terminologia

- **PID**: Identificativo del processo.
- Ogni processo ha un processo parent che lo ha creato.
- In Unix, i processi formano una struttura ad albero con il processo parent.
- Il processo padre di tutto è il processo `init` con PID 1. Il PPID (parent PID) di `init` è 0.
- Se mandi un segnale a 0, lo manda a te stesso (non ha molto senso).
- I PID bassi sono di root.
- `pstree -p | less`: serve per vedere l'albero dei processi.

### Funzioni di Sistema

- `pid_t getpid(void)`: ritorna il PID del processo chiamante (un intero).
- `pid_t getppid(void)`: ritorna il PID del processo padre.

### Creazione di un Processo in Unix

- **fork()**: è una system call che crea un processo figlio tramite `<unistd.h>` e `<sys/types.h>`.
    - Il processo figlio avrà gli stessi indirizzi virtuali del processo padre, ma finirà in memoria fisica diversa.
    - Il risultato della `fork` stampa due valori diversi:
        - Nel processo padre, il valore di ritorno della `fork` è il PID del processo figlio.
        - Se la `fork` ritorna -1, c'è stato un errore e non ha creato il processo figlio.
        - Se la `fork` ritorna 0, siamo nel processo figlio e il figlio eseguirà solo il codice successivo alla `fork`.
## Wait
- **wait()**: serve per sincronizzare il processo padre con il processo figlio.
    - `wait` ritorna -1 se il processo è terminato.
    - `wait(NULL)`: Il processo padre utilizza `wait(NULL)` per attendere la terminazione di uno dei suoi processi figli.
    - Dopo che il processo padre ha chiamato `wait(NULL)` e ha ricevuto il PID di un processo figlio terminato, può continuare a chiamare `wait(NULL)` per attendere la terminazione di altri processi figli.
## Waitpid
Waitpid serve per attendere la terminazione di un processo figlio specifico. `waitpid` prende tre argomenti:
- `pid_t pid`: il PID del processo figlio da attendere.
- `int *status`: un puntatore a un intero in cui verrà memorizzato lo stato di uscita del processo figlio.
- `int options`: opzioni aggiuntive.
    - `WNOHANG`: ritorna immediatamente se non ci sono processi figli terminati.
    - `WUNTRACED`: ritorna se un processo figlio è fermato, ma non terminato.
    - `WCONTINUED`: ritorna se un processo figlio è stato continuato.
- se c'è stato un errore, `waitpid` ritorna -1.
- se `waitpid` ha successo, ritorna il PID del processo figlio.
## Execve
Execve serve a sostituire un processo in un altro processo. E' una system call che prende tre argomenti:
- `const char *path`: il percorso del file eseguibile.
- `char *const argv[]`: un array di stringhe che rappresentano gli argomenti del programma.
(argv[0] è il nome del programma solitamente)
- `char *const envp[]`: un array di stringhe che rappresentano l'ambiente del programma.
-  **<span style="color:red">( L'ultimo elemento di `argv` e `envp` deve essere NULL.)</span>**
- `execve` ritorna -1 se c'è stato un errore.
- `execve` non ritorna se ha successo.
- Il PID del processo rimane lo stesso.
- Tutti i segmeni di memoria del processo vengono sostituiti con quello del nuovo processo.
## System
System è una funzione che esegue un comando shell. Diversame da `execve`, `system` non sostituisce il processo corrente, ma crea un processo figlio che esegue il comando shell.
## Signal
il segnale e un software interrupt che viene inviato a un processo per notificare un evento. I segnali sono inviati da un processo a un altro processo o a se stesso. I segnali sono numerati da 1 a 31. I segnali piu  usati sono:
- `SIGKILL` (9): termina il processo.
- `SIGSTOP` (19): ferma il processo.
- `SIGCONT` (18): riprende il processo.
- `SIGINT` (2): interruzione da tastiera.
- `SIGTERM` (15): termina il processo.
- `SIGCHLD` (17): inviato al processo padre quando un processo figlio termina.
ecc... \
Inviare un segnale a ogni processo :
La sistem call 'kill' invia un segnale a un processo. La sintassi e:
```c
int kill(pid_t pid, int sig);
```
- `pid`: il PID del processo a cui inviare il segnale.
- `sig`: il segnale da inviare.
- `kill` ritorna 0 se ha successo e -1 se c'è stato un errore.
- Se il PID non esiste, `kill` errno è impostato a `ESRCH`.
- Se il processo non ha i permessi per inviare il segnale, `kill` errno è impostato a `EPERM`.
- `Sudo kill -9 -1`: invia il segnale di kill a tutti i processi tranne a se stesso.
# Raise
Raise e una funzione che invia un segnale al processo chiamante. La sintassi e:
```c
int raise(int sig);
```
- `sig`: il segnale da inviare.
- `raise` ritorna 0 se ha successo e -1 se c'è stato un errore.
- `raise(SIGKILL)`: termina il processo.
- `raise(SIGSTOP)`: ferma il processo.
- se il figlio si auto termina, il padre riceve figlio terminato con segnale.
# Alarm
Alarm e una funzione che invia un segnale di `SIGALRM` al processo chiamante dopo un certo numero di secondi. La sintassi e:
```c
unsigned int alarm(unsigned int seconds);
```
- `seconds`: il numero di secondi prima che `SIGALRM` venga inviato.
- `alarm` ritorna il numero di secondi rimanenti prima che `SIGALRM` venga inviato.
- `alarm(0)`: annulla un allarme già impostato.
- `SIGALRM` e inviato al processo chiamante.
- `SIGALRM` e inviato solo una volta.
- `SIGALRM` e inviato dopo che il processo ha dormito per il numero di secondi specificato.
- `SIGALRM` e inviato anche se il processo e bloccato in una chiamata di sistema.
# Che cosa succede quando riceviamo un segnale?
Ci sono 5 possibili azioni che un processo può intraprendere quando riceve un segnale:
- TERM: termina il processo.
- IGN: ignora il segnale.
- STOP: ferma il processo.
- CONT: riprende il processo.
- CORE: termina il processo e genera un core dump.
# Handling Signal
Un signal handing e una funzione scritta dal programmatore che puo associre alla ricezione di un segnale. Impostiamo un handler per un segnale tramite la struttura `sigaction`. La sintassi e:
```c
struct sigaction {
    void (*sa_handler)(int signum);
    sigset_t sa_mask;
    int sa_flags;
};
```
- `sa_handler`: la funzione sa_handler prende in input un intero e da in output un void.
- `sa_mask`: Per il momento, possiamo ignorare questo campo.
- `sa_flags`: Per il momento, possiamo ignorare questo campo.

sa_handler e un puntatore a una funzione che prende un intero come argomento e restituisce void. La funzione viene chiamata quando il processo riceve il segnale specificato.Quando arrriva questa segnale il paramentro sara compilato dal sistema operativo con il numero del segnale che ha scatenato l'handler.
la vera funzione che gestisce il segnale e la funzione `sigaction` che prende tre argomenti:
```c
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```
- `signum`: il numero del segnale.
- `act`: un puntatore alla struttura `sigaction` che contiene l'handler.
- `oldact`: un puntatore alla struttura `sigaction` in cui verra memorizzato l'handler precedente.
Se vuoi gestire un certo segnale con un handler deve essere scritto prima prima che quel segnate posso arruvare. Quindi se vuoi gestire un segnale devi scriverlo prima di tutto perche in caso contrario il segnale verra gestito con handler di default.
a volte si gestiscuno tutti i segnali in un unica funzione, tramite uno switch case.
# Esempio
```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
void handler_signal(int signum){
 switch(signum){
  case SIGINT:
   printf("SIGINT\n");
   break;
  case SIGTERM:
   printf("SIGTERM\n");
   break;
  case SIGKILL:
   printf("SIGKILL\n");
   break;
  case SIGSTOP:
   printf("SIGSTOP\n");
   break;
  case SIGCONT:
   printf("SIGCONT\n");
   break;
  case SIGCHLD:
   printf("SIGCHLD\n");
   break;
  default:
   printf("Signal not handled\n");
   break;
 }
 ``` 
 Gestione di segnali e processi figli se tu imposti degli handle per i segnali e poi crei un processo figlio, il processo figlio eredita gli handler del processo padre. Quindi se il processo padre ha impostato un handler per SIGINT e crea un processo figlio, il processo figlio eredita l'handler per SIGINT.Se il figlio esegue un execve, tutti gli handler vengono elimitati
 
 # Pipes 
    Una pipe e un meccanismo di comunicazione tra processi. Una pipe e un file temporaneo in memoria che permette la comunicazione tra due processi. Una pipe e unidirezionale, quindi se vuoi comunicare in entrambe le direzioni, devi creare due pipe. Una pipe e creata con la system call `pipe` che prende un array di due interi come argomento. La sintassi e:
    ```c
    int pipe(int pipefd[2]);
    ```
    - `pipefd`: un array di due interi che conterrà i file descriptor della pipe.
    - `pipe` ritorna 0 se ha successo e -1 se c'è stato un errore.
    - `pipefd[0]`: il file descriptor per la lettura dalla pipe.
    - `pipefd[1]`: il file descriptor per la scrittura nella pipe.
    - `pipe` crea una pipe e restituisce due file descriptor.