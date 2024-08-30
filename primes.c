#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0   // file descriptor de lectura
#define WRITE 1  // file descriptor de escritura

void WrapperCribaDeEratostenes(int fds[]);
void IniciarCribaDeEratostenes(int input);

/*
    Esta función recursiva toma por parámetro un pipe el cual tendrá escrito por
    un proceso anterior a una serie de núemeros (o el caso base de estar vacío).
    Posteriormente toma el primer número y lo considera primo, lo imprime y finalmente
    escribe en un nuevo pipe a aquellos números que el proceso anterior escribió
    y no son múltiplos del que se consideró como primo en un primer momento.
*/
void
WrapperCribaDeEratostenes(int pipeInicial[])
{
	// Como nunca voy a escribir de este pipe y solo leer, lo cierro.
	close(pipeInicial[WRITE]);

	// Creo el pipe mediante el cual me comunicaré con el siguiente proceso.
	int pipeSiguiente[2];
	if (pipe(pipeSiguiente) < 0) {
		printf("Error creando el pipe.\n");
		exit(EXIT_FAILURE);
	}

	int pid = fork();
	if (pid < 0) {
		printf("Error creando el fork.\n");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		// Caso del proceso hijo.
		int primo, numeroEntrante, valor;

		// Asumimos al primer número leído del pipe como primo.
		valor = read(pipeInicial[READ], &primo, sizeof(primo));
		if (valor == -1) {
			printf("Error al leer del pipe.\n");
			exit(EXIT_FAILURE);
		} else if (valor == 0) {
			// Ya se que no voy a leer nada más (anteriormente ya cerré la escritura).
			close(pipeInicial[READ]);
			return;
		} else {
			printf("primo %d.\n", primo);
		}

		// Mientras leamos números del pipe...
		while (read(pipeInicial[READ],
		            &numeroEntrante,
		            sizeof(numeroEntrante)) > 0) {
			if (numeroEntrante == -1) {
				printf("Error al leer del pipe.\n");
				exit(EXIT_FAILURE);
			}

			// Le paso el número (en caso de que no sea múltiplo del
			// primo que imprimí) al pipe para que el siguiente proceso siga filtrando.
			if (numeroEntrante % primo != 0) {
				valor = write(pipeSiguiente[WRITE],
				              &numeroEntrante,
				              sizeof(numeroEntrante));

				if (valor == -1) {
					printf("Error al escribir.\n");
					exit(EXIT_FAILURE);
				}
			}
		}

		// Ya terminé de leer.
		close(pipeInicial[READ]);

		// Llamamos de manera recursiva para que un siguiente proceso siga filtrando.
		WrapperCribaDeEratostenes(pipeSiguiente);

	} else {
		// Caso del proceso padre.
		close(pipeInicial[WRITE]);
		close(pipeSiguiente[READ]);
		close(pipeSiguiente[WRITE]);
		wait(NULL);
	}
}

/*
    Creamos un primer pipe para pasarlo como parámetro a nuestra función recursiva.
    Posteriormente comenzamos a escribir en él los números del 2 al ingresado.
*/
void
IniciarCribaDeEratostenes(int input)
{
	int pipeInicial[2];
	if (pipe(pipeInicial) == -1) {
		printf("Error creando el pipe.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 2; i <= input; i++) {
		if (write(pipeInicial[WRITE], &i, sizeof(i)) == -1) {
			printf("Error al escribir en el pipe.\n");
			exit(EXIT_FAILURE);
		}
	}

	WrapperCribaDeEratostenes(pipeInicial);
}

int
main(int argc, char *argv[])
{
	// Validamos el input.
	if (argc != 2) {
		printf("Error de uso, debe ingresarse un número N >= 2.\n");
		exit(EXIT_FAILURE);
	}

	// Levantamos el N ingresado.
	int n = atoi(argv[1]);

	if (n < 2) {
		printf("Error de uso, debe ingresarse un número N >= 2.\n");
		exit(EXIT_FAILURE);
	}

	IniciarCribaDeEratostenes(n);

	exit(EXIT_SUCCESS);
}
