/*--------------------------------------------------------------------*/
/* NodeFT.c                                                           */
/* Author: Christopher Moretti                                        */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "NodeFT.h"

/* A NodeFT in a FT */
struct NodeFT {
   /* the object corresponding to the NodeFT's absolute path */
   Path_T oPPath;
   /* this NodeFT's parent */
   Node_T oNParent;
   /* the object containing links to this NodeFT's file children */
   DynArray_T oDFiles;
   /* the object containing links to this NodeFT's directory children*/
   DynArray_T oDDirectories;
   /* the boolean representing if the NodeFT is a file */
   boolean isFile;
   /* pointer to the object itself of the file */
   void* pvFile;
   /*Size of file*/
   size_t fileSize;
};


/*
  Links new child oNChild into oNParent's children array at index
  ulIndex. Returns SUCCESS if the new child was added successfully,
  or MEMORY_ERROR if allocation fails adding oNChild to the array.
*/
static int NodeFT_addChild(Node_T oNParent, Node_T oNChild,
                         size_t ulIndex) {
   assert(oNParent != NULL);
   assert(oNChild != NULL);
   
   /* If we get a directionry child */
   if(!oNChild->isFile) {
      if(DynArray_addAt(oNParent->oDDirectories, ulIndex, oNChild))
         return SUCCESS; /* insert into Directory*/
      else 
         return MEMORY_ERROR;
   }
   else { 
      if(DynArray_addAt(oNParent->oDFiles, ulIndex, oNChild))
         return SUCCESS; /* insert into Directory*/
      else 
         return MEMORY_ERROR;
   }
}

/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a NodeFT's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
   static int NodeFT_compareString(const Node_T oNFirst,
                                 const char *pcSecond) {
   assert(oNFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oNFirst->oPPath, pcSecond); 
} 


/*
  Creates a new NodeFT with path oPPath and parent oNParent. Returns an
  int SUCCESS status and sets *poNResult to be the new NodeFT if
  successful. Otherwise, sets *poNResult to NULL and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oNParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oNParent's path is not oPPath's direct parent
                 or oNParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oNParent already has a child with this path
*/
int NodeFT_new(Path_T oPPath, Node_T oNParent, boolean isFile,
 void* pvFile, size_t fileSize, Node_T *poNResult) {
   struct NodeFT *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);

   /* allocate space for a new NodeFT */
   psNew = malloc(sizeof(struct NodeFT));
   if(psNew == NULL) {
      *poNResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new NodeFT's path */
   iStatus = Path_dup(oPPath, &oPNewPath);
   if(iStatus != SUCCESS) {
      free(psNew);
      *poNResult = NULL;
      return iStatus;
   }
   psNew->oPPath = oPNewPath;

   /* validate and set the new NodeFT's parent */
   if(oNParent != NULL) {
      size_t ulSharedDepth;

      oPParentPath = oNParent->oPPath;
      ulParentDepth = Path_getDepth(oPParentPath);
      ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);
      /* parent must be an ancestor of child */
      if(ulSharedDepth < ulParentDepth) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }

      /* parent must not already have child with this path */
      if(NodeFT_hasDirectoryChild(oNParent, oPPath, &ulIndex) || NodeFT_hasFileChild(oNParent, oPPath, &ulIndex)) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return ALREADY_IN_TREE;
      }
   }
   else {
      /* new NodeFT must be root */
      /* can only create one "level" at a time */
      if(Path_getDepth(psNew->oPPath) != 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return NO_SUCH_PATH;
      }
   }
   psNew->oNParent = oNParent;

   /* initialize the new NodeFT */
   psNew->isFile = isFile;
   if(isFile) {
      psNew->pvFile = pvFile;
      psNew->oDFiles = NULL;
      psNew->oDDirectories = NULL;
   } else {
      psNew->oDFiles = DynArray_new(0);
      if(psNew->oDFiles == NULL) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return MEMORY_ERROR;
      }
      psNew->oDDirectories = DynArray_new(0);
      if(psNew->oDDirectories == NULL) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return MEMORY_ERROR;
      }
   }
   /* Link into parent's children list */
   if(oNParent != NULL) {
      if(isFile) {
         (void) (NodeFT_hasFileChild(oNParent, psNew->oPPath, &ulIndex));
      }
       else {
         (void) (NodeFT_hasDirectoryChild(oNParent, psNew->oPPath, &ulIndex));
      }
      iStatus = NodeFT_addChild(oNParent, psNew, ulIndex);
      if(iStatus != SUCCESS) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poNResult = NULL;
         return iStatus;
      }
   }

   *poNResult = psNew;

   return SUCCESS;
}

