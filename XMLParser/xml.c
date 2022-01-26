#include "xml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int SYNTAX = XML_SYN_HTML;
int XML_PARSE_STATUS = XML_PARSE_SUCCESS;
char XML[1024 * 1024];
const char* open_labels[] = { "br", "hr", "img", "input",
							  "meta", "area", "base", "col",
							  "command", "embed", "keygen", "param",
							  "source", "track", "wbr", "link" };

int blank(char c)
{
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

const char* skip(const char* pointer)
{
	while (*pointer != '\0' && blank(*pointer))
		pointer++;
	return pointer;
}

int is_open_label(char* name)
{
	for (int i = 0; i < 16; i++)
		if (!strcmp(name, open_labels[i]))
			return 1;	
	return 0;
}

XMLNode_ptr new_node()
{
	XMLNode_ptr node = (XMLNode_ptr)malloc(sizeof(XMLNode));
	node->name = node->attributes = node->text = NULL;
	node->backRef = NULL;
	node->children = NULL;
	node->next = node->prev = node->parent = NULL;
}

attr_ptr_t new_attr()
{
	attr_ptr_t attr = (attr_ptr_t)malloc(sizeof(attr_t));
	attr->name = NULL;
	attr->value = NULL;
	attr->next = attr->prev = NULL;
	return attr;
}

text_ptr_t new_text()
{
	text_ptr_t text = (text_ptr_t)malloc(sizeof(text_t));
	text->next = text->text = NULL;
	return text;
}

void free_attrs(attr_ptr_t* attrs)
{
	attr_ptr_t head = *attrs;
	attr_ptr_t tail = head;
	while (tail != NULL)
	{
		tail = tail->next;
		if (head->name != NULL)
		{
			free(head->name);
			head->name = NULL;
		}
		if (head->value != NULL)
		{
			free(head->value);
			head->value = NULL;
		}
		free(head);
		head = tail;
	}
	*attrs = NULL;
}

void free_texts(text_ptr_t* texts)
{
	text_ptr_t head = *texts;
	text_ptr_t tail = head;
	while (tail != NULL)
	{
		tail = tail->next;
		if (head->text != NULL)
		{
			free(head->text);
			head->text = NULL;
		}
		free(head);
		head = tail;
	}
	*texts = NULL;
}

void xml_free(XMLNode_ptr node)
{
	while (node != NULL)
	{
		if (node->name != NULL)
			free(node->name);
		attr_ptr_t attr = node->attributes;
		while (attr != NULL)
		{
			if (attr->name != NULL)
				free(attr->name);
			if (attr->value != NULL)
				free(attr->value);
			attr_ptr_t nextAttr = attr->next;
			free(attr);
			attr = nextAttr;
		}
		text_ptr_t text = node->text;
		while (text != NULL)
		{
			if (text->text != NULL)
				free(text->text);
			text_ptr_t nextText = text->next;
			free(text);
			text = nextText;
		}
		if (node->children != NULL)
			xml_free(node->children);
		XMLNode_ptr next = node->next;
		free(node);
		node = next;
	}
}

XMLNode_ptr parse_from_string(const char* xml)
{
	const char* pointer = xml;
	if (!strlen(pointer))
	{
		XML_PARSE_STATUS = XML_EMPTY_CONTENT;
		return pointer;
	}
	while (*pointer != '<')
		pointer++;
	char dt[100];
	memset(dt, '\0', sizeof(dt));
	strncpy(dt, pointer + 2, 7);
	if (!strcmp(strlwr(dt), "doctype"))
	{
		while (*pointer != '>')
			pointer++;
		pointer += 1;
	}	
	XMLNode_ptr root = new_node();
	pointer = parse_node(pointer, root);
	if (XML_PARSE_STATUS != XML_PARSE_SUCCESS)
	{
		//释放所有内存
		xml_free(root);
		return NULL;
	}
	XMLNode_ptr head = new_node();
	head->children = root;
	return head;
}

XMLNode_ptr parse_from_file(const char* filename)
{
	int c = 0;
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
		return NULL;
	memset(XML, '\0', sizeof(XML));
	char ch;
	while ((ch = fgetc(fp)) != EOF)
	{
		XML[c] = ch;
		c++;
	}
	return parse_from_string(XML);
}

//解析节点名称
const char* parse_element_name(const char* pointer, char** elem_name)
{
	int len = 0;
	const char* p = pointer;
	while (*p != ' ' && *p != '>')
	{
		if (*p == '\0' || *p == '<' || *p == '\"' || *p == '=') //非法字符
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			return p;
		}
		len++;
		p++;
	}	
	*elem_name = (char*)malloc((len + 1) * sizeof(char));
	memset(*elem_name, '\0', len + 1);
	strncpy(*elem_name, pointer, len);
	return p;
}

