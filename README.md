# 20 Questions - Guess the Animal

## Overview

 This is a terminal-based implementation of the classic "20 Questions" game with a twist: when the computer guesses wrong, it asks you to teach it about the new animal and adds the information to its decision tree.


**Features:**
- 🎯 Interactive guessing game
- 🧠 Learns from mistakes
- 💾 Saves/loads tree to binary file
- ↩️  Undo/redo edits
- 🌳 Visual tree explorer with scrolling
- 🔍 Tree integrity checking
- ⚡ **No recursion** in gameplay (uses explicit stacks!)

---


### What Makes This Game Special

**Learning AI**: The game starts with minimal knowledge (just 3 animals) but grows smarter as you play. Each time it fails to guess your animal, it learns by asking you for a distinguishing question and adding both the question and animal to its decision tree.

**Advanced Data Structures**: This implements multiple data structures from scratch in C:
- **Binary Decision Tree** - Stores questions (internal nodes) and animals (leaves)
- **Stacks** - For iterative tree traversal and undo/redo operations
- **Queue** - For breadth-first traversal during save/load
- **Hash Table** - For indexing attributes and potential query optimization

**No Recursion for Gameplay**: Unlike typical tree problems, this uses explicit stacks instead of recursion for the game loop. This taught me how compilers convert recursive calls to iterative code.

**Persistent Storage**: My tree saves to a compact binary format using BFS traversal, so the game "remembers" everything it learned across sessions.

**Undo/Redo System**: Made a mistake teaching the game? No problem! Full undo/redo support using dual stacks, just like a text editor.

### Example Game Session

```
Think of an animal, and I'll try to guess it!

Does it live in water? (y/n): n
Is it a Dog? (y/n): n

What animal were you thinking of? Cat
Give me a yes/no question to distinguish Cat from Dog: Does it meow?
For Cat, what is the answer? (y/n): y

Thanks! I'll remember that.

[Play again...]

Does it live in water? (y/n): n
Does it meow? (y/n): y
Is it a Cat? (y/n): y

Yay! I guessed it!
```

### Tree Growth Visualization

```
Initial tree:
       "Does it live in water?"
        /                    \
     "Fish"                "Dog"

After learning about Cat:
       "Does it live in water?"
        /                    \
     "Fish"          "Does it meow?"
                      /            \
                   "Cat"          "Dog"

After learning about Bird:
       "Does it live in water?"
        /                    \
     "Fish"          "Does it meow?"
                      /            \
                   "Cat"      "Does it fly?"
                               /           \
                            "Bird"        "Dog"
```

### What I Learned

By completing this lab, I:
- ✅ Improved my skills in manual memory management in C (malloc/free)
- ✅ Implemented iterative algorithms using explicit stacks
- ✅ Built multiple data structures from scratch
- ✅ Worked with binary file I/O and serialization
- ✅ Handled complex pointer relationships
- ✅ Debugd with valgrind and gdb
- ✅ Designed undo/redo systems
- ✅ Implemented a working User Interface
- ✅ Managed a multi-week project

### Project Scope

**Time Required**: 33-49 hours total
- Week 1: Data structures (20-25 hours)
- Week 2: Game logic and persistence (8-12 hours)
- Week 3: Testing and polish (5-12 hours)

**Lines of Code**: I wrote 370-530 lines across 4 files

**Delivered**:
1. Complete C implementation
2. All unit tests passing
3. Zero memory leaks (verified with valgrind or memory sanitizer)

---


## Implementation Plan

### Phase 1: Core Data Structures (Week 1)

#### Tree Nodes (~30-60 min)
```c
// In ds.c
create_question_node()  // Malloc node, strdup text, isQuestion=1
create_animal_node()    // Similar but isQuestion=0
free_tree()            // Recursive: free children, then self
count_nodes()          // Recursive: count all nodes
```
 
**Test:** `make test` - node tests should pass

