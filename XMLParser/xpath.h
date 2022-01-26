#ifndef __XPATH_H__
#define __XPATH_H__
#include "xml_structs.h"
#include "array.h"

#define XPATH_SYNTAX_ERROR -3

#define XPATH_TYPE_TEXT 0
#define XPATH_TYPE_ATTR 1
#define XPATH_TYPE_ELEM 2

typedef int (*validator)(XMLNode_ptr elem, char* arg);

typedef struct constraint
{
	validator valid_func;
	char* args;
}constraint;

Result new_result();

Array* xpath(const char* exp, XMLNode_ptr root);
Array* xpath_end(char* exp, Result res, int type, int isDirect);
Result xpath_find(const char* pointer, XMLNode_ptr root, int isDirect);
Result find(XMLNode_ptr root, int isDirect, constraint* constraints[], int constraint_num);
int _find(XMLNode_ptr root, int isDirect, constraint* constraints[], int constraint_num);
Result find_parent(Result res);

constraint* new_constraint(validator f, char* exp);
int elem_name_equal(XMLNode_ptr elem, char* name);
int elem_attr_equal(XMLNode_ptr elem, char* condition);
int elem_rank_equal(XMLNode_ptr elem, char* rank);

int isNumber(char* str);

#endif
