#include <stdbool.h>

bool event(int *array) {
    int count;
    for(int i = 0; i < 2; i++){
        count = 0;
        for (int j = i+1; j < 4; j++){
            if (array[i] == array[j] && array[j] != -1) {
                count ++;
            }
        }
        if (count > 1) {
            return true;
        }
    }
    return false;
}