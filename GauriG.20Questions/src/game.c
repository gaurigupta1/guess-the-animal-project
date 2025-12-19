#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"

extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;

/* play_game
 * Main game loop using iterative traversal with a stack
 *
 * Key requirements:
 * - Use FrameStack (NO recursion!)
 * - Push frames for each decision point
 * - Track parent and answer for learning
 *
 * Steps:
 * 1. Initialize and display game UI
 * 2. Initialize FrameStack
 * 3. Push root frame with answeredYes = -1
 * 4. Set parent = NULL, parentAnswer = -1
 * 5. While stack not empty:
 *    a. Pop current frame
 *    b. If current node is a question:
 *       - Display question and get user's answer (y/n)
 *       - Set parent = current node
 *       - Set parentAnswer = answer
 *       - Push appropriate child (yes or no) onto stack
 *    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
B * 6. Free stack
 */
void play_game() {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Playing 20 Questions");
    attroff(COLOR_PAIR(5) | A_BOLD);

    mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
    mvprintw(3, 2, "Press any key to start...");
    refresh();
    getch();

	int row = 4;

    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently

	//Initialize FrameStack
    	FrameStack stack;
    	fs_init(&stack);
while(1){

	//use echo to enable character echoing when typed
	echo();

	//if NULL leave
	if(g_root == NULL)
		goto free_all;

	//Push root frame with answeredYes = -1
	fs_push(&stack, g_root, -1);

	//parent = NULL, parentAnswer = -1
	Node* parent = NULL;
	int parentAnswer = -1;


	//5. While stack not empty:
	while(fs_empty(&stack) != 1){
		//a. Pop current frame
		Frame popped = fs_pop(&stack);

		//b. If current node is a question:
		if(popped.node->isQuestion) {

			// - Display question and get user's answer (y/n)
			//Get coordinates
			row++;
			mvprintw(row, 2, "%s (y/n): ", popped.node->text);
			refresh();

                        char answer;
                        answer = getch();

			// - Set parent = current node
			parent = popped.node;

			// - Push appropriate child (yes or no) onto stack
			if(answer == 'y' || answer == 'Y'){
				fs_push(&stack, popped.node->yes, 1);

				// - Set parentAnswer = answer
				parentAnswer = 1;
			}
			else if(answer == 'n' || answer == 'N'){
				fs_push(&stack, popped.node->no, 0);

				// - Set parentAnswer = answer
				parentAnswer = 0;
			}
		}
		else if(popped.node->isQuestion == 0) {
			//Ask "Is it a [animal]?"
			row++;
			mvprintw(row, 2, "Is it a %s? (y/n): ", popped.node->text);
			refresh();
			char feedback;
			feedback = getch();

			//If correct: celebrate and break
			if(feedback == 'y' || feedback == 'Y') {
				row++;
				mvprintw(row, 2, "Yay! I guessed it!");
				refresh();

				row++;
				mvprintw(row, 2, "Press any key to leave :) ");
				getch();
				goto free_all;
			}

			else if(feedback == 'n' || feedback == 'N') {
				// - If wrong: LEARNING PHASE
				//i. Get correct animal name from user
				row++;
				mvprintw(row, 2, "What animal were you thinking of? ");
				refresh();
				char accAnimal[50];
				getnstr(accAnimal, sizeof(accAnimal) - 1);

				//i.5: edge case if user puts the already guessed animal
				if(strcmp(accAnimal, popped.node->text) == 0){
					row++;
                                	mvprintw(row, 2, "I already guessed that! Press any key to leave. ");
                                	getch();
                                	goto free_all;
				}

				//ii. Get distinguishing question
				row++;
				mvprintw(row, 2, "Give me a yes/no question to distinguish %s from %s: ", accAnimal, popped.node->text);
				refresh();

				char newQ[1000];
				getnstr(newQ, sizeof(newQ) - 1);

				//iii. Get answer for new animal (y/n for the question)
				getRealAns:
				row++;
				mvprintw(row, 2, "For %s, what is the answer? (y/n): ", accAnimal);
				refresh();

				char newAnswer;
				newAnswer = getch();

				if(newAnswer != 'y' && newAnswer != 'Y' && newAnswer != 'n' && newAnswer != 'N'){
					row++;
					mvprintw(row, 2, "Invalid input. Try again.");
					goto getRealAns;
				}

				row++;
				mvprintw(row, 2, "Thanks! I'll remember that.");
				refresh();

				row++;
				mvprintw(row, 2, "[Play again...]");
				refresh();

				row++;
				mvprintw(row, 2, "Press any key to leave :)");
				getch();

				//iv. Create new question node and new animal node
				Node* newNode = create_question_node(newQ);
				Node* newAnimal = create_animal_node(accAnimal);

				//v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
				if(newAnswer == 'y' || newAnswer == 'Y'){
					newNode->yes = newAnimal;
					newNode->no = popped.node;
				}

				//if newAnser is no, newQuestion->no = newAnimal
				else if(newAnswer == 'n' || newAnswer == 'N'){
					newNode->no = newAnimal;
					newNode->yes = popped.node;
				}
				//vi. Update parent pointer (or g_root if parent is NULL)
				if(parent == NULL)
					g_root = newNode;
				else if(parentAnswer == 1)
					parent->yes = newNode;
				else if(parentAnswer == 0)
					parent->no = newNode;

				//vii. Create Edit record and push to g_undo
                		Edit record;
                		record.parent = parent;
                		record.wasYesChild = parentAnswer;
                		record.oldLeaf = popped.node;
                		record.newQuestion = newNode;
                		record.newLeaf = newAnimal;

                		es_push(&g_undo, record);

                		//viii. Clear g_redo stack
                		es_clear(&g_redo);

				//leave to menu
				goto free_all;
			}

		}
	}
}
free_all:
    fs_free(&stack);

}

