/* STS.C - Versio GNU C (1995) */

/* C */
#include <genpari.h>
#ifdef DOS
 #include <djgppstd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
/* propis */
#include <sts.h>

#ifndef DOS
char *strupr(char *string)
{
	int i;

	for (i=0;i<strlen(string);i++)
	{
		if ((string[i]>=96) && (string[i]<123)) string[i]-=32;
	}

	return(string);
}
#endif


static void init_rndm()
{
	static long rseed=0;
	if (rseed==0)
	{
		time(&rseed);
		srandom(rseed);
	}
/*	printf("\nLlavor: %ld\n",rseed); */
}

GEN multimod(GEN base, GEN exponent, GEN elmodul)
{
		GEN comodi,resultat;

		comodi=gun;
		resultat=gmodulo(base,elmodul);
		while (gcmp1(glt(comodi,exponent)))
		{
			 gmulz(resultat,base,resultat);
			 gaddz(comodi,gun,comodi);
		}
		return(resultat);
}

GEN genera_primer(int digits)
{
	/* calcula un nombre primer de 'digits' digits decimals */
	unsigned char *numero;
	int i;
	GEN primer;

	numero=(unsigned char *)malloc(sizeof(unsigned char)*(digits+1));
	for (i=0;i<digits;i++)
	{
		numero[i]=(unsigned char)((unsigned char)random()*9/255+48);
	}
	numero[digits]='\x0';
	primer=lisexpr(numero); /* convertim el numero de 200 xifres a Pari */
	free(numero);
	primer=bigprem(primer); /* trobem el primer superior mes proper */

	return(primer);
}

GEN genera_random(GEN cos)
{
	/* troba un numero aleatori x del cos Z/cos */
	int digits,i;
	unsigned char *numero;
	GEN x;

	digits=gsize(cos)/2;
	numero=(unsigned char *)malloc(sizeof(unsigned char)*(digits+1));
	for (i=0;i<digits;i++)
	{
		numero[i]=(unsigned char)((unsigned char)random()*9/255+48);
	}
	numero[digits]='\x0';
	x=lisexpr(numero);
	free(numero);

	return(x);
}

GEN genera_primitiu(GEN elmodul,int digits,int precisio)
{
	/* donat el cos Z/elmodul ('elmodul' primer de 'precisio' digits decimals)
		 troba un element primitiu entre els divisors de 'elmodul'-1 */
	GEN primitiu,comodi,bucle2,resultat;
	char trobat,final,trobat2;
	FILE *fitxer,*sortida,*virtual;
	FILE pantalla;
	int i,j;
	char *cadena;
	long ltop,lbot,ttop,tbot,contador; /* variables per alliberar memoria */

	alfa=gsub(elmodul,gun);
	fitxer=fopen("divikk.sts","wt");
	pantalla=*stdout;
	*stdout=*fitxer;
	alfa=divisors(alfa);
	outbeaut(alfa);
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;

	/* arreglem el fitxer de divisors */
	fitxer=fopen("divikk.sts","rt");
	sortida=fopen("divisors.sts","wt");
	final=0;
	while ((!feof(fitxer)) && (!final))
	{
		i=fgetc(fitxer);
		switch(i)
		{
			case 91: break; /* caracter "[" */
			case 93: final=1;
							 break; /* caracter "]" */
			case 44: fputc(10,sortida); /* caracter ",", canviem per salt linia */
							 break;
			default: fputc(i,sortida);
		}
	}
	fflush(sortida);
	fclose(sortida); fclose(fitxer);


	comodi=gsub(extract(matsize(alfa),gdeux),gtovec(gun));
	/*
	printf("numero divisors = ");
	outbeaut(numbdiv(alfa));
	*/

	trobat2=0;
	fitxer=fopen("divisors.sts","rt");

	primitiu=gzero;
	/* ens situem al primer divisor amb els digits demanats */
	cadena=malloc(sizeof(char)*tamany_text); i=0;
	while((!trobat2) && (!feof(fitxer)))
	{
		j=fgetc(fitxer);
		switch(j)
		{
			case 10: if (i<digits) i=0; else { cadena[i]='\x0'; trobat2=1;
																				 primitiu=lisexpr(cadena); }
							 break;
			default: cadena[i++]=j;
							 break;
		}
	}

	trobat=0; final=0;
	while ((!trobat) && (!feof(fitxer)) && (!final))
	{
		/* primitiu=extract(alfa,comodi); */
		while ((!trobat) && (!feof(fitxer)) && (gcmp1(glt(lift(primitiu),elmodul))))
		{
			printf("Testing ");
			outbeaut(primitiu);
			ltop=avma;
			bucle2=lisexpr("1"); /* no podem posar "=gun" per culpa del gaddz */
			contador=0;
			ttop=avma;
			while ((!final) && (gcmp1(glt(lift(bucle2),elmodul))))
			{
				contador++;
				gaddz(bucle2,gun,bucle2);
				resultat=puissmodulo(primitiu,bucle2,elmodul);
				if (gcmp1(geq(lift(resultat),primitiu))) final=1;
					else if (contador>50000)
							 {
								 printf(".");
								 contador=0;
								 virtual=fopen("temp.sts","wt");
								 pantalla=*stdout;
								 *stdout=*virtual;
								 outbeaut(primitiu);
								 outbeaut(bucle2);
								 outbeaut(elmodul);
								 fflush(stdout);
								 fclose(virtual);
								 *stdout=pantalla;
								 tbot=avma;
								 primitiu=gerepile(ttop,tbot,primitiu);
								 ttop=avma;
								 virtual=fopen("temp.sts","rt");
								 fscanf(virtual,"%s\n",cadena);
								 primitiu=lisexpr(cadena);
								 fscanf(virtual,"%s\n",cadena);
								 bucle2=lisexpr(cadena);
								 fscanf(virtual,"%s\n",cadena);
								 elmodul=lisexpr(cadena);
								 fclose(virtual);
								 remove("temp.sts");
							 }
			}
			lbot=avma;
			primitiu=gerepile(ltop,lbot,primitiu);
			if (gcmp1(gge(bucle2,elmodul))) trobat=1;
				else
				{
					i=0; trobat2=0;
					while((!trobat2) && (!feof(fitxer)))
					{
						j=fgetc(fitxer);
						switch(j)
						{
							case 10: cadena[i]='\x0'; trobat2=1;
											 primitiu=lisexpr(cadena);
											 break;
							default: cadena[i++]=j;
											 break;
						}
					}
				}
				final=0;
		}
		final=1; /* hem acabat amb els divisors */
		/* comodi=gsub(comodi,gun); */
	}
	free(cadena);
	fclose(fitxer);
	if (!trobat) { primitiu=gzero; printf("Primitiu no trobat."); }

	remove("divikk.sts"); remove("divisors.sts");
	return(primitiu);
}

