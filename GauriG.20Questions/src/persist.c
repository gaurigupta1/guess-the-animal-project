#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab5.h"

extern Node *g_root;

#define MAGIC 0x41544C35  /* "ATL5" */
#define VERSION 1

typedef struct {
    Node *node;
    int id;
} NodeMapping;

/* save_tree
 * Save the tree to a binary file using BFS traversal
 *
 * Binary format:
 * - Header: magic (4 bytes), version (4 bytes), nodeCount (4 bytes)
 * - For each node in BFS order:
 *   - isQuestion (1 byte)
 *   - textLen (4 bytes)
 *   - text (textLen bytes, no null terminator)
 *   - yesId (4 bytes, -1 if NULL)
 *   - noId (4 bytes, -1 if NULL)
 *
 * Steps:
 * 1. Return 0 if g_root is NULL
 * 2. Open file for writing binary ("wb")
 * 3. Initialize queue and NodeMapping array
 * 4. Use BFS to assign IDs to all nodes:
 *    - Enqueue root with id=0
 *    - Store mapping[0] = {g_root, 0}
 *    - While queue not empty:
 *      - Dequeue node and id
 *      - If node has yes child: add to mappings, enqueue with new id
 *      - If node has no child: add to mappings, enqueue with new id
 * 5. Write header (magic, version, nodeCount)
 * 6. For each node in mapping order:
 *    - Write isQuestion, textLen, text bytes
 *    - Find yes child's id in mappings (or -1)
 *    - Find no child's id in mappings (or -1)
 *    - Write yesId, noId
 * 7. Clean up and return 1 on success
 */
int save_tree(const char *filename) {
	//1. Return 0 if g_root is NULL
	if(g_root == NULL)
		return 0;

	//2. Open file for writing binary ("wb")
	FILE* fp = fopen(filename, "wb");
	if (fp == NULL){
		printf("There was a problem opening the file\n");
		return 0;
	}

	//3. Initialize queue and NodeMapping array
	Queue* bfs = (Queue*)malloc(sizeof(Queue));
	q_init(bfs);

	NodeMapping* mapping = (NodeMapping*)malloc(100*sizeof(NodeMapping));
	int size = 1;
	int capacity = 100;
	int initId = 0;

	//4. Use BFS to assign IDs to all nodes:
	// - Enqueue root with id=0
	q_enqueue(bfs, g_root, initId);

	// Store mapping[0] = {g_root, 0}
	mapping[0] = (NodeMapping){g_root, 0};

	//While queue not empty:
	while(q_empty(bfs) == 0) {

		//Dequeue node and id
		int deId = 0;
		Node* deNode = NULL;
		q_dequeue(bfs, &deNode, &deId);

		//If node has yes child: add to mappings, enqueue with new id
		if(deNode->yes != NULL) {
			initId++;
			if(size >= capacity) {
				capacity *= 2;
				mapping = (NodeMapping*) realloc(mapping, capacity * sizeof(NodeMapping));
			}
			mapping[size++] = (NodeMapping){deNode->yes, initId};
			q_enqueue(bfs, deNode->yes, initId);
		}

		//If node has no child: add to mappings, enqueue with new id
               if(deNode->no != NULL) {
			initId++;
                        if(size >= capacity) {
                                capacity *= 2;
                                mapping = (NodeMapping*) realloc(mapping, capacity * sizeof(NodeMapping));
                        }
                        mapping[size++] = (NodeMapping){deNode->no, initId};
                        q_enqueue(bfs, deNode->no, initId);
               }
	}

	//5. Write header (magic, version, nodeCount) and all of these 4 bytes each
	uint32_t magic = (uint32_t)MAGIC;
	uint32_t version = (uint32_t)VERSION;
	uint32_t nodeCount = (uint32_t)size;

	fwrite(&magic, sizeof(uint32_t), 1, fp);
	fwrite(&version, sizeof(uint32_t), 1, fp);
	fwrite(&nodeCount, sizeof(uint32_t), 1, fp);

	//6. For each node in mapping order:
	for(int i = 0; i < size; i++){
		Node* writing = mapping[i].node;

		// - Write isQuestion, textLen, text bytes
		uint8_t isQ = writing->isQuestion;
		fwrite(&isQ, 1, 1, fp);

		uint32_t textLen = (uint32_t)strlen(writing->text);
		fwrite(&textLen, sizeof(uint32_t), 1, fp);
		fwrite(writing->text, 1, textLen, fp);

		// - Find yes child's id in mappings (or -1)
		int32_t yesID = -1;
		if(writing->yes) {
			for(int j = 0; j < size; j++) {
				if(mapping[j].node == writing->yes){
					yesID = mapping[j].id;
					break;
				}
			}
		}

		// - Find no child's id in mappings (or -1)
		int32_t noID = -1;
		if(writing->no) {
                        for(int j = 0; j < size; j++) {
                                if(mapping[j].node == writing->no){
                                        noID = mapping[j].id;
					break;
				}
                        }
                }

		// - Write yesId, noId
		fwrite(&yesID, sizeof(int32_t), 1, fp);
		fwrite(&noID, sizeof(int32_t), 1, fp);

	}

	//7. Clean up and return 1 on success
	free(mapping);
	q_free(bfs);
	free(bfs);
	fclose(fp);
	return 1;
}

