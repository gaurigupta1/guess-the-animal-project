#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab5.h"

extern Node *g_root;

/* Implement check_integrity
 * Use BFS to verify tree structure:
 * - Question nodes must have both yes and no children (not NULL)
 * - Leaf nodes (isQuestion == 0) must have NULL children
 * 
 * Return 1 if valid, 0 if invalid
 * 
 * Steps:
 * 1. Return 1 if g_root is NULL (empty tree is valid)
 * 2. Initialize queue and enqueue root with id=0
 * 3. Set valid = 1
 * 4. While queue not empty:
 *    - Dequeue node
 *    - If node->isQuestion:
 *      - Check if yes == NULL or no == NULL
 *      - If so, set valid = 0 and break
 *      - Otherwise, enqueue both children
 *    - Else (leaf node):
 *      - Check if yes != NULL or no != NULL
 *      - If so, set valid = 0 and break
 * 5. Free queue and return valid
 */
int check_integrity() {
	//1. Return 1 if g_root is NULL (empty tree is valid)
	if(g_root == NULL)
		return 1;

	//2. Initialize queue and enqueue root with id=0
	Queue* q = (Queue*)malloc(sizeof(Queue));
	q_init(q);
	q_enqueue(q, g_root, 0);
	int initId = 0;

	//3. Set valid = 1
	int valid = 1;

	//4. While queue not empty:
	while(q_empty(q) != 1){
 		// - Dequeue node
		int qId = 0;
		Node* qNode = NULL;
		q_dequeue(q, &qNode, &qId);

		// - If node->isQuestion:
		if(qNode->isQuestion){
		//   - Check if yes == NULL or no == NULL
			if(qNode->yes == NULL || qNode->no == NULL){
				//   - If so, set valid = 0 and break
				valid = 0;
				break;
			}
			else {
				//   - Otherwise, enqueue both children
				initId++;
				q_enqueue(q, qNode->yes, initId);
				initId++;
				q_enqueue(q, qNode->no, initId);
			}
		} else {
		// - Else (leaf node):
			//   - Check if yes != NULL or no != NULL
			if(qNode->yes != NULL || qNode->no != NULL) {
				//   - If so, set valid = 0 and break
				valid = 0;
				break;
			}
		}

	}
	q_free(q);
	free(q);
	return valid;
}


