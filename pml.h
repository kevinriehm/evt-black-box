typedef enum {
	PMLATTR_ID,
	PMLATTR_CLASS,
	PMLATTR_UNKNOWN,
	PMLATTR_COUNT // Always last
} pml_attr_type_t;

typedef enum {
	PMLNODE_TEXT, // Not a tag, just text
	
	// Tags that can contain other tags
	PMLNODE_BUTTON,
	PMLNODE_P,
	PMLNODE_SPAN,
	
	// Tags that cannot contain other tags
	PMLNODE_IMG,
	
	PMLNODE_UNKNOWN
} pml_node_type_t;

// Tag attribute
typedef struct pml_attr {
	pml_attr_type_t type;
	char *typestring;
	
	int hasvalue;
	char *value;
	
	struct pml_attr *prev;
	struct pml_attr *next;
} pml_attr_t;

// Node in markup tree
typedef struct pml_node {
	pml_node_type_t type;
	char *typestring;
	
	pml_attr_t *attrs;
	
	char *text; // For PMLNODE_TEXT
	
	struct pml_node *parent;
	struct pml_node *children;
	struct pml_node *prev;
	struct pml_node *next;
} pml_node_t;
