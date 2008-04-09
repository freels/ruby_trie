#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */
#include "ruby.h"

// typdefs!
typedef enum { false = 0, true} bool;

typedef struct node {
	char character;
	VALUE value;
	struct node * first_child;
	struct node * next_sibling;
} trie_node;

static VALUE rb_cTrie;

// =========================
// = function declarations =
// =========================

//trie implementation
static trie_node * trie_node_for_key(trie_node * root, char * key, bool create_missing_nodes);
static trie_node * trie_sibling_for_char(trie_node * node, char ch);
static trie_node * trie_add_sibling_for_char(trie_node * node, char ch);
static trie_node * trie_new_node_with_char(char ch);
static trie_node * trie_new_node();
static void trie_traverse(trie_node * trie, void (*lambda_func)(void *));
static void free_trie(trie_node * trie);


// ========================
// = function definitions =
// ========================

// instance methods
static VALUE rb_trie_get_key(VALUE self, VALUE key) {
	trie_node * root;
	trie_node * node;
	char * key_cstring;
	
	//Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);

	Data_Get_Struct(self, trie_node, root);
	
	node = trie_node_for_key(root, key_cstring, false);
	if (node == NULL) return Qnil;
	return node->value;
}

static VALUE rb_trie_set_key_to_value(VALUE self, VALUE key, VALUE value) {
	trie_node * root;
	trie_node * node;
	char * key_cstring;
	
	//Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);

	Data_Get_Struct(self, trie_node, root);

	node = trie_node_for_key(root, key_cstring, true);	
	node->value = value;
	
	return Qnil;
}

static VALUE rb_trie_undef_key(VALUE self, VALUE key) {
	trie_node * root, * node, * prev, * next;
	VALUE return_value;	
	char * key_cstring;
	int steps;
	int i;
	
	//Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);
	
	Data_Get_Struct(self, trie_node, root);
	next = root;
	node = NULL;
	prev = NULL;

	steps = strlen(key_cstring);
	
	for (i = 0; i < steps; i++) {
		if (next == NULL) return Qnil;
		
		while(next->character != key_cstring[i]) {
			if (next == NULL) return Qnil;
			next = next->next_sibling;
		}
		prev = node;
		node = next;
		next = node->first_child;
	}
	
	return_value = node->value;
	node->value = Qnil;
	
	if (node->first_child == NULL) { //node has no children. we can delete it.
		if (prev == NULL)  {
			//printf("should delete root");
		} else if (prev->first_child == node) {
			prev->first_child = node->next_sibling;
			free(node);
		} else if (prev->next_sibling == node) {
			prev->next_sibling = node->next_sibling;
			free(node);
		}
	}
	
	return return_value;
}

// garbage collection and allocation
static void trie_mark_value(void * t) {	
	rb_gc_mark( ((trie_node *)t)->value );
}

static void rb_trie_mark(trie_node * t) {
	trie_traverse(t, trie_mark_value);
}

static void rb_trie_free(trie_node * t) {
	free_trie(t);
}

static VALUE rb_trie_allocate (VALUE klass) {
	trie_node * t = trie_new_node();

	return Data_Wrap_Struct(klass, rb_trie_mark, rb_trie_free, t);
}

// extension init
void Init_trie() {
	rb_cTrie = rb_define_class("Trie", rb_cObject);

	rb_define_alloc_func (rb_cTrie, rb_trie_allocate);

	int arg_count = 0;
	//rb_define_method(rb_cTrie, "inspect", rb_trie_inspect, arg_count);
	
	arg_count = 1;
	rb_define_method(rb_cTrie, "[]", rb_trie_get_key, arg_count);
	rb_define_method(rb_cTrie, "delete", rb_trie_undef_key, arg_count);

	arg_count = 2;
	rb_define_method(rb_cTrie, "[]=", rb_trie_set_key_to_value, arg_count);
}


// =======================
// = trie implementation =
// =======================

static trie_node * trie_node_for_key(trie_node * root, char * key, bool create_missing_nodes) {		
	int steps, i;
	trie_node * next, * node;

	steps = strlen(key);
	next = root;

	for (i = 0; i < steps; i++) {
		if (next == NULL) {
			if (create_missing_nodes) {
				node->first_child = trie_new_node();
				next = node->first_child;
			}
			else return NULL;
		}

		node = trie_sibling_for_char(next, key[i]);

		if (node == NULL) {
			if (create_missing_nodes) {
				node = trie_add_sibling_for_char(next, key[i]);
			}	
			else return NULL;
		}

		next = node->first_child;
	}	
		
	return node;    
}

static trie_node * trie_sibling_for_char(trie_node * node, char ch) {
	while(true) {
		if (node == NULL) return NULL;

		if (node->character == ch) return node;

		node = node->next_sibling;
	}
	return node;
}

static trie_node * trie_add_sibling_for_char(trie_node * node, char ch) {
	trie_node * current_next;

	current_next = node->next_sibling;
	node->next_sibling = trie_new_node_with_char(ch);
	node->next_sibling->next_sibling = current_next;

	return node->next_sibling;
}

static trie_node * trie_new_node_with_char(char ch) {
	trie_node * trie;
	trie = malloc(sizeof(trie_node));
	trie->character = ch;
	trie->value = Qnil;
	trie->first_child = NULL;
	trie->next_sibling = NULL;
	return trie;
}

static trie_node * trie_new_node() {
	return trie_new_node_with_char('s'); //insert most common starting letter here.
}

static void trie_traverse(trie_node * trie, void (* lambda_func)(void *)) {
	if (trie->next_sibling != NULL) {
		trie_traverse(trie->next_sibling, lambda_func);
	}

	if (trie->first_child != NULL) {
		trie_traverse(trie->first_child, lambda_func);
	}

	lambda_func(trie);
}

static void free_trie(trie_node * trie) {
	trie_traverse(trie, free);
}