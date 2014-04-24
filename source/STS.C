/*
	 STS.C - Llibreria criptografica

	 Compilador: GNU C
	 Autor: Joanmi Bardera, 1995
*/

#ifdef DOS
 #include <djgppstd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "mime64.c"
#include "sts.h"

#ifndef DOS
char *strupr(char *string)
{
	/* converteix una cadena a majuscules */
	int i;

	for (i=0;i<strlen(string);i++)
	{
		if ((string[i]>=96) && (string[i]<123)) string[i]-=32;
	}

	return(string);
}
#endif

static void init_rndm(void)
{
	static long rseed=0;
	if (rseed==0)
	{
		time(&rseed);
		srandom(rseed);
	}
}

void llegeix_config(char *nomconfig)
{
	/*
		Llegeix la configuracio del fitxer "nomconfig", per tal de saber
		a quins fitxers buscar el cos+primitiu, les claus, etc.
	*/
	FILE *fitxerin;
	int i,car;
	char final,comentari;

	fitxerin=fopen(nomconfig,"r");
	fpub[0]=0; fpvt[0]=0; fcer[0]=0; fcentral[0]=0;
	if (!fitxerin)
	{
		printf("\nError obrint fitxer '%s'\n",nomconfig);
		exit(99);
	}
	i=0; final=0; comentari=0;
	while ((!feof(fitxerin)) && (!final))
	{
		fcos[i]=(char)fgetc(fitxerin);
		if ((int)fcos[i]==59) comentari=1; /* ascii(59)=';', comentaris */
		if ((int)fcos[i]==10)
		{
			fcos[i]=0; if (!comentari) final=1; else i=0;
			comentari=0;
		} else i++;
	}
	i=0; final=0; comentari=0;
	while ((!feof(fitxerin)) && (!final))
	{
		fp[i]=(char)fgetc(fitxerin);
		if ((int)fp[i]==59) comentari=1;
		if ((int)fp[i]==10)
		{
			fp[i]=0; if (!comentari)	final=1; else i=0;
			comentari=0;
		} else i++;
	}
	#ifdef DOS
		if (strlen(fp)>7) fp[8]='\x0';
	#endif
	strcpy(fpub,fp); strcpy(fpvt,fp); strcpy(fcer,fp);
	i=0; final=0; comentari=0;
	while ((!feof(fitxerin)) && (!final))
	{
		fcentral[i]=(char)fgetc(fitxerin);
		if ((int)fcentral[i]==59) comentari=1;
		if ((int)fcentral[i]==10)
		{
			fcentral[i]=0; if (!comentari) final=1; else i=0;
			comentari=0;
		} else i++;
	}
	strcat(fpub,".pub");
	strcat(fpvt,".pvt");
	strcat(fcer,".cer");

	if (!(fitxerin=fopen(fpub,"r")))
	{
		printf("\nError al fitxer '%s' amb la clau publica propia.\n",fpub);
		exit(-98);
	}
	fclose(fitxerin);
	if (!(fitxerin=fopen(fpvt,"r")))
	{
		printf("\nError al fitxer '%s' amb la clau privada propia.\n",fpvt);
		exit(-98);
	}
	fclose(fitxerin);
	if (!(fitxerin=fopen(fcer,"r")))
	{
		printf("\nError al fitxer '%s' amb el certificat.\n",fcer);
		exit(-98);
	}
	fclose(fitxerin);
	if (!(fitxerin=fopen(fcentral,"r")))
	{
		printf("\nError al fitxer '%s' amb la clau publica aut. central.\n",fcentral);
		exit(-98);
	}
	fclose(fitxerin);
}

GEN multimod(GEN base, GEN exponent, GEN elmodul)
{
	/* multiplica dos nombres modul un tercer */
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
	GEN primerp;

	numero=(unsigned char *)malloc(sizeof(unsigned char)*(digits+1));
	if (!numero)
	{
		printf("\nError de memoria [genera_primer/numero].\n");
		exit(-100);
	}

	numero[0]=0;
	while (!numero[0]) { numero[0]=(unsigned char)((unsigned char)random()*9/255+48); }
	for (i=1;i<digits;i++)
	{
		numero[i]=(unsigned char)((unsigned char)random()*9/255+48);
	}
	numero[digits]='\x0';
	primerp=lisexpr(numero); /* convertim el numero a Pari */

	free(numero);
	primerp=bigprem(primerp); /* trobem el primer superior mes proper */

	return(primerp);
}

