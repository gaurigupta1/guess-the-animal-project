#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab5.h"

/* ========== Node Functions ========== */

/* create_question_node
 * - Allocate memory for a Node structure
 * - Use strdup() (I USED STRCPY INSTEAD) to copy the question string (heap allocation)
 * - Set isQuestion to 1
 * - Initialize yes and no pointers to NULL
 * - Return the new node
 */
Node *create_question_node(const char *question) {
	//Allocate Memory for a Node Structure
	Node* qNode = (Node*)malloc(sizeof(Node));
	if(qNode == NULL)
		return NULL;

	//Use malloc and strcpy to copy the question string (heap allocation)
	qNode->text = (char*)malloc(strlen(question) + 1);
	if(qNode->text != NULL)
		strcpy(qNode->text, question);

	//Set isQuestion to 1
	qNode->isQuestion = 1;

	//Initialize yes and no pointers to NULL
	qNode->yes = NULL;
	qNode->no = NULL;

	//Return the new node
	return qNode;
}

/* create_animal_node
 * - Similar to create_question_node but set isQuestion to 0
 * - This represents a leaf node with an animal name
 */
Node *create_animal_node(const char *animal) {
	//Allocate Memory for a Node Structure
        Node* aNode = (Node*)malloc(sizeof(Node));
	if(aNode == NULL)
		return NULL;

        //Use strcpy() to copy the animal string (heap allocation)
        aNode->text = (char*)malloc(strlen(animal) + 1);
	if(aNode->text != NULL)
		strcpy(aNode->text, animal);

        //Set isQuestion to 0
        aNode->isQuestion = 0;

        //Initialize yes and no pointers to NULL
        aNode->yes = NULL;
        aNode->no = NULL;

        //Return the new node
        return aNode;
}

/*   free_tree (recursive)
 * - This is one of the few recursive functions allowed
 * - Base case: if node is NULL, return
 * - Recursively free left subtree (yes)
 * - Recursively free right subtree (no)
 * - Free the text string
 * - Free the node itself
 * IMPORTANT: Free children before freeing the parent!
 */
void free_tree(Node *node) {
	//Base case: if node is NULL, return
	if(node == NULL) {
		return;
	}

	//Recursively free left subtree (yes)
	free_tree(node->yes);

	//Recursively free right subtree (no)
	free_tree(node->no);

	//Free the text string
	free(node->text);

	//Free the node itself
	free(node);
}

/* count_nodes (recursive)
 * - Base case: if root is NULL, return 0
 * - Return 1 + count of left subtree + count of right subtree
 */
int count_nodes(Node *root) {
	//Base case: if root is NULL, return 0
	if(root == NULL) {
		return 0;
	}

	//Return 1 + count of left subtree + count of right subtree
	return (1 + count_nodes(root->yes) + count_nodes(root->no));
}

/* ========== Frame Stack (for iterative tree traversal) ========== */

/* fs_init
 * - Allocate initial array of frames (start with capacity 16)
 * - Set size to 0
 * - Set capacity to 16
 */
void fs_init(FrameStack *s) {
	//Allocate initial array of frames (start with capacity 16)
	s->frames = (Frame*)malloc(sizeof(Frame)*16);

	//Set size to 0
	s->size = 0;

	//Set capacity to 16
	s->capacity = 16;
}

/* fs_push
 * - Check if size >= capacity
 *   - If so, double the capacity and reallocate the array
 * - Store the node and answeredYes in frames[size]
 * - Increment size
 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
	//Check if size >= capacity
	if(s->size >= s->capacity) {
		//If so, double the capacity and reallocate the array
		s->capacity = s->capacity * 2;
		s->frames = (Frame*)realloc(s->frames, sizeof(Frame) * s->capacity);
	}

	//Store the node and answeredYes in frames[size]
	s->frames[s->size].node = node;
	s->frames[s->size].answeredYes = answeredYes;

	//Increment Size
	s->size = s->size + 1;
}

/* fs_pop
 * - Decrement size
 * - Return the frame at frames[size]
 * Note: No need to check if empty - caller should use fs_empty() first
 */
Frame fs_pop(FrameStack *s) {
	//Decrement size
	s->size = s->size - 1;

	//Return the frame at frames[size]
	return s->frames[s->size];
}

/* fs_empty
 * - Return 1 if size == 0, otherwise return 0
 */
int fs_empty(FrameStack *s) {
	//if empty, return 1
	if(s->size == 0)
		return 1;
	//otherwise, return 0
	else
		return 0;
}

