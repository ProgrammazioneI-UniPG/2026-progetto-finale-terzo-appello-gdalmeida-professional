#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gamelib.h"
#define FILE_VITTORIE "ultimi_tre_vincitori.txt" // Nome del file che verrà creato nella cartella del progetto per visualizzare gli ultimi 3 vincitori

// VARIABILI STATICHE: Visibili solo in questo file
static struct Giocatore *giocatori[4] = {NULL}; // Array di puntatori per max 4 giocatori
static int n_giocatori = 0;
static int undici_preso = 0; // 0 = disponibile, 1 = già occupato

static struct Zona_mondoreale *prima_zona_mondoreale = NULL;

static struct Zona_soprasotto *prima_zona_soprasotto = NULL;
static int mappa_chiusa = 0; // 0 = aperta/incompleta, 1 = chiusa/pronta

// --- VARIABILI GLOBALI STATICHE PER STATISTICHE ---
static char array_vincitori[3][51];   // Array per i nomi degli ultimi 3 vincitori
static int n_vittorie_registrate = 0; // Contatore: quanti nomi ci sono (0, 1, 2 o 3)
static int partite_giocate = 0;       // Contatore totale partite

// DICHIARAZIONE FUNZIONI STATICHE
static const char *get_nome_nemico(enum Tipo_nemico n);
static const char *get_nome_oggetto(enum Tipo_oggetto o);
static const char *get_nome_zona(enum Tipo_zona z);
static void pulisci_giocatori();
static void libera_mappa();
static int reset_gioco();
static void imposta_giocatori();
static void stampa_giocatori();
static void menu_mappa();
static void genera_mappa();
static void stampa_mappa();
static void stampa_zona();
static void inserisci_zona();
static void cancella_zona();
static void chiudi_mappa();
static int conta_demotorzoni();
static void get_zona(struct Zona_mondoreale *mr, int i);
static int conta_zone();
static void pulisci_schermo();
static void premi_invio();
static void stampa_giocatore_corrente(struct Giocatore *g);
static void avanza(struct Giocatore *g, int *ha_avanzato);
static void indietreggia(struct Giocatore *g);
static void cambia_mondo(struct Giocatore *g, int *ha_avanzato);
static void combatti(struct Giocatore *g, int *gioco_finito);
static void raccogli_oggetto(struct Giocatore *g);
static void utilizza_oggetto(struct Giocatore *g);
static void carica_ultime_vittorie();
static void scrivi_ultime_vittorie();
static void registra_vittoria(char *nome_vincitore);


static void registra_vittoria(char *nome_vincitore)
{
    partite_giocate++;

    // Scorrimento array
    if (n_vittorie_registrate == 3) // Se pieno, spostiamo
    {
        strcpy(array_vincitori[2], array_vincitori[1]);
        strcpy(array_vincitori[1], array_vincitori[0]);
    }
    else // Se c'è spazio, spostiamo solo quelli che ci sono
    {
        for (int i = n_vittorie_registrate; i > 0; i--)
        {
            strcpy(array_vincitori[i], array_vincitori[i - 1]);
        }
        n_vittorie_registrate++;
    }

    // Inseriamo il nuovo in testa
    strcpy(array_vincitori[0], nome_vincitore);

    scrivi_ultime_vittorie();
}

static void carica_ultime_vittorie()
{
    FILE *f = fopen(FILE_VITTORIE, "r"); // Apre in lettura
    if (f == NULL)
        return; // Se il file non esiste (prima partita mai fatta), non fa nulla

    n_vittorie_registrate = 0;
    char buffer[100];

    // Legge riga per riga fino a max 3 righe
    while (fgets(buffer, sizeof(buffer), f) != NULL && n_vittorie_registrate < 3)
    {
        // Rimuove l'invio '\n' alla fine della stringa letta dal file
        buffer[strcspn(buffer, "\n")] = 0;

        // Copia nel nostro array statico
        strncpy(array_vincitori[n_vittorie_registrate], buffer, 50);
        array_vincitori[n_vittorie_registrate][50] = '\0';
        n_vittorie_registrate++;
    }

    fclose(f);
}

static void scrivi_ultime_vittorie()
{
    FILE *f = fopen(FILE_VITTORIE, "w"); // Apre in scrittura (sovrascrive tutto)
    if (f == NULL)
    {
        printf("Errore nel salvataggio del file vittorie.\n");
        return;
    }

    // Scrive i nomi presenti nell'array su file
    for (int i = 0; i < n_vittorie_registrate; i++)
    {
        fprintf(f, "%s\n", array_vincitori[i]);
    }

    fclose(f);
}

static void avanza(struct Giocatore *g, int *ha_avanzato)
{
    if (*ha_avanzato)
    {
        printf("\nHai già avanzato in questo turno!\n");
        premi_invio();
        return;
    }
    enum Tipo_nemico nemico_qui = nessun_nemico;

    // controllo del nemico in base al mondo in cui ci troviamo ora
    if (g->mondo == 0)
    {
        nemico_qui = g->posiz_mondoreale->nemico;
    }
    else
    {
        nemico_qui = g->posiz_soprasotto->nemico;
    }

    if (nemico_qui != nessun_nemico)
    {
        printf("\nC'è un nemico (%s) che ti sbarra la strada!", get_nome_nemico(nemico_qui));
        printf("\nDevi sconfiggerlo (o cambiare mondo) prima di poter avanzare.\n");
        premi_invio();
        return;
    }

    // Avanzamento
    if (g->mondo == 0) // Siamo nel Mondo Reale
    {
        if (g->posiz_mondoreale->avanti == NULL) // Controllo dell'esistenza della zona successiva
        {
            printf("\nSei arrivato all'ultima zona della mappa! Non puoi andare oltre.\n");
        }
        else
        {
            // il giocatore va alla zona successiva
            g->posiz_mondoreale = g->posiz_mondoreale->avanti;

            // Aggiornamento del linker
            g->posiz_soprasotto = g->posiz_mondoreale->link_soprasotto;

            printf("\nTi sei spostato in AVANTI nel Mondo Reale.\n");
            *ha_avanzato = 1; // Impostando ha_avanzato a 1 indichiamo che il giocatore ha avanzato alla zona successiva per evitare che possa riavanzare o se ha cambiato mondo
        }
    }
    else
    {
        if (g->posiz_soprasotto->avanti == NULL)
        {
            printf("\nSei all'ultima zona oscura! Non puoi andare oltre.\n");
        }
        else
        {

            g->posiz_soprasotto = g->posiz_soprasotto->avanti;

            g->posiz_mondoreale = g->posiz_soprasotto->link_mondoreale;

            printf("\nTi sei spostato in AVANTI nel Soprasotto.\n");
            *ha_avanzato = 1;
        }
    }

    premi_invio();
}

static void indietreggia(struct Giocatore *g)
{

    enum Tipo_nemico nemico_qui = nessun_nemico;

    if (g->mondo == 0) // Mondo Reale
    {
        nemico_qui = g->posiz_mondoreale->nemico;
    }
    else // Soprasotto
    {
        nemico_qui = g->posiz_soprasotto->nemico;
    }

    if (nemico_qui != nessun_nemico)
    {
        printf("\nC'è un nemico (%s) che ti blocca! Non puoi indietreggiare.\n", get_nome_nemico(nemico_qui));
        printf("Devi sconfiggerlo prima di muoverti.\n");
        premi_invio();
        return;
    }

    if (g->mondo == 0) // Mondo Reale
    {
        if (g->posiz_mondoreale->indietro == NULL)
        {
            printf("\nSei all'inizio del percorso! Non puoi andare piu' indietro di così.\n");
        }
        else
        {
            // Indietreggiamento
            g->posiz_mondoreale = g->posiz_mondoreale->indietro;

            g->posiz_soprasotto = g->posiz_mondoreale->link_soprasotto;

            printf("\nSei tornato INDIETRO nel Mondo Reale.\n");
        }
    }
    else // Soprasotto
    {
        if (g->posiz_soprasotto->indietro == NULL)
        {
            printf("\nSei all'inizio del percorso nel SOPRASOTTO! Non puoi andare piu' indietro.\n");
        }
        else
        {
            g->posiz_soprasotto = g->posiz_soprasotto->indietro;

            g->posiz_mondoreale = g->posiz_soprasotto->link_mondoreale;

            printf("\nSei tornato INDIETRO nel Soprasotto.\n");
        }
    }
    premi_invio();
}

