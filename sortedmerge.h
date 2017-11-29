#ifndef SORTEDMERGE_H
#define SORTEDMERGE_H

#include "mergesort.h"

//global structure for sorted lists
typedef struct _gNode
{
  node* sortedList;
  struct * _gNode;

}gNode;

//merges sorted lists
node* sortedMerge( node*,  node*, int, int (*)(void*,void*) );


#endif