GEN genera_primitiu2(GEN elmodul,int digits,int precisio)
{
	/* donat el cos Z/elmodul ('elmodul' nombre primer, clar) calcula un
		 element primitiu del cos amb 'digits' digits,
		 precisio=numero de digits que te 'elmodul' */
	/* metode: "emanuense", anar probant un a un... */
	GEN primitiu,bucle2,resultat;
	char trobat=0,final;
	char *numero,*cadena;
	int i;
	long ltop,lbot,ttop,tbot,contador;
	FILE *virtual;
	FILE pantalla;

	numero=(char *)malloc(sizeof(char)*tamany_text);
	numero[0]='1';
	for (i=1;i<digits;i++) numero[i]='0';
	numero[digits]='\x0';
	primitiu=lisexpr(numero);
	primitiu=bigprem(primitiu);

	while ((!trobat) && (gcmp1(glt(primitiu,elmodul))))
	{
		printf("Mirant ");
		outbeaut(primitiu);
		ltop=avma;
		final=0;
		bucle2=lisexpr("1"); /* no podem posar "=gun" per culpa del gaddz */
		contador=0;
		ttop=avma;
		while ((!final) && (gcmp1(glt(bucle2,elmodul))))
		{
			gaddz(bucle2,gun,bucle2);
			resultat=puissmodulo(primitiu,bucle2,elmodul);
			if (gcmp1(geq(resultat,primitiu))) final=1;
			else if (contador>50000)
							 {
								 printf(".");
								 contador=0;
								 virtual=fopen("temp.sts","wt");
								 pantalla=*stdout;
								 *stdout=*virtual;
								 outbeaut(primitiu);
								 outbeaut(bucle2);
								 outbeaut(elmodul);
								 fflush(stdout);
								 fclose(virtual);
								 *stdout=pantalla;
								 tbot=avma;
								 primitiu=gerepile(ttop,tbot,primitiu);
								 ttop=avma;
								 virtual=fopen("temp.sts","rt");
								 fscanf(virtual,"%s\n",numero);
								 primitiu=lisexpr(numero);
								 fscanf(virtual,"%s\n",numero);
								 bucle2=lisexpr(numero);
								 fscanf(virtual,"%s\n",numero);
								 elmodul=lisexpr(numero);
								 fclose(virtual);
								 remove("temp.sts");
							 }
		}
		lbot=avma;
		primitiu=gerepile(ltop,lbot,primitiu);
		if (gcmp1(gge(bucle2,elmodul))) trobat=1;
			else { primitiu=gadd(primitiu,gun); primitiu=bigprem(primitiu); }
	}
	if (!trobat) primitiu=gzero;
	free(numero);
	return(primitiu);
}