//解析节点属性
const char* parse_element_attr(const char* pointer, attr_ptr_t first)
{
	if (first == NULL)
		return pointer;
	first->prev = NULL;
	pointer = parse_one_attr(pointer, first);
	if (first->name == NULL) //没有属性或者解析出错，直接返回
	{
		free(first);
		return pointer;
	}

	attr_ptr_t prev = first;
	while (XML_PARSE_STATUS == XML_PARSE_SUCCESS)
	{
		attr_ptr_t attr = new_attr();
		if (*pointer != ' ' && *pointer != '>' && *pointer != '/') //一个属性结束后应至少有一个空格，或">"，对于单标签可能为"/"，否则为语法错误
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			free(attr);
			attr = NULL;
			return pointer;
		}
		pointer = skip(pointer);
		if (*pointer == '/' || *pointer == '>')
		{
			free(attr);
			attr = NULL;
			break;
		}
		pointer = parse_one_attr(pointer, attr);
		if (attr->name != NULL) 
		{
			prev->next = attr;
			attr->prev = prev;
			prev = attr;
		}
		else //如果没有解析到属性名或者解析中出现错误
		{
			free(attr);
			attr = NULL;
			break;
		}
	}
	return pointer;
}

const char* parse_one_attr(const char* pointer, attr_ptr_t attr)
{
	pointer = skip(pointer);
	if (*pointer == '<' || *pointer == '"')
	{
		XML_PARSE_STATUS = XML_SYNTAX_ERROR;
		return pointer;
	}
	if (*pointer == '>') //说明没有属性
		return pointer;
	const char* p = pointer;
	int len = 0;
	while (1)
	{
		if (*p == '=' || *p == ' ') //遇到等号或空格说明属性名称结束
			break;
		else if (*p == '\"' || *p == '<' || *p == '>') //遇到不合法字符, 返回语法错误标志
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			return p;
		}
		else
		{
			len++;
			p++; //指针后移
		}
	}
	p = skip(p);
	if (*p != '=') //属性名后不是等号则为语法错误
	{
		XML_PARSE_STATUS = XML_SYNTAX_ERROR;
		return p;
	}
	char* attr_name = (char*)malloc((len + 1) * sizeof(char));
	memset(attr_name, '\0', len + 1);
	strncpy(attr_name, pointer, len);
	p += 1;
	p = skip(p);
	if (*p != '\"')
	{
		XML_PARSE_STATUS = XML_SYNTAX_ERROR;
		return p;
	}
	p += 1; //跳过第一个双引号，读属性值
	pointer = p;
	len = 0;
	while (*p != '\"')
	{
		if (*p == '<')
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			return p;
		}
		len++;
		p++;
	}
	char* attr_value = (char*)malloc((len + 1) * sizeof(char));
	memset(attr_value, '\0', len + 1);
	strncpy(attr_value, pointer, len);
	attr->name = attr_name;
	attr->value = attr_value;
	return p + 1; //*p = '>' 或 *p = ' '
}

const char* parse_element_text(const char* pointer, text_ptr_t* text)
{
	if (text == NULL || *pointer == '<')
		return pointer;
	*text = new_text();
	const char* p = pointer;
	while (1)
	{
		while (*p != '<')
			p++;
	
		const char* p2 = skip(p + 1);
		if (*p2 == '/')
			break;

		while (*p2 != '\0' && *p2 != '<' && *p2 != '>')
			p2++;
		if (*p2 == '<')
			p = p2;
		else if (*p2 == '>')
			break;
		else
		{
			free(text);
			text = NULL;
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			return p;
		}
	}
	(*text)->text = (char*)calloc((p - pointer + 1), sizeof(char));
	strncpy((*text)->text, pointer, p - pointer);
	return p;
}

