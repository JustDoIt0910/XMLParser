#ifndef __XML_STRUCTS_H__
#define __XML_STRUCTS_H__


typedef struct XMLNode_attr
{
	char* name;
	char* value;
	struct XMLNode_attr* next;
	struct XMLNode_attr* prev;
}attr_t, * attr_ptr_t;

typedef struct XMLNode_text
{
	char* text;
	struct XMLNode_text* next;
}text_t, * text_ptr_t;

struct XPATHResult;

typedef struct XMLNode
{
	char* name;
	attr_ptr_t attributes;
	text_ptr_t text;
	char* content;
	int content_length;
	struct XMLNode* next; //同级的下一个节点
	struct XMLNode* prev; //同级的前一个节点
	struct XMLNode* children; //子节点
	struct XMLNode* parent; //父节点
	struct XPATHResult* backRef;
}XMLNode, * XMLNode_ptr;

typedef struct XPATHResult
{
	XMLNode_ptr element;
	struct XPATHResult* next;
	int visited;
}XPATHResult, * Result;

#endif

