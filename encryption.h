#include <openssl/aes.h>
#include <string.h>

#define KEY "SecReTAeSKEY"  // 16 bytes for AES-128

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
