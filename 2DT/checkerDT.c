/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"



/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;
   Node_T oNChild;
   Node_T oNChild1 = NULL;
   size_t ulIndex;
   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   /*Test if nodes in lexicographical order / for duplicate nodes */
   for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++) {
      Node_T oNChild = NULL;
      int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
      if(iStatus != SUCCESS) {
         fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
         return FALSE;
      }
      if(oNChild1 != NULL) { /*oNChild1 represents the previous child*/
         int compareResult = Node_compare(oNChild1, oNChild);
         if(compareResult > 0) {
            fprintf(stderr, "Unsorted node DynArray");
            return FALSE;
         } else if(compareResult == 0) {
            fprintf(stderr, "Duplicate child");
            return FALSE;
         }
      }
      oNChild1 = oNChild;
   }
   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode, size_t *ptrueCount) {
   size_t ulIndex;
   if(oNNode!= NULL) {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;
      *ptrueCount += 1;
      if(*ptrueCount > 1) {  /*We are not analyzing the root node*/
         if(Node_getParent(oNNode) == NULL) {
            return FALSE; /*Non-root nodes must have parents*/
         }
      }
      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }
         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, ptrueCount))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {
   size_t trueCount = 0;
   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }
   /*If our root is null, then there MUST BE 0 nodes*/
   if(oNRoot == NULL) {
      if(ulCount != 0) {
         fprintf(stderr, "Null root, but count is not 0\n");
         return FALSE;
      }
   }
   /*Root nodes cannot have parents*/
   if(oNRoot != NULL) {
      if(Node_getParent(oNRoot) != NULL) {
         fprintf(stderr, "Root node cannot have parent");
         return FALSE;
      }
   }
   /* Now checks invariants recursively at each node from the root. 
      Number of nodes should be equivalent to the true count.*/
   if(CheckerDT_treeCheck(oNRoot, &trueCount)) {
      if(trueCount == ulCount) {
         return TRUE;
      } else {
         fprintf(stderr, "Wrong number of nodes");
      }
   } else {
      return FALSE;
   }
}