static void cambia_mondo(struct Giocatore *g, int *ha_avanzato)
{
    // Caso in cui dal mondo reale vogliamo andare nel soprasotto
    if (g->mondo == 0)
    {
        if (*ha_avanzato)
        {
            printf("\nHai già avanzato in questo turno. Non puoi cambiare mondo ora.\n");
            premi_invio();
            return;
        }

        // Controllo del nemico
        if (g->posiz_mondoreale->nemico != nessun_nemico)
        {
            printf("\nC'è un nemico in questa zona!\n");
            printf("Devi prima sconfiggerlo per cambiare mondo.\n");
            premi_invio();
            return;
        }

        // Esecuzione del cambio mondo
        printf("\nStai Entrando nel SOPRASOTTO...");
        premi_invio();

        // Aggiornamento della posizione nel soprasotto usando il link diretto dalla zona attuale
        g->posiz_soprasotto = g->posiz_mondoreale->link_soprasotto;

        g->mondo = 1;     // Impostazione dello stato a Soprasotto
        *ha_avanzato = 1; // Impostando il flag a 1 in questo turno non si potranno piu eseguire avanza o cambio mondo
        pulisci_schermo();
        printf("\nBenvenuto nel SOPRASOTTO.\n");
    }
    // Caso in cui dal Soprasotto vogliamo andare nel Mondo reale
    else
    {

        // Prova del dado
        int dado = (rand() % 20) + 1;
        printf("\nHai tirato un %d (La tua Fortuna e': %d)", dado, g->fortuna);

        // Se il dado è strettamente inferiore alla fortuna
        if (dado < g->fortuna)
        {
            // Avviene lo spostamento al mondo reale
            g->posiz_mondoreale = g->posiz_soprasotto->link_mondoreale;

            g->mondo = 0;

            // Qui non ci sono altri tipi di vincoli
            printf("\nCe l'hai fatta! Sei tornato nel Mondo Reale.\n");
        }
        else
        {
            // Fallimento
            printf("\nLa Fortuna non è dalla tua parte. Rimani bloccato nel Sottosopra.\n");
        }
    }
    premi_invio();
}
static void combatti(struct Giocatore *g, int *gioco_finito)
{
    // inizializzamo un puntatore che punterà al nemico di dove si trova il giocatore
    enum Tipo_nemico *nemico_ptr = NULL;
    if (g->mondo == 0)
    {
        nemico_ptr = &(g->posiz_mondoreale->nemico);
    }
    else
    {
        nemico_ptr = &(g->posiz_soprasotto->nemico);
    }

    enum Tipo_nemico nemico_attuale = *nemico_ptr;

    // controllo se è presente
    if (nemico_attuale == nessun_nemico)
    {
        printf("\nNon c'è nessuno da combattere qui!\n");
        premi_invio();
        return;
    }

    pulisci_schermo();
    printf("\n--- INIZIO COMBATTIMENTO CONTRO: %s ---", get_nome_nemico(nemico_attuale));

    // impostazione difficoltà nemico
    // sogli_vittoria serve a determinare quanto serve per sconfiggere il nemico
    // per vincere devi raggiungere o superare con la somma del tuo Attacco + Dado + Fortuna per vincere.
    int soglia_vittoria = 0;

    // danno indica quanti danni ti fa il nemico se perdi lo scontro
    int danno = 0;

    switch (nemico_attuale)
    {
    case billi:
        soglia_vittoria = 15; // facile
        danno = 2;
        break;
    case democane:
        soglia_vittoria = 22; // medio
        danno = 4;
        break;
    case demotorzone:
        soglia_vittoria = 35; // boss
        danno = 100;          // danno letale (quasi impossibile sopravvivere se perdi)
        break;
    default:
        break;
    }

    // Calcolo del punteggio di attacco
    int dado = (rand() % 20) + 1;

    // Formula: Attacco + Dado + (Fortuna / 2)
    int forza_totale = g->attacco_psichico + dado + (g->fortuna / 2);

    printf("\n\nLancio del dado (da 1 a 20): %d", dado);

    printf("\nTua Forza Totale: %d (Att) + %d (Dado) + %d (Bonus Fort) = %d",
           g->attacco_psichico, dado, (g->fortuna / 2), forza_totale);
    printf("\nDifesa del Nemico: %d", soglia_vittoria);

    // SCONTRO
    if (forza_totale >= soglia_vittoria)
    {
        // VITTORIA
        printf("\n\nHAI VINTO LO SCONTRO! Il nemico è stato sconfitto.");

        // Se era il Demotorzone, il gioco finisce con una vittoria
        if (nemico_attuale == demotorzone)
        {
            pulisci_schermo();
            printf("\n\n*****************************************");
            printf("\n* HAI SCONFITTO IL DEMOTORZONE!     *");
            printf("\n* IL MONDO E' SALVO GRAZIE A TE!    *");
            printf("\n*****************************************\n");

            registra_vittoria(g->nome);

            *gioco_finito = 0; // Segnala al ciclo while di gioca() di fermarsi
            premi_invio();
            return;
        }
        if ((rand() % 100) < 50)
        {
            printf("\nIl nemico svanisce nell'oscurità (Rimosso dalla mappa).");
            *nemico_ptr = nessun_nemico; // Modifica effettiva sulla mappa
        }
        else
        {
            printf("\nIl nemico è a terra stordito, ma rimane nella zona.");
        }
    }
    else
    {
        // SCONFITTA
        printf("\n\nSEI STATO COLPITO! Il nemico era troppo forte.");
        printf("\nSubisci %d danni a Attacco, Difesa e Fortuna.", danno);

        g->attacco_psichico -= danno;
        g->difesa_psichica -= danno;
        g->fortuna -= danno;

        // Non permettiamo che vadano sotto zero, ci penserà poi il controllo morte in gioca()
        if (g->attacco_psichico < 0)
            g->attacco_psichico = 0;
        if (g->difesa_psichica < 0)
            g->difesa_psichica = 0;
        if (g->fortuna < 0)
            g->fortuna = 0;

        printf("\nNuove Statistiche -> ATT: %d | DIF: %d | FORT: %d",
               g->attacco_psichico, g->difesa_psichica, g->fortuna);
    }
    premi_invio();
    /*  NOTA SULLE CONSEGUENZE DELLA SCONFITTA:
     * Se il giocatore sopravvive (le statistiche non scendono a 0),
     * la posizione NON cambia e il nemico NON viene rimosso.
     * * Di conseguenza:
     * 1. Il giocatore resta bloccato nella zona corrente.
     * 2. Le azioni 'Avanza' e 'Indietreggia' saranno impedite dal nemico ancora vivo.
     * 3. Il giocatore dovrà scegliere se combattere di nuovo o scappare (Cambia Mondo).
     */
}

static void raccogli_oggetto(struct Giocatore *g)
{
    // Controllo del Mondo: Gli oggetti sono solo nel Mondo Reale
    if (g->mondo == 1)
    {
        printf("\nNel Sottosopra non sono presenti oggetti, solo polvere e cenere.\n");
        premi_invio();
        return;
    }

    struct Zona_mondoreale *zona = g->posiz_mondoreale;

    // Controllo Nemico: Non si può raccogliere un oggetto se prima non si è ucciso il nemico
    if (zona->nemico != nessun_nemico)
    {
        printf("\nC'è un nemico (%s) che ti osserva minaccioso!", get_nome_nemico(zona->nemico));
        printf("\nNon puoi raccogliere oggetti mentre sei minacciato. Sconfiggilo prima.\n");
        premi_invio();
        return;
    }

    // Controllo esistenza oggetto nella stanza
    if (zona->oggetto == nessun_oggetto)
    {
        printf("\nIn questa zona non c'e' nulla da raccogliere.\n");
        premi_invio();
        return;
    }

    // Ricerca del primo slot libero nello zaino
    int slot_libero = -1;
    for (int i = 0; i < 3; i++)
    {
        // Cerchiamo uno slot che contenga "nessun_oggetto"
        if (g->zaino[i] == nessun_oggetto)
        {
            slot_libero = i;
            break; // slot trovato, uscita dal ciclo
        }
    }

    if (slot_libero == -1)
    {
        printf("\nIl tuo zaino e' PIENO! (3/3 oggetti)");
        printf("\nDevi usare un oggetto per liberare spazio prima di raccoglierne un altro.\n");
    }

    else
    {
        int scelta_raccolta = -1;
        int input_valido = 0; // Flag per controllare se l'input è corretto

        do
        {
            printf("\nHai trovato: %s!", get_nome_oggetto(zona->oggetto));
            printf("\nVuoi raccoglierlo e metterlo nello zaino?");
            printf("\n[1] SI, raccogli");
            printf("\n[0] NO, lascia a terra");
            printf("\nScelta: ");

            int result = scanf("%d", &scelta_raccolta);

            while (getchar() != '\n')
                ;

            // Controllo validità:
            // result != 1 significa che non ha inserito un numero
            // oppure il numero non è né 0 né 1
            if (result != 1 || (scelta_raccolta != 0 && scelta_raccolta != 1))
            {
                printf("\nErrore: Devi inserire SOLO 0 o 1. Riprova.\n");
                input_valido = 0; // Ripete il ciclo
            }
            else
            {
                input_valido = 1; // Esce dal ciclo
            }
        } while (!input_valido);

        // --- ESECUZIONE SCELTA ---
        if (scelta_raccolta == 1)
        {
            // RACCOLTA
            g->zaino[slot_libero] = zona->oggetto;
            printf("\nHai raccolto: %s!\n", get_nome_oggetto(zona->oggetto));

            // Rimuoviamo l'oggetto dalla stanza
            zona->oggetto = nessun_oggetto;
        }
        else
        {
            printf("\nHai deciso di lasciare l'oggetto dove si trova.\n");
        }
    }

    premi_invio();
}

