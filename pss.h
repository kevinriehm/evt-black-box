typedef struct pss_attr {
	struct pss_attr *prev;
	struct pss_attr *next;
} pss_attr_t;

typedef struct pss_block {
	struct pss_block *prev;
	struct pss_block *next;
} pss_block_t;
