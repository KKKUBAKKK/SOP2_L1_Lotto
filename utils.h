#pragma once

#include <stdlib.h>

#define NUMBERS_MAX 49
#define NUMBERS 6

#define SWAP(x, y)        \
    do                    \
    {                     \
        typeof(x) _x = x; \
        typeof(y) _y = y; \
        x = _y;           \
        y = _x;           \
    } while (0)

void draw(int *numbers)
{
    int numbers_all[NUMBERS_MAX];
    for (int i = 1; i <= NUMBERS_MAX; ++i) {
        numbers_all[i - 1] = i;
    }
    for (int i = 0; i < NUMBERS_MAX - 1; i++)
    {
        int j = i + rand() / (RAND_MAX / (NUMBERS_MAX - i) + 1);
        SWAP(numbers_all[i], numbers_all[j]);
    }
    for (int i = 0; i < NUMBERS; ++i) {
        numbers[i] = numbers_all[i];
    }
}

int compare(const int *bet, const int *numbers) {
    int result = 0;
    for (int i = 0; i < NUMBERS; ++i) {
        int number = numbers[i];
        for (int j = 0; j < NUMBERS; ++j) {
            if(number == bet[j]) {
                result++;
            }
        }
    }
    return result;
}

int get_reward(int matches) {
    int rewards[] = {0, 0, 0, 24, 160, 6000, 10000000};
    return rewards[matches];
}