static void utilizza_oggetto(struct Giocatore *g)
{
    // Mostra a schermo lo zaino
    pulisci_schermo();
    printf("\n--- ZAINO DI %s ---", g->nome);
    int oggetti_presenti = 0;

    for (int i = 0; i < 3; i++)
    {
        if (g->zaino[i] != nessun_oggetto)
        {
            printf("\n[%d] %s", i + 1, get_nome_oggetto(g->zaino[i]));
            oggetti_presenti++;
        }
        else
        {
            printf("\n[%d] [Vuoto]", i + 1);
        }
    }

    if (oggetti_presenti == 0)
    {
        printf("\n\nIl tuo zaino e' vuoto. Non puoi usare nulla.\n");
        premi_invio();
        return;
    }

    int scelta = -1;
    int input_valido = 0;

    do
    {
        printf("\n\nScegli l'oggetto da usare (1-3) o premi 0 per annullare: ");
        int result = scanf("%d", &scelta);

        while (getchar() != '\n')
            ;

        // Validazione: deve essere un numero, compreso tra 0 e 3
        if (result != 1 || scelta < 0 || scelta > 3)
        {
            printf("\nErrore: Inserisci un numero tra 0 e 3.");
            input_valido = 0;
        }
        else if (scelta != 0 && g->zaino[scelta - 1] == nessun_oggetto)
        {
            // Se sceglie uno slot vuoto (es. preme 2 ma lo slot 2 è vuoto)
            printf("\nErrore: Lo slot %d e' vuoto!", scelta);
            input_valido = 0;
        }
        else
        {
            input_valido = 1;
        }
    } while (!input_valido);

    // Se l'utente ha scelto 0, annulliamo tutto
    if (scelta == 0)
    {
        printf("\nAzione annullata.\n");
        premi_invio();
        return;
    }

    // Applicazione Effetti
    // l'utente ha inserito 1, 2 o 3, ma l'array parte da 0. Quindi usiamo [scelta - 1]
    enum Tipo_oggetto oggetto_usato = g->zaino[scelta - 1];

    printf("\nStai utilizzando %s...", get_nome_oggetto(oggetto_usato));

    switch (oggetto_usato)
    {
    case maglietta_fuocoinferno:
        printf("\nIndossi la maglietta! Ti senti piu' forte. (Attacco +2)\n");
        g->attacco_psichico += 2;
        break;

    case bicicletta:
        printf("\nSali in sella! Sei piu' agile. (Difesa +2)\n");
        g->difesa_psichica += 2;
        break;

    case bussola:
        printf("\nOra sai dove andare! La fortuna ti assiste. (Fortuna +3)\n");
        g->fortuna += 3;
        break;

    case schitarrata_metallica:
        printf("\nSUONI UN ASSOLO LEGGENDARIO! (Attacco +5, Fortuna +5)\n");
        g->attacco_psichico += 5;
        g->fortuna += 5;
        break;

    default:
        printf("\nNon succede nulla di speciale.\n");
        break;
    }

    // Rimozione oggetto dallo zaino (utilizzato)
    g->zaino[scelta - 1] = nessun_oggetto;

    printf("\nL'oggetto e' stato usato e rimosso dallo zaino.\n");
    premi_invio();
}

static void stampa_giocatore_corrente(struct Giocatore *g)
{
    pulisci_schermo();
    printf("\n--- STATISTICHE DI %s ---", g->nome);
    printf("\nAttacco: %d", g->attacco_psichico);
    printf("\nDifesa:  %d", g->difesa_psichica);
    printf("\nFortuna: %d", g->fortuna);
    printf("\nZaino: ");
    for (int i = 0; i < 3; i++)
        printf("[%s] ", get_nome_oggetto(g->zaino[i]));
    printf("\n---------------------------\n");
    premi_invio();
}

// FUNZIONI DI GIOCO
static void mischia_giocatori(int *ordine, int n);
static void uccidi_giocatore(int i);
static int conta_giocatori_vivi();

void gioca()
{
    if (n_giocatori == 0)
    {
        printf("\nErrore: Nessun giocatore. Impostali prima.\n");
        premi_invio();
        return;
    }
    if (mappa_chiusa == 0)
    {
        printf("\nErrore: Mappa non pronta. Chiudila nel menu mappa.\n");
        premi_invio();
        return;
    }

    // Posizionamento iniziale nella prima zona del mondo reale

    for (int i = 0; i < n_giocatori; i++)
    {
        if (giocatori[i] != NULL)
        {
            giocatori[i]->posiz_mondoreale = prima_zona_mondoreale;
            giocatori[i]->mondo = 0; // Parte sempre nel Mondo Reale
        }
    }

    int gioco_in_corso = 1; // 1 (partita attiva) / 0 (partita terminata o non attiva)
    int ordine_turni[4];    // Array per l'ordine casuale
    int turno_round = 1;    // Contatore dei round

    while (gioco_in_corso)
    {
        if (conta_giocatori_vivi() == 0)
        {
            pulisci_schermo();
            printf("\n\n!!! GAME OVER !!!\n--- Tutti i giocatori sono morti. ---\n");
            termina_gioco();
            premi_invio();
            return;
        }

        mischia_giocatori(ordine_turni, n_giocatori);

        printf("\n--- INIZIO ROUND %d ---\n", turno_round);
        premi_invio();

        // scorrimento dei giocatori nell'ordine casuale
        for (int k = 0; k < n_giocatori; k++)
        {
            int giocatore_di_turno = ordine_turni[k]; // recupera il giocatore di turno

            // Se il giocatore è morto (puntatore NULL), saltiamo il turno
            if (giocatori[giocatore_di_turno] == NULL)
                continue;

            struct Giocatore *g = giocatori[giocatore_di_turno];

            int fine_turno = 0;  // flag per lo stato del turno del gicoatore
            int ha_avanzato = 0; // flag per far usare la funzione avanza solo 1 volta o 0 volte se si cambia il mondo

            while (!fine_turno && giocatori[giocatore_di_turno] != NULL) // Controllo != NULL vitale se muore nel suo turno
            {
                pulisci_schermo();
                printf("\n##################################################");
                printf("\n# TURNO DI: %s", g->nome);
                printf("\n# STATS: ATT %d | DIF %d | FORT %d", g->attacco_psichico, g->difesa_psichica, g->fortuna);
                const char *nome_zona = get_nome_zona((g->mondo == 0) ? g->posiz_mondoreale->tipo : g->posiz_soprasotto->tipo); // recuperiamo il nome della zona in base a se sta nel mondoreale o nel soprasotto
                printf("\n# POSIZIONE: %s (", nome_zona);

                if (g->mondo == 0)
                    printf("Mondo Reale)");
                else
                    printf("SOPRASOTTO)");

                printf("\n##################################################");

                printf("\nAZIONI DISPONIBILI:");
                printf("\n1) Avanza");
                if (ha_avanzato)
                    printf(" [NON DISPONIBILE]");
                printf("\n2) Indietreggia");
                printf("\n3) Cambia Mondo");
                printf("\n4) Combatti");
                printf("\n5) Raccogli Oggetto");
                printf("\n6) Utilizza Oggetto");
                printf("\n7) Stampa Info Giocatore");
                printf("\n8) Stampa Info Zona");
                printf("\n9) PASSA IL TURNO");
                printf("\nScelta: ");

                int scelta;
                if (scanf("%d", &scelta) != 1)
                {
                    printf("\n!!! Errore: Inserisci un numero valido !!!\n");
                    while (getchar() != '\n')
                        ;
                    premi_invio();
                    continue;
                }
                while (getchar() != '\n')
                    ;

                switch (scelta)
                {
                case 1:
                    avanza(g, &ha_avanzato);
                    break;
                case 2:
                    indietreggia(g);
                    break;
                case 3:
                    cambia_mondo(g, &ha_avanzato);
                    break;
                case 4:
                    // si passa &gioco_in_corso perché se batte il Boss, il gioco finisce (diventa 0) e la variabile non si rinizializza dentro gioca() ma si ferma al while prima vedendo 0 senza rientrare nel corpo del while
                    combatti(g, &gioco_in_corso);

                    // Se il gioco è finito (vittoria Boss), usciamo subito dallo switch e dal loop
                    if (gioco_in_corso == 0)
                    {
                        fine_turno = 1;
                        break;
                    }

                    // Se una statistica è scesa a 0 o meno, il giocatore muore.
                    if (g->attacco_psichico <= 0 || g->difesa_psichica <= 0 || g->fortuna <= 0)
                    {
                        uccidi_giocatore(giocatore_di_turno); // Free e NULL
                        fine_turno = 1;                       // Esce dal menu azioni
                    }
                    break;
                case 5:
                    raccogli_oggetto(g);
                    break;
                case 6:
                    utilizza_oggetto(g);
                    break;
                case 7:
                    stampa_giocatore_corrente(g);
                    break;
                case 8:
                    // Stampa dettagli zona corrente usando la funzione che già avevamo
                    printf("\n--- ZONA ATTUALE ---");
                    printf("\n-------------------------------------------------------------------------------------------------------------------------------");
                    printf("\n%-4s | %-25s | %-25s | %-15s || %-25s | %-15s", "POS", "ZONA (MR)", "OGGETTO (MR)", "NEMICO (MR)", "ZONA (SS)", "NEMICO (SS)");
                    printf("\n-------------------------------------------------------------------------------------------------------------------------------");

                    struct Zona_mondoreale *zona_target = NULL;
                    // Se siamo nel mondo reale usiamo il puntatore diretto, se siamo nel soprasotto usiamo il link inverso
                    if (g->mondo == 0)
                        zona_target = g->posiz_mondoreale;
                    else
                        zona_target = g->posiz_soprasotto->link_mondoreale;

                    // Calcoliamo l'indice scorrendo la lista dall'inizio
                    int indice_zona = 1;
                    struct Zona_mondoreale *temp = prima_zona_mondoreale;

                    while (temp != NULL && temp != zona_target)
                    {
                        temp = temp->avanti;
                        indice_zona++;
                    }

                    // Stampiamo passando l'indice calcolato
                    get_zona(zona_target, indice_zona);
                    printf("\n-------------------------------------------------------------------------------------------------------------------------------\n");
                    premi_invio();
                    break;
                case 9:
                    printf("\nTurno passato.\n");
                    premi_invio();
                    fine_turno = 1; // Esce dal while del turno
                    break;
                default:
                    printf("\nScelta non valida.\n");
                    premi_invio();
                    break;
                }

                // quando vince qualcuno esce
                if (gioco_in_corso == 0)
                    return;
            }
        }
        turno_round++;
    }
    termina_gioco();
}