/* fs_free
 * - Free the frames array
 * - Set frames pointer to NULL
 * - Reset size and capacity to 0
 */
void fs_free(FrameStack *s) {
	//Free the frames array
	free(s->frames);

	//Set frames pointer to NULL
	s->frames = NULL;

	//Reset size and capacity to 0
	s->capacity = 0;
	s->size = 0;
}

/* ========== Edit Stack (for undo/redo) ========== */

/* es_init
 * Similar to fs_init but for Edit structs
 */
void es_init(EditStack *s) {
	//literally the exact same thing as fs_init but for Edit
        s->edits = (Edit*)malloc(sizeof(Edit)*16);
        s->size = 0;
        s->capacity = 16;
}

/* es_push
 * Similar to fs_push but for Edit structs
 * - Check capacity and resize if needed
 * - Add edit to array and increment size
 */
void es_push(EditStack *s, Edit e) {
	//literally the exact same thing as fs_push but for Edit
        if(s->size >= s->capacity) {
                s->capacity = s->capacity * 2;
                s->edits = (Edit*)realloc(s->edits, sizeof(Edit) * s->capacity);
        }

        s->edits[s->size] = e;
        s->size = s->size + 1;
}

/* es_pop
 * Similar to fs_pop but for Edit structs
 */
Edit es_pop(EditStack *s) {
	//literally the exact same thing as fs_pop but for Edit
        s->size = s->size - 1;
        return s->edits[s->size];
}

/* es_empty
 * Return 1 if size == 0, otherwise 0
 */
int es_empty(EditStack *s) {
	//literally the exact same thing as fs_empty but for Edit
	if(s->size == 0) {
		return 1;
	} else {
		return 0;
	}
}

/* es_clear
 * - Set size to 0 (don't free memory, just reset)
 * - This is used to clear the redo stack when a new edit is made
 */
void es_clear(EditStack *s) {
	//set the size to zero
	s->size = 0;
}

void es_free(EditStack *s) {
	//literally the exact same thing as fs_free but for edits
    free(s->edits);
    s->edits = NULL;
    s->size = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) {
	//free the actual stack
    es_free(s);
}

/* ========== Queue (for BFS traversal) ========== */

/* q_init
 * - Set front and rear to NULL
 * - Set size to 0
 */
void q_init(Queue *q) {
	//Set front and rear to NULL
	q->front = NULL;
	q->rear = NULL;

	//Set size to 0
	q->size = 0;
}

/* q_enqueue
 * - Allocate a new QueueNode
 * - Set its treeNode and id fields
 * - Set its next pointer to NULL
 * - If queue is empty (rear == NULL):
 *   - Set both front and rear to the new node
 * - Otherwise:
 *   - Link rear->next to the new node
 *   - Update rear to point to the new node
 * - Increment size
 */
void q_enqueue(Queue *q, Node *node, int id) {
    	//Allocate a new QueueNode
	QueueNode* addNode = (QueueNode*)malloc(sizeof(QueueNode));

	//Set its treeNode and id fields
	addNode->treeNode = node;
	addNode->id = id;

	//Set its next pointer to NULL
	addNode->next = NULL;

	//If queue is empty (rear == NULL):
	if(q->rear == NULL) {
		//Set both front and rear to the new node
		q->front = addNode;
		q->rear = addNode;
	} else {
	//Otherwise:
		//Link rear->next to the new node
		q->rear->next = addNode;
		q->rear = addNode;
	}
	//Increment Size
	q->size = q->size + 1;
}

/* q_dequeue
 * - If queue is empty (front == NULL), return 0
 * -B Save the front node's data to output parameters (*node, *id)
 * - Save front in a temp variable
 * - Move front to front->next
 * - If front is now NULL, set rear to NULL too
 * - Free the temp node
 * - Decrement size
 * - Return 1
 */
int q_dequeue(Queue *q, Node **node, int *id) {
    	//If queue is empty (front == NULL), return 0
	if(q->front == NULL){
		return 0;
	}

	//Save the front node's data to output parameters (*node, *id)
	*id = q->front->id;
	*node = q->front->treeNode;

	//Save front in a temp variable
	QueueNode* temp = q->front;

	//Move front to front->next
	q->front = q->front->next;

	//If front is now NULL, set rear to NULL too
	if(q->front == NULL) {
		q->rear = NULL;
	}

	//Free the temp node
	free(temp);

	//Decrement Size
	q->size = q->size - 1;

	//return 1
	return 1;
}