/* undo_last_edit
 * Undo the most recent tree modification
 *
 * Steps:
 * 1. Check if g_undo stack is empty, return 0 if so
 * 2. Pop edit from g_undo
 * 3. Restore the tree structure:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.oldLeaf
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.oldLeaf
 *    - Else:
 *      - Set edit.parent->no = edit.oldLeaf
 * 4. Push edit to g_redo stack
 * 5. Return 1
 *
 * Note: We don't free newQuestion/newLeaf because they might be redone
 */
int undo_last_edit() {
	//1. Check if g_undo stack is empty, return 0 if so
	int checkEmpty = es_empty(&g_undo);
	if(checkEmpty == 1)
		return 0;

	//2. Pop edit from g_undo
	Edit edit = es_pop(&g_undo);

	//3. Restore the tree structure:

	//If edit.parent is NULL:
	if(edit.parent == NULL)
		//Set g_root = edit.oldLeaf
		g_root = edit.oldLeaf;

	//Else if edit.wasYesChild:
	else if(edit.wasYesChild)
		//Set edit.parent->yes = edit.oldLeaf
		edit.parent->yes = edit.oldLeaf;

	//Else:
	else
		//Set edit.parent->no = edit.oldLeaf
		edit.parent->no = edit.oldLeaf;

	//4. Push edit to g_redo stack
	es_push(&g_redo, edit);

	//5. Return 1
	return 1;
}

/* redo_last_edit
 * Redo a previously undone edit
 *
 * Steps:
 * 1. Check if g_redo stack is empty, return 0 if so
 * 2. Pop edit from g_redo
 * 3. Reapply the tree modification:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.newQuestion
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.newQuestion
 *    - Else:
 *      - Set edit.parent->no = edit.newQuestion
 * 4. Push edit back to g_undo stack
 * 5. Return 1
 */
int redo_last_edit() {
	//1. Check if g_redo stack is empty, return 0 if so
	int checkEmpty = es_empty(&g_redo);
	if(checkEmpty == 1)
		return 0;

	//2. Pop edit from g_redo
	Edit edit = es_pop(&g_redo);

	//3. Reapply the tree modification:
	//If edit.parent is NULL:
	if(edit.parent == NULL)
		//Set g_root = edit.newQuestion
                g_root = edit.newQuestion;

	//Else if edit.wasYesChild:
        else if(edit.wasYesChild)
		//Set edit.parent->yes = edit.newQuestion
                edit.parent->yes = edit.newQuestion;
	//Else:
        else
		//Set edit.parent->no = edit.newQuestion
                edit.parent->no = edit.newQuestion;

	//4. Push edit back to g_undo stack
        es_push(&g_undo, edit);

	//5. Return 1
        return 1;
}