void rsa_init(int precisio)
{
	int i,digits;
	char *numero;
	char final;
	FILE *fitxer;
	FILE pantalla;

	p=genera_primer(precisio);
	q=gun;
	while (gcmp1(gle(q,p))) /* q ha de ser major que p */
	{
		q=genera_primer(precisio);
		gaddz(q,stoi(1),q); q=bigprem(q);
	}
	n=gmul(p,q);
	printf("\nRSA p = ");
	outbeaut(p);
	printf("RSA q = ");
	outbeaut(q);
	printf("RSA n (pq) = ");
	outbeaut(n);
	fi=(gmul(gsub(p,stoi(1)),gsub(q,stoi(1))));
	printf("fi(n) = ");
	outbeaut(fi);
	/* 0<x<n i 0<e<fi(n), x i e aleatoris */
	/* e i n son els valors que es fan publics */
	i=3;
	digits=gsize(fi); /* e<fi(n) */
	final=0;
	numero=(unsigned char *)malloc(sizeof(unsigned char)*digits);
	while (!final)
	{
		for (i=0;i<digits;i++)
		{
			numero[i]=(unsigned char)((unsigned char)random()*9/255+48);
		}
		numero[digits]='\x0';
		e=lisexpr(numero);
		e=bigprem(e);
		if (gcmp1(ggt(fi,e))) final=1;
	}
	free(numero);
	printf("\nCalculant 'e' i 'd'...\n");
	printf("e = "); e=gmodulo(e,fi);
					/* d=ginvmod(e,fi);
						 d=gmodulo(d,fi); */
	d=lift(ginv(e)); e=lift(e); outbeaut(e);
	printf("d = e^-1 mod fi(n) = "); outbeaut(d);
}

void rsa_crypt(unsigned long size)
{
	/*
		 encripta el que hi ha a la variable global "text", "size" caracters
		 i ho volca al fitxer "crypted.sts"
	*/
	GEN scryp,kaka,comodi,num256;
	unsigned long bloc,l,j;
	FILE *fitxer;
	FILE pantalla;

	comodi=n;
	num256=stoi(256);
	bloc=0;
	while (gcmp1(ggt(comodi,num256)))
	{
		comodi=gdivent(comodi,num256);
		bloc++;
	}
	printf("\nText a xifrar:\n");
	for (l=0;l<size;l++) printf("%c",text[l]);
	printf("\nTamany en bytes= %ld",size);
	printf("\nTamany calculat de bloc a xifrar en bytes= %ld",bloc);
	if (bloc>=size)
	{
		bloc=size;
		printf("\nTamany de bloc reassignat a %ld",bloc);
	}
	fflush(stdout);
	scryp=stoi(0);
	l=0;
	fitxer=fopen("crypted.sts","wt");
	if (!fitxer)
	{
		printf("\nError obrint fitxer 'crypted.sts'\n");
		exit(-1);
	}
	pantalla=*stdout;
	*stdout=*fitxer;
	l=0; j=1; /* anem sumant j a l per processar tots els caracters del bloc */
	while (l+j<=size)
	{
		 scryp=stoi((unsigned char)text[l]); /* primer caracter del bloc */
		 comodi=num256; /* comencem multiplicant per 256 el segon caracter */
		 while ((j<bloc) && (l+j<size))
		 {
			 scryp=gadd( scryp, gmul(stoi((unsigned char)text[l+j]),comodi));
			 j++;
			 comodi=gmul2n(comodi,8); /* multiplica comodi per 2^8=256 */
		 }
		 scryp=puissmodulo(scryp,lift(e),n);
		 outbeaut(scryp);
		 l+=bloc; j=1;
	}
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;
}

