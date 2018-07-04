/**************
*

Copyright (C) 2018 Caetán Tojeiro Carpente

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see https://www.gnu.org/licenses/agpl-3.0.html

*
***************/

#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int i, done = 0, n;
    double PI25DT = 3.141592653589793238462643;
    double pi, h, sum, x, aux;
    
    int numprocs, miId;
	
	MPI_Status st;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &miId);
	

	//MPI_FLATTREECOLECTIVA (como la primitiva de Bcast)
	int MPI_FlattreeColectiva(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
		int i;
		MPI_Status st;
		MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
		MPI_Comm_rank(MPI_COMM_WORLD, &miId);
		
		if(miId==root){ //Si es el root envía los datos
			for(i=0; i<numprocs; i++){ //Para todos los procesos (incluido 0 porque es general y no se sabe cuál es el root)
				if(i!=root){  //y onviando el root ya que a este no hay que mandarle el dato, puesto que ya lo tiene
					MPI_Send(buf, count, datatype, i, 0, comm);  //se envía el dato a los procesos
				}
			}
		} else{ //Si no es root recibe los datos
			MPI_Recv(buf, count, datatype, root, 0, comm, &st);
		}
	}
	
	
	//MPI_BINOMIALCOLECTIVA (como la primitiva de Bcast)
	int MPI_BinomialColectiva(void *buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
		int i, destino, origen;
		MPI_Status st;
		MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
		MPI_Comm_rank(MPI_COMM_WORLD, &miId);
		
		for(i=1; i<=ceil(log2(numprocs)); i++){  //numero de pasos a realizar
			if(miId<pow(2,i-1)){  //si el proceso tiene que enviar
				destino = miId+pow(2,i-1);  //se calcula cuales son los procesos destino que le corresponden a este
				if (destino<numprocs){  //se comprueba que no nos excedemos del número de procesos
					MPI_Send(buf, count, datatype, destino, 0, comm);  //se envía el dato
				}
			} else{
				if(miId<pow(2,i)){  
					origen = miId-pow(2,i-1);  //se reciben datos de los procesos que le corresponden a este
					MPI_Recv(buf, count, datatype, origen, 0, comm, &st);
				}
			}
			
		} 
		
	}

    while (!done)
    {
		if(miId==0){ 
			printf("Enter the number of intervals: (0 quits) \n");
			scanf("%d",&n);
		}
	
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		//MPI_FlattreeColectiva(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		//MPI_BinomialColectiva(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
	  
	  	if(n==0) break;
	  		
	  	if(n>0){
			h   = 1.0 / (double) n;
			sum = 0.0;
			for (i = (miId + 1); i <= n; i+= numprocs) {
				x = h * ((double)i - 0.5);
				sum += 4.0 / (1.0 + x*x);
			}
			
			pi = h * sum;
			
		
			MPI_Reduce(&pi, &aux, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);			
			
			
			if(miId==0){
				printf("pi is approximately %.16f, Error is %.16f\n", aux, fabs(aux - PI25DT));			
			}
		
		} else {
			printf("Error: el numero introducido no es valido\n");
		}
    }
  
    MPI_Finalize();

}