/* q_empty
 * Return 1 if size == 0, otherwise 0
 */
int q_empty(Queue *q) {
	//Return 1 if size == 0, otherwise 0
	if(q->size == 0) {
		return 1;
	} else {
		return 0;
	}
}

/* q_free
 * - Dequeue all remaining nodes
 * - Use a loop with q_dequeue until queue is empty
 */
void q_free(Queue *q) {
	//place to put dequeued items, just temp variables
	int *id = 0;
	Node **node = NULL;

	//if nothing to free, return
	if(q->front == NULL) { return; }

	//while you can dequeue, keep doing it (while dequeue returns 1, keep dequeueing)
	int a = 1;
	while(a != 0) {
		a = q_dequeue(q, node, id);
	}

	//USER MUST FREE ACTUAL QUEUE

}

/* ========== Hash Table ========== */

/* canonicalize
 * Convert a string to canonical form for hashing:
 * - Convert to lowercase
 * - Keep only alphanumeric characters
 * - Replace spaces with underscores
 * - Remove punctuation
 * Example: "Does it meow?" -> "does_it_meow"
 *
 * Steps:
 * - Allocate result buffer (strlen(s) + 1)
 * - Iterate through input string
 * - For each character:
 *   - If alphanumeric: add lowercase version to result
 *   - If whitespace: add underscore
 *   - Otherwise: skip it
 * - Null-terminate result
 * - Return the new string
 */
char *canonicalize(const char *s) {
	//malloc the string that will have the canonicalized phrase
	char* buffer = (char*)malloc(strlen(s) + 1);

	//track current index of actual string
	int i = 0;

	//track current index of buffer
	int nullIdx = 0;

	//loop through actual string
	for(i = 0; s[i] != '\0'; i++) {

		//if alphanumeric, put the lowercase version in the buffer
		if(isalnum(s[i])){
			buffer[i] = tolower(s[i]);
			nullIdx++;

		//if space, convert to underscore, put in buffer
		} else if(isspace(s[i])){
			buffer[i] = '_';
			nullIdx++;
		}
	}

	//put null at end of buffered string
	buffer[nullIdx] = '\0';

	//return buffer
	return buffer;
}

/* h_hash (djb2 algorithm)
 * unsigned hash = 5381;
 * For each character c in the string:
 *   hash = ((hash << 5) + hash) + c;  // hash * 33 + c
 * Return hash
 */
unsigned h_hash(const char *s) {
	//init has to 5381
	unsigned hash = 5381;

	//For each character c in the string:
	int i = 0;
	for(i = 0; s[i] != '\0'; i++){
		//hash = ((hash << 5) + hash) + c;  // hash * 33 + c
		hash = ((hash << 5) + hash) + (unsigned)s[i];
	}
	return hash;
}

/* h_init
 * - Allocate buckets array using calloc (initializes to NULL)
 * - Set nbuckets field
 * - Set size to 0
 */
void h_init(Hash *h, int nbuckets) {
	//Allocate buckets array using calloc (initializes to NULL)
	h->buckets = (Entry**)calloc(nbuckets, sizeof(Entry*));

	//Set nbuckets field
	h->nbuckets = nbuckets;

	//Set size to 0
	h->size = 0;
}

/* h_put
 * Add animalId to the list for the given key
 *
 * Steps:
 * 1. Compute bucket index: idx = h_hash(key) % nbuckets
 * 2. Search the chain at buckets[idx] for an entry with matching key
 * 3. If found:
 *    - Check if animalId already exists in the vals list
 *    - If yes, return 0 (no change)
 *    - If no, add animalId to vals.ids array (resize if needed), return 1
 * 4. If not found:
 *    - Create new Entry with strdup(key) (I USED STRCPY INSTEAD)
 *    - Initialize vals with initial capacity (e.g., 4)
 *    - Add animalId as first element
 *    - Insert at head of chain (buckets[idx])
 *    - Increment h->size
 *    - Return 1
 */