void rsa_decrypt(char *filename)
{
	/*
			Desxifra el fitxer 'filename' i deixa el text desxifrat
			a la variable "text". Utilitza la clau privada RSA que estigui
			carregada a memoria.

			Variable "text" LIMITADA a tamany_text (64k en principi).
	*/
	FILE *fitxer;
	GEN scryp,caracter,num256;
	char *cadena;
	int i;

	num256=stoi(256);
	cadena=malloc(sizeof(char)*15000);
	fitxer=fopen(filename,"rt");
	if (!fitxer)
	{
		printf("\nError obrint fitxer '%s'\n",filename);
		exit(-1);
	}
	/* rewind(fitxer); */
	printf("\nText desxifrat:\n");
	i=0;
	while (!feof(fitxer))
	{
		fscanf(fitxer,"%s\n",cadena);
		scryp=lisexpr(cadena);
		scryp=puissmodulo(scryp,lift(d),n);
		while (gcmp1(ggt(scryp,stoi(0))))
		{
			if (gcmp1(gge(scryp,num256)))
			{
				caracter=lift(gmodulo(scryp,num256));
				text[i++]=(char)gtolong(caracter);
				printf("%c",text[i-1]);
				scryp=gdivent(scryp,num256);
			} else
			{
				caracter=scryp;
				text[i++]=(char)gtolong(caracter);
				printf("%c",text[i-1]);
				scryp=stoi(0);
			}
		}
		text[i]='\x0';
	}
	fclose(fitxer);
	free(cadena);
}

void signa(unsigned long size)
{
	/* encripta el que hi ha a la variable global "text", "size" caracters
		 i ho volca al fitxer "crypted.sts" */
	GEN scryp,comodi,num256;
	unsigned long bloc,l,j;
	FILE *fitxer;
	FILE pantalla;

	comodi=n;
	num256=stoi(256);
	bloc=0;
	while (gcmp1(ggt(comodi,num256)))
	{
		comodi=gdivent(comodi,num256);
		bloc++;
	}
	printf("\nText a signar:\n");
	for (l=0;l<size;l++) printf("%c",text[l]);
	printf("\nTamany en bytes= %ld",size);
	printf("\nTamany calculat de bloc a xifrar en bytes= %ld",bloc);
	if (bloc>=size)
	{
		bloc=size;
		printf("\nTamany de bloc reassignat a %ld",bloc);
	}
	fflush(stdout);
	fitxer=fopen("crypted.sts","wt");
	if (!fitxer)
	{
		printf("\nError obrint fitxer 'crypted.sts'\n");
		exit(-1);
	}
	pantalla=*stdout;
	*stdout=*fitxer;
	l=0; j=1; /* anem sumant j a l per processar tots els caracters del bloc */
	while (l+j<=size)
	{
		 scryp=stoi((unsigned char)text[l]); /* primer caracter del bloc */
		 comodi=num256; /* comencem multiplicant per 256 el segon caracter */
		 while ((j<bloc) && (l+j<size))
		 {
			 scryp=gadd( scryp, gmul(stoi((unsigned char)text[l+j]),comodi));
			 j++;
			 comodi=gmul2n(comodi,8); /* multiplica comodi per 2^8=256 */
		 }
		 scryp=puissmodulo(scryp,lift(d),n);
		 outbeaut(scryp);
		 l+=bloc; j=1;
	}
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;
}

void comprova(char *filename)
{
	/*
		 Comprova la signatura que hi hagi a "filename",
		 retorna el text en clar a la variable "text"
	*/
	FILE *fitxer;
	GEN scryp,caracter,num256;
	char *cadena;
	unsigned long j;

	num256=stoi(256);
	cadena=malloc(sizeof(char)*tamany_text);
	fitxer=fopen(filename,"rt");
	if (!fitxer)
	{
		printf("\nError obrint fitxer '%s'\n",filename);
		exit(-1);
	}
	/* rewind(fitxer); */
	j=0;
	printf("\nComprovacio signatura, text en clar:\n");
	while (!feof(fitxer))
	{
		fscanf(fitxer,"%s\n",cadena);
		scryp=lisexpr(cadena);
		scryp=puissmodulo(scryp,lift(e),n);
		while (gcmp1(ggt(scryp,stoi(0))))
		{
			if (gcmp1(gge(scryp,num256)))
			{
				printf("%c",(char)gtolong(lift(gmodulo(scryp,num256))));
				text[j++]=(char)gtolong(lift(gmodulo(scryp,num256)));
				scryp=gdivent(scryp,num256);
			} else
			{
				printf("%c",(char)gtolong(scryp));
				text[j++]=(char)gtolong(scryp);
				scryp=stoi(0);
			}
		}
	}
	text[j]='\x0';
	fclose(fitxer);
	free(cadena);
}

