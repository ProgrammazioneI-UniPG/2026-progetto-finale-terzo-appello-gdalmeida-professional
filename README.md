[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/5fsIc7xe)

# Progetto-finale-2025-Cosestrane

Progetto finale Programmazione Procedurale UniPG Informatica

## Nome: GIORDANO FEMI

## Cognome: D'ALMEIDA

## Matricola: 392361

## Commenti/modifiche al progetto:

**MODIFICHE**
Il progetto implementa tutte le specifiche richieste e introduce diverse ottimizzazioni tecniche per garantire stabilità e funzionalità avanzate:

### Tracciamento degli ultimi vincitori tramite file per una persistenza una volta terminata l'esecuzione del programma

- **File:** `ultimi_tre_vincitori.txt` (creato automaticamente)
- **Funzioni:** `carica_ultime_vittorie()`, `scrivi_ultime_vittorie()`, `registra_vittoria()`:

  -`carica_ultime_vittorie()`: All'avvio della visualizzazione dei crediti o prima di registrare una nuova vittoria, questa funzione apre il file in modalità lettura ("r"). Legge i nomi salvati e popola l'array array_vincitori in memoria RAM, garantendo che il programma lavori sempre con lo storico più recente.

  -`scrivi_ultime_vittorie()`: Viene richiamata subito dopo la registrazione di una nuova vittoria. Apre il file in modalità scrittura ("w"), sovrascrivendo il contenuto precedente con l'array aggiornato. Questo assicura che il file su disco rispecchi esattamente lo stato della memoria.

  -`registra_vittoria()`:Gestisce l'aggiornamento della classifica utilizzando un algoritmo di shifting (scorrimento). Se la lista è piena (3 vincitori), fa scorrere gli elementi verso il basso (eliminando il più vecchio) per liberare la prima posizione (indice 0), dove viene inserito il nuovo vincitore.

**COMMENTI**

- **Gestione Avanzata della Memoria (Anti-Crash):**
  È stata posta particolare attenzione alla gestione della memoria dinamica nel ciclo di gioco. La deallocazione (free) avviene in modo controllato prima della chiusura del programma o in caso di Game Over, prevenendo memory leaks e segmentation fault, specialmente durante la rimozione dei giocatori sconfitti.

- **Input Sanitization (Robustezza):**
  Tutti gli input da tastiera sono gestiti tramite un sistema di pulizia del buffer. Questo previene i loop infiniti comuni alla funzione scanf quando vengono inseriti caratteri al posto di numeri, garantendo che i menu non si blocchino mai in caso di errore di digitazione.

- **Strutture Dati Dinamiche:**
  La mappa di gioco è implementata tramite liste doppiamente collegate con gestione dinamica dei puntatori per l'inserimento e la rimozione di nodi (zone) in testa, coda e posizioni intermedie, mantenendo sempre coerenti i collegamenti tra Mondo Reale e Soprasotto.

---

## Come Giocare

### Menu Principale

1. **Imposta Gioco**: Configura giocatori e mappa
2. **Gioca**: Inizia la partita
3. **Termina gioco**: Libera memoria dinamica e chiude il programma
4. **Visualizza crediti**: Mostra sviluppatore e ultimi 3 vincitori

### Setup Iniziale

1. Scegliere il numero di giocatori (1-4)
2. Configurare ogni giocatore:
   - Inserire nome (max 50 caratteri)
   - Statistiche generate casualmente (1-20)
   - Opzione per modificare ATT/DIF (+3/-3)
   - Scelta personaggio speciale "UndiciVirgolaCinque"
3. Accedere al Menu del Game Master
4. Generare la mappa (minimo 15 zone)
5. Opzionale: Inserire/cancellare zone manualmente
6. **IMPORTANTE**: Chiudere la mappa prima di giocare

### Durante la Partita

Ogni giocatore, nel proprio turno, può:

- **Avanza**: Muoversi alla zona successiva (se non c'è un nemico)
- **Indietreggia**: Tornare alla zona precedente
- **Cambia Mondo**: Passare tra Mondo Reale e Soprasotto
- **Combatti**: Affrontare un nemico nella zona corrente
- **Raccogli Oggetto**: Prendere oggetti trovati (solo Mondo Reale)
- **Utilizza Oggetto**: Consumare oggetti dallo zaino per bonus
- **Stampa Info**: Visualizzare statistiche o dettagli zona
- **Passa il Turno**: Terminare il turno

### Obiettivo

Raggiungere e **sconfiggere il Demotorzone** senza morire!
