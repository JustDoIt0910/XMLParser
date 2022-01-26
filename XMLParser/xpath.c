#include "xpath.h"
#include "xml.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static int Rank = 0;
Result head = NULL;
Result tail = NULL;

Result new_result()
{
	Result res = (Result)malloc(sizeof(XPATHResult));
	res->visited = 0;
	res->element = res->next = NULL;
	return res;
}

Result find(XMLNode_ptr root, int isDirect, constraint* constraints[], int constraint_num)
{
	_find(root, isDirect, constraints, constraint_num);
	Result res = head;
	head = tail = NULL;
	return res;
}

int _find(XMLNode_ptr root, int isDirect, constraint* constraints[], int constraint_num)
{
	int len = 0;
	if (root->backRef != NULL)
		root->backRef->visited = 1;

	root = root->children;
	while (root != NULL)
	{
		int ok = 1;
		for (int i = 0; i < constraint_num; i++)
		{
			constraint* cons = constraints[i];
			if (!cons->valid_func(root, cons->args))
			{
				ok = 0;
				break;
			}
		}
		if (ok)
		{
			if (head == NULL)
			{
				head = new_result();
				head->element = root;
				if (root->backRef)
					root->backRef->visited = 1;
				root->backRef = head;
				tail = head;
			}
			else
			{
				Result r = new_result();
				r->element = root;
				if (root->backRef)
					root->backRef->visited = 1;
				root->backRef = r;
				tail->next = r;
				tail = tail->next;
			}
			len += 1;
		}
		if (!isDirect && root->children != NULL)
			len += _find(root, 0, constraints, constraint_num);
		root = root->next;
	}
	return len;
}

int elem_name_equal(XMLNode_ptr elem, char* name)
{
	if (!strcmp(name, "*"))
		return 1;
	return (!strcmp(elem->name, name));
}

int elem_attr_equal(XMLNode_ptr elem, char* condition)
{
	char cond[50];
	strcpy(cond, condition);
	char* attr_name = strtok(cond, "=");
	char* attr_value = strtok(NULL, "=");
	if (attr_name == NULL || !strcmp(attr_name, "*"))
		return 1;
	attr_ptr_t attr = elem->attributes;
	while (attr != NULL) //遍历该元素的属性
	{
		if (!strcmp(attr->name, attr_name))
		{
			if (attr_value == NULL)
				return 1;
			if (!strcmp(attr->value, attr_value))
				return 1;
		}
		attr = attr->next;
	}
	return 0;
}

int elem_rank_equal(XMLNode_ptr elem, char* rank)
{
	Rank++;
	int r = atoi(rank);
	if (Rank == r)
		return 1;
	return 0;
}

constraint* new_constraint(validator f, char* exp)
{
	int len = strlen(exp);
	constraint* cons = (constraint*)malloc(sizeof(constraint));
	cons->valid_func = f;
	cons->args = (char*)calloc((len + 1), sizeof(char));
	strncpy(cons->args, exp, len);
	return cons;
}

void free_constraints(constraint* constraints[], int constraint_num)
{
	for (int i = 0; i < constraint_num; i++)
	{
		free(constraints[i]->args);
		constraints[i]->args = NULL;
		free(constraints[i]);
		constraints[i] = NULL;
	}
	Rank = 0;
}

Result xpath_find(const char* pointer, XMLNode_ptr root, int isDirect)
{
	constraint* constraints[3];
	char exp[80];
	char* p = pointer;
	int len = 0;
	while (*p != '\0' && *p != '[')
	{
		len++;
		p++;
	}
	memset(exp, '\0', 80);
	strncpy(exp, pointer, len);
	constraints[0] = new_constraint(elem_name_equal, exp);
	if (*p == '\0') //没有其他筛选条件，纯纯查找元素名
	{
		Result res = find(root, isDirect, constraints, 1);
		free_constraints(constraints, 1);
		return res;
	}
	else if (*p == '[')
	{
		len = 0;
		p += 1;
		pointer = p;
		while (*p != '\0' && *p != ']')
		{
			p++;
			len++;
		}
		memset(exp, '\0', 80);
		strncpy(exp, pointer, len);
		if (isNumber(exp)) //选择root下第n个目标元素
		{
			constraints[1] = new_constraint(elem_rank_equal, exp);
			Result res = find(root, isDirect, constraints, 2);
			free_constraints(constraints, 2);
			return res;
		}
		else  //其他的筛选条件
		{
			if (*pointer == '@') //属性筛选
			{
				memset(exp, '\0', 80);
				strncpy(exp, pointer + 1, len - 1);
				constraints[1] = new_constraint(elem_attr_equal, exp);
				Result res = find(root, isDirect, constraints, 2);
				free_constraints(constraints, 2);
				return res;
			}
		}
	}
}

