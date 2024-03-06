#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"

// DODALEM
#include <signal.h>
#include <errno.h>
#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))
// NA MACU
#define TEMP_FAILURE_RETRY(expression) \
    ({                                 \
        ssize_t result;                \
        do {                           \
            result = (expression);     \
        } while (result == -1 && errno == EINTR); \
        result;                        \
    })
#define GUESS_LENGTH 6

void usage(char *name)
{
    fprintf(stderr, "USAGE: %s N T\n", name);
    fprintf(stderr, "N: N >= 1 - number of players\n");
    fprintf(stderr, "T: T >= 1 - number of weeks (iterations)\n");
    exit(EXIT_FAILURE);
}

char *get_guess_rand(int8_t lowest, int8_t highest)
{
    int8_t *res = calloc(GUESS_LENGTH, sizeof(int8_t));
    srand(getpid());
    for (int i = 0; i < GUESS_LENGTH; i++)
        res[i] = lowest + rand() % highest;
    return (char *) res;
}

void player_work(int fdsIn, int fdsOut)
{
    char *guess = get_guess_rand(1, 49);
    char *luckyNumbers = calloc(GUESS_LENGTH, sizeof(char));
    if (TEMP_FAILURE_RETRY(write(fdsOut, guess, GUESS_LENGTH)) < 0)
        ERR("write from child");
    if (TEMP_FAILURE_RETRY(read(fdsIn, luckyNumbers, GUESS_LENGTH)) < 1)
        ERR("read");
    printf("%d received lucky numbers: %d, %d, %d, %d, %d ,%d\n", getpid(), luckyNumbers[0], luckyNumbers[1],
           luckyNumbers[2], luckyNumbers[3], luckyNumbers[4], luckyNumbers[5]);
}

void totalizator_work(int n, int *fdsIn, int *fdsOut)
{
//    char **players_guesses = malloc(n * sizeof(char*));
    char *players_guesses = malloc(6 * sizeof(char));

    for (int i = 0; i < n; i++)
    {
//        players_guesses[i] = malloc(6 * sizeof(char));
        if (TEMP_FAILURE_RETRY(read(fdsIn[i], players_guesses, 6)) < 1)
            ERR("read");
        printf(); // trzeba dostac jakos pid dziecka, moze wyslac je od niego pipem???
    }


}

void create_players_and_pipes(int n, int *fdsIn, int *fdsOut)
{
    int tmpfdIn[2];
    int tmpfdOut[2]
    int max = n;
    while (n)
    {
        if (pipe(tmpfdIn))
            ERR("pipe");
        if (pipe(tmpfdOut))
            ERR("pipe");
        switch (fork())
        {
            case 0:
                while (n < max) {
                    if (fdsIn[n] && TEMP_FAILURE_RETRY(close(fdsIn[n++])))
                        ERR("close");
                    if (fdsOut[n] && TEMP_FAILURE_RETRY(close(fdsOut[n++])))
                        ERR("close");
                }
                free(fdsIn);
                free(fdsOut);
                if (TEMP_FAILURE_RETRY(close(tmpfdIn[1])))
                    ERR("close");
                if (TEMP_FAILURE_RETRY(close(tmpfdOut[0])))
                    ERR("close");
                printf("%d: I'm going to play Lotto\n", getpid());
                player_work(tmpfdIn[0], tmpfdOut[1]);
                if (TEMP_FAILURE_RETRY(close(tmpfdIn[0])))
                    ERR("close");
                if (TEMP_FAILURE_RETRY(close(tmpfdOut[1])))
                    ERR("close");
                exit(EXIT_SUCCESS);

            case -1:
                ERR("Fork:");
        }
        if (TEMP_FAILURE_RETRY(close(tmpfdIn[0])))
            ERR("close");
        if (TEMP_FAILURE_RETRY(close(tmpfdOut[1])))
            ERR("close");
        fdsIn[--n] = tmpfdOut[0];
        fdsOut[n] = tmpfdIn[1];
    }
}

int main(int argc, char **argv)
{
    pid_t pid = getpid();
    srand(pid);

    int bet[6], numbers[6];

    for (int i = 0; i < 100; ++i) {
        draw(bet);
        draw(numbers);
        int matches = compare(bet, numbers);
        int reward = get_reward(matches);
        if(reward == 0) continue;
        printf("bet: %d, %d, %d, %d, %d, %d\n", bet[0], bet[1], bet[2], bet[3], bet[4], bet[5]);
        printf("draw: %d, %d, %d, %d, %d, %d\n", numbers[0], numbers[1], numbers[2], numbers[3], numbers[4], numbers[5]);
        printf("%d numbers matches\n", matches);
        printf("%d payout\n", reward);
    }

//    usage(argv[0]);

    // moj kod odtad
    if (argc < 3)
        usage(argv[0]);
    int N = atoi(argv[1]);
    int T = atoi(argv[2]);
    int *fdsIn, *fdsOut;
    if (NULL == (fdsIn = (int *)malloc(sizeof(int) * n)))
        ERR("malloc");
    if (NULL == (fdsOut = (int *)malloc(sizeof(int) * n)))
        ERR("malloc");
    create_players_and_pipes(N, fdsIn, fdsOut);
    totalizator_work(); //co tu dac

    exit(EXIT_SUCCESS);
}
