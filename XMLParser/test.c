#include "parser.h"

int main()
{
    XMLNode_ptr root = parse_from_file("C:\\Users\\just do it\\Desktop\\test3.html");
    Array* res = xpath("//head", root);
    printf("%d\n", res->size);
    for (int i = 0; i < res->size; i++)
        printf("%s\n", Get(res, i));
 
    xml_free(root);
    return 0;
}