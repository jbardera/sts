/*
	 STSUTIL.C - Creacio de claus RSA i generacio certificats

	 Compilador: GNU C
	 Autor: Joanmi Bardera, 1995
*/

#include <sts.c>

void main(int argc, char *argv[])
{
	int dig;
	char fit1[256],fit2[256];

	printf("\n1. Crear claus publica i privada RSA");
	printf("\n2. Certificar un fitxer que contingui una clau publica");
	printf("\n3. Crear cos i trobar element primitiu per el STS");
	printf("\n4. Sortir");
	printf("\n\nEntra opcio: ");
	scanf("%i",&dig);
	prepara_pari();
	switch(dig)
	{
				case 1:
								printf("Quin nom d'usuari vols? ");
								scanf("%s",fit1);
								printf("Quants digits decimals (10-1024)? ");
								scanf("%i",&dig);
								if ((dig<10) || (dig>1024)) dig=100;
								printf("Espera, creant clau publica i privada...");
								crea_claus_rsa(fit1,dig);
								printf("Llestos, claus creades.\n");
								break;
				case 2: printf("Nom del fitxer amb la clau privada RSA? ");
								scanf("%s",fit1);
								printf("Nom del fitxer a certificar? ");
								scanf("%s",fit2);
								printf("Espera, certificant...");
								crea_certificat(fit1,fit2);
								printf("\nLlestos.\n");
								break;
				case 3: printf("Nom del fitxer on guardar els valors? ");
								scanf("%s",fcos);
								printf("Quants digits decimals (10-1024)? ");
								scanf("%i",&dig);
								if ((dig<10) || (dig>1024)) dig=100;
								printf("Espera...");
								crea_cos_sts(dig);
								printf("\nLlestos. Valors guardats a '%s'\n",fcos);
								break;
				 default:
								break;
	}
	allibera_pari();
}
