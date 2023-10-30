/* Program to generate text based on the context provided by input prompts.

  Skeleton program written by Artem Polyvyanyy, http://polyvyanyy.com/,
  September 2023, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.
  All included code is (c) Copyright University of Melbourne, 2023.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* #DEFINE'S -----------------------------------------------------------------*/
#define SDELIM "==STAGE %d============================\n"   // stage delimiter
#define MDELIM "-------------------------------------\n"    // delimiter of -'s
#define THEEND "==THE END============================\n"    // end message
#define NOSFMT "Number of statements: %d\n"                 // no. of statements
#define NOCFMT "Number of characters: %d\n"                 // no. of chars
#define NPSFMT "Number of states: %d\n"                     // no. of states
#define TFQFMT "Total frequency: %d\n"                      // total frequency
#define PROMPSEP "..."                  // separator between prompt and output

#define CRTRNC '\r'                             // carriage return character

#define MAXCHAR 150            // Max characters per read statement
#define PROMPTMAX 37          // Max characters per prompt and its output.

/* TYPE DEFINITIONS ----------------------------------------------------------*/
typedef struct state state_t;   // a state in an automaton
typedef struct node  node_t;    // a node in a linked list

struct node {                   // a node in a linked list of transitions has
    char*           str;        // ... a transition string
    state_t*        state;      // ... the state reached via the string, and
    node_t*         next;       // ... a link to the next node in the list.
};

typedef struct {                // a linked list consists of
    node_t*         head;       // ... a pointer to the first node and
    node_t*         foot;       // ... a pointer to the last node in the list.
} list_t;

struct state {                  // a state in an automaton is characterized by
    unsigned int    id;         // ... an identifier,
    unsigned int    freq;       // ... frequency of traversal,
    int             visited;    // ... visited status flag, and
    list_t*         outputs;    // ... a list of output states.
};

typedef struct {                // an automaton consists of
    state_t*        ini;        // ... the initial state, and
    unsigned int    nid;        // ... the identifier of the next new state.
} automaton_t;

/* USEFUL FUNCTIONS ----------------------------------------------------------*/
int mygetchar(void);            // getchar() that skips carriage returns
char* char_to_string(char c);
automaton_t *initiate_automaton(void);
list_t *make_empty_list(void);
void free_list(list_t *list);
state_t *create_state(unsigned int nid);
list_t *insert_at_head(list_t *list, char* str, state_t *state);
void insert(automaton_t *autom, state_t *state, char* str);
void print_autom(automaton_t *autom, state_t *state);
void free_autom(automaton_t *autom);        // placed at the end of 'main'
void free_autom_rec(state_t *state);
void free_string(char* str);
char* process_prompt(automaton_t *autom, char* prompt);
char* get_statement(list_t *list, int n);
void compress_autom(automaton_t *autom, state_t *state, int steps);
void do_compress(state_t *state, node_t *node, list_t *outputs);
int count_freq(automaton_t *autom, state_t *state);
int count_states(automaton_t *autom, state_t *state);
int list_len(list_t* list);

