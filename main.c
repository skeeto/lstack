#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "sha1.h"
#include "lstack.h"

struct job {
    lstack_t *results;
    size_t min_results, nzeros;
};

const char EMPTY[SHA1_DIGEST_SIZE] = {0};

void print_hash(unsigned char hash[SHA1_DIGEST_SIZE])
{
    for (int i = 0; i < SHA1_DIGEST_SIZE; i++)
        printf("%02x", hash[i]);
    putchar('\n');
}

void fillrandom(unsigned char *buffer, size_t n)
{
    FILE *urandom = fopen("/dev/urandom", "r");
    if (urandom == NULL || !fread(buffer, n, 1, urandom))
        abort();
    fclose(urandom);
}

void hash(uint8_t in[SHA1_DIGEST_SIZE], uint8_t out[SHA1_DIGEST_SIZE])
{
    SHA1_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, in, SHA1_DIGEST_SIZE);
    SHA1_Final(&ctx, out);
}

void *worker(void *arg)
{
    struct job *job = arg;
    uint8_t buffer[SHA1_DIGEST_SIZE], output[SHA1_DIGEST_SIZE];
    fillrandom(buffer, sizeof(buffer)); // seed
    while (lstack_size(job->results) < job->min_results) {
        hash(buffer, output);
        if (memcmp(output, EMPTY, job->nzeros) == 0) {
            unsigned char *copy = malloc(SHA1_DIGEST_SIZE);
            if (copy == NULL)
                abort();
            memcpy(copy, buffer, SHA1_DIGEST_SIZE);
            if (lstack_push(job->results, copy) != 0)
                abort();
        }
        memcpy(buffer, output, SHA1_DIGEST_SIZE);
    }
    return NULL;
}

int main()
{
    int nthreads = sysconf(_SC_NPROCESSORS_ONLN); // Linux
    lstack_t results;
    struct job job = {
        .results = &results,
        .min_results = 4096,
        .nzeros = 2
    };
    lstack_init(&results, job.min_results + nthreads);

    /* Spawn threads. */
    pthread_t threads[nthreads];
    printf("Using %d thread%s.\n", nthreads, nthreads == 1 ? "" : "s");
    for (int i = 0; i < nthreads; i++)
        pthread_create(threads + i, NULL, worker, &job);

    /* Monitor progress */
    size_t nresults, last = -1;
    while ((nresults = lstack_size(&results)) < job.min_results) {
        if (nresults != last)
            fprintf(stderr, "\033[0GFound %zd hashes ...", nresults);
        last = nresults;
        sleep(1);
    }
    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);
    nresults = lstack_size(&results);
    fprintf(stderr, "\033[0G\033[KFound %zd hashes.\n", nresults);

    /* Print results */
    unsigned char *content;
    while ((content = lstack_pop(&results)) != NULL) {
        print_hash(content);
        free(content);
    }
    return 0;
}