GEN genera_random(int digits)
{
	/* troba un numero aleatori de 'digits' digits decimals */
	int i;
	unsigned char *numero;
	GEN x;

	numero=(unsigned char *)malloc(sizeof(unsigned char)*(digits+1));
	numero[0]='0';
	while (numero[0]!='0') { numero[0]=(unsigned char)((unsigned char)random()*9/255+48); }
	for (i=1;i<digits;i++)
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
	fitxer=fopen("divikk.sts","w");
	pantalla=*stdout;
	*stdout=*fitxer;
	alfa=divisors(alfa);
	outbeaut(alfa);
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;

	/* arreglem el fitxer de divisors */
	fitxer=fopen("divikk.sts","r");
	sortida=fopen("divisors.sts","w");
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

	trobat2=0;
	fitxer=fopen("divisors.sts","r");

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
								 virtual=fopen("temp.sts","w");
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
								 virtual=fopen("temp.sts","r");
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
	numero[digits]='\x0';	primitiu=genera_random(digits);
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
								 virtual=fopen("temp.sts","w");
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
								 virtual=fopen("temp.sts","r");
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

	printf("\nBuscant un primer de %i digits decimals...",precisio);
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
	printf("Calculant 'e' i 'd'...");
	printf("\n");
	printf("e = "); e=gmodulo(e,fi);
					/* d=ginvmod(e,fi);
						 d=gmodulo(d,fi); */
	d=lift(ginv(e)); e=lift(e); outbeaut(e);
	printf("d = e^-1 mod fi(n) = "); outbeaut(d);
}

void rsa_encripta(unsigned long size)
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
		if (video)
		{
			printf("\nText a xifrar:\n");
			for (l=0;l<size;l++) printf("%c",text[l]);
			printf("\nTamany en bytes= %ld",size);
			printf("\nTamany calculat de bloc a xifrar en bytes= %ld",bloc);
		}
	if (bloc>=size)
	{
		bloc=size;
		if (video) printf("\nTamany de bloc reassignat a %ld",bloc);
	}
	fflush(stdout);
	scryp=stoi(0);
	l=0;
	fitxer=fopen("crypted.sts","w");
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

