#define SHA256_HASH_BYTES 32

extern void hmac_sha256(uint8_t hmac[SHA256_HASH_BYTES], const void *key,
	int keysize, const void *data, int datasize);
extern void sha256_str(char *str, uint8_t hash[SHA256_HASH_BYTES]);