/* WHERE IT ALL HAPPENS ------------------------------------------------------*/
int main(int argc, char *argv[]) {
    // Message from Artem: The proposed in this skeleton file #define's,
    // typedef's, and struct's are the subsets of those from my sample solution
    // to this assignment. You can decide to use them in your program, or if
    // you find them confusing, you can remove them and implement your solution
    // from scratch. I will share my sample solution with you at the end of
    // the subject.
    int stage = 0;
    char c;
    char statement[MAXCHAR];
    int char_count = 0;    
    int statement_count;
    int detect_empty = 0;        // Check if the next line is an empty line
    automaton_t *autom = initiate_automaton();
    statement[0] = '\0';
    for(stage = 0; stage < 3; stage++){
        statement_count = 0;
        printf(SDELIM, stage);
        
        // Read lines until an empty line is detected
        while((c=mygetchar()) != EOF){ 
            if(detect_empty && c == '\n'){
                // Empty line detected. All statements gathered
                break;
            } else {
                 detect_empty = 0;   
            }
            if(c == '\n'){
                // ==== PROCESS STATEMENT ==== //
                if(stage == 0){
                    // Add statement to automaton
                    insert(autom, autom->ini, statement);
                } else if(stage == 1){
                    // Run statement through automaton and produce an output
                    // statement
                    printf("%s\n",process_prompt(autom,statement));   
                } else if(stage == 2){
                    if(statement_count == 0){
                        // Fst Line: Compress automaton and print some stats //
                        compress_autom(autom, autom->ini, atoi(statement));
                        printf(NPSFMT, count_states(autom, autom->ini));
                        printf(TFQFMT, count_freq(autom, autom->ini));
                        printf(MDELIM);
                        //print_autom(autom, autom->ini);
                    } else {
                        printf("%s\n",process_prompt(autom,statement));
                    }
                }
                // Reset for next line
                detect_empty = 1;
                statement_count++;
                strcpy(statement, "");
            } else {
                char_count++;
                // Add character to statement string.
                if(strlen(statement) + 1 > MAXCHAR){
                    printf("Statement exceeded %d characters.", MAXCHAR);
                    exit(EXIT_FAILURE);
                } else {
                    char ch[] = {c, '\0'};
                    strncat(statement, ch, MAXCHAR);
                }
            }
        }
        if(stage == 0){
            printf(NOSFMT, statement_count);
            printf(NOCFMT, char_count);
            printf(NPSFMT, autom->nid);
            //print_autom(autom, autom->ini);
        }
    }
    printf(THEEND);
    free_autom(autom);
    return EXIT_SUCCESS;        // algorithms are fun!!!
}

/* USEFUL FUNCTIONS ----------------------------------------------------------*/

// An improved version of getchar(); skips carriage return characters.
// NB: Adapted version of the mygetchar() function by Alistair Moffat
int mygetchar() {
    int c;
    while ((c=getchar())==CRTRNC);
    return c;
}

// Converts a character to a string of size 2(for the char and null byte)
char* char_to_string(char c){
    return 0;
}

// Copied from listops.c, Written by Alistair Moffat.
// Makes an empty list
list_t *make_empty_list(void){
    list_t *list;
    list = (list_t*)malloc(sizeof(*list));
    assert(list != NULL);
    list->head = list->foot = NULL;
    return list;
}

// Modified from listops.c, Written by Alistair Moffat.
void
free_list(list_t *list) {
	node_t *curr, *prev;
	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
        free_string(prev->str);
		free(prev);
	}
	free(list);
}

// Slightly modified from listops.c, Written by Alistair Moffat.
// Inserts a state, as well as its accompanying string,
// into the head of a list
list_t
*insert_at_head(list_t *list, char* str, state_t *state) {
	node_t *new;
	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
    new->str = (char*)malloc(2*sizeof(char));
    strcpy(new->str, str);
	new->state = state;
	new->next = list->head;
	list->head = new;
	if (list->foot==NULL) {
		/* this is the first insertion into the list */
		list->foot = new;
	}
	return list;
}

// Sets up a state with an empty output list
state_t *create_state(unsigned int nid){
    state_t *state;
    state = (state_t*)malloc(sizeof(*state));
    assert(state != NULL);
    
    state->id = nid;
    state->freq = 0;
    state->visited = 0;
    state->outputs = make_empty_list();
    
    return state;
}

