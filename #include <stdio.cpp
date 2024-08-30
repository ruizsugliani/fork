#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

void
ejecutarComando(char *comandoPorEjecutar, char *args, int count)
{
	// Asegura que el último elemento sea NULL
	args[count] = NULL;
	int childPid = fork();
	if (childPid == -1) {
		printf("Error al crear el proceso hijo");
		exit(EXIT_FAILURE);
	} else if (childPid == 0) {
		// child
		execvp(comandoPorEjecutar, args);
		perror("Error al ejecutar el comando");
		_exit(EXIT_FAILURE);
	} else {
		// parent
		wait(NULL);
	}
}

void
Xargs(char *comandoPorEjecutar)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	char *args[NARGS];
	int count = 0;

	while ((nread = getline(&line, &len, stdin))) {
		if (nread != EOF) {
			line[nread - 1] = '\0';
			args[count] = line;
			count++;
			line = NULL;
		}

		if (count == NARGS || nread == EOF) {
			int aux = count;
			ejecutarComando(comandoPorEjecutar, args, count);
			for (int i = 1; i < aux; i++) {
				free(args[i]);
			}
			if (nread == EOF) {
				break;
			}
			count = 1;
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "No se ha proporcionado el comando.\n");
		exit(EXIT_FAILURE);
	}

	Xargs(argv[1]);

	return EXIT_SUCCESS;
}
