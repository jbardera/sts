/*
	 PHONE.C - client rpcgen
	 Compilador: GNU C
	 Autor: Joanmi Bardera, 1995
*/

#include <sys/types.h>
#include <netdb.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pwd.h>
#include <rpc/rpc.h>
#include <rpc_phone.h>
#include <stdio.h>
#include <string.h>
#include <sts.c>

int main(int argc, char *argv[])
{
	CLIENT *remote;
	static char *missatge;
	static char *rebut;
	char myhostname[MAXHOSTNAMELEN+1];
	char username[MAXHOSTNAMELEN+1];
	register int uid;
	struct passwd *pw;
	struct hostent *hp;
	int errorlevel;
	FILE *fitxer;
	int i,j;
	char tecla;

	setbuf(stdout,NULL);
	setbuf(stdin,NULL);
	errorlevel=0;

	missatge=(char *)malloc(sizeof(char)*tamany_text);
	rebut=(char *)malloc(sizeof(char)*tamany_text);
	uid=getuid();
	if ((pw=getpwuid(uid))==NULL)
	{
		printf("Error! Login propi no trobat.\n");
		exit(-1);
	}
	strcpy(username,pw->pw_name);
	if (gethostname(myhostname,sizeof(myhostname))==-1)
	{
		printf("Error! Domini propi no trobat a la configuracio local de TCP/IP.\n");
		exit(-2);
	}
	hp=gethostbyname(myhostname); /* trobem la nostra adreca IP */
	if (hp)
	{
		if (argc<2)
		{
			printf("Phone - versio rpcgen\n");
			printf("Sintaxi: phone [domini] [log]\n");
			printf("\n         (indicant 'log' guarda els missatges STS a disc)\n");
			errorlevel=1;
		} else
		{
			/* obrim la connexio TCP amb la maquina remota */
			remote=clnt_create(argv[1],RPC_PHONE_PRG,RPC_PHONE_VERS,"tcp");
			if (remote!=NULL)
			{
				/* el primer a fer, incialitzar globals */
				prepara_pari();
				/* llegim la configuracio per saber quins arxius tractar */
				prepara_sts("sts.cfg");
				/* creem el primer missatge, es volca al fitxer 1m.sts */
				if (argc>2) video=1; else video=0;
				missatge_1();
				/* l'enviem a l'altre extrem */
				fitxer=fopen("1m.sts","rt");
				if (!fitxer)
				{
					printf("\nError obrint fitxer 1m.sts\n");
					exit(3);
				}
				i=0;
				while (!feof(fitxer)) missatge[i++]=(char)fgetc(fitxer);
				missatge[--i]=0; /* final de cadena, restem 1 per la marca EOF llegida */
				fclose(fitxer);
				rebut=*sts_m1_1(&missatge,remote);
				i=strlen(rebut);
				if (!i)
				{
					printf("\nEl destinatari no ha acceptat el 1r missatge STS.");
					printf("\nImpossible establir connexio.\n");
					errorlevel=10;
				} else
				{
					/* a 'rebut' tenim el 2n missatge STS, el processem i generem el 3r si escau */
					i=missatge_3(rebut);
					if (!i)
					{
						/* a 3m.sts tenim volcat el tercer missatge */
						fitxer=fopen("3m.sts","rt");
						if (!fitxer)
						{
							printf("\nError obrint fitxer 3m.sts\n");
							exit(3);
						}
						i=0;
						while (!feof(fitxer)) missatge[i++]=(char)fgetc(fitxer);
						missatge[--i]=0; /* final de cadena, restem 1 per la marca EOF llegida */
						fclose(fitxer);
						rebut=*sts_m3_1(&missatge,remote);
						i=strlen(rebut);
						if (i)
						{
							if (!video)
							{
								remove("1m.sts"); remove("2m.sts"); remove("3m.sts");
							}
							strcpy(missatge,"\nConnexio STS realitzada amb exit\nL'usuari ");
							strcat(missatge,username);
							strcat(missatge," ens esta trucant des de ");
							strcat(missatge,(char *)inet_ntoa(*hp->h_addr_list));
							strcat(missatge," (");
							strcat(missatge,myhostname);
							strcat(missatge,").\n");
							missatge=encripta(missatge); /* encriptem amb IDEA i codifiquem el text */
							send_1(&missatge,remote); 	 /* enviem el missatge al remot */
							tecla=0; missatge[0]='<'; i=1;
							while (tecla!=27)
							{
								if (i<78)
								{
									tecla=(char)getchar();
									switch (tecla)
									{
										case '\n': missatge[i++]='>'; missatge[i++]='\n';
															 missatge[i]='\x0';
															 missatge=encripta(missatge);
															 send_1(&missatge,remote);
															 missatge[0]='<'; i=1;
															 break;
										default: missatge[i++]=tecla;
														 break;
									}
								} else
								{
									i=2; missatge[79]='>';
									missatge[80]='\x0'; missatge=encripta(missatge); send_1(&missatge,remote);
									missatge[0]='<'; missatge[1]=tecla;
								}
							}
							strcpy(missatge,"\nEn ");
							strcat(missatge,username);
							strcat(missatge," ha tancat la seva connexio.\n");
							missatge=encripta(missatge);
							send_1(&missatge,remote);
						} else
						{
							printf("\nEl destinatari no ha acceptat el 3r missatge STS.");
							printf("\nImpossible establir connexio.\n");
							errorlevel=20;
						}
					} else
					{
						printf("\nConnexio no realitzada. Abortada en el 2n missatge STS.");
						switch(i)
						{
							case 1: printf("\nError en el format del 2n missatge STS.\n");
											break;
							case 2: printf("\nError en el certificat que el remot ens ha enviat.\n");
											break;
							case 3: printf("\nError en les claus signades i encriptades rebudes.\n");
											break;
							case 4: printf("\nError signant les claus al preparar el 3r missatge STS.\n");
						}
					}
				}
				clnt_destroy(remote); /* tanquem la connexio */
				allibera_sts();
			} else
			{
				printf("%s: unknown host\n",argv[1]);
				exit(-3);
			}
		} /* if (argc<2) */
	} else
	{
		printf("Error. Domini propi no trobat al DNS.\n");
		exit(-4);
	}

	/* final phone, alliberament memoria */
	free(rebut);
	free(missatge);
	allibera_pari();

	return(errorlevel);
}