#### Frame Stack (~1-2 hours)
Stack for iterative tree traversal. Dynamic array that doubles when full.
```c
fs_init()   // Malloc array, capacity=16
fs_push()   // Resize if needed, add frame
fs_pop()    // Return frames[--size]
fs_empty()  // Return size == 0
fs_free()   // Free array
```

**Test:** `make test` - stack tests should pass

#### Queue (~1-2 hours)
Linked list for BFS traversal.
```c
q_init()      // front=NULL, rear=NULL
q_enqueue()   // Malloc node, link at rear
q_dequeue()   // Remove from front, update rear if empty!
q_empty()     // Return front == NULL
q_free()      // Dequeue all
```

**Test:** `make test` - queue tests should pass

#### Hash Table (~3-5 hours)
Separate chaining for attribute indexing.
```c
canonicalize()  // "Does it meow?" → "does_it_meow"
h_hash()        // djb2: hash = ((hash << 5) + hash) + c
h_init()        // Calloc buckets
h_put()         // Search chain, add to list or create entry
h_contains()    // Search for key-value pair
h_get_ids()     // Return all IDs for key
h_free()        // Free chains, keys, arrays
```

**Test:** `make test` - hash tests should pass

### Phase 2: Game Logic (Week 2)

#### Edit Stack (~1 hour)
Similar to Frame Stack but for Edit structs.

#### Undo/Redo (~1-2 hours)
```c
undo_last_edit()  // Pop from g_undo, restore pointers, push to g_redo
redo_last_edit()  // Pop from g_redo, reapply, push to g_undo
```

#### Game Loop (~3-5 hours) ⭐ **HARDEST**
Iterative traversal using explicit stack. Learning phase creates new nodes and records edits.

**Key steps:**
1. Push root frame
2. While stack not empty:
   - Pop frame
   - If question: ask, push child based on answer
   - If leaf: guess, or learn if wrong
3. Learning: get animal/question, create nodes, update tree, record edit

**Test:** `make run` and play!

### Phase 3: Persistence (Week 2-3)

#### Binary I/O (~3-5 hours)
Use BFS to serialize tree with node IDs.

**save_tree():**
1. BFS to assign IDs (0, 1, 2, ...)
2. Write header (magic, version, count)
3. Write each node (isQuestion, textLen, text, yesId, noId)

**load_tree():**
1. Read header, validate
2. Read all nodes into array
3. Link using stored IDs
4. Set g_root = nodes[0]

**Test:** `make test` - persistence tests should pass

#### Integrity Checker (~30-60 min)
BFS to verify: questions have 2 children, leaves have 0 children.

---

## My Testing Strategy

### After Each Function
```bash
make test  # See which tests pass
```

### Full Testing Cycle
```bash
# 1. Unit tests
make clean && make test

# 2. Memory leaks
make valgrind-test

# 3. Interactive
make run
# Test: play, learn, undo, redo, save, load, integrity

# 4. Final check
make valgrind  # Run full program, should show no leaks
```

## Build Commands

```bash
make            # Build main program
make test       # Build and run tests
make run        # Run the game
make valgrind   # Check for memory leaks
make clean      # Remove build files
make help       # Show all targets
```

---

## My Debugging Guide

### Segfault?
```bash
gdb ./guess_animal
(gdb) run
(gdb) bt  # Backtrace shows where it crashed
```

### Memory Leak?
```bash
valgrind --leak-check=full --track-origins=yes ./run_tests
# Look for "definitely lost" - those are real leaks
```

### Test Failure?
```bash
# Read the assertion that failed
# Check line number in tests.c
# Add printf to see what's happening
```

---

## File Organization

### src:
- **ds.c** - All data structures
- **game.c** - Game logic and undo/redo
- **persist.c** - Save/load
- **utils.c** - Integrity checker

- **lab5.h** - All type definitions
- **main.c** - UI
- **tests.c** - Unit tests
- **Makefile** - Build system

### Documentation:
- **README.md** - This file (complete guide)
- **BUILD.md** - Full build and debugging reference

---