/* load_tree
 * Load a tree from a binary file and reconstruct the structure
 *
 * Steps:
 * 1. Open file for reading binary ("rb")
 * 2. Read and validate header (magic, version, count)
 * 3. Allocate arrays for nodes and child IDs:
 *    - Node **nodes = calloc(count, sizeof(Node*))
 *    - int32_t *yesIds = calloc(count, sizeof(int32_t))
 *    - int32_t *noIds = calloc(count, sizeof(int32_t))
 * 4. Read each node:
 *    - Read isQuestion, textLen
 *    - Validate textLen (e.g., < 10000)
 *    - Allocate and read text string (add null terminator!)
 *    - Read yesId, noId
 *    - Validate IDs are in range [-1, count)
 *    - Create Node and store in nodes[i]
 * 5. Link nodes using stored IDs:
 *    - For each node i:
 *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
 *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
 * 6. Free old g_root if not NULL
 * 7. Set g_root = nodes[0]
 * 8. Clean up temporary arrays
 * 9. Return 1 on success
 *
 * Error handling:
In load_error: free all allocated memory and return 0 * - If any read fails or validation fails, goto load_error
 * - In load_error: free all allocated memory and return 0
 */
int load_tree(const char *filename) {
	//1. Open file for reading binary ("rb")
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL)
		return 0;

	//2. Read and validate header (magic, version, count)
	uint32_t magic = 0;
	uint32_t version = 0;
	uint32_t count = 0;

	Node** nodes = NULL;
        int32_t* yesIds = NULL;
        int32_t* noIds = NULL;

	//use goto to get to load_error if problem, otherwise just read and move on
	if(fread(&magic, sizeof(uint32_t), 1, fp) != 1)
		goto load_error;

	if(fread(&version, sizeof(uint32_t), 1, fp) != 1)
		goto load_error;

	if(fread(&count, sizeof(uint32_t), 1, fp) != 1)
		goto load_error;

	if(magic != MAGIC || version != VERSION || count == 0)
		goto load_error;

	//3. Allocate arrays for nodes and child IDs:
	nodes = calloc(count, sizeof(Node*));
	yesIds = calloc(count, sizeof(int32_t));
	noIds = calloc(count, sizeof(int32_t));

	//make sure that they were allocated
	if(!nodes || !yesIds || !noIds)
		goto load_error;

	//4. Read each node
	uint8_t isQ = 0;
	uint32_t textLen = 0;
	for(uint32_t i = 0; i < count; i++){
		//// - Read isQuestion, textLen
		if(fread(&isQ, 1, 1, fp) != 1)
			goto load_error;

		if(fread(&textLen, sizeof(uint32_t), 1, fp) != 1)
			goto load_error;

	// - Validate textLen (e.g., < 10000)
		if(textLen > 10000)
			goto load_error;

	// - Allocate and read text string (add null terminator!)
		char* text = (char*)malloc(sizeof(char) * (textLen + 1));

		if(text == NULL)
			goto load_error;

		if(fread(text, 1, textLen, fp) != textLen){
			free(text);
			goto load_error;
		}

		text[textLen] = '\0';

	// - Read yesId, noId (these are prolly signed)
		int32_t yesId;
		int32_t noId;

		if(fread(&yesId, sizeof(int32_t), 1, fp) != 1){
			free(text);
			goto load_error;
		}

		if(fread(&noId, sizeof(int32_t), 1, fp) != 1){
			free(text);
			goto load_error;
		}

	// - Validate IDs are in range [-1, count)
		if(yesId < -1 || yesId  >= (int32_t)count) {
			free(text);
			goto load_error;
		}

		if(noId < -1 || noId  >= (int32_t)count) {
                        free(text);
                        goto load_error;
                }

	// - Create Node and store in nodes[i]

		Node* node = (Node*)malloc(sizeof(Node));

		if(node == NULL){
			free(text);
			goto load_error;
		}

		node->text = text;
		node->yes = NULL;
		node->no = NULL;

		node->isQuestion = isQ ? 1 : 0;

		nodes[i] = node;
		yesIds[i] = yesId;
		noIds[i] = noId;
	}
	//5. Link nodes using stored IDs:
	// - For each node i:
	for(uint32_t i = 0; i < count; i++){
		// - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
		if(yesIds[i] >= 0)
			nodes[i]->yes = nodes[yesIds[i]];

		// - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
		if(noIds[i] >= 0)
			nodes[i]->no = nodes[noIds[i]];
	}

	//6. Free old g_root if not NULL
	if(g_root != NULL)
		free_tree(g_root);
	//7. Set g_root = nodes[0]
	g_root = nodes[0];

	//8. Clean up temporary arrays
	free(yesIds);
	free(noIds);
	free(nodes);
	fclose(fp);

	//9. Return 1 on success
	return 1;

	//In load_error: free all allocated memory and return 0
	load_error:
	if(nodes != NULL){
		for(uint32_t i = 0; i < count; i++) {
			if(nodes[i] != NULL){
				free(nodes[i]->text);
				free(nodes[i]);
			}
		}
	}

	free(yesIds);
	free(noIds);
	free(nodes);

	if(fp != NULL)
		fclose(fp);

	return 0;
}