static int conta_giocatori_vivi()
{
    int vivi = 0;
    for (int i = 0; i < n_giocatori; i++)
    {
        if (giocatori[i] != NULL)
        {
            vivi++;
        }
    }
    return vivi;
}

static void uccidi_giocatore(int i)
{
    if (giocatori[i] != NULL)
    {
        printf("\n\nIL GIOCATORE %s E' MORTO!\n", giocatori[i]->nome);
        printf("(Giocatore rimosso dalla partita)\n");

        free(giocatori[i]);
        giocatori[i] = NULL;

        premi_invio();
    }
}

static void mischia_giocatori(int *ordine, int n)
{
    // inizialliza l'array con gli indici ordinati
    for (int i = 0; i < n; i++)
    {
        ordine[i] = i;
    }

    // partendo dall'ultimo elemento mischia l'array in maniera casuale
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);

        int temp = ordine[i];
        ordine[i] = ordine[j];
        ordine[j] = temp;
    }
}

static void premi_invio()
{
    printf("\nPremi INVIO per continuare...");
    while (getchar() != '\n')
        ;
}

static void pulisci_schermo()
{
    system("clear");
}

static int conta_zone()
{
    int conta_zone = 0;
    struct Zona_mondoreale *temp = prima_zona_mondoreale;
    while (temp != NULL)
    {
        conta_zone++;
        temp = temp->avanti;
    }
    return conta_zone;
}

static void get_zona(struct Zona_mondoreale *mr, int i)
{
    struct Zona_soprasotto *ss = mr->link_soprasotto;

    printf("\n%-4d | %-25s | %-25s | %-15s || %-25s | %-15s",
           i,
           get_nome_zona(mr->tipo),
           get_nome_oggetto(mr->oggetto),
           get_nome_nemico(mr->nemico),
           get_nome_zona(ss->tipo),
           get_nome_nemico(ss->nemico));
}

static void stampa_zona()
{
    pulisci_schermo();
    if (prima_zona_mondoreale == NULL)
    {
        printf("\n!!! Mappa vuota !!!\n");
        return;
    }

    int n_zone = conta_zone();

    int pos = -1;
    do
    {
        printf("\n--- VISUALIZZA DETTAGLIO ZONA ---");
        printf("\nAttualmente ci sono %d zone.", n_zone);
        printf("\nQuale zona vuoi vedere? (Inserisci un valore da 1 a %d)", n_zone);
        printf("\n[0] per ANNULLARE");
        printf("\nScelta: ");

        if (scanf("%d", &pos) != 1)
        {
            pulisci_schermo();
            printf("\n!!! Errore: Inserisci un numero valido !!!\n");
            while (getchar() != '\n')
                ;
            pos = -1;
            continue;
        }
        while (getchar() != '\n')
            ;
        if (pos == 0)
            return;

        if (pos < 1 || pos > n_zone)
        {
            printf("\n!!! Errore: La zona %d non esiste! (Scegli tra 1 e %d) !!!\n", pos, n_zone);
        }
    } while (pos < 1 || pos > n_zone);

    struct Zona_mondoreale *zona_dastampare = prima_zona_mondoreale;

    for (int i = 1; i < pos; i++)
    {
        zona_dastampare = zona_dastampare->avanti;
    }

    printf("\n-------------------------------------------------------------------------------------------------------------------------------");
    printf("\n%-4s | %-25s | %-25s | %-15s || %-25s | %-15s",
           "POS", "ZONA (MR)", "OGGETTO (MR)", "NEMICO (MR)", "ZONA (SS)", "NEMICO (SS)");
    printf("\n--------------------------------------------------------------------------------------------------------");

    get_zona(zona_dastampare, pos);

    printf("\n-------------------------------------------------------------------------------------------------------------------------------");
}

static int conta_demotorzoni()
{
    int count = 0;
    struct Zona_soprasotto *curr = prima_zona_soprasotto;
    while (curr != NULL)
    {
        if (curr->nemico == demotorzone)
        {
            count++;
        }
        curr = curr->avanti;
    }
    return count;
}

static void chiudi_mappa()
{
    pulisci_schermo();
    if (prima_zona_mondoreale == NULL)
    {
        printf("\n!!! Errore: La mappa non esiste neanche! Generala prima. !!!\n");
        return;
    }

    int n_zone = conta_zone();

    int n_boss = conta_demotorzoni();

    // Verifica che ci siano almeno 15 zone
    if (n_zone < 15)
    {
        printf("\n!!! Errore: La mappa è troppo corta (%d zone) !!!", n_zone);
        printf("\nDeve avere almeno 15 zone per essere chiusa.\n");
        return;
    }

    // Verifica che ci sia un demotorzone
    if (n_boss == 0)
    {
        printf("\n!!! Errore Chiusura: Manca il DEMOTORZONE! Devi inserirne uno. !!!\n");
        return;
    }
    if (n_boss > 1)
    {
        printf("\n!!! Errore Chiusura: Ci sono troppi DEMOTORZONE (%d)! Ce ne può essere solo 1. !!!\n", n_boss);
        return;
    }

    mappa_chiusa = 1; // Questa variabile globale sbloccherà l'opzione "Gioca" nel menu principale

    printf("\n--------------------------------------------------");
    printf("\nMappa chiusa con successo!");
    printf("\nNumero totale zone: %d", n_zone);
    printf("\nOra puoi tornare al menu principale e iniziare a giocare.");
    printf("\n--------------------------------------------------\n");
}