void crea_certificat(char *nomfitx,char *signfitx)
{
	/*
		 1r parametre = fitxer on llegir clau privada per poder signar
		 2n parametre = fitxer amb els valors a signar (clau publica)
	*/
	FILE *fitxerin,*comofitx,*fitxerout;
			 /* fitxerout=fitxer on guardarem el certificat */
	FILE pantalla;
	char *comodi,*nom;
	char final;
	unsigned long i,j;
	GEN scryp,num256,cer_e,cer_n;

	num256=stoi(256);

	comodi=malloc(sizeof(char)*tamany_text);
	nom=malloc(sizeof(char)*tamany_text);
	/* carreguem les dades a signar en memoria */
	fitxerin=fopen(signfitx,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",signfitx);
		exit(-1);
	}
	final=0;
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		if (!feof(fitxerin))
		{
			fscanf(fitxerin,"%s\n",nom);
			printf("\nCreant certificat per: %s",nom);
			fscanf(fitxerin,"%s",comodi);
			cer_e=lisexpr(comodi);
			fscanf(fitxerin,"%s",comodi);
			cer_n=lisexpr(comodi);
		} else { printf("\nFitxer '%s' incomplet",signfitx); exit(-255); }
	} else { printf("\nError de consistencia a '%s'",signfitx); exit(-255); }
	fclose(fitxerin);
	/* volcat a fitxer de totes les dades, per fer despres la signatura */
	for (i=0;i<strlen(nom);i++) text[i]=nom[i];
	comofitx=fopen("temp.sts","wt");
	pantalla=*stdout;
	*stdout=*comofitx;
	outbeaut(cer_e);
	outbeaut(cer_n);
	fflush(stdout);
	fclose(comofitx);
	*stdout=pantalla;
	comofitx=fopen("temp.sts","rt");
	while (!feof(comofitx))
	{
	 final=(char)fgetc(comofitx);
	 if ((final!=13) && (final!=10)) text[i++]=final;
	}
	fclose(comofitx);
	remove("temp.sts");
	/* final de volcat a fitxer */

	/*
		 Llegim els parametres de l'autoritat central per fer la
		 signatura

		 IMPORTANT!!
		 Es signa el text format per
		 "nom+cer_e+cer_n+CRLF",
		 on el signe "+" equival a concatenacio de cadenes
		 de caracters
	*/

	fitxerin=fopen(nomfitx,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",nomfitx);
		exit(-1);
	}
	final=0;
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		if (!feof(fitxerin))
		{
			fscanf(fitxerin,"%s\n",comodi);
			printf("\nSignatari: %s",comodi);
			fscanf(fitxerin,"%s\n",comodi);
			p=lisexpr(comodi);
			if (!feof(fitxerin))
			{
				fscanf(fitxerin,"%s\n",comodi);
				q=lisexpr(comodi);
				n=gmul(p,q);
				if (!feof(fitxerin))
				{
					fscanf(fitxerin,"%s\n",comodi);
					fi=lisexpr(comodi);
					if (!feof(fitxerin))
					{
						fscanf(fitxerin,"%s\n",comodi);
						d=lisexpr(comodi);
					}
					i--;
					signa(i); /* signa "text", resultat a "crypted.sts" */
					strcpy(comodi,nom);
					#ifdef DOS
						if (strlen(comodi)>8) comodi[8]='\x0';
					#endif
					strcat(comodi,".cer");
					fitxerout=fopen(comodi,"wt");
					if (!fitxerout)
					{
						printf("\nError obrint fitxer '%s'\n",comodi);
						exit(-1);
					}
					pantalla=*stdout;
					*stdout=*fitxerout;
					printf("~~~ Certificat STS ~~~\n");
					outbeaut(cer_e);
					outbeaut(cer_n);
					*stdout=pantalla;
					fprintf(fitxerout,"%s\n",nom);
					comofitx=fopen("crypted.sts","rt");
					if (!comofitx)
					{
						printf("\nError obrint fitxer 'crypted.sts'\n");
						exit(-1);
					}
					while (!feof(comofitx))
					{
						fscanf(comofitx,"%s\n",comodi);
						fprintf(fitxerout,"%s\n",comodi);
					}
					fclose(comofitx);
					fprintf(fitxerout,"~~~ Final Certificat STS ~~~");
					fflush(fitxerout);
					fclose(comofitx);
					fclose(fitxerout);
				}
			} else printf("\nError llegint segon parametre autoritat central.");
		} else printf("\nError llegint primer parametre autoritat central.");
	} else printf("\nError en el fitxer amb els parametres autoritat central.");
	fclose(fitxerin);
	free(comodi);
	free(nom);
}

