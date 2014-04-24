#include <secure.h> /* SecuDE, per el IDEA */
#include <genpari.h>

OctetString *char_string;
BitString *bit_string;
KeyInfo *clau,*clau_repe;

unsigned char *text;
GEN p,q,n,e,fi,d; /* variables per l'RSA */
GEN x,y,alfax,alfay,modulp,alfa,key; /* variables STS */

char fpvt[256],fpub[256],fcer[256],fcentral[256],fcos[256];
/*
	fitxers amb les claus propies privada i publica, certificat, clau publica
	autoritat central i amb el cos+primitiu
*/
char fp[256]; /* fpvt/fpub sense extensio */
char ncer[256];

#define tamany_text 64000  /* tamany maxim text a xifrar/desxifrar */
#define max_claus 1024		 /* numero maxim digits decimals de les claus */

int video=0; /* si video diferent de zero modus debugger */
