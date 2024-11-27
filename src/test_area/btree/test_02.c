#include "../../btree.h"
#include "../test_utils.h"
#include <afc/afc.h>

int check_tree(BTree *btr, char *p)
{
	BTreeNode *node;
	int pos, postot = 0;
	List *nm = afc_list_new();
	afc_list_add(nm, btr->root, AFC_LIST_ADD_TAIL);
	while (!afc_list_is_empty(nm))
	{
		afc_list_first(nm);
		node = afc_list_get(nm)->ln_Name;
		afc_list_del(nm);
		for (pos = 1; pos <= node->counter; pos++)
		{
			if (!btr->equal(p[postot++], (BTreeNode *)node->entry[pos]))
				return (0);
		}
		for (pos = 0; pos <= node->counter; pos++)
		{
			if (node->branch[pos] != NULL)
			{
				afc_list_add(nm, node->branch[pos], AFC_LIST_ADD_TAIL);
			}
		}
	}
	return (1);
}

struct chiave_s
{
	char *key;
	unsigned long id;
};

int create_key(void *entry, void *buffer, unsigned short *size)
{
	*size = (unsigned long)sizeof(void *);
	memcpy(buffer, &entry, *size);
	return (AFC_ERR_NO_ERROR);
}

int create_key_s(void *entry, void *buffer, unsigned short *size)
{
	*size = afc_string_len(((struct chiave_s *)entry)->key);
	memcpy(buffer, ((struct chiave_s *)entry)->key, *size);
	return (AFC_ERR_NO_ERROR);
}

int write_node(FILE *file, void *entry)
{
	TRY(int)
	if (fwrite(&entry, sizeof(void *), 1, file) != 1)
		; // RAISE_RC ( AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE );
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int write_node_s(FILE *file, void *entry)
{
	TRY(int)
	if (fwrite(&((struct chiave_s *)entry)->id, sizeof(unsigned long), 1, file) != 1)
		; //	RAISE_RC ( AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE );
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int read_key(void **entry, void *buffer, unsigned short size)
{

	memcpy(entry, &buffer, size);
	return (AFC_ERR_NO_ERROR);
}

int read_key_s(void **entry, void *buffer, unsigned short size)
{
	struct chiave_s *str;
	*entry = (struct chiave_s *)afc_malloc(sizeof(struct chiave_s));
	str = *entry;
	((struct chiave_s *)(*entry))->key = afc_string_new(size + 1);
	memcpy(((struct chiave_s *)(*entry))->key, buffer, size);
	((struct chiave_s *)(*entry))->key[size] = '\0';
	afc_string_reset_len(((struct chiave_s *)(*entry))->key);
	return (AFC_ERR_NO_ERROR);
}

int read_node(void **entry, FILE *file, unsigned short size)
{
	TRY(int)
	if (fread(entry, sizeof(void *), 1, file) != 1)
		; //	RAISE_RC ( AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE );
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}

int read_node_s(void **entry, FILE *file, unsigned short size)
{
	TRY(int)
	if (fread(&((((struct chiave_s *)*entry))->id), sizeof(unsigned long), 1, file) != 1)
		; //	RAISE_RC ( AFC_LOG_ERROR, AFC_BTREE_ERR_WRITE_FILE, "Could not write file", 0, AFC_BTREE_ERR_WRITE_FILE );
	RETURN(AFC_ERR_NO_ERROR);
	EXCEPT

	FINALLY

	ENDTRY
}
BOOL lt(void *key, void *entry)
{
	return ((key < entry) ? 1 : 0);
}

BOOL eq(void *key, void *entry)
{
	return ((key == entry) ? 1 : 0);
}

BOOL lt_s(void *key, void *entry)
{
	if (strcmp(((struct chiave_s *)key)->key, ((struct chiave_s *)entry)->key) < 0)
		return (TRUE);
	return (FALSE);
}

BOOL eq_s(void *key, void *entry)
{
	if ((strcmp(((struct chiave_s *)key)->key, ((struct chiave_s *)entry)->key)) == 0)
		return (TRUE);
	return (FALSE);
}

int clear(void *p)
{
	afc_string_delete(((struct chiave_s *)p)->key);
	afc_free(p);
	return (AFC_ERR_NO_ERROR);
}

struct chiave_s *crea_chiave_struct(char *s, unsigned long id)
{
	struct chiave_s *str;
	str = afc_malloc(sizeof(struct chiave_s));
	str->key = afc_string_dup(s);
	str->id = id;
	return (str);
}

int main()
{
	AFC *afc = afc_new();
	BTree *btr;
	int t, pos;
	char letters[20] = {'a', 'g', 'f', 'b', 'k', 'd', 'h', 'm', 'j', 'e', 's', 'i', 'r', 'x', 'c', 'l', 'n', 't', 'u', 'p'};
	char expected_tree[20] = {'j', 'c', 'f', 'm', 'r', 'a', 'b', 'd', 'e', 'g', 'h', 'i', 'k', 'l', 'n', 'p', 's', 't', 'u', 'x'};
	btr = afc_btree_new();
	afc_btree_init(btr, 5, lt, eq, create_key, write_node, read_key, read_node);
	// afc_btree_set_clear_func ( btr, clear );
	// expected_tree=afc_malloc ( sizeof ( void * ) * 20 );
	printf("\n Test creazione albero\n");
	test_header();
	for (t = 0; t < 20; t++)
	{
		print_res("afc_btree_add()", AFC_ERR_NO_ERROR, afc_btree_add(btr, (void *)letters[t]), 0);
	}

	print_res("check_tree()", 1, check_tree(btr, (void *)expected_tree), 0);

	for (t = 0; t < 20; t++)
	{
		print_res("afc_btree_find()", (void *)(letters[t]), afc_btree_find(btr, (void *)letters[t]), 0);
	}

	print_row();
	for (t = 0; t < 20; t++)
	{
		afc_btree_add(btr, (void *)letters[t]);
	}

	afc_btree_del(btr, (void *)'j');
	memcpy(&expected_tree, "cfkrabdeghilmnpstux", 20);

	print_res("check_tree()", 1, check_tree(btr, (void *)expected_tree), 0);
	for (t = 0; t < 20; t++)
	{
		print_res("afc_btree_find()", (void *)((letters[t] != 'j') * (letters[t])), afc_btree_find(btr, (void *)letters[t]), 0);
	}

	afc_btree_del(btr, (void *)'m');
	memcpy(&expected_tree, "cfkrabdeghilnpstux", 20);

	print_res("check_tree()", 1, check_tree(btr, (void *)expected_tree), 0);
	print_summary();

	afc_btree_delete(btr);
	afc_delete(afc);

	return (0);
}
