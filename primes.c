#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0   // file descriptor de lectura
#define WRITE 1  // file descriptor de escritura

/*
    Esta función recursiva toma por parámetro un pipe el cual tendrá escrito por
    un proceso anterior a una serie de núemeros (o el caso base de estar vacío).
    Posteriormente toma el primer número y lo considera primo, lo imprime y finalmente
    escribe en un nuevo pipe a aquellos números que el proceso anterior escribió
    y no son múltiplos del que se consideró como primo en un primer momento.
*/
void
WrapperCribaDeEratostenes(int pipeLectura[])
{
	int primo, valor;

	// Cierro fds que no voy a usar.
	close(pipeLectura[WRITE]);

	// Asumimos al primer número leído del pipe como primo.
	valor = read(pipeLectura[READ], &primo, sizeof(primo));
	// printf("Valor leido: %d.\n", primo);
	if (valor == EOF) {
		return exit(EXIT_SUCCESS);
	}

	if (valor == 0) {
		return exit(EXIT_FAILURE);
	}

	printf("primo %d.\n", primo);
	fflush(stdout);

	// Creo el pipe mediante el cual me comunicaré con el siguiente proceso.
	int pipeEscritura[2];
	if (pipe(pipeEscritura) < 0) {
		printf("Error creando el pipe.\n");
		exit(EXIT_FAILURE);
	}

	int pid = fork();

	if (pid > 0) {
		// Caso del proceso padre.
		// Cierro fds que no voy a usar.
		close(pipeEscritura[READ]);

		// Mientras leamos números del pipe...
		int numeroEntrante;

		while (read(pipeLectura[READ],
		            &numeroEntrante,
		            sizeof(numeroEntrante)) > 0) {
			if (numeroEntrante == -1) {
				printf("Error al leer del pipe.\n");
				exit(EXIT_FAILURE);
			}

			if (numeroEntrante == EOF) {
				return;
			}

			// Le paso el número (en caso de que no sea múltiplo del
			// primo que imprimí) al pipe para que el siguiente proceso siga filtrando.
			if (numeroEntrante % primo != 0) {
				valor = write(pipeEscritura[WRITE],
				              &numeroEntrante,
				              sizeof(numeroEntrante));

				if (valor == -1) {
					printf("Error al escribir.\n");
					exit(EXIT_FAILURE);
				}
			}
		}

		// Ya terminé de leer.
		close(pipeLectura[READ]);
		close(pipeEscritura[WRITE]);
		wait(NULL);
	} else if (pid == 0) {
		// Caso del proceso hijo.
		// Llamamos de manera recursiva para que un siguiente proceso siga filtrando.
		close(pipeLectura[READ]);
		WrapperCribaDeEratostenes(pipeEscritura);
	} else {
		printf("Error creando el fork.\n");
		exit(EXIT_FAILURE);
	}
}

/*
    Creamos un primer pipe para pasarlo como parámetro a nuestra función recursiva.
    Posteriormente comenzamos a escribir en él los números del 2 al ingresado.
*/
void
IniciarCribaDeEratostenes(int input)
{
	int pipeEscritura[2];
	if (pipe(pipeEscritura) == -1) {
		printf("Error creando el pipe.\n");
		exit(EXIT_FAILURE);
	}

	int pid = fork();

	if (pid == 0) {
		// Caso del proceso hijo.
		WrapperCribaDeEratostenes(pipeEscritura);
	} else if (pid > 0) {
		// Caso del proceso padre.
		close(pipeEscritura[READ]);
		for (int i = 2; i <= input; i++) {
			if (write(pipeEscritura[WRITE], &i, sizeof(i)) == -1) {
				printf("Error al escribir en el pipe.\n");
				exit(EXIT_FAILURE);
			}
		}
		close(pipeEscritura[WRITE]);
		wait(NULL);
	} else {
		printf("Error creando el fork.\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
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
