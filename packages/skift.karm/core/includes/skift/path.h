#pragma once

/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <skift/runtime.h>
#include <skift/list.h>

#define PATH_SEPARATOR '/'

// Lenght of a path element including the \0
#define PATH_ELEMENT_LENGHT 128

#define PATH_MAX_DEPTH 128

typedef struct
{
    list_t* elements;    
    bool is_absolue;
} path_t;

path_t* path(const char* raw_path);

void path_delete(path_t* p);

const char* path_filename(path_t* p);

const char* path_element(path_t* p, int index);

bool path_is_absolue(path_t* p);

bool path_is_relative(path_t* p);

int path_length(path_t* p);

void path_normalize(path_t* p);

void path_push(path_t* p, const char* element);

const char* path_pop(path_t* p);

// Combine two path into a new one.
path_t* path_combine(path_t* left, path_t* right);

// Split the path a the index (inclusif)
path_t* path_split_at(path_t* path, int index);

void path_dump(path_t* p);