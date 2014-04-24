typedef struct
{
	char *nom;		/* nom en ASCII */
	GEN pa; 			/* clau publica */
	GEN alfa; 		/* element primitiu del cos finit */
	GEN p;				/* nombre primer */
} dades;

unsigned char *text;
GEN alfa,p,q,n,e,fi,d; /* variables per l'RSA */
GEN alfax,alfay,modulp; /* variables STS */

#define tamany_text 65535  /* tamany maxim text a xifrar/desxifrar */
#define max_claus 1024		 /* numero maxim digits decimals de les claus */