static void cancella_zona()
{
    pulisci_schermo();
    if (mappa_chiusa == 1)
    {
        printf("\n!!! Errore: La mappa è sigillata. Non puoi più modificarla! !!!\n");
        return;
    }
    if (prima_zona_mondoreale == NULL)
    {
        printf("\n!!! Errore: La mappa è vuota, nulla da cancellare !!!\n");
        return;
    }
    int pos = -1;
    int n_zone = conta_zone();

    int boss_trovato_in_selezione = 0; // Flag per gestire l'errore del boss (0 se l'errore non c'è quindi il boss non è presente in quella posizione / 1 se è presente)
    do
    {

        boss_trovato_in_selezione = 0;
        printf("\n--- CANCELLAZIONE ZONA ---");
        printf("\nCi sono %d zone.", n_zone);
        printf("\nQuale posizione vuoi eliminare? (1 a %d)", n_zone);
        printf("\n[0] per ANNULLARE: ");

        if (scanf("%d", &pos) != 1)
        {
            printf("\n!!! Errore: Inserisci un numero valido !!!\n");
            while (getchar() != '\n')
                ;
            pos = -1;
            continue;
        }

        if (pos == 0)
            return;

        if (pos < 1 || pos > n_zone)
        {
            printf("\n!!! Errore: Posizione non valida !!!\n");
            continue;
        }

        // Scorriamo momentaneamente fino alla posizione scelta per controllare il nemico
        struct Zona_soprasotto *check_boss = prima_zona_soprasotto;
        for (int j = 1; j < pos; j++)
        {
            check_boss = check_boss->avanti;
        }

        if (check_boss->nemico == demotorzone)
        {
            printf("\n!!! AZIONE NEGATA: La zona %d contiene il DEMOTORZONE! !!!", pos);
            printf("\nIl Boss è fondamentale per il gioco. Non puoi cancellarlo.");
            printf("\nScegli un'altra zona da eliminare.\n");

            boss_trovato_in_selezione = 1; // Attiviamo il flag di errore
            pos = -1;
        }
    } while (((pos < 1) || (pos > n_zone)) || (boss_trovato_in_selezione == 1));

    struct Zona_mondoreale *da_eliminare_mondoreale = prima_zona_mondoreale;
    struct Zona_soprasotto *da_eliminare_soprasotto = prima_zona_soprasotto;

    // Scorrimento fino alla posizione da eliminare
    for (int i = 1; i < pos; i++)
    {
        da_eliminare_mondoreale = da_eliminare_mondoreale->avanti;
        da_eliminare_soprasotto = da_eliminare_soprasotto->avanti;
    }

    // RICOLLEGAMENTO DEI PUNTATORI
    // Ricollegamento del puntatore avanti
    if (da_eliminare_mondoreale->indietro != NULL)
    {
        // Se non è la testa, il nodo prima deve saltare quello attuale
        da_eliminare_mondoreale->indietro->avanti = da_eliminare_mondoreale->avanti;
        da_eliminare_soprasotto->indietro->avanti = da_eliminare_soprasotto->avanti;
    }
    else
    {
        // Se è la testa, aggiorniamo il punto di inizio della lista
        prima_zona_mondoreale = da_eliminare_mondoreale->avanti;
        prima_zona_soprasotto = da_eliminare_soprasotto->avanti;
    }

    //  Ricollegamento del puntatore indietro
    if (da_eliminare_mondoreale->avanti != NULL)
    {
        // Se non è l'ultima zona, il nodo dopo deve guardare a quello prima di quello attuale
        da_eliminare_mondoreale->avanti->indietro = da_eliminare_mondoreale->indietro;
        da_eliminare_soprasotto->avanti->indietro = da_eliminare_soprasotto->indietro;
    }

    // 5. LIBERAZIONE MEMORIA
    free(da_eliminare_mondoreale);
    free(da_eliminare_soprasotto);

    while (getchar() != '\n')
        ;
    printf("\nZona in posizione %d eliminata correttamente.\n", pos);
}

static void inserisci_zona()
{
    pulisci_schermo();
    if (mappa_chiusa == 1)
    {
        printf("\n!!! Errore: La mappa è sigillata. Non puoi più modificarla! !!!\n");
        return;
    }
    int pos = -1;
    int n_zone = conta_zone();

    do
    {
        printf("\n--- INSERIMENTO NUOVA ZONA ---");
        printf("\nZone attuali: %d", n_zone);
        printf("\nIn quale posizione vuoi inserire la nuova zona?");
        printf("\n[1] per la testa");
        printf("\n[%d] per l'ultima posizione", n_zone + 1);
        printf("\n[0] per ANNULLARE e tornare al menu");
        printf("\nScelta: ");

        if (scanf("%d", &pos) != 1)
        {
            printf("\n!!! Errore: Inserisci un numero valido !!!\n");
            while (getchar() != '\n')
                ;
            pos = -1;
            continue;
        }

        if (pos == 0)
        {
            printf("\nInserimento annullato.\n");
            return;
        }

        if (pos < 1 || pos > n_zone + 1)
        {
            printf("\n!!! Errore: Posizione non valida! Inserire un valore tra 1 e %d !!!\n", n_zone + 1);
        }

    } while (pos < 1 || pos > n_zone + 1);

    // Inserimento della zona
    struct Zona_mondoreale *nuova_mondoreale = (struct Zona_mondoreale *)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto *nuova_soprasotto = (struct Zona_soprasotto *)malloc(sizeof(struct Zona_soprasotto));

    if (!nuova_mondoreale || !nuova_soprasotto) //! nuova_.. significa -> se è NULL, perchè un puntatore è considerato vero se restituisce un indirizzo di memoria
    {
        printf("Errore fatale memoria!\n");
        exit(1);
    }

    // Generazione campi
    nuova_mondoreale->tipo = rand() % 10; // Tipo zona casuale (come in genera_mappa)
    nuova_soprasotto->tipo = nuova_mondoreale->tipo;

    // --- Scelta oggetto (Mondo Reale) ---
    int scelta = -1;
    do
    {
        printf("\nScegli l'oggetto per il Mondo Reale:");
        printf("\n[0] Nessuno");
        printf("\n[1] Bicicletta");
        printf("\n[2] Maglietta FuocoInferno");
        printf("\n[3] Bussola");
        printf("\n[4] Schitarrata Metallica");
        printf("\nScelta: ");
        if (scanf("%d", &scelta) != 1)
        {
            printf("\n!!! Errore: Inserisci un numero valido !!!\n");
            while (getchar() != '\n')
                ;
            scelta = -1;
            continue;
        }
    } while (scelta < 0 || scelta > 4);
    nuova_mondoreale->oggetto = (enum Tipo_oggetto)scelta;

    scelta = -1;
    // --- Scelta Nemico (Mondo Reale) ---
    do
    {
        printf("\nScegli il nemico per il Mondo Reale:");
        printf("\n[0] Nessuno");
        printf("\n[1] Billi");
        printf("\n[2] Democane");
        printf("\nScelta: ");
        if (scanf("%d", &scelta) != 1)
        {
            printf("\n!!! Errore: Inserisci un numero valido !!!\n");
            while (getchar() != '\n')
                ;
            scelta = -1;
            continue;
        }
    } while (scelta < 0 || scelta > 2);
    nuova_mondoreale->nemico = (enum Tipo_nemico)scelta;

    scelta = -1;

    // --- SCELTA NEMICO (Soprasotto) ---

    int boss_presente = conta_demotorzoni();
    do
    {
        printf("\nScegli il nemico per il Soprasotto:");
        printf("\n[0] Nessuno");
        printf("\n[2] Democane");

        // Mostriamo l'opzione Boss solo se non esiste già, oppure mostriamola ma diamo errore
        printf("\n[3] Demotorzone (BOSS) ");
        if (boss_presente)
            printf("[NON DISPONIBILE]");

        printf("\nScelta: ");
        if (scanf("%d", &scelta) != 1)
        {
            printf("\n!!! Errore: Inserisci un numero valido !!!\n");
            while (getchar() != '\n')
                ;
            scelta = -1;
            continue;
        }

        if (scelta != 0 && scelta != 2 && scelta != 3)
        {
            printf("\n!!! Errore: Opzione non valida! Scegli 0, 2 o 3 !!!\n");
            scelta = -1;
            continue;
        }

        if (scelta == 3 && boss_presente > 0)
        {
            printf("\n!!! ERRORE: Esiste già un Demotorzone nella mappa! Non puoi aggiungerne un altro. !!!\n");
            scelta = -1;
            continue;
        }

    } while (scelta != 0 && scelta != 2 && (scelta != 3 || boss_presente > 0));
    nuova_soprasotto->nemico = (enum Tipo_nemico)scelta;

    // Collegamenti tra mondi
    nuova_mondoreale->link_soprasotto = nuova_soprasotto;
    nuova_soprasotto->link_mondoreale = nuova_mondoreale;

    // Inserimento in lista
    if (pos == 1)
    {
        nuova_mondoreale->avanti = prima_zona_mondoreale;
        nuova_mondoreale->indietro = NULL;
        nuova_soprasotto->avanti = prima_zona_soprasotto;
        nuova_soprasotto->indietro = NULL;

        if (prima_zona_mondoreale != NULL) // serve se ancora non si è creata una mappa in modo tale che dopo non accada NULL -> indetro = nuova_mondoreale
        {
            prima_zona_mondoreale->indietro = nuova_mondoreale;
            prima_zona_soprasotto->indietro = nuova_soprasotto;
        }
        prima_zona_mondoreale = nuova_mondoreale;
        prima_zona_soprasotto = nuova_soprasotto;
    }
    else
    {
        struct Zona_mondoreale *curr_mondoreale = prima_zona_mondoreale;
        struct Zona_soprasotto *curr_soprasotto = prima_zona_soprasotto;
        for (int i = 1; i < pos - 1; i++)
        {
            curr_mondoreale = curr_mondoreale->avanti;
            curr_soprasotto = curr_soprasotto->avanti;
        }
        nuova_mondoreale->avanti = curr_mondoreale->avanti;
        nuova_mondoreale->indietro = curr_mondoreale;
        nuova_soprasotto->avanti = curr_soprasotto->avanti;
        nuova_soprasotto->indietro = curr_soprasotto;

        if (curr_mondoreale->avanti != NULL)
        {
            curr_mondoreale->avanti->indietro = nuova_mondoreale;
            curr_soprasotto->avanti->indietro = nuova_soprasotto;
        }
        curr_mondoreale->avanti = nuova_mondoreale;
        curr_soprasotto->avanti = nuova_soprasotto;
    }

    while (getchar() != '\n')
        ;
    printf("\n[OK] Zona inserita e configurata con successo!\n");
}