int comprova_certificat(char *nomfitx,char *acentral)
{
	/*
		 1r parametre = fitxer on llegirem el certificat
		 2n parametre = fitxer d'on llegir la clau publica de l'autoritat
										central
		 Retorna 1 si OK, 0 si hi ha hagut cap error
	*/
	FILE *fitxerin,*fitxerout;
	dades certi_usuari;
	char *comodi,*comodi_e,*comodi_n,*nom;
	char final;
	unsigned long i,j;
	GEN scryp,num256;
	int error;

	error=1;
	fitxerin=fopen(nomfitx,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",nomfitx);
		exit(-1);
	}
	if (fitxerin==NULL)
	{
		printf("\nCertificat no trobat.\n");
	} else
	{
		nom=malloc(sizeof(char)*256);
		comodi=malloc(sizeof(char)*tamany_text);
		comodi_e=malloc(sizeof(char)*1024);
		comodi_n=malloc(sizeof(char)*1024);
		if ((!feof(fitxerin)) && (fitxerin!=NULL))
		{
			final=0;
			while ((final<6) && (!feof(fitxerin)))
			{
				if (fgetc(fitxerin)==126) final++;
			}
			if ((final==6) && (!feof(fitxerin)))
			{
				fscanf(fitxerin,"%s\n",comodi_e);
				fscanf(fitxerin,"%s\n",comodi_n);
				fscanf(fitxerin,"%s\n",nom);
			} else printf("\nError llegint fitxer amb el certificat.\n");
		} else printf("\nError llegint fitxer amb el certificat.\n");
		fflush(stdout);
		printf("\nLlegint certificat de: %s\n",nom);

		final=0;
		num256=stoi(256); i=0;
		fitxerout=fopen("signat.sts","wt");
		while (!feof(fitxerin) && (!final))
		{
			 fscanf(fitxerin,"%s\n",comodi);
			 /* ho volquem a "signat.sts" */
			 if (comodi[0]==126) final=1; else fprintf(fitxerout,"%s\n",comodi);
		}
		fclose(fitxerin);
		fclose(fitxerout);

		/* hem de llegir la clau publica de l'autoritat central i comprovar */
		/* que la signatura es correcta */
		/* llegim la clau publica de l'autoritat central */
		fitxerin=fopen(acentral,"rt");
		if (!fitxerin)
		{
			printf("\nError obrint fitxer '%s'\n",acentral);
			exit(-1);
		}
		final=0;
		while ((final<6) && (!feof(fitxerin)))
		{
			if (fgetc(fitxerin)==126) final++;
		}
		if (final==6)
		{
			fscanf(fitxerin,"%s\n",comodi);
			printf("Comprovacio usant clau publica RSA de: %s",comodi);
			if (!feof(fitxerin))
			{
				fscanf(fitxerin,"%s\n",comodi);
				e=lisexpr(comodi);
				if (!feof(fitxerin))
				{
					fscanf(fitxerin,"%s\n",comodi);
					n=lisexpr(comodi);
				}
			}
		}
		fclose(fitxerin);
		comprova("signat.sts");
		remove("signat.sts");
		/*
			 a "text" tenim el text en clar a comparar amb
			 "nom+comodi_e+comodi_n"
		*/
		final=1; j=0;
		for (i=0;i<strlen(nom);i++)
			if ((char)nom[i]!=(char)text[j++]) final=0;
		if (final) for (i=0;i<strlen(comodi_e);i++)
								if ((char)comodi_e[i]!=(char)text[j++]) final=0;
		if (final) for (i=0;i<strlen(comodi_n);i++)
								if ((char)comodi_n[i]!=(char)text[j++]) final=0;
		if (final) printf("\nCertificat comprovat satisfactoriament.\n");
		 else
		 {
			 error=0;
			 printf("\nCertificat erroni.\n");
		 }
		free(nom);
		free(comodi);
		free(comodi_e);
		free(comodi_n);
	}
}

int llegeix_claus_certificat(char *nomfitx)
{
	/*
			llegeix la clau publica d'un fitxer amb el certificat
			(NO COMPROVA VALIDESA!! S'HA DE FER ABANS AMB
			"comprova_certificat()")

			retorna 1 si tot OK, 0 cas contrari
	*/

	FILE *fitxerin;
	char final;
	int error;
	char comodi[256];

	error=1;
	fitxerin=fopen(nomfitx,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",nomfitx);
		exit(-1);
	}
	final=0;
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		fscanf(fitxerin,"%s\n",comodi);
		e=lisexpr(comodi);
		if (!feof(fitxerin))
		{
			fscanf(fitxerin,"%s\n",comodi);
			n=lisexpr(comodi);
			if (!feof(fitxerin))
			{
				fscanf(fitxerin,"%s\n",comodi);
				printf("LLegida clau publica RSA de: %s",comodi);
			} else error=0;
		} else error=0;
	} else error=0;
	fclose(fitxerin);

	return(error);
}

