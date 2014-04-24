/* 
   PHONED.C - servidor rpcgen

   Compilador: GNU C
   Autor: Joanmi Bardera, 1995
*/   

#include <stdio.h>
#include <string.h>
#include <sts.c>

char *sts_m1(char *caracters)
{
  static char torna[tamany_text];
  FILE *fitxer;
  int i,j;
      
  /* el primer a fer, incialitzar globals */
  prepara_pari();
  /* llegim la configuracio per saber quins arxius tractar */
  prepara_sts("sts.cfg");
    
  i=missatge_2(caracters);
  if (!i)
  {
    fitxer=fopen("2m.sts","rt");
    j=0;
    while (!feof(fitxer)) torna[j++]=(char)fgetc(fitxer);
    torna[--j]=0; /* final de cadena, restem 1 per la marca EOF llegida */
  } else
  {
    /* errors */
    printf("\nSTS: connexio no realitzada, primer missatge no acceptat [%i]\n",i);
    torna[0]=0; /* si la longitud es 0 el client assumeix error */
  }

  return(torna);
}

char *sts_m3(char *caracters)
{
   /* 
    hem de comprovar que el tercer missatge es correcte, si ho
    es retornem cadena no nul.la i la connexio es materialitza
  */
  static char torna[3];
  int i;
    
   i=connexio(caracters);
   if (!i) 
   {
     strcpy(torna,"ok");
     if (video) printf("\n");
   } else
   {
     printf("\nSTS: connexio no realitzada, tercer missatge no acceptat\n");
     torna[0]=0;
   }
	  
  return(torna);
}

void send(char *caracters)
{
   static char *entrada;
   
   entrada=(char *)desencripta(caracters);
   
   printf("%s",entrada);
   if (entrada[0]==0)
   {
     /* El remot tanca la connexio i podem alliberar les variables */
     allibera_sts();
	   allibera_pari();
   }
}

/* rutines d'interficie */

char **sts_m1_1(char **caracters)
{
  static char *result;
  
  result=sts_m1(*caracters);
  
  return(&result);
}

char **sts_m3_1(char **caracters)
{
  static char *result;
  
  result=sts_m3(*caracters);
  
  return(&result);
}

void *send_1(char **caracter)
{
	static char result;

	send(*caracter);
	
	return((void *)&result);
}