static void stampa_mappa()
{
    pulisci_schermo();
    if (prima_zona_mondoreale == NULL)
    {
        printf("\n!!! Errore: DEVI GENERARE LA PRIMA MAPPA !!!\n");
        return;
    }

    struct Zona_mondoreale *curr_mondoreale = prima_zona_mondoreale;
    int i = 1;

    printf("\n%-4s | %-25s | %-25s | %-15s || %-25s | %-15s",
           "POS", "ZONA (MR)", "OGGETTO (MR)", "NEMICO (MR)", "ZONA (SS)", "NEMICO (SS)");
    printf("\n-------------------------------------------------------------------------------------------------------------------------------");
    while (curr_mondoreale != NULL)
    {
        get_zona(curr_mondoreale, i);
        curr_mondoreale = curr_mondoreale->avanti;
        i++;
    }
    printf("\n-------------------------------------------------------------------------------------------------------------------------------");
}

static const char *get_nome_nemico(enum Tipo_nemico n)
{
    switch (n)
    {
    case nessun_nemico:
        return "Nessuno";
    case billi:
        return "Billi";
    case democane:
        return "Democane";
    case demotorzone:
        return "Demotorzone";
    default:
        return "-----";
    }
}

static const char *get_nome_oggetto(enum Tipo_oggetto o)
{
    switch (o)
    {
    case nessun_oggetto:
        return "Nessuno";
    case bicicletta:
        return "Bicicletta";
    case maglietta_fuocoinferno:
        return "Maglietta FuocoInferno";
    case bussola:
        return "Bussola";
    case schitarrata_metallica:
        return "Schitarrata Metallica";
    default:
        return "-----";
    }
}

static const char *get_nome_zona(enum Tipo_zona z)
{

    switch (z)
    {
    case bosco:
        return "Bosco";
    case scuola:
        return "Scuola";
    case laboratorio:
        return "Laboratorio";
    case caverna:
        return "Caverna";
    case strada:
        return "Strada";
    case giardino:
        return "Giardino";
    case supermercato:
        return "Supermercato";
    case centrale_elettrica:
        return "Centrale Elettrica";
    case deposito_abbandonato:
        return "Deposito Abbandonato";
    case stazione_polizia:
        return "Stazione di Polizia";
    default:
        return "-----";
    }
}
static void libera_mappa()
{
    struct Zona_mondoreale *curr_mondoreale = prima_zona_mondoreale;
    struct Zona_soprasotto *curr_soprasotto = prima_zona_soprasotto;
    struct Zona_mondoreale *prossimo_mondoreale = NULL;
    struct Zona_soprasotto *prossimo_soprasotto = NULL;

    while (curr_mondoreale != NULL)
    {
        // Salviamo il puntatore al nodo successivo prima di cancellare l'attuale
        prossimo_mondoreale = curr_mondoreale->avanti;
        prossimo_soprasotto = curr_soprasotto->avanti;

        free(curr_mondoreale);
        free(curr_soprasotto);

        curr_mondoreale = prossimo_mondoreale;
        curr_soprasotto = prossimo_soprasotto;
    }

    // Reset dei puntatori globali
    prima_zona_mondoreale = NULL;
    prima_zona_soprasotto = NULL;
    mappa_chiusa = 0;

    // Reset dei puntatori alle mappe dei giocatori
    for (int i = 0; i < n_giocatori; i++)
    {
        if (giocatori[i] != NULL)
        {
            giocatori[i]->posiz_mondoreale = NULL;
            giocatori[i]->posiz_soprasotto = NULL;
        }
    }
    printf("\nVecchia mappa eliminata e posizioni giocatori resettate.\n");
}
void termina_gioco()
{
    if (prima_zona_mondoreale != NULL)
    {
        libera_mappa();
    }

    pulisci_giocatori();

    printf("\n[SISTEMA] Memoria liberata: Mappa e Giocatori cancellati correttamente.\n");
}
static void genera_mappa()
{
    pulisci_schermo();
    // Se esiste già una mappa, la eliminiamo (sovrascrive le precedenti)
    if (prima_zona_mondoreale != NULL)
    {
        libera_mappa();
    }
    struct Zona_mondoreale *ultima_mondoreale = NULL;
    struct Zona_soprasotto *ultima_soprasotto = NULL;
    int pos_demotorzone = rand() % 15; // si imposta la posizione del demotorzone in una delle prime 15 zone

    for (int i = 0; i < 15; i++)
    {
        // Allocazione di memoria per le due zone speculari
        struct Zona_mondoreale *nuova_mondoreale = (struct Zona_mondoreale *)malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto *nuova_soprasotto = (struct Zona_soprasotto *)malloc(sizeof(struct Zona_soprasotto));

        // Controllo in caso non ci sia memoria RAM disponibile per l'allocazione delle zone
        if (nuova_mondoreale == NULL || nuova_soprasotto == NULL)
        {
            printf("Errore fatale: memoria insufficiente per la mappa.\n");
            exit(1);
        }

        enum Tipo_zona tipo = rand() % 10;
        nuova_mondoreale->tipo = tipo;
        nuova_soprasotto->tipo = tipo;

        // --- OGGETTO (Solo Mondo Reale) ---
        // --- 30% di probabilità di trovare un oggetto utile ---
        if ((rand() % 100) < 30)
        {
            nuova_mondoreale->oggetto = (rand() % 4) + 1; // Un oggetto tra quelli disponibili escludendo nessun_oggetto
        }
        else
        {
            nuova_mondoreale->oggetto = nessun_oggetto;
        }

        // --- NEMICO MONDO REALE ---
        // Probabilità: 50% Nessuno, 30% Democane, 20% Billi
        int prob_mr = rand() % 100;
        if (prob_mr < 50)
            nuova_mondoreale->nemico = nessun_nemico;
        else if (prob_mr < 80)
            nuova_mondoreale->nemico = democane;
        else
            nuova_mondoreale->nemico = billi;

        // --- NEMICO SOPRASOTTO ---
        if (i == pos_demotorzone)
        {
            nuova_soprasotto->nemico = demotorzone;
        }
        else
        {
            // 50% Nessuno, 50% Democane
            nuova_soprasotto->nemico = (rand() % 100 < 50) ? nessun_nemico : democane;
        }

        // --- COLLEGAMENTI TRA MONDO E SOPRASOTTO  ---
        nuova_mondoreale->link_soprasotto = nuova_soprasotto;
        nuova_soprasotto->link_mondoreale = nuova_mondoreale;

        // --- COLLEGAMENTI LISTA DOPPIA ---
        nuova_mondoreale->avanti = NULL;
        nuova_soprasotto->avanti = NULL;

        if (prima_zona_mondoreale == NULL)
        {
            // Inserimento della prima zona
            prima_zona_mondoreale = nuova_mondoreale;
            prima_zona_soprasotto = nuova_soprasotto;
            nuova_mondoreale->indietro = NULL;
            nuova_soprasotto->indietro = NULL;
        }
        else
        {
            // Aggancio alle zone precedenti
            ultima_mondoreale->avanti = nuova_mondoreale;
            nuova_mondoreale->indietro = ultima_mondoreale;

            ultima_soprasotto->avanti = nuova_soprasotto;
            nuova_soprasotto->indietro = ultima_soprasotto;
        }

        // Aggiorno il puntatore all'ultima zona creata per il prossimo ciclo
        ultima_mondoreale = nuova_mondoreale;
        ultima_soprasotto = nuova_soprasotto;
    }

    mappa_chiusa = 0; // La mappa è stata generata ma non ancora "chiusa" ufficialmente
    printf("\n[OK] Generazione completata: 15 zone create (Boss posizionato casualmente).\n");
}

