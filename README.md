Station-To-Station Protocol

Titol: Implementacio d'un protocol criptografic STS
Autor: Joan Miquel Bardera i Bosch

Passes a seguir:

1. Compilacio del codi font.

			$rpcgen rpc_phone.x
			$make
			$make phoned
			$make stsutil
			
2. Generacio dels fitxers amb les claus RSA

			$stsutil
			
			Triar opcio 1
			
3. Generar certificat amb la clau de l'autoritat central

			primer cal generar les claus de l'autoritat central
			$stsutil (opcio 1)
			
			ara amb la clau privada de l'autoritat central s'ha
			de signar la nostra clau publica
			$stsutil (opcio 2)
			
4. Crear un cos i trobar element primitiu per si volem engegar
   una connexio nosaltres:
   
   		$stsutil
   		
   		Triar opcio 3
   		
5. Crear un fitxer de nom sts.cfg on cal indicar els fitxers
   que farem servir. Es un fitxer de texte on se li han
   d'indicar 3 noms de fitxer (1 per linia): el primer es el nom
   on esta el cos i el primitiu, el segon el nom (sense extensio)
   dels fitxers amb les nostres claus (afegeix automaticament 
   .pvt i .pub com extensions), i el tercer	el nom complet del
   fitxer amb la clau publica de l'autoritat central.
   
   Exemple:
   
   		cos.sts
   		alice
   		acentral.pub
   		
   (Si la linia comença amb un punt i coma s'interpreta com comentari).
      		
Totes aquestes passes serveixen tant pel client com pel
servidor (nota: els certificats als dos extrems s'han de crear
amb la mateixa clau privada d'autoritat central!!). Es necessari
que ambdos disposin de la mateixa clau d'autoritat central,
obviament!!
								
