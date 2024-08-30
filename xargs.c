#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

int
Execute(char *comando, char *args[])
{
	int pid = fork();

	if (pid == 0) {
		// Caso del hijo.
		execvp(comando, args);
		printf("Error en execvp.\n");
		return (EXIT_FAILURE);
	} else if (pid > 0) {
		// Caso del padre.
		wait(NULL);
	} else {
		printf("Error en el fork.\n");
		return (EXIT_FAILURE);
	}
	
	return(EXIT_SUCCESS);
}

void
Xargs(char *comando)
{
	// Parámetros utilizados en getline.
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

	// Por convencion, el primer argumento apunta al nombre del archivo por ejecutar y el último a NULL.
	char *args[NARGS + 1];
	args[0] = comando;
	size_t count = 1;

	while ((nread = getline(&line, &len, stdin)) != EOF) {
		line[nread - 1] = '\0';
		args[count] = strdup(line);
		count++;

		if (count == NARGS + 1 ) {
			// Estamos listos para ejecutar el comando.
			args[count] = NULL;
			Execute(comando, args);

			// Liberamos memoria alocada por getline.
			for (size_t i = 1; i < count; i++) {
				free(args[i]);
			}

			// Limpiamos el arreglo de char *.
			for (size_t i = 1; i < count; i++) {
				args[i] = NULL;
			}

			count = 1;
		}

		// Dejamos listo para la próxima iteración.
		free(line);
		line = NULL;
		len = 0;
	}

	// Ejecutamos lo que haya quedado en el buffer.
    if (count > 1) {  // Si hay elementos pendientes
        args[count] = NULL;
        Execute(comando, args);
        for (size_t i = 1; i < count; i++) {
            free(args[i]);
        }
    }

}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Error de uso. Debe proveer al menos un "
		       "argumento.\n");
		exit(EXIT_FAILURE);
	}

	Xargs(argv[1]);

	return 0;
}