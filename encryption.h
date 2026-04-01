#include <openssl/aes.h>
#include <string.h>

#define KEY "SecReTAeSKEY1234"  // 16 bytes for AES

int pad_data(char *data, int len) {
    int pad_len = AES_BLOCK_SIZE - (len % AES_BLOCK_SIZE);
    for (int i = 0; i < pad_len; i++) {
        data[len + i] = pad_len;
    }
    return len + pad_len;
}

int unpad_data(char *data, int len) {
    int pad_len = data[len - 1];
    return len - pad_len;
}

void encrypt(char *data, int len) {
    AES_KEY aesKey;
    AES_set_encrypt_key((const unsigned char*)KEY, 128, &aesKey);
    for (int i = 0; i < len; i += AES_BLOCK_SIZE) {
        AES_encrypt((unsigned char*)data + i, (unsigned char*)data + i, &aesKey);
    }
}

void decrypt(char *data, int len) {
    AES_KEY aesKey;
    AES_set_decrypt_key((const unsigned char*)KEY, 128, &aesKey);
    for (int i = 0; i < len; i += AES_BLOCK_SIZE) {
        AES_decrypt((unsigned char*)data + i, (unsigned char*)data + i, &aesKey);
    }
}
