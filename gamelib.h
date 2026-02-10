#ifndef GAMELIB_H //serve in fase di compilazione per evitare ridichiarazione multipla dato che viene compilato sia gamelib.c che il main che includono entrambi gamelib.h
#define GAMELIB_H
#define DADO_MIN 1
#define DADO_MAX 20

//PRE-DICHIARAZIONI 
struct Zona_soprasotto;
struct Zona_mondoreale;
struct Giocatore;
void imposta_gioco(); 
void gioca(); 
void termina_gioco();
void visualizza_crediti();


// DEFINIZIONE DEGLI ENUM

enum Tipo_zona
{
    bosco,
    scuola,
    laboratorio,
    caverna,
    strada,
    giardino,
    supermercato,
    centrale_elettrica,
    deposito_abbandonato,
    stazione_polizia
};

enum Tipo_nemico
{
    nessun_nemico,
    billi,
    democane,
    demotorzone
};

enum Tipo_oggetto
{
    nessun_oggetto,
    bicicletta,
    maglietta_fuocoinferno,
    bussola,
    schitarrata_metallica
};


// STRUCT MONDOREALE
struct Zona_mondoreale
{
    enum Tipo_zona tipo;
    enum Tipo_nemico nemico;
    enum Tipo_oggetto oggetto;
    struct Zona_mondoreale *avanti;
    struct Zona_mondoreale *indietro;
    struct Zona_soprasotto *link_soprasotto;
};

// STRUCT SOPRASOTTO
struct Zona_soprasotto
{
    enum Tipo_zona tipo;
    enum Tipo_nemico nemico;
    struct Zona_soprasotto *avanti;
    struct Zona_soprasotto *indietro;
    struct Zona_mondoreale *link_mondoreale;
};

// STRUCT GIOCATORE
struct Giocatore
{
    char nome[51];
    int mondo; // 0 = Mondo Reale, 1 = Soprasotto
    struct Zona_mondoreale *posiz_mondoreale;
    struct Zona_soprasotto *posiz_soprasotto;
    int attacco_psichico;       // Valore da 1 a 20
    int difesa_psichica;        // Valore da 1 a 20
    int fortuna;                // Valore da 1 a 20
    enum Tipo_oggetto zaino[3]; // Massimo 3 oggetti
};
 

#endif