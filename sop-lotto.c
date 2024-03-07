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

void player_work(int fdsIn, int fdsOut, int T)
{
    int guess[GUESS_LENGTH], luckyNumbers[GUESS_LENGTH], matches, amount_spent = 0, win, amount_won = 0;
    pid_t userId = getpid();
    srand(userId);
    for (int i = 0; i < T; i++) {
//        if (rand() % 9 == 0)
//        {
//            printf("%d: This is a waste of money\n", userId);
//            break;
//        }
        amount_spent += 3;
        draw(guess);
        if (TEMP_FAILURE_RETRY(write(fdsOut, &userId, sizeof(pid_t))) < 0)
            ERR("write");
        if (TEMP_FAILURE_RETRY(write(fdsOut, guess, sizeof(guess))) < 0)
            ERR("write");
        if (TEMP_FAILURE_RETRY(read(fdsIn, luckyNumbers, sizeof(luckyNumbers))) < 1)
            ERR("read");
        matches = compare(guess, luckyNumbers);
        win = get_reward(matches);
        amount_won += win;
        if (win > 0)
            printf("%d: I won %d zl.\n", userId, win);
    }
    if (TEMP_FAILURE_RETRY(write(fdsOut, &userId, sizeof(pid_t))) < 0)
        ERR("write");
    if (TEMP_FAILURE_RETRY(write(fdsOut, &amount_won, sizeof(int))) < 0)
        ERR("write");
    if (TEMP_FAILURE_RETRY(write(fdsOut, &amount_spent, sizeof(int))) < 0)
        ERR("write");
}

void totalizator_work(int n, int *fdsIn, int *fdsOut, int T)
{
    int players_guesses[GUESS_LENGTH], lucky_numbers[GUESS_LENGTH], amount_won, amount_spent;
    pid_t playerId;
    for (int t = 0; t < T; t++) {
        draw(lucky_numbers);
        for (int i = 0; i < n; i++) {
            if (TEMP_FAILURE_RETRY(read(fdsIn[i], &playerId, sizeof(pid_t))) < 1)
                ERR("read");
            if (TEMP_FAILURE_RETRY(read(fdsIn[i], players_guesses, sizeof(players_guesses))) < 1)
                ERR("read");
            printf("Totalizator Sportowy: %d bet: %d %d %d %d %d %d\n", playerId, players_guesses[0],
                   players_guesses[1], players_guesses[2], players_guesses[3], players_guesses[4], players_guesses[5]);
            if (TEMP_FAILURE_RETRY(write(fdsOut[i], lucky_numbers, sizeof(lucky_numbers))) < 0)
                ERR("write");
        }
        printf("Totalizator sportowy: %d %d %d %d %d %d are today's lucky numbers\n\n", lucky_numbers[0],
               lucky_numbers[1], lucky_numbers[2], lucky_numbers[3], lucky_numbers[4], lucky_numbers[5]);
    }
    printf("\nFINAL STATISTICS:\n");
    for (int i = 0; i < n; i++)
    {
        if (TEMP_FAILURE_RETRY(read(fdsIn[i], &playerId, sizeof(pid_t))) < 1)
            ERR("read");
        if (TEMP_FAILURE_RETRY(read(fdsIn[i], &amount_won, sizeof(int))) < 1)
            ERR("read");
        if (TEMP_FAILURE_RETRY(read(fdsIn[i], &amount_spent, sizeof(int))) < 1)
            ERR("read");
        printf("Player %d: won %d and spent %d\n", playerId, amount_won, amount_spent);
        if (TEMP_FAILURE_RETRY(close(fdsIn[i])))
            ERR("close");
        if (TEMP_FAILURE_RETRY(close(fdsOut[i])))
            ERR("close");
    }
}

void create_players_and_pipes(int n, int *fdsIn, int *fdsOut, int T)
{
    int tmpfdIn[2];
    int tmpfdOut[2];
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
                    if (fdsIn[n] && TEMP_FAILURE_RETRY(close(fdsIn[n])))
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
                player_work(tmpfdIn[0], tmpfdOut[1], T);
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
    if (argc < 3)
        usage(argv[0]);
    int N = atoi(argv[1]);
    int T = atoi(argv[2]);
    if (N < 1 || T < 1)
        usage(argv[0]);
    int *fdsIn, *fdsOut;
    if (NULL == (fdsIn = (int *)malloc(sizeof(int) * N)))
        ERR("malloc");
    if (NULL == (fdsOut = (int *)malloc(sizeof(int) * N)))
        ERR("malloc");
    create_players_and_pipes(N, fdsIn, fdsOut, T);
    totalizator_work(N, fdsIn, fdsOut, T);
    free(fdsIn);
    free(fdsOut);
    while (wait(NULL) != -1);

    exit(EXIT_SUCCESS);
}
