
void encrypt(int* num) {
    int key = 133;
    *num = *num ^ key;
}

void decrypt(int* num) {
    int key = 133;
    *num = *num ^ key;
}

void encrypt_array(int* num_array, int length) {
    int key = 133;
    #pragma omp parallel for
    for (int i = 0; i < length; i++) {
        num_array[i] = num_array[i] ^ key;
    }

}

void decrypt_array(int* num_array, int length) {
    int key = 133;
    #pragma omp parallel for
    for (int i = 0; i < length; i++) {
        if (num_array[i] != -1) {
            num_array[i] = num_array[i] ^ key;
        }
    }
}