const char* parse_node(const char* xml, XMLNode_ptr root)
{
	if (root == NULL)
		return xml;
	const char* pointer = xml;
	pointer = skip(pointer);
	if (*pointer != '<')
	{
		XML_PARSE_STATUS = XML_SYNTAX_ERROR;
		return pointer;
	}
	root->content = pointer;
	pointer += 1;

	char* elem_name = NULL;
	pointer = parse_element_name(pointer, &elem_name); //解析元素名称
	if (XML_PARSE_STATUS != XML_PARSE_SUCCESS)
		return pointer;	

	pointer = skip(pointer);
	if (*pointer == '/') //单标签
	{
		pointer = skip(pointer + 1);
		if(*pointer != '>') //语法错误
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			return pointer;
		}
		root->name = elem_name;
		root->content_length = pointer - root->content + 1;
		return pointer;
	}

	attr_ptr_t attrs = NULL;
	if (*pointer != '>')
	{
		attrs = new_attr();
		pointer = parse_element_attr(pointer, attrs);
		if (XML_PARSE_STATUS != XML_PARSE_SUCCESS) //如果解析属性过程中出现语法错误
		{
			free_attrs(&attrs);
			return pointer;
		}
		pointer = skip(pointer);
		if (*pointer == '/') //单标签
		{
			pointer = skip(pointer + 1);
			if (*pointer != '>') //语法错误
			{
				XML_PARSE_STATUS = XML_SYNTAX_ERROR;
				free_attrs(&attrs);
				return pointer;
			}
			root->name = elem_name;
			root->attributes = attrs;
			root->content_length = pointer - root->content + 1;
			return pointer;
		}
		if (*pointer != '>') //语法错误
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			free_attrs(&attrs);
			return pointer;
		}
	}

	if (SYNTAX == XML_SYN_HTML && is_open_label(elem_name)) //处理非严格XML语法模式下不闭合标签的情况
	{
		root->name = elem_name;
		root->attributes = attrs;
		root->content_length = pointer - root->content + 1;
		return pointer;
	}
	if (SYNTAX == XML_SYN_HTML && !strcmp(elem_name, "script")) //html的script标签内容复杂，需要特判一下
	{
		const char* th = pointer + 1;
		const char* tt = NULL;
		while (1)
		{
			while (*pointer != '/')
				pointer++;
			pointer = skip(pointer + 1);
			if (!strncmp(pointer, "script", 6))
			{
				tt = pointer;
				while (*tt != '<')
					tt--;
				tt--;
				break;
			}
		}
		text_ptr_t script = new_text();
		char* script_text = (char*)calloc((tt - th + 2), sizeof(char));
		strncpy(script_text, th, tt - th + 1);
		script->text = script_text;

		pointer += 6;
		pointer = skip(pointer);
		root->name = elem_name;
		root->attributes = attrs;
		root->content_length = pointer - root->content + 1;
		root->text = script;
		return pointer;
	}

	pointer += 1;
	text_ptr_t text_head = NULL;
	text_ptr_t text_tail = NULL;
	XMLNode_ptr tail = NULL;
	while (XML_PARSE_STATUS == XML_PARSE_SUCCESS)
	{
		pointer = skip(pointer);
		if (*pointer == '\0')
		{
			XML_PARSE_STATUS = XML_SYNTAX_ERROR;
			return pointer;
		}
		if (*pointer != '<')
		{
			if (text_head == NULL)
			{
				pointer = parse_element_text(pointer, &text_head);
				text_tail = text_head;
			}
			else
			{
				text_ptr_t text = NULL;
				pointer = parse_element_text(pointer, &text);
				text_tail->next = text;
				text_tail = text_tail->next;
			}
		}
		else if (*(skip(pointer + 1)) == '/') //若遇到结束标记
		{
			pointer = skip(pointer + 1) + 1;
			char* elem_endname = NULL;
			pointer = parse_element_name(pointer, &elem_endname);
			pointer = skip(pointer);
			if (is_open_label(elem_endname))
				pointer += 1;
			else if (!strcmp(elem_name, elem_endname)) //标签闭合
			{
				root->name = elem_name;
				root->attributes = attrs;
				root->text = text_head;
				root->content_length = pointer - root->content + 1;
				return pointer;
			}
			else
			{
				XML_PARSE_STATUS = XML_SYNTAX_ERROR;
				return pointer;
			}
		}
		else //解析子元素
		{
			if (!strncmp(pointer, "<!--", 4)) //跳过注释
			{
				pointer += 4;
				while (1)
				{
					while (*pointer != '-')
						pointer++;
					if (!strncmp(pointer, "-->", 3))
						break;
					while (*pointer == '-')
						pointer++;
				}
				pointer += 3;
				continue;
			}
			XMLNode_ptr childElem = new_node();
			pointer = parse_node(pointer, childElem);
			if (XML_PARSE_STATUS != XML_PARSE_SUCCESS)
				return pointer;
			if (root->children == NULL)
			{
				root->children = childElem;
				childElem->parent = root;
				tail = childElem;
			}
			else
			{
				tail->next = childElem;
				childElem->prev = tail;
				childElem->parent = root;
				tail = childElem;
			}
			pointer += 1;
		}
	}
}

Array* get_element_attr(XMLNode_ptr elem, const char* attr_name)
{
	attr_ptr_t attrs = elem->attributes;
	Array* arr = NewArray();
	while (attrs != NULL)
	{
		if (!strcmp(attrs->name, attr_name) || !strcmp(attr_name, "*"))
		{
			int name_len = strlen(attrs->name);
			int value_len = strlen(attrs->value);
			char* s = (char*)calloc(name_len + value_len + 4, sizeof(char));
			strcpy(s, attrs->name);
			strcpy(s + name_len, "=\"");
			strcpy(s + name_len + 2, attrs->value);
			strcpy(s + name_len + value_len + 2, "\"");
			Append(arr, s);
			free(s);
			s = NULL;
		}
		attrs = attrs->next;
	}
	return arr;
}

Array* get_element_text(XMLNode_ptr elem)
{
	text_ptr_t text = elem->text;
	Array* res = NULL;
	while (text != NULL)
	{
		if (res == NULL)
			res = NewArray();
		Append(res, text->text);
		text = text->next;
	}
	return res;
}