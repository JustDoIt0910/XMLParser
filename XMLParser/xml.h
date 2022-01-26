#ifndef __XML_H__
#define __XML_H__
#include "array.h"
#include "xml_structs.h"


#define XML_EMPTY_CONTENT  -1
#define XML_SYNTAX_ERROR   -2
#define XML_PARSE_SUCCESS  0

#define XML_SYN_HTML      0
#define XML_SYN_STRICT    1

XMLNode_ptr new_node();
attr_ptr_t new_attr();
void free_attrs(attr_ptr_t* attrs);
void free_texts(text_ptr_t* texts);
void xml_free(XMLNode_ptr node);

const char* skip(const char* pointer);
int blank(char c);
int is_open_label(char* name);
XMLNode_ptr parse_from_file(const char* filename);
XMLNode_ptr parse_from_string(const char* xml);
const char* parse_node(const char* xml, XMLNode_ptr root);
const char* parse_element_name(const char* pointer, char** elem_name);
const char* parse_one_attr(const char* pointer, attr_ptr_t attr);
const char* parse_element_attr(const char* pointer, attr_ptr_t first);
const char* parse_element_text(const char* pointer, text_ptr_t* text);

Array* get_element_attr(XMLNode_ptr elem, const char* attr_name);
Array* get_element_text(XMLNode_ptr elem);

#endif
