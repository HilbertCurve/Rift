/* date = September 27th 2021 3:04 pm */

#ifndef STR_H
#define STR_H

#include <string.h>
#include "mem.h"

typedef struct string_const {
    u8* str;
    u64 size;
} string_const;
typedef string_const string;

typedef struct string_const_list_node {
    u8* str;
    u64 size;
    
    struct string_const_list_node* next;
} string_const_list_node;
typedef string_const_list_node string_list_node;

typedef struct string_const_list {
    string_const_list_node* first;
    string_const_list_node* last;
    i32 node_count;
    u64 total_size;
} string_const_list;
typedef string_const_list string_list;

//-

#define str_lit(s) (string_const) { .str = (u8*)(s), .size = sizeof(s) - 1 }
#define str_expand(s) (i32)(s).size, (s).str
#define str_node_expand(s) (i32)(s)->size, (s)->str

string_const str_alloc(M_Arena* arena, u64 size); // NOTE(EVERYONE): this will try to get one extra byte for \0
string_const str_copy(M_Arena* arena, string_const other);
string_const str_cat(M_Arena* arena, string_const a, string_const b);
string_const str_from_format(M_Arena* arena, const char* format, ...);
string_const str_replace_all(M_Arena* arena, string_const to_fix, string_const needle, string_const replacement);
u64 str_substr_count(string_const str, string_const needle);
u64 str_find_first(string_const str, string_const needle, u32 offset);
u64 str_find_last(string_const str, string_const needle, u32 offset);

b8 str_eq(string_const a, string_const b); // NOTE(voxel): Absolute comparison (string, string)
b8 str_str_node_eq(string_const a, string_const_list_node* b); // NOTE(voxel): Absolute comparison (string, node)
b8 str_node_eq(string_const_list_node* a, string_const_list_node* b); // NOTE(voxel): Absolute comparison (node, node)

void string_list_push_node(string_const_list* list, string_const_list_node* node);
void string_list_push(M_Arena* arena, string_const_list* list, string_const str);
b8   string_list_equals(string_const_list* a, string_const_list* b);
b8   string_list_contains(string_const_list* a, string_const needle);
string_const string_list_flatten(M_Arena* arena, string_const_list* list);

#endif //STR_H