static void menu_mappa()
{
    int scelta = -1;

    do
    {
        pulisci_schermo();
        printf("\n----------- MENU DEL GAME MASTER -----------");
        if (mappa_chiusa)
            printf("\n----------- [STATO MAPPA]: CHIUSA -----------\n");
        else
            printf("\n----------- [STATO MAPPA]: APERTA -----------\n");
        printf("\n1) GENERA MAPPA (Crea 15 zone, Assegnazione dei valori Random) --- (!!!Se hai già generato delle zone verrano eliminate!!!)");
        printf("\n2) INSERISCI ZONA (In posizione i)");
        printf("\n3) CANCELLA ZONA (In posizione i)");
        printf("\n4) STAMPA INTERA MAPPA (Mondo Reale o Soprasotto)");
        printf("\n5) STAMPA DETTAGLI ZONA (MONDO REALE + SOPRASOTTO)");
        printf("\n6) VISUALIZZA GIOCATORI");
        printf("\n7) CHIUDI MAPPA (!!!SE LA MAPPA NON VIENE CHIUSA IL GIOCO NON POTRÀ PARTIRE)");
        printf("\n0) TORNA AL MENU PRINCIPALE PER GIOCARE");
        printf("\n-----------------------------------------------------");
        printf("\nScelta: ");

        if (scanf("%d", &scelta) != 1)
        {
            printf("\n!!! Errore: Inserisci un numero !!!\n");
            while (getchar() != '\n')
                ;
            premi_invio();
            continue;
        }
        while (getchar() != '\n')
            ;

        switch (scelta)
        {
        case 1:
            if (mappa_chiusa == 1)
            {
                int conferma = -1;
                printf("\n!!! ATTENZIONE: La mappa è CHIUSA (Sigillata). !!!");
                printf("\nSe rigeneri ora, la mappa corrente verrà cancellata e persa per sempre.");
                printf("\nVuoi davvero riaprirla e rigenerarla?");
                printf("\n[1] SI, distruggi e rigenera");
                printf("\n[0] NO, annulla operazione");
                printf("\nScelta: ");
                scanf("%d", &conferma);
                while (getchar() != '\n')
                    ;

                if (conferma == 1)
                {
                    mappa_chiusa = 0;
                    genera_mappa();
                    printf("\n[INFO] Mappa riaperta e rigenerata.\n");
                }
                else
                {
                    printf("\nOperazione annullata. La mappa resta chiusa.\n");
                }
            }
            else
            {
                genera_mappa();
            }
            break;
        case 2:
            inserisci_zona();
            break;
        case 3:
            cancella_zona();
            break;
        case 4:
            stampa_mappa();
            break;
        case 5:
            stampa_zona();
            break;
        case 6:
            stampa_giocatori();
            break;
        case 7:
            if (mappa_chiusa)
                printf("\nLa mappa è già stata chiusa correttamente.\n");
            else
                chiudi_mappa();
            break;
        case 0:
            if (n_giocatori > 0)
            {
                pulisci_schermo();
                printf("\n!!! ATTENZIONE !!!");
                printf("\nUSCENDO ORA SE SI REIMPOSTA IL GIOCO SI DOVRANNO REIMPOSTARE I GIOCATORI.");
                printf("\n(LA MAPPA IMPOSTATA RIMARRÀ TALE)");
                printf("\nConfermi di voler uscire e resettare tutto?");
                printf("\n[1] SI, Esci");
                printf("\n[0] NO, Resta qui");
                printf("\nScelta: ");
                int conf;
                scanf("%d", &conf);
                while (getchar() != '\n')
                    ;

                if (conf == 1)
                {
                    return;
                }
                else
                {
                    scelta = -1;
                }
            }
            break;
        default:
            printf("\nOpzione non valida!\n");
            break;
        }
        if (scelta != 0)
        {
            premi_invio();
        }
    } while (scelta != 0);
}

static void stampa_giocatori()
{
    pulisci_schermo();
    if (n_giocatori == 0)
    {
        printf("\n!!! Nessun giocatore impostato !!!\n");
        return;
    }

    printf("\n==============================================");
    printf("\n        RIEPILOGO GIOCATORI ATTUALI");
    printf("\n==============================================");
    printf("\n %-20s | %-3s | %-3s | %-4s", "NOME", "ATT", "DIF", "FORT");
    printf("\n----------------------------------------------");
    for (int i = 0; i < n_giocatori; i++)
    {
        if (giocatori[i] != NULL)
        {
            printf("\n %-20s | %-3d | %-3d | %-4d",
                   giocatori[i]->nome,
                   giocatori[i]->attacco_psichico,
                   giocatori[i]->difesa_psichica,
                   giocatori[i]->fortuna);
        }
    }
    printf("\n==============================================\n");
}

static void pulisci_giocatori()
{
    undici_preso = 0; // Fondamentale per permettere la scelta in una nuova partita dopo il reset
    for (int i = 0; i < 4; i++)
    {
        if (giocatori[i] != NULL)
        {
            free(giocatori[i]);
            giocatori[i] = NULL;
        }
    }
    n_giocatori = 0;
}

static int reset_gioco()
{
    int reset = -1;
    do
    {
        pulisci_schermo();
        printf("!!! ATTENZIONE: Il gioco è già stato impostato !!!\n");
        printf("Se reimposti, perderai tutti i dati della partita corrente.\n");
        printf("\nPremi [1] per reimpostare tutto");
        printf("\nPremi [0] per annullare e tornare al menu");
        printf("\nScelta: ");

        if (scanf("%d", &reset) != 1)
        {
            printf("\nErrore: Inserisci un numero valido (0 o 1).\n");
            while (getchar() != '\n')
                ;

            reset = -1;

            // pausa per leggere l'errore prima del clear, altrimenti si cancellerebbe tutto non facendo visualizzare l'errore
            printf("Premi INVIO per riprovare...");

            /*Usiamo una variabile 'control' per leggere il primo carattere inserito dall'utente.
            - Se l'utente preme solo INVIO, 'control' diventa '\n' e il programma prosegue subito.
            - Se l'utente scrive "ciao" + INVIO, 'control' diventa 'control' e il ciclo while successivo
              pulisce "iao\n", evitando che sporchi la prossima scanf.
         */
            int control = getchar();
            if (control != '\n' && control != EOF)
            {
                while (getchar() != '\n')
                    ;
            }
            continue;
        }

        else if (reset != 0 && reset != 1)
        {
            printf("\nScelta non valida. Inserisci 0 o 1.\n");
            printf("Premi INVIO per riprovare...");
            getchar();
            while (getchar() != '\n')
                ;
        }

    } while (reset != 0 && reset != 1);
    while (getchar() != '\n')
        ;

    if (reset == 1)
    {
        termina_gioco(); // LIBERA LE VECCHIE MAPPE E I GIOCATORI
        printf("\nReset completato. STAI REIMPOSTANDO IL GIOCO!!!\n");
    }

    return reset;
}