Array* xpath_end(char* exp, Result res, int type, int isDirect)
{
	Array* result = NULL;
	Result descendant = NULL;
	if (type == XPATH_TYPE_ATTR) //选择属性
	{
		while (res != NULL)
		{
			if (res->visited)
			{
				res = res->next;
				continue;
			}
			if (result == NULL)
				result = NewArray();
			Extend(result, get_element_attr(res->element, exp));
			if (!isDirect)
			{
				constraint* cons[] = { new_constraint(elem_attr_equal, exp) };
				descendant = find(res->element, 0, cons, 1);
				free_constraints(cons, 1);
				Result d = descendant;
				while (d != NULL)
				{
					Extend(result, get_element_attr(d->element, exp));
					d = d->next;
				}
				d = descendant;
				while (d != NULL)
				{
					d = d->next;
					descendant->element = NULL;
					descendant->next = NULL;
					free(descendant);
					descendant = d;
				}
			}
			res = res->next;
		}
	}
	else if (type == XPATH_TYPE_TEXT) //选择文本
	{
		Array* texts = NULL;
		while (res != NULL)
		{
			if (res->visited)
			{
				res = res->next;
				continue;
			}
			if (result == NULL)
				result = NewArray();
			texts = get_element_text(res->element);
			if(Extend(result, texts))
				FreeArray(texts);
			if (!isDirect)
			{
				descendant = find(res->element, 0, NULL, 0);
				Result d = descendant;
				while (d != NULL)
				{
					texts = get_element_text(d->element);
					if(Extend(result, texts))
						FreeArray(texts);
					d = d->next;
				}
				d = descendant;
				while (d != NULL)
				{
					d = d->next;
					descendant->element = NULL;
					descendant->next = NULL;
					free(descendant);
					descendant = d;
				}
			}
			res = res->next;
		}
		texts = NULL;
	}
	else if (type == XPATH_TYPE_ELEM) //选择元素内容
	{
		while (res != NULL)
		{
			if (result == NULL)
				result = NewArray();
			char* content = (char*)calloc(res->element->content_length + 1, sizeof(char));
			strncpy(content, res->element->content, res->element->content_length);
			Append(result, content);
			free(content);
			content = NULL;
			res = res->next;
		}
	}
	return result;
}

Result find_parent(Result res)
{
	Result h = NULL;
	Result t = NULL;
	for (Result r = res; r != NULL; r = r->next)
	{
		int exist = 0;
		XMLNode_ptr parent = r->element->parent;
		for(Result p = h; p != NULL; p = p->next)
			if (p->element == parent)
			{
				exist = 1;
				break;
			}
		if (!exist)
		{
			if (h == NULL)
			{
				h = new_result();
				h->element = parent;
				parent->backRef = h;
				t = h;
			}
			else
			{
				Result nr = new_result();
				nr->element = parent;
				parent->backRef = nr;
				t->next = nr;
				t = t->next;
			}
		}
	}
	return h;
}

Array* xpath(const char* exp, XMLNode_ptr root)
{
	if (root == NULL)
	{
		printf("Error when parsing xml");
		return NULL;
	}
	Result ResultSet = new_result();
	ResultSet->element = root;
	Array* arr = NULL;
	char subexp[80];
	int end = 0;
	const char* pointer = exp;
	while (*pointer != '\0')
	{
		memset(subexp, '\0', 80);
		const char* p = pointer;
		while (*p == '/')
			p++;
		if (*p == '\0')
			break;
		int cnt = p - pointer;
		if (cnt != 1 && cnt != 2)
			return NULL;
		int isDirect = (cnt == 1) ? 1 : 0;
		pointer = p;

		Result rs = ResultSet; //前级xpath得到的结果集
		for(rs = ResultSet; rs != NULL; rs = rs->next)
			rs->visited = 0;
		rs = ResultSet;
		Result nr = NULL; //新的结果集
		Result ta = NULL;

		if (*pointer == '@') //查找属性
		{
			pointer += 1;
			p = pointer;
			while (*p != '\0' && *p != '/')
				p++;
			strncpy(subexp, pointer, p - pointer);
			arr = xpath_end(subexp, ResultSet, XPATH_TYPE_ATTR, isDirect);
			end = 1;
			break;
		}

		if (strlen(pointer) >= 6 && !strncmp(pointer, "text()", 6)) //查找文本
		{
			arr = xpath_end(NULL, ResultSet, XPATH_TYPE_TEXT, isDirect);
			end = 1;
			break;
		}

		else //查找元素
		{
			while (*p != '\0' && *p != '/')
				p++;
			strncpy(subexp, pointer, p - pointer);
			if (!strcmp(subexp, ".."))
				ResultSet = find_parent(ResultSet);
			else if (!strcmp(subexp, ".")){}
			else
			{
				while (rs != NULL)
				{
					if (rs->visited)
					{
						rs = rs->next;
						if (ResultSet->element->backRef == ResultSet)
							ResultSet->element->backRef = NULL;
						ResultSet->element = NULL;
						ResultSet->next = NULL;
						free(ResultSet);
						ResultSet = rs;
						continue;
					}
					if (nr == NULL)
					{
						nr = xpath_find(subexp, rs->element, isDirect);
						ta = nr;
					}
					else
						ta->next = xpath_find(subexp, rs->element, isDirect);
					while (ta != NULL && ta->next != NULL)
						ta = ta->next;
					rs = rs->next;
					if(ResultSet->element->backRef == ResultSet)
						ResultSet->element->backRef = NULL;
					ResultSet->element = NULL;
					ResultSet->next = NULL;
					free(ResultSet);
					ResultSet = rs;
				}
				ResultSet = nr;
				nr = NULL;
				ta = NULL;
			}
		}
		pointer = p;
	}
	if (!end) //如果没有遇到结束表达式，则返回结果集中元素的doc内容
	{
		arr = xpath_end(NULL, ResultSet, XPATH_TYPE_ELEM, 0);
	}
	Result r = ResultSet;
	while (r != NULL) //释放最终结果集的内存
	{
		r = r->next;
		ResultSet->element = NULL;
		ResultSet->next = NULL;
		free(ResultSet);
		ResultSet = r;
	}
	return arr;
}

int isNumber(char* str)
{
	while (*str != '\0')
	{
		if (!isdigit(*str))
			return 0;
		str++;
	}
	return 1;
}