void crea_claus_rsa(char *nom_user, int digits)
{
	FILE *fitxer;
	FILE pantalla;
	char comodi[256];
	int i;

	rsa_init(digits);
	strcpy(comodi,nom_user);
	#ifdef DOS
		if (strlen(comodi)>7) comodi[8]='\x0';
	#endif
	strcat(comodi,".pvt");
	fitxer=fopen(comodi,"wt");
	if (!fitxer)
	{
		printf("\nError obrint fitxer '%s'\n",comodi);
		exit(-1);
	}
	pantalla=*stdout;
	*stdout=*fitxer;
	printf("~~~ Principi Clau Privada STS ~~~\n");
	printf("%s\n",nom_user);
	outbeaut(p);
	outbeaut(q);
	outbeaut(fi);
	outbeaut(d);
	printf("~~~ Final Clau Privada STS ~~~");
	fflush(stdout);
	fclose(fitxer);
	strcpy(comodi,nom_user);
	#ifdef DOS
		if (strlen(comodi)>7) comodi[8]='\x0';
	#endif
	strcat(comodi,".pub");
	fitxer=fopen(comodi,"wt");
	if (!fitxer)
	{
		printf("\nError obrint fitxer '%s'\n",comodi);
		exit(-1);
	}
	*stdout=*fitxer;
	printf("~~~ Principi Clau Publica STS ~~~\n");
	printf("%s\n",nom_user);
	outbeaut(e);
	outbeaut(n);
	printf("~~~ Final Clau Publica STS ~~~");
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;
	fclose(fitxer);
}

int llegeix_claus_rsa(char *nom_user)
{
	/*
		 llegeix clau privada i publica RSA dels fitxers
		 nom_user.pub i nom_user.pvt

		 Retorna 1 si hi ha hagut cap error, 0 si tot correcte
	*/
	int i;
	char comodi[256];
	char final,error;
	FILE *fitxerin;

	error=0;
	/*
		 llegim primer la clau privada, i despres la publica (resulta
		 mes rapid que no pas calcular-la de nou)
	*/
	strcpy(comodi,nom_user);
	#ifdef DOS
		if (strlen(comodi)>7) comodi[8]='\x0';
	#endif
	strcat(comodi,".pvt");
	printf("\nLlegint clau privada a '%s'...\n",comodi);
	fitxerin=fopen(comodi,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",comodi);
		exit(-1);
	}
	final=0;
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		fscanf(fitxerin,"%s\n",comodi);
		printf("Clau privada de: %s",comodi);
		if (!feof(fitxerin))
		{
			fscanf(fitxerin,"%s\n",comodi);
			p=lisexpr(comodi);
			if (!feof(fitxerin))
			{
				fscanf(fitxerin,"%s\n",comodi);
				q=lisexpr(comodi);
				if (!feof(fitxerin))
				{
					fscanf(fitxerin,"%s\n",comodi);
					fi=lisexpr(comodi);
					if (!feof(fitxerin))
					{
						fscanf(fitxerin,"%s\n",comodi);
						d=lisexpr(comodi);
					} else
					{
						printf("\nERROR. Impossible de trobar valor 'd'\n");
						error=0;
					}
				} else
				{
					printf("\nERROR. Impossible de trobar valor 'fi'\n");
					error=0;
				}
			} else
			{
				printf("\nERROR. Impossible de trobar valor 'q'\n");
				error=0;
			}
		} else
		{
			printf("\nERROR. Impossible de trobar valor 'p'\n");
			error=0;
		}
		n=gmul(p,q);
	} else
	{
		printf("\nERROR. Fitxer '%s' erroni.\n",comodi);
		error=0;
	}
	fclose(fitxerin);

	if (!error)
	{
		strcpy(comodi,nom_user);
		#ifdef DOS
			if (strlen(comodi)>7) comodi[8]='\x0';
		#endif
		strcat(comodi,".pub");
		printf("\nLlegint clau publica a '%s'...\n",comodi);
		fitxerin=fopen(comodi,"rt");
		if (!fitxerin)
		{
			printf("\nError obrint fitxer '%s'\n",comodi);
			exit(-1);
		}
		final=0;
		while ((final<6) && (!feof(fitxerin)))
		{
			if (fgetc(fitxerin)==126) final++;
		}
		if (final==6)
		{
			fscanf(fitxerin,"%s\n",comodi);
			printf("Clau publica de: %s",comodi);
			if (!feof(fitxerin))
			{
				fscanf(fitxerin,"%s\n",comodi);
				e=lisexpr(comodi);
				if (!feof(fitxerin))
				{
					fscanf(fitxerin,"%s\n",comodi);
					n=lisexpr(comodi);
				}
			}
		}
		fclose(fitxerin);
	}

	return(error);
}