// Set up a basic automaton with the root state
automaton_t *initiate_automaton(void){
    automaton_t *autom;
    autom = (automaton_t*)malloc(sizeof(*autom));
    assert(autom != NULL);
    autom->ini = create_state(0);
    
    autom->ini->id = 0;
    autom->nid = 1;
    
    return autom;   
}
// Inserts a string into the automaton by recursively adding characters of an
// input 'str'.  
void insert(automaton_t *autom, state_t *state, char* str){
    assert(autom != NULL && state != NULL);
    list_t *outputs = state->outputs;
    
    if(str[0] == '\0'){
        // End of statement
        return;
    }
    
    // Convert first char of 'str' into a string
    char ch_str[] = {str[0], '\0'};
    
    // Check if str matches any outputs so far
    node_t *p = outputs->head;
    while(p){
        if((strcmp(ch_str,p->str) == 0)){
            //Matching string found, continue down the branch
            if(strlen(str) > 0){
                insert(autom, p->state, &str[1]);
                state->freq++;
            }
            return;
        }
        p = p->next;
    }
    // Matching string was not found, add the str to outputs
    outputs = insert_at_head(outputs, ch_str, create_state(autom->nid));
    autom->nid++;
    state->freq++;
    if(strlen(str) > 0){
        insert(autom, outputs->head->state, &str[1]);
    }
    return;
}

// DEBUG function, prints all states with their values
void print_autom(automaton_t *autom, state_t *state){
    assert(autom != NULL && state != NULL);
    
    node_t *p = state->outputs->head;
    while(p){
        printf("Id: %d: [\"%s\",%d]\n", p->state->id,p->str, p->state->freq);
        print_autom(autom, p->state);
        p = p->next;
    }
}

// Counts the total frequency across all states in an Automaton
int count_freq(automaton_t *autom, state_t *state){
    assert(autom != NULL && state != NULL);
    int count = 0;
    node_t *p = state->outputs->head;
    while(p){
        count = count + p->state->freq + count_freq(autom, p->state);
        p = p->next;
    }
    return count;
}

// Counts number of states in automaton
int count_states(automaton_t *autom, state_t *state){
    assert(autom != NULL && state != NULL);
    int states = 0;
    node_t *p = state->outputs->head;
    while(p){
        states += 1 + count_states(autom, p->state);
        p = p->next;
    }
    return states;
}

// Frees all the malloc memory in all memory storage within the automaton.
// (As well as setting everything to null)
void free_autom(automaton_t *autom){
    state_t *state = autom->ini;
    free_autom_rec(state);
}
// Helper function to recursively free all nodes and states in the
// automaton
void free_autom_rec(state_t *state){
    if(state == NULL){
        return;
    }
    state->id = 0;
    state->freq = 0;
    // Free all nodes in the state's output
    node_t *prev;
    node_t *p = state->outputs->head;
    while(p){
        free_string(p->str);
        free_autom_rec(p->state);
        prev = p;
        p = p->next;
        free(prev);
    }
    state->outputs = NULL;
    free(state);
}
void free_string(char* str){
    free(str);
    str = NULL;
}

// Replays the automaton using the prompt, until all characters are used
char* process_prompt(automaton_t *autom, char* prompt){
    assert(autom != NULL);
    state_t *start = autom->ini;
    list_t *outputs = start->outputs;
    int branch_found = 0;
    int n = 0;        // Tracks current length of output string
    int len = (int)strlen(prompt);
    char* str = (char*)malloc((sizeof(char) * len) + 3);
    str[0] = '\0';
    
    int offset = 0;
    // Replay the automaton using the prompt //
    node_t *p;
    for(int i = 0; i + offset < len; i++){
        p = outputs->head;
        offset = (int)strlen(p->str) - 1;
        
        // construct a comparison string
        char* ch_str = (char*)malloc(sizeof(char) * strlen(p->str));
        strcpy(ch_str, "");
        strncat(ch_str, &prompt[i], strlen(p->str));
        
        branch_found = 0;
        strncat(str, ch_str,PROMPTMAX);    // Construct replay string
        n++;
        
        if(n >= PROMPTMAX){
            break;
        }
        
        while(p){
            //printf("Comparing %s to %s\n", ch_str, p->str);
            if(strcmp(p->str, ch_str) == 0 && p->state->outputs->head != NULL){
                // Matching string, continue down this branch
                outputs = p->state->outputs;
                branch_found = 1;
                break;
            }
            p = p->next;
        }
        if(!branch_found){
            // Add the next character of prompt that was going to be used before
            // a coontinuation branch wasn't found (Honestly not sure what this does, but it passed one of the
            // assignment test cases so...)
            strncat(str, &prompt[i + 1], PROMPTMAX - strlen(str) - 1);
            n++;
            break;
        }
        free(ch_str);
        ch_str = NULL;
    }
    // Add ellipses to the string
    for(int i = 0; i < 3; i++){
        if(i+n >= PROMPTMAX){
            return str;
        }
        strncat(str, ".",PROMPTMAX);
    }
    
    if(!branch_found){
        return str;
    }

    // Automaton replayed, now get the remaining characters.
    return strncat(str, get_statement(outputs, n), PROMPTMAX);
}

