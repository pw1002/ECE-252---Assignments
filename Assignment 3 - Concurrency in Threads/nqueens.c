// from the Cilk manual: http://supertech.csail.mit.edu/cilk/manual-5.4.6.pdf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

struct thread_args{
	char *config;
	int i;
	int n;
	int j;
};

int safe(char * config, int i, int j)
{
	int r, s;

	for (r = 0; r < i; r++)
	{
		s = config[r];
		if (j == s || i-r==j-s || i-r==s-j)
			return 0;
	}
	return 1;
}

int count = 0;
sem_t sem;

void nqueens(char *config, int n, int i)
{
	char *new_config;
	int j;
	
	sem_wait(&sem);
	if (i==n)
	{
		count++;
	}
	sem_post(&sem);

	/* try each possible position for queen <i> */
	for (j=0; j<n; j++)
	{
		/* allocate a temporary array and copy the config into it */
		new_config = malloc((i+1)*sizeof(char));
		memcpy(new_config, config, i*sizeof(char));
		if (safe(new_config, i, j))
		{
			new_config[i] = j;
			nqueens(new_config, n, i+1);
		}
		free(new_config);
	}
	return;
}

void *do_work(void *arg){
	struct thread_args *p_in = arg;
	char *c = p_in->config;
	int i = p_in->i;
	int n = p_in->n;
	int j = p_in->j;

	char *new_config;
	/* allocate a temporary array and copy the config into it */
	new_config = malloc((i+1)*sizeof(char));
	memcpy(new_config, c, i*sizeof(char));
	if (safe(new_config, i, j))
	{
		new_config[i] = j;
		nqueens(new_config, n, i+1);
	}
	free(new_config);
}

int main(int argc, char *argv[])
{
	int n;
	char *config;
	sem_init(&sem, 0, 1);

	if (argc < 2)
	{
		printf("%s: number of queens required\n", argv[0]);
		return 1;
	}

	n = atoi(argv[1]);
	config = malloc(n * sizeof(char));

	pthread_t *p_tids = malloc(sizeof(pthread_t) * n);
	struct thread_args in_params[n];

	printf("running queens %d\n", n);
	for(int i = 0; i < n; i++){
		in_params[i].config = config;
		in_params[i].i = 0;
		in_params[i].n = n;
		in_params[i].j = i;
		pthread_create(p_tids + i, NULL, do_work, in_params + i);
	}

	for(int i = 0; i < n; i ++){
		pthread_join(p_tids[i], NULL);
	}
	printf("# solutions: %d\n", count);

	free(p_tids);
	free(config);

	return 0;
}
