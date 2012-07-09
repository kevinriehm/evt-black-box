#include "angel.h"

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5C

#define SHA256_BLOCK_BYTES 64

struct sha256_state {
	int byteshashed;
	uint32_t h[8];
	uint32_t w[64];
};

static inline uint32_t ror(uint32_t a, int b) { return a >> b | a << (32 - b); }

static void sha256_init(struct sha256_state *state)
{
	state->byteshashed = 0;
	state->h[0] = 0x6a09e667, state->h[1] = 0xbb67ae85,
	state->h[2] = 0x3c6ef372, state->h[3] = 0xa54ff53a,
	state->h[4] = 0x510e527f, state->h[5] = 0x9b05688c,
	state->h[6] = 0x1f83d9ab, state->h[7] = 0x5be0cd19;
}

static void sha256_munch(struct sha256_state *state, const void *data, int datasize)
{
	static const uint32_t k[] = {
		0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
		0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
		0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
		0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
		0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
		0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
		0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
		0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
		0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
		0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
		0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
		0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
		0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
		0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
		0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
		0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
	};

	int i;
	uint32_t *w = state->w,
		a, b, c, d, e, f, g, h, ch, maj, s0, s1, t1, t2;

	while(datasize)
	{
		// Load data into w[]
		for(i = state->byteshashed & 0x3F; datasize && i < 0x40;
			state->byteshashed++, datasize--, i++)
			w[i/4] = (w[i/4] & ~(0xFF000000 >> 8*(i & 3)))
				| (*(uint8_t *) data++ << (24 - 8*(i & 3)));
		if(!datasize && i != 0x40) break; // Stop if we need more data

		// Extend w[]
		for(i = 16; i < 64; i++)
		{
			s0 = ror(w[i-15],7) ^ ror(w[i-15],18) ^ w[i-15] >> 3;
			s1 = ror(w[i-2],17) ^ ror(w[i-2],19)  ^ w[i-2] >> 10;
			w[i] = w[i-16] + s0 + w[i-7] + s1;
		}

		// Round and round we go; where we stop, nobody knows!

		a = state->h[0], b = state->h[1],
		c = state->h[2], d = state->h[3],
		e = state->h[4], f = state->h[5],
		g = state->h[6], h = state->h[7];

		for(i = 0; i < 64; i++)
		{
			s0 = ror(a,2) ^ ror(a,13) ^ ror(a,22);
			maj = (a & b) ^ (a & c) ^ (b & c);
			t2 = s0 + maj;
			s1 = ror(e,6) ^ ror(e,11) ^ ror(e,25);
			ch = (e & f) ^ (~e & g);
			t1 = h + s1 + ch + k[i] + w[i];

			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		// Add these results
		state->h[0] += a, state->h[1] += b,
		state->h[2] += c, state->h[3] += d,
		state->h[4] += e, state->h[5] += f,
		state->h[6] += g, state->h[7] += h; 
	}
}

static void sha256_finish(struct sha256_state *state,
	uint8_t hash[SHA256_HASH_BYTES])
{
	const uint64_t msgbits = state->byteshashed << 3;

	uint8_t b[8];
	int fill = (0x40 - ((state->byteshashed + 1 + 8) & 0x3F)) & 0x3F,
		i;

	// Padding
	b[0] = 0x80;
	sha256_munch(state,b,1);
	b[0] = 0x00;
	while(fill--)
		sha256_munch(state,b,1);

	// Message size
	b[0] = msgbits >> 56 & 0xFF, b[1] = msgbits >> 48 & 0xFF,
	b[2] = msgbits >> 40 & 0xFF, b[3] = msgbits >> 32 & 0xFF,
	b[4] = msgbits >> 24 & 0xFF, b[5] = msgbits >> 16 & 0xFF,
	b[6] = msgbits >>  8 & 0xFF, b[7] = msgbits >>  0 & 0xFF;
	sha256_munch(state,b,8);

	// Store the hash
	for(i = 0; i < 8; i++)
	{
		hash[4*i + 0] = state->h[i] >> 24 & 0xFF,
		hash[4*i + 1] = state->h[i] >> 16 & 0xFF,
		hash[4*i + 2] = state->h[i] >>  8 & 0xFF,
		hash[4*i + 3] = state->h[i] >>  0 & 0xFF;
	}
}

void sha256_hash(uint8_t hash[SHA256_HASH_BYTES], const void *data,
	int datasize)
{
	struct sha256_state state;
	sha256_init(&state);
	sha256_munch(&state,data,datasize);
	sha256_finish(&state,hash);
}

void hmac_sha256(uint8_t hmac[SHA256_HASH_BYTES], const void *key,
	int keysize, const void *data, int datasize)
{
	int i;
	uint8_t fullkey[SHA256_BLOCK_BYTES];
	struct sha256_state state;

	// Adjust the key
	if(keysize > SHA256_BLOCK_BYTES)
	{
		sha256_hash(fullkey,key,keysize);
		keysize = SHA256_HASH_BYTES;
	} else memcpy(fullkey,key,keysize);
	memset(fullkey + keysize,0x00,SHA256_BLOCK_BYTES - keysize);

	// Inner hash
	for(i = 0; i < SHA256_BLOCK_BYTES; i++)
		fullkey[i] ^= HMAC_IPAD;
	sha256_init(&state);
	sha256_munch(&state,fullkey,SHA256_BLOCK_BYTES);
	sha256_munch(&state,data,datasize);
	sha256_finish(&state,hmac);

	// Outer hash
	for(i = 0; i < SHA256_BLOCK_BYTES; i++)
		fullkey[i] ^= HMAC_IPAD ^ HMAC_OPAD;
	sha256_init(&state);
	sha256_munch(&state,fullkey,SHA256_BLOCK_BYTES);
	sha256_munch(&state,hmac,SHA256_HASH_BYTES);
	sha256_finish(&state,hmac);
}

void sha256_str(char *str, uint8_t hash[SHA256_HASH_BYTES])
{
	int i;
	for(i = 0; i < SHA256_HASH_BYTES; i++)
		sprintf(str + 2*i,"%.2x",hash[i]);
}