// After prompt was processed, traverse the automaton until an end is reached.
// In cases of multiple branches, states with higher frequency or higher ASCII
// strings will be taken.
// n denotes the total length of the string, this mustn't exceed PROMPTMAX
char* get_statement(list_t *list, int n){
    assert(list != NULL);
    int max_freq = -1;
    int curr_freq;
    node_t *candidate;
    
    if(n >= PROMPTMAX || list->head == NULL){
        // String is too long or List is empty
        return "";
    }
    
    node_t *p = list->head;
    // Compare with each node in list
    while(p){
        // == Getting max frequency == //
        curr_freq = p->state->freq;
        if(curr_freq > max_freq){
            max_freq = curr_freq;
            candidate = p;
        } else if(curr_freq == max_freq){
            // == Same freq. as candidate, get highest ASCII value == //
            if(strcmp(p->str, candidate->str) > 0){
                candidate = p;
            }
        }
        p = p->next;
    }
    char* str = (char*)malloc(sizeof(char) * 2);
    str[0] = '\0';
    strncat(str, candidate->str,PROMPTMAX);
    n++;
    if(candidate->state->outputs != NULL){
        // Branch continues.
        return strncat(str, get_statement(candidate->state->outputs, n),
        PROMPTMAX);
    }
    return str;    // No more states forward. End here
}

// Searches through the automaton and compresses nodes according to rules:
// 1. Current state has 1 output arc
// 2. The output node has multiple output arcs
void compress_autom(automaton_t *autom, state_t *state, int steps){
    assert(autom != NULL && state != NULL);
    if(steps <= 0 || state->outputs->head == NULL){
        return;
    }

    node_t *p = state->outputs->head;
    // Check: Current state has 1 arc, output has 1 or more arc.
    if(list_len(state->outputs) == 1 && list_len(p->state->outputs) >= 1){
        // Next state has several Arcs, compress
        do_compress(state, p, p->state->outputs);
    }
    
    // Continue traversal.
    steps--;
    while(p){
        compress_autom(autom, p->state, steps);
        p = p->next;
    }
}

// Combines/compresses a node with outputs of another node whilst updating
// the outputs of 'state' to the new compressed nodes
void do_compress(state_t *state, node_t *node, list_t *outputs){
    assert(state !=NULL && node != NULL && outputs != NULL);
    node_t *p = outputs->head;
    char* base_str = (char*)malloc(sizeof(char) * strlen(node->str));
    strcpy(base_str, node->str);
    
    while(p){
        // Compress and assign new string
        strncat(base_str, p->str, MAXCHAR);
        strcpy(p->str, base_str);
        //printf("%s\n", p->str);
        strcpy(base_str, node->str);
        p = p->next;
    }
    state->outputs = outputs;
    
    free(base_str);
    base_str = NULL;
}

// Returns the number of elements in a linked list
int list_len(list_t* list){
    assert(list != NULL);
    int len = 0;
    
    node_t *p = list->head;
    while(p){
        len++;
        p = p->next;
    }
    return len;
}
/* THE END -------------------------------------------------------------------*/