static void imposta_giocatori()
{

    // CONTROLLO SE CI SONO GIÀ I GIOCATORI QUINDI IL GIOCO È GIA IMPOSTATO
    if (n_giocatori > 0)
        if (reset_gioco() == 0)
            return; // l'utente ha scelto 0, quindi torna al menu di gioco
    pulisci_schermo();
    do
    {
        printf("----IMPOSTA GIOCO----\n");
        printf("Inserisci il numero di giocatori per questa partita\n");
        printf("Min 1 giocatore - Max 4 giocatori\n");

        if (scanf("%d", &n_giocatori) != 1)
        {
            while (getchar() != '\n')
                ;
            n_giocatori = -1;
        }
        if (n_giocatori < 1 || n_giocatori > 4)
        {
            system("clear");
            printf("!!! ERRORE: Inserire un numero tra 1 e 4 !!!\n");
        }

    } while (n_giocatori < 1 || n_giocatori > 4);
    while (getchar() != '\n')
        ;

    for (int i = 0; i < n_giocatori; i++)
    {
        giocatori[i] = (struct Giocatore *)malloc(sizeof(struct Giocatore)); // malloc dei giocatori uno alla volta per una gestione migliore all'interno del gioco in caso si deva fare qualche free in modo tale da eliminare solo 1 giocatore senza far crashare il programma

        if (giocatori[i] == NULL)
        {
            printf("\nErrore fatale: Allocazione memoria fallita!\n");
            exit(1); // Se finisce la RAM e la malloc non riesce ad allocare memoria il programma viene chiuso per una gestione migliore e per evitare eventuali segmentation fault
        }

        giocatori[i]->mondo = 0;               // Inizia nel Mondo Reale (0)
        giocatori[i]->posiz_mondoreale = NULL; // Non ha ancora una zona
        giocatori[i]->posiz_soprasotto = NULL; // Non è nel soprasotto

        // Inizializza lo zaino come vuoto
        for (int j = 0; j < 3; j++)
        {
            giocatori[i]->zaino[j] = nessun_oggetto; // Iniziano tutti senza oggetti nello zaino
        }
        pulisci_schermo();
        printf("\nInserisci il nome del Giocatore (Max 50 caratteri) %d: ", i + 1);

        // fgets(dove, quanto_spazio, da_dove)
        fgets(giocatori[i]->nome, sizeof(giocatori[i]->nome), stdin);
        giocatori[i]->nome[strcspn(giocatori[i]->nome, "\n")] = 0; // evita che sia preso anche l'invio (\n) all'interno dell'array contenente il nome

        // inizializziamo le statistiche
        giocatori[i]->attacco_psichico = (rand() % DADO_MAX) + DADO_MIN;
        giocatori[i]->difesa_psichica = (rand() % DADO_MAX) + DADO_MIN;
        giocatori[i]->fortuna = (rand() % DADO_MAX) + DADO_MIN;
        premi_invio();
        // gestione delle statistiche
        int scelta_stat = -1;
        do
        {

            printf("\n--- Personalizzazione statistiche di: %s ---", giocatori[i]->nome);
            printf("\nATT: %d | DIF: %d | FORTUNA: %d",
                   giocatori[i]->attacco_psichico,
                   giocatori[i]->difesa_psichica,
                   giocatori[i]->fortuna);
            printf("\nVuoi modificare le statistiche? (Opzione unica)");
            printf("\n!!!ATTENZIONE, VARIANDO LE TUE STATISTICHE I VALORI SARANNO COMUNQUE COMPRESI TRA 1 E 20!!!");
            printf("\n!!!VALUTA BENE LA TUA DECISIONE!!!");
            printf("\n1) +3 Attacco / -3 Difesa");
            printf("\n2) -3 Attacco / +3 Difesa");
            printf("\n0) NO, Accetto le mie statistiche");
            printf("\nScelta: ");

            if (scanf("%d", &scelta_stat) != 1)
            {
                while (getchar() != '\n')
                    ;
                scelta_stat = -1;
            }

            switch (scelta_stat)
            {
            case 1: //(+3 ATT) / (-3 DIF)
                giocatori[i]->attacco_psichico += 3;
                giocatori[i]->difesa_psichica -= 3;

                // se attacco psichico è maggiore di 20, viene impostato a 20 dato che è il suo massimo
                if (giocatori[i]->attacco_psichico > 20)
                {
                    giocatori[i]->attacco_psichico = 20;
                }

                // se difesa psichica è minore di 1, viene impostato a 1 dato che è il suo minimo
                if (giocatori[i]->difesa_psichica < 1)
                {
                    giocatori[i]->difesa_psichica = 1;
                }

                printf("\nStatistiche modificate con successo!\n");
                break;

            case 2: //(+3 DIF) / (-3 ATT)
                giocatori[i]->difesa_psichica += 3;
                giocatori[i]->attacco_psichico -= 3;

                // se attacco psichico è maggiore di 20, viene impostato a 20 dato che è il suo massimo
                if (giocatori[i]->difesa_psichica > 20)
                {
                    giocatori[i]->difesa_psichica = 20;
                }

                // se difesa psichica è minore di 1, viene impostato a 1 dato che è il suo minimo
                if (giocatori[i]->attacco_psichico < 1)
                {
                    giocatori[i]->attacco_psichico = 1;
                }

                printf("\nStatistiche modificate con successo!\n");
                break;
            case 0:
                printf("\nStatistiche confermate.\n");
                break;

            default:
                pulisci_schermo();
                printf("\n!!! Opzione non valida! !!!\n");
                break;
            }
        } while (scelta_stat < 0 || scelta_stat > 2);

        while (getchar() != '\n')
            ;

        if (undici_preso == 0)
        {
            int scelta_undici = -1;
            do
            {
                pulisci_schermo();
                printf("\n---Vuoi diventare UNDICIVIRGOLACINQUE?--- ");
                printf("\n---([+4 ATT][+4 DIF][-7 FORTUNA])--- ");
                printf("\nPremi [1] per diventare UNDICIVIRGOLACINQUE");
                printf("\nPremi [0] per proseguire");
                printf("\nScelta: ");

                // Controllo carattere
                if (scanf("%d", &scelta_undici) != 1)
                {
                    printf("\n!!! ATTENZIONE: Hai inserito un carattere non idoneo !!!\n");
                    while (getchar() != '\n') // elimina i caratteri nel buffer finchè non trova il terminatore di riga
                        ;
                    scelta_undici = -1; // gli si riassegna -1 per resettare la variabile ad un valore non valido che non sia 0
                }
                // Controllo se il numero è diverso da 0 e 1
                else if (scelta_undici != 0 && scelta_undici != 1)
                {
                    printf("\n!!! ATTENZIONE: Inserisci solo 0 o 1! !!!\n");
                }

            } while (scelta_undici != 0 && scelta_undici != 1); // Ripete finché non è 0 o 1
            while (getchar() != '\n')
                ;

            if (scelta_undici == 1)
            {
                undici_preso = 1;
                strcpy(giocatori[i]->nome, "UndiciVirgolaCinque");
                giocatori[i]->attacco_psichico += 4;
                if (giocatori[i]->attacco_psichico > 20)
                    giocatori[i]->attacco_psichico = 20;

                giocatori[i]->difesa_psichica += 4;
                if (giocatori[i]->difesa_psichica > 20)
                    giocatori[i]->difesa_psichica = 20;

                giocatori[i]->fortuna -= 7;
                if (giocatori[i]->fortuna < 1)
                    giocatori[i]->fortuna = 1;
            }
        }

        printf("\nConfigurazione completata per %s: ATT %d | DIF %d | FORTUNA %d\n",
               giocatori[i]->nome,
               giocatori[i]->attacco_psichico,
               giocatori[i]->difesa_psichica,
               giocatori[i]->fortuna);
    }
    stampa_giocatori();
}

void imposta_gioco()
{
    imposta_giocatori();
    if (n_giocatori == 0)
        return;
    if (n_giocatori > 0)
    {
        menu_mappa();
    }
}

void visualizza_crediti()
{

    carica_ultime_vittorie();
    pulisci_schermo();
    printf("\n===========================================");
    printf("\n             COSESTRANE - 2026             ");
    printf("\n===========================================");

    // Dati Sviluppatore
    printf("\nSVILUPPATORE: [Giordano Femi D'almeida]");
    printf("\nMATRICOLA:    [392361]");
    printf("\n-------------------------------------------");

    printf("\n\n--- ULTIMI 3 VINCITORI ---");

    if (n_vittorie_registrate == 0)
    {
        printf("\nNessun vincitore registrato... per ora.");
        printf("\nIl Demotorzone attende ancora la sua prima sconfitta.");
    }
    else
    {
        for (int i = 0; i < n_vittorie_registrate; i++)
        {
            printf("\n%d. %s", i + 1, array_vincitori[i]);
        }
    }

    printf("\n\n===========================================\n");
    premi_invio();
}