int sts_llegir_certificat(char *nomfitx,char *acpublica,char num_miss)
{
	/*
		 Carrega alfa^y a memoria.
		 Comprova el certificat inclos al segon missatge STS
		 i extreu la clau publica per tal de poder fer el
		 xifratge al tercer missatge.
		 Considerem que al fitxer 'nomfitx' hi ha el segon
		 missatge STS, tal qual, i al fitxer 'acpublica' la clau
		 publica de l'autoritat central.

		 Retorna 1 si tot OK, 0 cas contrari.
	*/
	FILE *fitxerin,*fitxerout;
	char final;
	int error,car;
	char *comodi;

	final=0; error=1;
	fitxerin=fopen(nomfitx,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",nomfitx);
		exit(-1);
	}
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		if (num_miss==2)
		{
			comodi=(char *)malloc(sizeof(char)*tamany_text);
			fscanf(fitxerin,"%s\n",comodi);
			alfay=lisexpr(comodi); /* carreguem el valor d'alfa^y */
			free(comodi);
		}
		fitxerout=fopen("temp.sts","wt");
		final=0;
		while (!feof(fitxerin) && (final<12))
		{
			car=fgetc(fitxerin);
			fputc(car,fitxerout);
			if (car==126) final++;
		}
		fclose(fitxerout);
	} else error=0;
	fclose(fitxerin);
	if (!comprova_certificat("temp.sts",acpublica))
	{
		printf("\nError al certificat del 2n missatge STS.\n");
		error=0;
	} else error=llegeix_claus_certificat("temp.sts");
	remove("temp.sts");

	return(error);
}

int signa_claus_sts()
{
	/*
		 Encripta alfa^x+alfa^y (concatenacio) usant la clau privada
		 carregada a memoria (e i n) i deixa el resultat al
		 fitxer "crypted.sts"

		 Retorna 1 si OK, 0 cas contrari
	*/

	FILE *fitxer;
	FILE pantalla;
	int error;
	char *comodi;

	error=1;
	fitxer=fopen("ptext.sts","wt");
	pantalla=*stdout;
	*stdout=*fitxer;
	outbeaut(alfax);
	outbeaut(alfay);
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;
	fclose(fitxer);
	comodi=(char *)malloc(sizeof(char)*tamany_text);
	fitxer=fopen("ptext.sts","rt");
	fscanf(fitxer,"%s\n",text);
	fscanf(fitxer,"%s\n",comodi);
	strcat(text,comodi);
	fclose(fitxer);
	remove("ptext.sts");
	signa(strlen(text));
	free(comodi);

	return(error);
}

int comprova_claus_sts(char *nomfitx)
{
	/*
		 Comprova signatura de alfa^x+alfa^y (concatenacio) usant la clau publica
		 carregada a memoria. Pren alfa^x i alfa^y del segon missatge
		 STS, grabat a 'nomfitx'.
		 Comprova que hagi consistencia amb els valors carregats
		 a memoria.

		 Retorna 1 si OK, 0 cas contrari.
	*/

	FILE *fitxerin,*fitxerout;
	FILE pantalla;
	int error,i;
	char *comodi,*comodi2;
	char final;

	error=1;

	comodi=(char *)malloc(sizeof(char)*max_claus*2);
	comodi2=(char *)malloc(sizeof(char)*max_claus);
	fitxerin=fopen(nomfitx,"rt");
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",nomfitx);
		exit(-1);
	}
	final=0;
	while ((final<18) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==18)
	{
		fitxerout=fopen("signed.sts","wt");
		final=0;
		while (!feof(fitxerin) && (!final))
		{
			 fscanf(fitxerin,"%s\n",comodi);
			 /* ho volquem a "signed.sts" */
			 if (comodi[0]==126) final=1; else fprintf(fitxerout,"%s\n",comodi);
		}
		fclose(fitxerout);
		comprova("signed.sts");
	} else error=0;
	fclose(fitxerin);
	remove("signed.sts");
	fitxerout=fopen("temp.sts","wt");
	pantalla=*stdout;
	*stdout=*fitxerout;
	outbeaut(alfax);
	outbeaut(alfay);
	fflush(stdout);
	fclose(fitxerout);
	*stdout=pantalla;
	fclose(fitxerout);
	fitxerin=fopen("temp.sts","rt");
	fscanf(fitxerin,"%s\n",comodi);
	fscanf(fitxerin,"%s\n",comodi2);
	fclose(fitxerin);
	strcat(comodi,comodi2);
	remove("temp.sts");
	if (strlen(comodi)!=strlen(text))
		error=0;
	else
		for (i=0;i<strlen(comodi);i++) if ((unsigned char)text[i]!=(unsigned char)comodi[i])
																		 { error=0; i=strlen(comodi); }

	free(comodi2);
	free(comodi);

	return(error);
}