void rsa_decripta(char *filename)
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
	fitxer=fopen(filename,"r");
	/* rewind(fitxer); */
	if (video) printf("\nText desxifrat:\n");
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
				if (video) printf("%c",text[i-1]);
				scryp=gdivent(scryp,num256);
			} else
			{
				caracter=scryp;
				text[i++]=(char)gtolong(caracter);
				if (video) printf("%c",text[i-1]);
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
		if (video)
		{
		  printf("\nText a signar:\n");
			for (l=0;l<size;l++) printf("%c",text[l]);
			printf("\nTamany en bytes= %ld",size);
			printf("\nTamany calculat de bloc a signar en bytes= %ld",bloc);
		}
	if (bloc>=size)
	{
		bloc=size;
		if (video) printf("\nTamany de bloc reassignat a %ld",bloc);
	}
	fflush(stdout);
	fitxer=fopen("crypted.sts","w");
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
	fitxer=fopen(filename,"r");
	/* rewind(fitxer); */
	j=0;
	if (video) 
	{
	  printf("\nComprovacio signatura.");
	  printf("\ne = "); outbeaut(e);
	  printf("n = "); outbeaut(n);
    printf("\nText en clar:\n");
  }
 	while (!feof(fitxer))
	{
		fscanf(fitxer,"%s\n",cadena);
		scryp=lisexpr(cadena);
		scryp=puissmodulo(scryp,lift(e),n);
		while (gcmp1(ggt(scryp,stoi(0))))
		{
			if (gcmp1(gge(scryp,num256)))
			{
				if (video) printf("%c",(char)gtolong(lift(gmodulo(scryp,num256))));
				text[j++]=(char)gtolong(lift(gmodulo(scryp,num256)));
				scryp=gdivent(scryp,num256);
			} else
			{
				if (video) printf("%c",(char)gtolong(scryp));
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
	fitxerin=fopen(signfitx,"r");
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
	comofitx=fopen("temp.sts","w");
	pantalla=*stdout;
	*stdout=*comofitx;
	outbeaut(cer_e);
	outbeaut(cer_n);
	fflush(stdout);
	fclose(comofitx);
	*stdout=pantalla;
	comofitx=fopen("temp.sts","r");
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

	fitxerin=fopen(nomfitx,"r");
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
			if (video) printf("\nSignatari: %s",comodi);
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
					if (video) printf("\nFitxer amb el certificat: %s",comodi);
					fitxerout=fopen(comodi,"w");
					pantalla=*stdout;
					*stdout=*fitxerout;
					printf("~~~ Certificat STS ~~~\n");
					outbeaut(cer_e);
					outbeaut(cer_n);
					*stdout=pantalla;
					fprintf(fitxerout,"%s\n",nom);
					comofitx=fopen("crypted.sts","r");
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
	char *comodi,*comodi_e,*comodi_n,*nom;
	char final;
	unsigned long i,j;
	GEN scryp,num256;
	int error;

	error=1;
	fitxerin=fopen(nomfitx,"r");
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
		if (video) printf("\nLlegint certificat de: %s\n",nom);

		final=0;
		num256=stoi(256); i=0;
		fitxerout=fopen("signat.sts","w");
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
		fitxerin=fopen(acentral,"r");
		if (!fitxerin)
		{
			printf("\nError al fitxer '%s' amb la clau publica de l'autoritat central.\n",acentral);
			exit(3);
		}
		final=0;
		while ((final<6) && (!feof(fitxerin)))
		{
			if (fgetc(fitxerin)==126) final++;
		}
		if (final==6)
		{
			fscanf(fitxerin,"%s\n",comodi);
			if (video) printf("Comprovacio usant clau publica RSA de: %s",comodi);
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
		if (final) { if (video) printf("\nCertificat comprovat satisfactoriament.\n"); }
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
	return(error);
}

int llegeix_claus_certificat(char *nomfitx)
{
	/*
			Llegeix la clau publica d'un fitxer amb el certificat
			(NO COMPROVA VALIDESA!! S'HA DE FER ABANS AMB
			"comprova_certificat()")

			Retorna 1 si tot OK, 0 cas contrari
	*/

	FILE *fitxerin;
	char final;
	int error;
	char comodi[256];

	error=1;
	fitxerin=fopen(nomfitx,"r");
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
				if (video) 
				{
				  printf("Llegida clau publica RSA de: %s",comodi);
				  printf("\ne = "); outbeaut(e);
				  printf("n = "); outbeaut(n);
				}
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
	fitxer=fopen(comodi,"w");
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
	fitxer=fopen(comodi,"w");
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
	if (video) printf("\nLlegint clau privada a '%s'...\n",comodi);
	fitxerin=fopen(comodi,"r");
	final=0;
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		fscanf(fitxerin,"%s\n",comodi);
		if (video) printf("Clau privada de: %s",comodi);
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
		if (video) printf("\nLlegint clau publica a '%s'...\n",comodi);
		fitxerin=fopen(comodi,"r");
		final=0;
		while ((final<6) && (!feof(fitxerin)))
		{
			if (fgetc(fitxerin)==126) final++;
		}
		if (final==6)
		{
			fscanf(fitxerin,"%s\n",comodi);
			if (video) printf("Clau publica de: %s",comodi);
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

int sts_llegir_certificat(char *nomfitx,char num_miss)
{
	/*
		 Comprova el certificat inclos al segon o tercer missatge STS (num_miss=2 si 2n miss.)
		 i extreu la clau publica per tal de poder fer el
		 xifratge al tercer missatge.
		 Considerem que al fitxer 'nomfitx' hi ha el segon o tercer
		 missatge STS, tal qual.

		 Retorna 1 si tot OK.
	*/
	FILE *fitxerin,*fitxerout;
	char final;
	int error,car;
	char comodi[max_claus];

	final=0; error=1;
	fitxerin=fopen(nomfitx,"r");
	while ((final<6) && (!feof(fitxerin)))
	{
		if (fgetc(fitxerin)==126) final++;
	}
	if (final==6)
	{
		if (num_miss==2) fscanf(fitxerin,"%s\n",comodi);
		fitxerout=fopen("temp.sts","w");
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
	if (num_miss==3)
	{
	  /* ensenyem el nom que ve al certificat, per saber qui ens truca */
	  fitxerin=fopen("temp.sts","r");
	  final=0;
	  while ((final<6) && (!feof(fitxerin)))
	  {
		  if (fgetc(fitxerin)==126) final++;
	  }
	  if (final==6)
	  {
	    if (!feof(fitxerin)) fscanf(fitxerin,"%s\n",comodi);
	    if (!feof(fitxerin)) fscanf(fitxerin,"%s\n",comodi);
	    if (!feof(fitxerin)) fscanf(fitxerin,"%s\n",comodi);
	    printf("\nSTS: intentant connexio amb el nom certificat %s",comodi);
    }
	  fclose(fitxerin);
	}
	if (!comprova_certificat("temp.sts",fcentral))
	{
		printf("\nError al certificat del %i missatge STS.\n",num_miss);
		error=0;
	} else error=llegeix_claus_certificat("temp.sts");
	remove("temp.sts");

	return(error);
}

void trunc_key(void)
{
	/*
		 Converteix la clau "GEN key" a clau soportada
		 per idea (128 bits) utilitzant truncacio de bits.
		 La clau idea es guarda a "char *clau->subjectkey.bits"
	*/
	unsigned int i,j;
	char byte;
	GEN quocient,residu;

	quocient=key;
	for (i=0;i<16;i++)
	{
		clau->subjectkey.bits[i]=0;
		for (j=0;j<8;j++)
		{
			residu=lift(gmodulo(quocient,stoi(2)));
			quocient=gdivent(quocient,stoi(2));
			if (gcmp0(residu))
			{
				byte=clau->subjectkey.bits[i]; byte=byte<<1;
				clau->subjectkey.bits[i]=byte;
			} else
			{
				byte=clau->subjectkey.bits[i]; byte=(byte<<1)|1;
				clau->subjectkey.bits[i]=byte;
			}
		}
	}
	clau->subjectkey.nbits=128;
	if (video)
	{
		printf("\nClau IDEA en binari (grups de 8 bits):\n|");
		for (i=0;i<16;i++) printf("%i|",clau->subjectkey.bits[i]);
	}
}

int signa_claus_sts(int mnum)
{
	/*
		 Signa alfa^x+alfa^y o alfa^y+alfa^x (concatenacio) usant la clau privada
		 carregada a memoria (e i n) i les encripta usant la clau
		 idea generada. Deixa el resultat al fitxer "crypted.sts"

		 Retorna 1 si OK, 0 cas contrari
	*/

	FILE *fitxer;
	FILE pantalla;
	int error;
	char *comodi;
	int i,j;
	char *parametres[3];
	char caracter;

  if (video)
  {
    printf("\nSignant les claus Diffie-Hellman.");
    printf("\nd = "); outbeaut(d);
    printf("e = "); outbeaut(e);
    printf("n = "); outbeaut(n);
  }
  error=1;
  if (mnum==2)
	{
	  if (video) printf("\nOrdre claus al 2n missatge: alfa^y+alfa^x");
  } else
	{
	  if (video) printf("\nOrdre claus al 3r missatge: alfa^x+alfa^y");
	}
	fitxer=fopen("ptext.sts","w");
	pantalla=*stdout;
	*stdout=*fitxer;
	if (mnum==2)
	{
	  outbeaut(alfay);
		outbeaut(alfax);
	} else
	{
		outbeaut(alfax);
		outbeaut(alfay);
	}
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;
	fclose(fitxer);
	comodi=(char *)malloc(sizeof(char)*tamany_text);
	fitxer=fopen("ptext.sts","r");
	fscanf(fitxer,"%s\n",text);
	fscanf(fitxer,"%s\n",comodi);
	strcat(text,comodi);
	fclose(fitxer);
	remove("ptext.sts");
	signa(strlen(text));
	/*
		A "crypted.sts" hi ha les claus signades, ara les hem
		d'encriptar amb idea. A la variable global "key" hi ha la clau
		a fer servir, l'hem de truncar si supera 2^128 (clau idea son 128 bits)
	*/

	fitxer=fopen("crypted.sts","r");
	if (!fitxer)
	{
		printf("\nError llegint fitxer 'crypted.sts'\n");
		exit(3);
	}
	i=0;
	while (!feof(fitxer))
	{
	  caracter=(char)fgetc(fitxer);
	  char_string->octets[i++]=caracter;
	}
	char_string->octets[i]='\x0';  /* canviem la marca de final de fitxer per final string */
	i-=1; char_string->octets[i]='\x0'; /* un caracter mes nul, per si de cas */
	while (i%8)
	{
	  char_string->octets[++i]='\x0'; 
	}
	if (video) printf("\nText signat:\n%s",char_string->octets);
	if (video) printf("\n%i caracters signats a xifrar per IDEA.",i);
	fclose(fitxer); remove("crypted.sts");
	char_string->noctets=i; /* i caracters a xifrar */
	j=idea_encrypt(char_string,bit_string,SEC_END,clau);
	fitxer=fopen("crypted.sts","wb");
	if (!fitxer)
	{
		printf("\nError grabant fitxer 'crypted.sts'\n");
		exit(3);
	}
	for (j=0;j<i;j++) fputc(bit_string->bits[j],fitxer);
	fclose(fitxer);
	if (video)
	{
		printf("\nClau IDEA en binari (grups de 8 bits):\n|");
		for (j=0;j<16;j++) printf("%i|",clau->subjectkey.bits[j]);
		printf("\nXifrats %i caracters:\n",i);
		for (j=0;j<i;j++) printf("%c",bit_string->bits[j]);
	}
	bit_string->nbits=i;
	parametres[0]=(char *)malloc(sizeof(char)*256);
	parametres[1]=(char *)malloc(sizeof(char)*256);
	parametres[2]=(char *)malloc(sizeof(char)*256);
	strcpy(parametres[1],"crypted.sts"); strcpy(parametres[2],"-e");
	mime64(3,parametres);
	free(parametres[0]); free(parametres[1]); free(parametres[2]);
	free(comodi);

	return(error);
}

int comprova_claus_sts(char *nomfitx,int nm)
{
	/*
		 Comprova signatura de alfa^x+alfa^y/alfa^y+alfa^x (concatenacio) usant la clau publica
		 carregada a memoria. Pren alfa^x i alfa^y del segon missatge
		 STS, grabat a 'nomfitx'.
		 Comprova que hagi consistencia amb els valors carregats
		 a memoria.

		 Retorna 1 si OK, 0 cas contrari.
	*/

	FILE *fitxerin,*fitxerout;
	FILE pantalla;
	int error,i,j;
	char *comodi,*comodi2;
	char final;
	char *parametres[2];
	OctetString *dc_byt;
	BitString *ec_bit;

	error=1;

	/* primer decodifiquem */
	parametres[0]=(char *)malloc(sizeof(char)*256);
	parametres[1]=(char *)malloc(sizeof(char)*256);
	strcpy(parametres[1],nomfitx);
	mime64(2,parametres); /* a "crypted.sts" tenim el xifrat IDEA */
	free(parametres[0]); free(parametres[1]);
	/* ara desencriptem IDEA */
	ec_bit=(BitString *)malloc(sizeof(BitString));
	dc_byt=(OctetString *)malloc(sizeof(OctetString));
	ec_bit->bits=(char *)malloc(sizeof(char)*tamany_text);
	dc_byt->octets=(char *)malloc(sizeof(char)*tamany_text);
	fitxerin=fopen("crypted.sts","rb");
	i=0;
	while (!feof(fitxerin))
	{
		ec_bit->bits[i++]=fgetc(fitxerin);
	}
	ec_bit->bits[--i]='\x0'; /* treiem la marca de final de fitxer */
	ec_bit->nbits=i*8;
	if (video)
	{
	  printf("\n%i caracters a desxifrar per IDEA:\n",i);
	  for (j=0;j<i;j++) printf("%c",ec_bit->bits[j]);
	}
	fclose(fitxerin);
	remove("crypted.sts");
	j=idea_decrypt(ec_bit,dc_byt,SEC_END,clau);
	
	if (video)
	{
		printf("\nClau IDEA en binari (grups de 8 bits):\n|");
		for (j=0;j<16;j++) printf("%i|",clau->subjectkey.bits[j]);
		printf("\nDesxifrats %i caracters utils:\n%s",strlen(dc_byt->octets),dc_byt->octets);
		printf("\n");
	}
	comodi=(char *)malloc(sizeof(char)*max_claus*2);
	comodi2=(char *)malloc(sizeof(char)*max_claus);
	fitxerout=fopen("signed.sts","w");
	for (i=0;i<strlen(dc_byt->octets);i++) fputc(dc_byt->octets[i],fitxerout);
	fclose(fitxerout);
	free(dc_byt->octets);
	free(ec_bit->bits);
	free(dc_byt);
	free(ec_bit);
	comprova("signed.sts"); /* desfa la signatura */
	remove("signed.sts");
	fitxerout=fopen("temp.sts","w");
	pantalla=*stdout;
	*stdout=*fitxerout;
	if (nm==2)
	{
		outbeaut(alfay);
		outbeaut(alfax);
	} else
	{
		outbeaut(alfax);
		outbeaut(alfay);
	}
	fflush(stdout);
	fclose(fitxerout);
	*stdout=pantalla;
	fclose(fitxerout);
	fitxerin=fopen("temp.sts","r");
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

void llegeix_cos_sts(void)
{
	/*
		carrega a memoria el primer que defineix el cos i el primitiu, del
		fitxer 'fcos'
	*/
	FILE *fitxerin;
	char comodi[1024];

	if (fitxerin=fopen(fcos,"r"))
	{
		if (!feof(fitxerin))
		{
			fscanf(fitxerin,"%s",comodi);
			modulp=lisexpr(comodi);
			if (!feof(fitxerin))
			{
				fscanf(fitxerin,"%s",comodi);
				alfa=lisexpr(comodi);
			} else
			{
				printf("\nError. Al fitxer '%s' falta l'element primitiu a fer servir.\n",fcos);
				exit(-98);
			}
		} else
		{
			 printf("\nError. El fitxer '%s' amb el cos i element primitiu esta buit.\n",fcos);
			 exit(-98);
		}
	} else
	{
		printf("\nError obrint fitxer '%s' amb el cos i el primitiu.\n",fcos);
		exit(-99);
	}
	fclose(fitxerin);
}

void crea_cos_sts(int d)
{
	/*
		Genera el cos Z/modulp amb 'modulp' tants digits com li indiquem
		amb el parametre.
		Guarda 'modulp' i el primitiu 'alfa' al fitxer 'fcos'
	*/
	FILE *fitxerout;
	FILE pantalla;

	modulp=genera_primer(d);
	alfa=lift(gener(modulp));

	fitxerout=fopen(fcos,"w");
	if (!fitxerout)
	{
		printf("\nImpossible grabar fitxer '%s'\n",fcos);
		exit(-98);
	}
	pantalla=*stdout;
	*stdout=*fitxerout;
	outbeaut(modulp);
	outbeaut(alfa);
	fflush(stdout);
	fclose(fitxerout);
	*stdout=pantalla;
}

void missatge_1(void)
{
	/*
		genera el primer missatge STS a enviar, fa un volcat al fitxer "1m.sts"
	*/
	FILE *fitxer;
	FILE pantalla;

	x=genera_random(100);
	alfax=puissmodulo(alfa,x,modulp);
	if (video)
	{
	  printf("\nGenerant primer missatge.\n");
		printf("modulp = ");
		outbeaut(modulp);
		printf("alfa^x = ");
		outbeaut(alfax);
	}
	fitxer=fopen("1m.sts","w");
	if (!fitxer)
	{
		printf("\nError volcant primer missatge al fitxer '1m.sts'\n");
		exit(2);
	}
	pantalla=*stdout;
	*stdout=*fitxer;
	printf("~~~ Primer missatge STS. Usuari #1 -> Usuari #2 ~~~\n");
	outbeaut(alfa);
	outbeaut(modulp);
	outbeaut(alfax);
	printf("~~~ Final primer missatge STS ~~~");
	fflush(stdout);
	fclose(fitxer);
	*stdout=pantalla;
}

int missatge_2(char *m1)
{
	/*
		Llegeix el missatge rebut (primer) i
		genera el 2n missatge. Fa un volcat al fitxer "2m.sts"
		Retorna:
		0 si tot OK
		1 si error al format del missatge
		2 si error llegint clau propia RSA
		3 si error signant claus propies RSA
		4 si el remot ens envia un alfa^x <= 1
		5 si el primer que genera el cos te menys digits que els recomenables
	*/
	FILE *fitxer,*canal;
	FILE pantalla;
	int i,j,error,car;
	char final;
	char *comodi;

  if (video) printf("\nComprovant 1r missatge.\n");
	error=0; comodi=(char *)malloc(sizeof(char)*tamany_text);
	j=strlen(m1); /* a j tenim el numero de caracters del missatge */
	/* fem el volcat del 1r missatge rebut al fitxer 'm1.sts' */
	fitxer=fopen("m1.sts","w");
	for (i=0;i<j;i++) fputc(m1[i],fitxer);
	fclose(fitxer);
	/* carreguem els valors que ens han enviat amb el primer missatge */
	fitxer=fopen("m1.sts","r");
	final=0;
	while ((final<6) && (!feof(fitxer)))
	{
		if (fgetc(fitxer)==126) final++;
	}
	if (final==6)
	{
		if (!feof(fitxer)) fscanf(fitxer,"%s\n",comodi); else error=1;
		if (!error)
		{
			alfa=lisexpr(comodi);
			if (!feof(fitxer)) fscanf(fitxer,"%s\n",comodi); else error=1;
			if (!error)
			{
				modulp=lisexpr(comodi);
				if (!feof(fitxer)) fscanf(fitxer,"%s\n",comodi); else error=1;
				if (!error) alfax=lisexpr(comodi);
			}
		}
	} else error=1;
	fclose(fitxer);
	if (gcmp1(gle(alfax,stoi(1)))) error=4;
	if ((gsize(modulp))<10) error=5;
	if (!error)
	{
	  final=0;
	  while (!final)
	  {
	  	y=genera_random(100);
	  	alfay=puissmodulo(alfa,y,modulp);
	  	if (gcmp1(gne(alfay,alfax))) final=1;
	  }
		if (video)
		{
		  printf("\nValors llegits del primer missatge:\n");
			printf("modulp = ");
			outbeaut(modulp);
			printf("alfa = ");
			outbeaut(alfa);
			printf("alfa^x = ");
			outbeaut(alfax);
			printf("\nCalculem:\n");
  		printf("alfa^y = ");
			outbeaut(alfay);
		}
		key=puissmodulo(alfax,y,modulp); /* generem la clau Idea en format Pari */
		trunc_key(); /* convertim la clau Idea de Pari als 128 bits */
		if (llegeix_claus_rsa(fp)) error=2;
		else
		{
			if (!signa_claus_sts(2)) error=3;
			else
			{
			  if (video) printf("\nGenerant el 2n missatge.");
				fitxer=fopen("2m.sts","w");
				pantalla=*stdout;
				*stdout=*fitxer;
				printf("~~~ Segon missatge STS. Usuari #2 -> Usuari #1 ~~~\n");
				outbeaut(alfay);
				strcpy(comodi,fp);
				#ifdef DOS
					if(strlen(comodi)>8) comodi[8]=0;
				#endif
				strcat(comodi,".cer");
				canal=fopen(comodi,"r");
				while (!feof(canal))
				{
					car=fgetc(canal);
					if ((car<127)&&(car>9)) printf("%c",(char)car);
				}
				printf("\n");
				fclose(canal);
				fflush(stdout);
				fclose(fitxer);
				*stdout=pantalla;
				fitxer=fopen("2m.sts","a+");
				canal=fopen("crypted.sts","r");
				while (!feof(canal))
				{
					car=fgetc(canal);
					if ((car<127) && (car>9)) fputc(car,fitxer);
				}
				fprintf(fitxer,"~~~ Final segon missatge STS ~~~");
				fclose(canal); fclose(fitxer);
				remove("crypted.sts");
			}
		}
	}
	free(comodi);

	return(error);
}

int missatge_3(char *m1)
{
	/*
		Llegeix el missatge rebut (segon) i
		genera el 3r missatge. Fa un volcat al fitxer "3m.sts"
		Retorna:
		0 si tot OK
		1 si error al format del missatge rebut (2n missatge)
		2 error en el certificat que viatja al 2n missatge
		3 error en les claus signades i encriptades del 2n missatge
		4 error signant les claus
		5 error en el format del nostre certificat
	*/
	FILE *fitxer,*fitxerout;
	int i,j,error;
	char final,car;
	char *comodi;

  if (video) printf("\nComprovant 2n missatge.");
	comodi=(char *)malloc(sizeof(char)*tamany_text);
	error=0;
	fitxer=fopen("m2.sts","w");
	if (!fitxer)
	{
		printf("\nError grabant fitxer 'm2.sts'\n");
		exit(3);
	}
	for (i=0;i<strlen(m1);i++) fputc(m1[i],fitxer);
	fclose(fitxer);
	fitxer=fopen("m2.sts","r");
	if (!fitxer)
	{
		printf("\nError obrint fitxer 'm2.sts'\n");
		exit(3);
	}
	final=0;
	while ((final<6) && (!feof(fitxer)))
	{
		if (fgetc(fitxer)==126) final++;
	}
	if (final==6)
	{
		if (!feof(fitxer)) fscanf(fitxer,"%s\n",comodi); else error=1;
		if (!error)
		{
			alfay=lisexpr(comodi);
			key=puissmodulo(alfay,x,modulp);
			trunc_key();
			if(!sts_llegir_certificat("m2.sts",2)) error=2;
		}
	} else error=1;
	fclose(fitxer);

	/*
	 * ara hem de decodificar el mime64, desencriptar amb IDEA i comprovar la
	 * signatura feta sobre les dues claus al segon missatge
	 */
	if (!error)
	{
		final=0;
		fitxer=fopen("m2.sts","r");
		if (!fitxer)
		{
			printf("\nError obrint fitxer 'm2.sts'\n");
			exit(3);
		}
		final=0;
		while ((final<18) && (!feof(fitxer)))
		{
			if (fgetc(fitxer)==126) final++;
		}
		if (final==18)
		{
			fitxerout=fopen("tempm2.sts","w");
			if (!feof(fitxer)) car=fgetc(fitxer); else error=1; /* llegim el '\n' */
			final=0;
			while ((!feof(fitxer)) && (!final))
			{
				car=fgetc(fitxer);
				if (car!=126) fputc(car,fitxerout); else final=1;
			}
			fclose(fitxerout);
			if (!comprova_claus_sts("tempm2.sts",2)) error=3;
			remove("tempm2.sts");
		} else error=1;
		fclose(fitxer);
	}

	if (!error)
	{
		/* creem el 3r missatge */
				/*
			hem de signar alfa^x i alfa^y, encriptar-ho, i enviar-ho juntament amb el
			nostre certificat
		*/
		if (video) printf("\nCreant el 3r missatge.");
		llegeix_claus_rsa(fp);
		if (!signa_claus_sts(3)) error=4;
		if (!error)
		{
			fitxerout=fopen("3m.sts","w");
			fprintf(fitxerout,"~~~ Tercer missatge STS. Usuari #1 -> Usuari #2 ~~~\n");
			fitxer=fopen(fcer,"r");
			if (!fcer)
			{
				printf("\nError obrint fitxer '%s' amb el nostre certificat.\n",fcer);
				exit(3);
			}
			final=0;
			while ((final<6) && (!feof(fitxer)))
			{
				if (fgetc(fitxer)==126) final++;
			}
			if (final==6)
			{
				if (!feof(fitxer)) fscanf(fitxer,"%s",ncer); else error=5; /* llegim alfa i p */
				if (!error) if (!feof(fitxer)) fscanf(fitxer,"%s",ncer); else error=5;
				if (!error) if (!feof(fitxer)) fscanf(fitxer,"%s",ncer); else error=5; /* ara si llegim el nom */
				if (video) printf("\nNom al nostre certificat: %s",ncer);
			}
			fclose(fitxer);
			fitxer=fopen(fcer,"r");
			while (!feof(fitxer))
			{
				car=(char)fgetc(fitxer);
				if ((car>9)&&(car<127)) fputc(car,fitxerout);
			}
			fputc('\n',fitxerout);
			fclose(fitxer);
			fitxer=fopen("crypted.sts","r");
			while (!feof(fitxer))
			{
				car=fgetc(fitxer);
			  if ((car<127) && (car>9)) fputc(car,fitxerout);
			}
			fclose(fitxer);
			remove("crypted.sts");
			fprintf(fitxerout,"~~~ Final tercer missatge STS ~~~");
			fclose(fitxerout);
		}
		if (video) printf("\n3r missatge creat. Grabat a '3m.sts'\n");
	}
	free(comodi);

	return(error);
}

int connexio(char *m1)
{
	/*
		comprova que el 3r missatge no tingui cap error
		retorna: 0 si tot OK
						 1 error al format del 3r missatge
						 2 error al certificat
						 3 error en la signatura encriptada de les claus
	*/

	char *comodi;
	FILE *fitxer,*fitxerout;
	int error,i,car;
	char final;

  if (video) printf("\nComprovant el 3r missatge.");
	error=0;
	comodi=(char *)malloc(sizeof(char)*tamany_text);
	error=0;
	fitxer=fopen("m3.sts","w");
	if (!fitxer)
	{
		printf("\nError grabant fitxer 'm3.sts'\n");
		exit(3);
	}
	for (i=0;i<strlen(m1);i++) fputc(m1[i],fitxer);
	fclose(fitxer);
	if(!sts_llegir_certificat("m3.sts",3)) error=2;

	/*
	 * ara hem de decodificar el mime64, desencriptar amb IDEA i comprovar la
	 * signatura feta sobre les dues claus al tercer missatge
	 */
	if (!error)
	{
		final=0;
		fitxer=fopen("m3.sts","r");
		if (!fitxer)
		{
			printf("\nError obrint fitxer 'm3.sts'\n");
			exit(3);
		}
		final=0;
		while ((final<18) && (!feof(fitxer)))
		{
			if (fgetc(fitxer)==126) final++;
		}
		if (final==18)
		{
			fitxerout=fopen("tempm3.sts","w");
			if (!feof(fitxer)) car=fgetc(fitxer); else error=1; /* llegim el '\n' */
			final=0;
			while ((!feof(fitxer)) && (!final))
			{
				car=fgetc(fitxer);
				if (car!=126) fputc(car,fitxerout); else final=1;
			}
			fclose(fitxerout);
			if (!comprova_claus_sts("tempm3.sts",3)) error=3;
			remove("tempm3.sts");
		} else error=1;
		fclose(fitxer);
	}


	free(comodi);

	return(error);
}

char *encripta(char *m)
{
	/* encripta amb IDEA i codifica mime64, retornant el resultat */
	int i,j;
	FILE *fitxer;
	char *parametres[3];
	OctetString *to_crypt;
	BitString *crypted;

	/* primer encriptem */
	to_crypt=(OctetString *)malloc(sizeof(OctetString));
	to_crypt->octets=(char *)malloc(sizeof(char)*tamany_text);
	crypted=(BitString *)malloc(sizeof(BitString));
	crypted->bits=(char *)malloc(sizeof(char)*tamany_text);
	for (i=0;i<strlen(m);i++) to_crypt->octets[i]=m[i];
	to_crypt->noctets=strlen(m);
	j=idea_encrypt(to_crypt,crypted,SEC_END,clau);
	/* ara codifiquem mime64 */
	fitxer=fopen("tosend.sts","wb");
	if (!fitxer)
	{
		printf("\nError grabant fitxer 'tosend.sts'\n");
		exit(3);
	}
	for (i=0;i<j/8;i++) fputc(crypted->bits[i],fitxer);
	fclose(fitxer);
	free(crypted->bits); free(to_crypt->octets); free(crypted); free(to_crypt);
	parametres[0]=(char *)malloc(sizeof(char)*256);
	parametres[1]=(char *)malloc(sizeof(char)*256);
	parametres[2]=(char *)malloc(sizeof(char)*256);
	strcpy(parametres[1],"tosend.sts"); strcpy(parametres[2],"-e");
	mime64(3,parametres);
	free(parametres[0]); free(parametres[1]); free(parametres[2]);
	fitxer=fopen("tosend.sts","r");
	if (!fitxer)
	{
		printf("\nError en l'aplicacio. El fitxer 'tosend.sts' no existeix.");
		exit(3);
	}
	j=0;
	while (!feof(fitxer)) m[j++]=fgetc(fitxer);
	fclose(fitxer);
	remove("tosend.sts");

	return(m);
}

char *desencripta(char *m)
{
	/*
		 decodifica mime64 el text i posteriorment el desencripta, retornant el text en clar
	*/
	char *parametres[2];
	int i,j;
	FILE *fitxer;
	OctetString *decrypted;
	BitString *to_decrypt;

	 /* primer decodifiquem */
	fitxer=fopen("crypted.sts","w");
	for (i=0;i<strlen(m);i++) fputc(m[i],fitxer);
	fclose(fitxer);
	parametres[0]=(char *)malloc(sizeof(char)*256);
	parametres[1]=(char *)malloc(sizeof(char)*256);
	strcpy(parametres[1],"crypted.sts");
	mime64(2,parametres); /* a "crypted.sts" tenim el xifrat IDEA */
	free(parametres[0]); free(parametres[1]);
	/* ara desencriptem IDEA */
	decrypted=(OctetString *)malloc(sizeof(OctetString));
	decrypted->octets=(char *)malloc(sizeof(char)*tamany_text);
	to_decrypt=(BitString *)malloc(sizeof(BitString));
	to_decrypt->bits=(char *)malloc(sizeof(char)*tamany_text);
	fitxer=fopen("tosend.sts","rb");
	i=0;
	while (!feof(fitxer)) to_decrypt->bits[i++]=fgetc(fitxer);
	to_decrypt->nbits=i*8;
	fclose(fitxer);
	remove("tosend.sts");
	j=idea_decrypt(to_decrypt,decrypted,SEC_END,clau);
	for (i=0;i<strlen(decrypted->octets);i++) m[i]=decrypted->octets[i]; m[i]=0;
	free(decrypted->octets); free(to_decrypt->bits); free(decrypted); free(to_decrypt);

	return(m);
}

void prepara_pari()
{
	/*
		Inicialitzacio pila Pari i variables globals.
		S'ha de cridar al principi de l'aplicacio.
	*/

	init(2000000,1000);
	init_rndm();
	text=malloc(tamany_text*sizeof(char));
	setbuf(stdout,NULL);
	setbuf(stdin,NULL);
}

void prepara_sts(char *nomcfg)
{
	/* reserva memoria per Idea (SecuDE) */
	clau=(KeyInfo *)malloc(sizeof(clau));
	char_string=(OctetString *)malloc(sizeof(OctetString));
	bit_string=(BitString *)malloc(sizeof(BitString));

	char_string->octets=(char *)malloc(sizeof(char)*tamany_text);
	bit_string->bits=(char *)malloc(sizeof(char)*tamany_text);
	clau->subjectkey.bits=(char *)malloc(sizeof(char)*128);

	clau->subjectkey.nbits=128;
	clau->subjectAI=(AlgId *)malloc(sizeof(AlgId)); /* no el fem servir, pero... */

	llegeix_config(nomcfg);
	llegeix_cos_sts(); /* llegeix el primer que genera el cos i el primitiu */
}

void allibera_pari(void)
{
	fflush(stdout);
}

void allibera_sts(void)
{
	/*
		Alliberament de memoria i tancament de buffers.
		S'ha de cridar al final de l'aplicacio.
	*/
	free(clau->subjectAI);
	free(clau->subjectkey.bits);
	free(bit_string->bits);
	free(char_string->octets);

	free(bit_string);
	free(char_string);
	free(clau);

	free(text);
	remove("crypted.sts");
}
