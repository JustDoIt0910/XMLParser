# XMLParser

## c语言 XML 解析器，支持基本的 xpath 语法

```c
#include "parser.h"

int main()
{
    XMLNode_ptr root = parse_from_file("C:\\Users\\just do it\\Desktop\\test3.html");
    Array* res = xpath("//head", root);

    if (res == NULL)
    {
        printf("Not found\n");
        return 0;
    }
       
    printf("%d\n", res->size);
    for (int i = 0; i < res->size; i++)
        printf("%s\n", Get(res, i));
 
    xml_free(root);
    return 0;
}
``` 


`parse_from_file(const char* filename)` 用以解析xml文件 

`parse_from_string(const char* xml)` 用以解析xml字符串 

二者返回文档树的根节点(在xml根节点上加了一个空父节点)

### 如果解析xml文件过程中发现语法错误，root为NULL


### xpath的查找结果以`Array*`形式返回，如果未查找到返回 NULL。

Array 为动态(字符串)数组，支持已下操作:

- `NewArray()` 创建一个空数组

- `Append(Array* arr, char* str)`  向末尾添加一个新元素

- `Extend(Array* arr1, Array* arr2)` 将 arr2 扩充进 arr1 中

- `Get(Array* arr, int index)` 取下标 index 的元素, 下标越界返回NULL

- `FreeArray(Array* arr)` 释放arr

  

### 支持的 xpath 语法：

- /name 选择当前元素子元素中的name元素
- //name 选择当前元素后代元素中的name元素
- /. 选择当前元素
- /.. 选择父元素
- /name[@attr=value] 属性筛选，选择attr属性的值为value的name元素(属性值不加分号)
- /name[@attr] 属性筛选，选择有attr属性的元素
- /name[n] 选择当前元素下第n个name元素
- /text() 返回当前元素中的文本
- /@attr 返回当前元素attr属性的值
- //text() 返回当前元素以及它所有后代元素中的文本
- //@attr 返回当前元素以及它所有后代元素中attr属性的值
- `*` 匹配任意元素
- @* 匹配任意属性

### examples:
### 1.
```c
#include "parser.h"

int main()
{
    XMLNode_ptr root = parse_from_string("\
<bookstore>\n\
    <book category=\"CHILDREN\">\n\
        <title>Harry Potter</title>\n\
        <author>J K.Rowling</author>\n\
        <year>2005 < / year >\n\
        <price>29.99 </price>\n\
    </book>\n\
    <book category=\"WEB\">\n\
        <title>Learning XML</title>\n\
        <author>Erik T.Ray</author>\n\
        <year>2003 </year>\n\
        <price>39.95 </price>\n\
    </book>\n\
 </bookstore>");

    Array* res = xpath("//book", root);
    if (res == NULL)
    {
        printf("Not found\n");
        return 0;
    }
    for (int i = 0; i < res->size; i++)
        printf("%s\n", Get(res, i));
 
    xml_free(root);
    return 0;
}
```

结果：
```xml
<book category="CHILDREN">
        <title>Harry Potter</title>
        <author>J K.Rowling</author>
        <year>2005 < / year >
        <price>29.99 </price>
    </book>
<book category="WEB">
        <title>Learning XML</title>
        <author>Erik T.Ray</author>
        <year>2003 </year>
        <price>39.95 </price>
    </book>
```

### 2.
```c
Array* res = xpath("//book[2]", root);
```

结果：
```xml
<book category="WEB">
        <title>Learning XML</title>
        <author>Erik T.Ray</author>
        <year>2003 </year>
        <price>39.95 </price>
    </book>
```

### 3.
```c
Array* res = xpath("//book[@category=WEB]/author/text()", root);
```

结果：
```xml
Erik T.Ray
```
