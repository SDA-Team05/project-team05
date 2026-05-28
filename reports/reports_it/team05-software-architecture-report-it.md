# Architecture Report

## C4 Model - Context Level

Si elencano i principali software systems, stakeholders e external libraries che interagiscono direttamente con Wireshark come entità esterne:

### External Libraries

#### libpcap/npcap
Per motivi di sicurezza e stabilità, il sistema operativo (Windows, macOS o Linux) non permette a un programma normale (Wireshark) di parlare direttamente con la scheda di rete. Per risolvere questo problema, l'architettura di Wireshark si affida a delle librerie esterne (`libpcap` per macOS/Linux, `npcap` per Windows) fornite dal sistema operativo.
*   Il modulo di cattura di Wireshark (`dumpcap`) chiama le API della libreria `libpcap`/`npcap`, le quali invitano a catturare il traffico dati tramite dei speciali permessi. parametri inseriti nel comando `dumpcap` vengono tradotti dal `libpcap`/`npcap` tramite la funzione `pcap_compile()`. Attraverso un mini-compilatore, la stringa viene trasformata in un linguaggio macchina chiamato **BPF (Berkeley Packet Filter) Bytecode**. Tramite la funzione `pcap_setfilter()`, il mini-programma appena compilato può essere iniettato direttamente nel **Kernel** del Sistema Operativo (l'operazione eseguita nel *Kernel Space* permette un significativo aumento in termini di efficienza temporale).
*   Durante l'ascolto, il Kernel registra i dati che passano nella rete e li impacchetta in un buffer di memoria. I pacchetti di byte vengono prelevati dalle funzioni di `libpcap`npcap` per essere normalizzati in una struttura dati in linguaggio C uguale per tutti, chiamata `struct pcap_pkthdr` (**Pcap Packet Header**). A questo contenitore vengono aggiuntdei metadati fondamentali (*timestamp*, *original-length*, *captured-length*).

### Software Systems

#### File System
*   Per evitare colli di bottiglia (**I/O Bottleneck**), `dumpcap` scrive i pacchetti alla massima velocità in un file temporaneo `.pcapng` nel File System. Contemporaneamente, la GUIdi Wireshark legge asincronamente quello stesso file temporaneo dal File System per mostrare i dati a schermo. In questo modo il File System funge da buffer condiviso tra i dueprocessi.
*   Wireshark interroga il File System all'avvio per cercare file di librerie dinamiche (`.dll` su Windows, `.so` su Linux) o script in linguaggio Lua all'interno di specifichecartelle. Se li trova, li carica in memoria (**Open-Closed Principle**).
*   Wireshark legge e scrive continuamente dal File System i file di configurazione (*preferences*, *colorfilters*, *recent*) situati nella directory utente. Questo permette di isolarei settaggi (**Separation of Concerns**) e trasferire "Profili" interi da un PC all'altro semplicemente copiando una cartella.

### Stakeholders


---


## C4 Model - Container Level

Si elencano i principali container di Wireshark:

### dumpcap (.exe/.app)
Programma a riga di comando scritto in C il cui unico e solo scopo è catturare pacchetti di rete e salvarli su disco. 

*   **Invocazione e Configurazione:** Quando si inizia a catturare il traffico rete, viene lanciato l'eseguibile `dumpcap` in background con degli eventuali parametri. I parametri inseriti dalla UI (interface, path, capture-filter, promiscuous-mode, ecc.) vengono tradotti dalla UI per generare una lunga riga di comando per invocare `dumpcap`. Quest'ultimo, a sua volta, usa le API della libreria del Sistema Operativo `libpcap`/`npcap` per catturare il traffico di rete nella modalità desiderata.
*   **Ciclo di Cattura:** `dumpcap` entra in un ciclo di ascolto continuo (tramite la funzione `pcap_loop()`). Ogni volta che un pacchetto fisico attraversa la scheda di rete, il Kernel lo copia in un buffer di memoria, `libpcap` lo normalizza e invoca una funzione (callback) dentro `dumpcap` per consegnargli il pacchetto.
*   **Persistenza:** `dumpcap` prende il pacchetto normalizzato da `libpcap`/`npcap` e lo scrive in un file temporaneo sul disco rigido (in formato `.pcapng`).
*   **Comunicazione IPC:** `dumpcap` avvisa la GUI di Wireshark del salvataggio dei nuovi pacchetti tramite una pipe di sistema. La GUI legge il file in modo asincrono, decodifica i byte e li mostra a schermo all'utente.

Solo `dumpcap` ha bisogno di girare con i privilegi di Amministratore/Root per accedere alla scheda di rete. La GUI di Wireshark (enorme e potenzialmente vulnerabile) gira come utente normale. Se un pacchetto maligno fa crashare la GUI, il sistema operativo è salvo. Inoltre, essendo separato, `dumpcap` si occupa solo dell'operazione di I/O (Rete -> Disco) alla massima velocità.