int h_put(Hash *h, const char *key, int animalId) {
	//1. Compute bucket index: idx = h_hash(key) % nbuckets
	int idx = h_hash(key) % h->nbuckets;

	//2. Search the chain at buckets[idx] for an entry with matching key
	Entry* ptr = h->buckets[idx];
	while(ptr != NULL) {

		//3. If found:
		if(strcmp(key, ptr->key) == 0){

			//Check if animalId already exists in the vals list
			int i = 0;
			for(i = 0; i < ptr->vals.count; i++) {
				if(ptr->vals.ids[i] == animalId){
					//If yes, return 0 (no change)
					return 0;
				}
			}

			//If no, add animalId to vals.ids array (resize if needed), return 1
			if(ptr->vals.count == ptr->vals.capacity){
				//resize if full
				ptr->vals.ids = (int*)realloc(ptr->vals.ids, ptr->vals.capacity + 4);
				ptr->vals.capacity += 4;

			}

			//put animalID in arr
			ptr->vals.ids[ptr->vals.count] = animalId;
			ptr->vals.count++;
			return 1;
		}

		//Check next node
		else {
			ptr = ptr->next;
		}
	}

	//If not found:

	//Create new Entry with malloc and probably also strcpy
	Entry* notFound = (Entry*)malloc(sizeof(Entry));
	notFound->key = (char*)malloc(strlen(key) + 1);
	strcpy(notFound->key, key);

	//Initialize vals with initial capacity (e.g., 4)
	notFound->vals.capacity = 4;
	notFound->vals.ids = (int*)calloc(notFound->vals.capacity,sizeof(int));

	//Add animalId as first element
	notFound->vals.ids[0] = animalId;
	notFound->vals.count = 1;

	//Insert at head of chain (buckets[idx])
	notFound->next = h->buckets[idx];
	h->buckets[idx] = notFound;

	//Increment h->size
	h->size++;

	//Return 1
	return 1;
}

/* h_contains
 * Check if the hash table contains the given key-animalId pair
 *
 * Steps:
 * 1. Compute bucket index
 * 2. Search the chain for matching key
 * 3. If found, search vals.ids array for animalId
 * 4. Return 1 if found, 0 otherwise
 */
int h_contains(const Hash *h, const char *key, int animalId) {
	//1. Compute bucket index
	int idx = h_hash(key) % h->nbuckets;
	Entry* ptr = h->buckets[idx];

	//2. Search the chain for matching key
        while(ptr != NULL) {

                //3. If found:
                if(strcmp(key, ptr->key) == 0){

                        //Check if animalId already exists in the vals list
                        int i = 0;
                        for(i = 0; i < ptr->vals.count; i++) {
                                if(ptr->vals.ids[i] == animalId){
                                        //If yes, return 0 (no change)
                                        return 1;
                                }
                        }

			return 0;
                }

                //Check next node
                else {
                        ptr = ptr->next;
		}
	}
    return 0;
}

/* h_get_ids
 * Return pointer to the ids array for the given key
 * Set *outCount to the number of ids
 * Return NULL if key not found
 *
 * Steps:
 * 1. Compute bucket index
 * 2. Search chain for matching key
 * 3. If found:
 *    - Set *outCount = vals.count
 *    - Return vals.ids
 * 4. If not found:
 *    - Set *outCount = 0
 *    - Return NULL
 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) {
    	//1. Compute bucket index
        int idx = h_hash(key) % h->nbuckets;
        Entry* ptr = h->buckets[idx];

	//2. Search chain for matching key
        while(ptr != NULL) {

                //3. If found:
                if(strcmp(key, ptr->key) == 0){
			//Set *outCount = vals.count
                        *outCount = ptr->vals.count;

			//Return vals.ids
                        return ptr->vals.ids;
                }

                //Check next node
                else {
                        ptr = ptr->next;
                }
        }

	//If not found:

	//Set *outCount = 0
	*outCount = 0;

	//return NULL
	return NULL;
}

/* h_free
 * Free all memory associated with the hash table
 *
 * Steps:
 * - For each bucket:
 *   - Traverse the chain
 *   - For each entry:
 *     - Free the key string
 *     - Free the vals.ids array
 *     - Free the entry itself
 * - Free the buckets array
 * - Set buckets to NULL, size to 0
 */
void h_free(Hash *h) {
	int i = 0;

	//For each bucket:
	for(i = 0; i < h->nbuckets; i++) {
		//head ptr of chain
		Entry* ptr = h->buckets[i];

		//traverse the chain
		while(ptr != NULL){
			//Free the key string
			free(ptr->key);

			//Free the vals.ids array
			free(ptr->vals.ids);

			//Free the entry itself (used temp variable)
			Entry* temp = ptr;
			ptr = ptr->next;
			free(temp);
		}
	}

	//Free Buckets Array
	free(h->buckets);

	//set buckets to NULL, size to 0
	h->buckets = NULL;
	h->nbuckets = 0;
	h->size = 0;
}
