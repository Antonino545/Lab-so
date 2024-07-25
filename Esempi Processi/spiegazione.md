
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

- **wait()**: serve per sincronizzare il processo padre con il processo figlio.
    - `wait` ritorna -1 se il processo è terminato.
    - `wait(NULL)`: Il processo padre utilizza `wait(NULL)` per attendere la terminazione di uno dei suoi processi figli.
    - Dopo che il processo padre ha chiamato `wait(NULL)` e ha ricevuto il PID di un processo figlio terminato, può continuare a chiamare `wait(NULL)` per attendere la terminazione di altri processi figli.