size_t NodeFT_free(Node_T oNNodeFT) {
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oNNodeFT != NULL);
   /* remove from parent's list */
   if(oNNodeFT->oNParent != NULL && oNNodeFT->isFile) {
      if(DynArray_bsearch(
            oNNodeFT->oNParent->oDFiles,
            oNNodeFT, &ulIndex,
            (int (*)(const void *, const void *)) Path_comparePath)
        )
         (void) DynArray_removeAt(oNNodeFT->oNParent->oDFiles,
                                  ulIndex);
   


   /* remove path */
   Path_free(oNNodeFT->oPPath);
   
   /* finally, free the struct NodeFT */
   free(oNNodeFT);
   ulCount++;
   return ulCount;

   }

   else if (oNNodeFT->oNParent != NULL && !(oNNodeFT->isFile)){ 
   if(DynArray_bsearch(
               oNNodeFT->oNParent->oDDirectories,
               oNNodeFT, &ulIndex,
               (int (*)(const void *, const void *)) Path_comparePath)
         )
            (void) DynArray_removeAt(oNNodeFT->oNParent->oDDirectories,
                                    ulIndex);
   }
      

      /* recursively remove children */
      while(DynArray_getLength(oNNodeFT->oDDirectories) != 0) {
         ulCount += NodeFT_free(DynArray_get(oNNodeFT->oDDirectories, 0));
      }
      DynArray_free(oNNodeFT->oDDirectories);

  /* remove path */
   Path_free(oNNodeFT->oPPath);
   
   /* finally, free the struct NodeFT */
   free(oNNodeFT);
   ulCount++;
   return ulCount;

   
}

 Path_T NodeFT_getPath(Node_T oNNodeFT) {
   assert(oNNodeFT != NULL);

   return oNNodeFT->oPPath; 
} 

boolean NodeFT_hasFileChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oNParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);
  /* assert(oNParent->isFile == FALSE); */
   /* *pulChildID is the index into oNParent->oDChildren */
   return DynArray_bsearch(oNParent->oDFiles,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) NodeFT_compareString);
}

boolean NodeFT_hasDirectoryChild(Node_T oNParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oNParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);
   /* assert(oNParent->isFile == FALSE); */
   /* *pulChildID is the index into oNParent->oDChildren */
   return DynArray_bsearch(oNParent->oDDirectories,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) NodeFT_compareString);
}
 size_t NodeFT_getNumChildren(Node_T oNParent) {
   assert(oNParent != NULL);

   return DynArray_getLength(oNParent->oDFiles) + DynArray_getLength(oNParent->oDDirectories); 
}

size_t NodeFT_getNumDirectoryChildren(Node_T oNParent) {
   return DynArray_getLength(oNParent->oDDirectories);
}

size_t NodeFT_getNumFileChildren(Node_T oNParent) {
   return DynArray_getLength(oNParent->oDFiles);
}

int  NodeFT_getFileChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult) {

   assert(oNParent != NULL);
   assert(poNResult != NULL);

   /* ulChildID is the index into oNParent->oDChildren */
   if(oNParent->isFile) {
      return NO_SUCH_PATH; /*Files cannot have children*/
   }              
   else {
      *poNResult = DynArray_get(oNParent->oDFiles, ulChildID);
      return SUCCESS;
   }
}
int  NodeFT_getDirectoryChild(Node_T oNParent, size_t ulChildID,
                   Node_T *poNResult) {

   assert(oNParent != NULL);
   assert(poNResult != NULL);

   /* ulChildID is the index into oNParent->oDChildren */
   if(oNParent->isFile) {
      return NO_SUCH_PATH; /*Files cannot have children*/
   }              
   else {
      *poNResult = DynArray_get(oNParent->oDDirectories, ulChildID);
      return SUCCESS;
   }
}

Node_T NodeFT_getParent(Node_T oNNodeFT) {
   assert(oNNodeFT != NULL);

   return oNNodeFT->oNParent; 
} 

int NodeFT_compare(Node_T oNFirst, Node_T oNSecond) {
   assert(oNFirst != NULL);
   assert(oNSecond != NULL);

   return Path_comparePath(oNFirst->oPPath, oNSecond->oPPath);
} 

char *NodeFT_ToString(Node_T oNNodeFT) {
   char *copyPath;

   assert(oNNodeFT != NULL);

   copyPath = malloc(Path_getStrLength((oNNodeFT->oPPath))+1); /* plus one indicates nullbyte */
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(oNNodeFT->oPPath));
}

boolean NodeFT_isFile(Node_T oNNodeFT) {
   assert(oNNodeFT != NULL);
   return oNNodeFT->isFile;
}

void* NodeFT_getFileContents(Node_T oNNodeFT) {

}

size_t NodeFT_getFileLength(Node_T oNNodeFT) {

}

int NodeFT_setFile(void* pvContents, size_t ulLength) {
   
}

