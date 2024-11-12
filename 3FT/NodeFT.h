/*--------------------------------------------------------------------*/
/* NodeFT.h                                                           */
/* Author: Matthew Okechukwu, Pinrui Huang                            */
/*--------------------------------------------------------------------*/

#ifndef NodeFT_INCLUDED
#define NodeFT_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"


/* A Node_T is a NodeFT in a Directory Tree */
typedef struct NodeFT *Node_T;

/*
  Creates a new NodeFT in the Directory Tree, with path oPPath and
  parent oNParent. Returns an int SUCCESS status and sets *poNResult
  to be the new NodeFT if successful. Otherwise, sets *poNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oNParent already has a child with this path
*/
int NodeFT_new(Path_T oPPath, Node_T oNParent, boolean isFile,
 void* pvFile, size_t fileSize, Node_T *poNResult);
/*
  Destroys and frees all memory allocated for the subtree rooted at
  oNNodeFT, i.e., deletes this NodeFT and all its descendents. Returns the
  number of NodeFTs deleted.
*/
size_t NodeFT_free(Node_T oNNodeFT);

/* Returns the path object representing oNNodeFT's absolute path. */
Path_T NodeFT_getPath(Node_T oNNodeFT);

/*
  Returns TRUE if oNParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oNParent has such a child, stores in *pulChildID the child's
  identifier (as used in NodeFT_getChild). If oNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean NodeFT_hasFileChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID);

/*
  Returns TRUE if oNParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oNParent has such a child, stores in *pulChildID the child's
  identifier (as used in NodeFT_getChild). If oNParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/

boolean NodeFT_hasDirectoryChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID);

/* Returns the number of children that oNParent has. */
size_t NodeFT_getNumChildren(Node_T oNParent);

/* Returns the number of children in the directory dynarray */
size_t NodeFT_getNumDirectoryChildren(Node_T oNParent);

/* Returns the number of children in the file dynArray */
size_t NodeFT_getNumFileChildren(Node_T oNParent);
/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  NodeFT of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/
int  NodeFT_getFileChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult);

/*
  Returns an int SUCCESS status and sets *poNResult to be the child
  NodeFT of oNParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oNParent
*/

int  NodeFT_getDirectoryChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult);         

/*
  Returns a the parent NodeFT of oNNodeFT.
  Returns NULL if oNNodeFT is the root and thus has no parent.
*/
Node_T NodeFT_getParent(Node_T oNNodeFT);

/*
  Compares oNFirst and oNSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if onFirst is "less than", "equal to", or
  "greater than" oNSecond, respectively.
*/
int NodeFT_compare(Node_T oNFirst, Node_T oNSecond);

/*
  Returns a string representation for oNNodeFT, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *Node_ToString(Node_T oNNodeFT);

/*
  Returns TRUE or FALSE for if NodeFT oNNodeFT contains a file (TRUE)
  or a directory (FALSE).
*/
boolean NodeFT_isFile(Node_T oNNodeFT);

/*
  Returns the contents fo the file
*/
void* NodeFT_getFileContents(Node_T oNNodeFT);

/*
  Returns the length of the node files
*/
size_t NodeFT_getFileLength(Node_T oNNodeFT);

/*
--------
*/
int NodeFT_setFile(void* pvContents, size_t ulLength);



#endif
