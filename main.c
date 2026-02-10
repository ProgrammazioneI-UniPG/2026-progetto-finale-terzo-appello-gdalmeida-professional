#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gamelib.h"

int main()
{
    time_t t;
    srand((unsigned)time(&t)); // viene inizializzato il generatore di numeri casuali
    int scelta = 0;
    do
    {
        system("clear");
        printf("\n***********************************");
        printf("\n*     BENVENUTO IN COSESTRANE     *");
        printf("\n***********************************\n");

        // stampa del menu di gioco
        printf("\n========= MENU DI GIOCO =========\n");
        printf("1) Imposta Gioco\n");
        printf("2) Gioca\n");
        printf("3) Termina gioco\n");
        printf("4) Visualizza i crediti\n");
        printf("\nScegli un'opzione: ");

        // lettura della scelta con pulizia del buffer in caso ci siano caratteri non idonei
        if (scanf("%d", &scelta) != 1) // il controllo verifica solo che la scanf prenda un intero e restituisce 1 in quel caso e 0 se viene presa un qualsiasi altro carattere
        {
            printf("\nErrore: Inserisci un numero valido.\n");
            while (getchar() != '\n')
                ;

            printf("\nPremi INVIO per riprovare...");
            getchar();
            continue;
        }
        while (getchar() != '\n')
            ;

        // switch case per la scelta
        switch (scelta)
        {
        case 1:
            imposta_gioco();
            break;

        case 2:
            gioca();
            break;

        case 3:
        {
            system("clear");
            // le graffe servono perchè abbiamo inizializzato la variabile locale conferma
            int conferma = -1; // inizializziamo a un valore non valido in caso il programma salti lo scanf cosi che entri nel ciclo per forza

            do
            {
                system("clear");
                printf("\nSEI SICURO DI VOLER TERMINARE IL GIOCO?");
                printf("\nPremi [1] per terminare veramente");
                printf("\nPremi [0] per tornare al menu principale");
                printf("\nScelta: ");

                // avviene un controllo per verificare che sia stato un inserito un carattere idoneo ed eventuale pulizia del buffer
                if (scanf("%d", &conferma) != 1) // il controllo verifica solo che la scanf prenda un intero e restituisce 1 in quel caso e 0 se viene presa un qualsiasi altro carattere
                {
                    printf("\n!!! ATTENZIONE: Hai inserito un carattere non idoneo !!!\n");
                    while (getchar() != '\n') // elimina i caratteri nel buffer finchè non trova il terminatore di riga
                        ;
                    printf("Premi INVIO per riprovare...");
                    getchar();
                    system("clear");
                    conferma = -1; // gli si riassegna -1 per resettare la variabile ad un valore non valido che non sia 0
                    continue;
                }
                else
                {
                    while (getchar() != '\n')
                        ;
                } 

                // 2. Controllo se il numero è diverso da 0 e 1
                if (conferma != 0 && conferma != 1)
                {
                    printf("\n!!! ATTENZIONE: Inserisci solo 0 o 1! !!!\n");
                    printf("Premi INVIO per riprovare...");
                    getchar();
                    system("clear");
                }

            } while (conferma != 0 && conferma != 1); // Ripete finché non è 0 o 1

            if (conferma == 1)
            {
                termina_gioco(); // Funzione obbligatoria per liberare la memoria
                printf("\nUscita in corso... Arrivederci!\n");
                // scelta rimane 3, quindi il ciclo esterno si interrompe
            }
            else
            {
                printf("\nRitorno al menu principale...\n");
                scelta = 0;
            }
            break;
        }

        case 4:
            visualizza_crediti();
            break;

        default:
            system("clear");
            printf("!!! ATTENZIONE L'OPZIONE SCELTA NON È VALIDA\n");
            printf("\nPremi INVIO per riprovare...");
            getchar();
            break;
        }
    } while (scelta != 3);

    return 0;
}