#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>


#define SIDE_LENGTH 4
#define NEW_NODES_ARR_SIZE 4

enum move_type{up=-2, down, left, right};
enum Bool_type{False, True};

struct node_struct{
    int                  state[SIDE_LENGTH][SIDE_LENGTH];
    int                  hole_pos[2];
    enum move_type       move;
    int                  depth;      // g
    int                  heuristic;  // h
    int                  f;          // f = h + g
};

typedef struct node_struct nodes;
typedef enum Bool_type bool;
typedef enum move_type moves;


struct list_node{
    nodes *puzzle_node;
    struct list_node *previous_node;
    struct list_node *next_node;
};

typedef struct list_node list;


list *list_head = NULL;
list *last_node = NULL;


void list_append(nodes *puzzle_node){
    list *new_node = malloc(sizeof(list));
    new_node->puzzle_node = puzzle_node;
    new_node->previous_node = NULL;
    new_node->next_node = NULL;
    
    if(list_head == NULL){
        list_head = new_node;
    }
    else{
        new_node->previous_node = last_node;
        last_node->next_node = new_node;
    }
    last_node = new_node;
}


void list_pop(){
    if(list_head->next_node == NULL){
        last_node = NULL;
        free(list_head);
        list_head = NULL;
        return;
    }

    last_node = last_node->previous_node;
    free(last_node->next_node);
    last_node->next_node = NULL;
}

bool is_in_list(nodes *node){
    list *cur = list_head;
    int i, j, same=0, to_break=0;

    while(cur != NULL){
        same = 0;
        // hole_pos not the same --> means two state is different, so continue
        if((cur->puzzle_node->hole_pos[0] != node->hole_pos[0]) || (cur->puzzle_node->hole_pos[1] != node->hole_pos[1])){
            cur = cur->next_node;
            continue;
        }
        if(cur->puzzle_node->heuristic != node->heuristic){
            cur = cur->next_node;
            continue;
        }
        for(i=0, to_break=0; i<SIDE_LENGTH && to_break == 0; i++){
            for(j=0; j<SIDE_LENGTH; j++){
                if(cur->puzzle_node->state[i][j] != node->state[i][j]){
                    to_break = 1;
                    break;
                }
            }
        }
        if(to_break == 0) return True;
        cur = cur->next_node;
    }
    return False;
}



int goal_state[SIDE_LENGTH][SIDE_LENGTH];
int infinite=500000;
int min;
unsigned long long total_nodes=0;
int move_count=0;
bool found_goal_state = False;
nodes *start_state;

FILE *fp;





void swap(nodes* *a, nodes* *b){
    nodes *temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

void bubble_sort(nodes* s[], int size){
    int i, j;

    for(i=0; i<size-1; i++){
        for(j=0; (j+1 < size-i) && (s[j+1] != NULL); j++){
            if(s[j]->heuristic > s[j+1]->heuristic){
                swap(&s[j], &s[j+1]);
            }
        }
    }
}






void set_goal_state(){
    int i, j, num=1;
    for(i=0; i<SIDE_LENGTH; i++){
        for(j=0; j<SIDE_LENGTH; j++, num++){
            if(i==SIDE_LENGTH-1 && j==SIDE_LENGTH-1) num = 0;
            goal_state[i][j] = num;
        }
    }
}



int row_lc(nodes *node, int r){
    int j, k, conf=0;
    
    for(j=0; j<SIDE_LENGTH-1; j++){
        if(node->state[r][j] == 0) continue;
        if((node->state[r][j]-1) / SIDE_LENGTH == r){
            for(k=j+1; k<SIDE_LENGTH; k++){
                if(node->state[r][k] == 0 || ((node->state[r][k]-1) / SIDE_LENGTH != r)) continue;
                else if(node->state[r][k] < node->state[r][j]) conf++;
            }
        }
    }
    return conf;
}


int col_lc(nodes *node, int c){
    int i, k, conf=0;
    
    for(i=0; i<SIDE_LENGTH-1; i++){
        if(node->state[i][c] == 0) continue;
        if((node->state[i][c]-1) % SIDE_LENGTH == c){
            for(k=i+1; k<SIDE_LENGTH; k++){
                if(node->state[k][c] == 0 || ((node->state[k][c]-1) % SIDE_LENGTH != c)) continue;
                else if(node->state[k][c] < node->state[i][c]) conf++;
            }
        }
    }
    return conf;
}



int manhattan_dis(nodes *node, int r, int c){
    return abs(r - ((node->state[r][c] - 1) / SIDE_LENGTH))
            + abs(c - ((node->state[r][c] - 1) % SIDE_LENGTH));
}


int heuristic_calculate(nodes *node){
    int i, j, dis=0;
    for(i=0; i<SIDE_LENGTH; i++){
        for(j=0; j<SIDE_LENGTH; j++){
            if((node->state[i][j] != goal_state[i][j]) && (node->state[i][j] != 0)){
                dis += manhattan_dis(node, i, j);
            }
        }

        dis += (row_lc(node, i) + col_lc(node, i)) * 2;
    }
        
    return dis;
}


bool at_goal(nodes *node){
    int i, j;

    for (i = 0; i < SIDE_LENGTH; i++)
        for (j = 0; j < SIDE_LENGTH; j++)
            if ((node->state[i][j]) != goal_state[i][j]){
                return False;
            }

    return True;
}


nodes *get_start_state(){
    int  i, j, n=0;
    char check[8];  // just for store user choise
    nodes *node= malloc(sizeof(nodes));

    do {
        printf("Enter a VALID situation of a %d puzzle\n", SIDE_LENGTH*SIDE_LENGTH-1);
        printf("Use a zero to present the hole\n");
        printf("Example: ");
        for(i=0; i<SIDE_LENGTH; i++){
            for(j=0; j<SIDE_LENGTH; j++){
                printf(" %2d", goal_state[i][j]);
            }
            printf("\n\t ");
        }
        printf("\n\n");


        for(i=0; i<SIDE_LENGTH; i++){
            for(j=0; j<SIDE_LENGTH; j++){
                scanf("%d", &node->state[i][j]);
            }
        }
        fflush(stdin);

        printf("\nStarting configuration is: \n");
        for (i = 0; i < SIDE_LENGTH; i++){
            printf("     ");
            for (j = 0; j < SIDE_LENGTH; j++) printf(" %2d", node->state[i][j]);
            printf("\n");
        }
        printf("\nIs that correct? (y/n): ");
        gets(check);
    }while((check[0] != 'Y') && (check[0] != 'y'));


    for(i=0; i<SIDE_LENGTH; i++){
        for(j=0; j<SIDE_LENGTH; j++){
            if(node->state[i][j] == 0){
                node->hole_pos[0] = i;
                node->hole_pos[1] = j;
            }
            fprintf(fp, " %2d", node->state[i][j]);
        }
        fprintf(fp, "\n");
    }

    node->depth = 0;
    node->heuristic = heuristic_calculate(node);
    node->f = node->depth + node->heuristic;
    



    return node;
}



void expand_nodes(nodes *new_nodes[]){
    nodes *current_node = last_node->puzzle_node;
    nodes *new_puzzle_node;
    moves move;
    int row_of_0, column_of_0, next_r_of_0, next_c_of_0, prev_r_of_0, prev_c_of_0;
    int i, j, pos=0;

    if(last_node->previous_node == NULL){
        prev_r_of_0 = -1;
        prev_c_of_0 = -1;
    }
    else{
        prev_r_of_0 = last_node->previous_node->puzzle_node->hole_pos[0];
        prev_c_of_0 = last_node->previous_node->puzzle_node->hole_pos[1];
    }

    row_of_0 = current_node->hole_pos[0];
    column_of_0 = current_node->hole_pos[1];


    for(move = up; move <= right; move++){
        switch(move){
            case up:
            next_r_of_0 = row_of_0 - 1;
            next_c_of_0 = column_of_0;
            break;

            case down:
            next_r_of_0 = row_of_0 + 1;
            next_c_of_0 = column_of_0;
            break;

            case left:
            next_r_of_0 = row_of_0;
            next_c_of_0 = column_of_0 - 1;
            break;

            case right:
            next_r_of_0 = row_of_0;
            next_c_of_0 = column_of_0 + 1;
            break;
        }

        

        if((next_r_of_0 < 0) || (next_r_of_0 >= SIDE_LENGTH) || (next_c_of_0 < 0) || 
           (next_c_of_0 >= SIDE_LENGTH) || ((next_r_of_0 == prev_r_of_0) && (next_c_of_0 == prev_c_of_0))){
            continue;
        }

        new_puzzle_node = malloc(sizeof(nodes));

        total_nodes++;


        for(i=0; i<SIDE_LENGTH; i++){
            for(j=0; j<SIDE_LENGTH; j++){
                new_puzzle_node->state[i][j] = current_node->state[i][j];
            }
        }
        new_puzzle_node->hole_pos[0] = next_r_of_0;
        new_puzzle_node->hole_pos[1] = next_c_of_0;
        new_puzzle_node->state[row_of_0][column_of_0] = new_puzzle_node->state[next_r_of_0][next_c_of_0];
        new_puzzle_node->state[next_r_of_0][next_c_of_0] = 0;
        new_puzzle_node->move = move;
        new_puzzle_node->heuristic = current_node->heuristic - manhattan_dis(current_node, next_r_of_0, next_c_of_0) + manhattan_dis(new_puzzle_node, row_of_0, column_of_0);  
        if(new_puzzle_node->move == up || new_puzzle_node->move == down){
            new_puzzle_node->heuristic = new_puzzle_node->heuristic - (row_lc(current_node, row_of_0) + row_lc(current_node, next_r_of_0))*2
                                        + (row_lc(new_puzzle_node, row_of_0) + row_lc(new_puzzle_node, next_r_of_0))*2;
        }
        else{
            new_puzzle_node->heuristic = new_puzzle_node->heuristic - (col_lc(current_node, column_of_0) + col_lc(current_node, next_c_of_0))*2
                                        + (col_lc(new_puzzle_node, column_of_0) + col_lc(new_puzzle_node, next_c_of_0))*2;
        }
        
        new_puzzle_node->depth = current_node->depth + 1;
        new_puzzle_node->f = new_puzzle_node->heuristic + new_puzzle_node->depth;

        new_nodes[pos] = new_puzzle_node;
        pos++;
    }
    bubble_sort(new_nodes, pos);
}


int search(int threshold){
    static int t=0;

    nodes *current = last_node->puzzle_node;
    nodes *new_nodes[NEW_NODES_ARR_SIZE]={NULL};
    int min, temp, i, j;

    if(current->f > threshold){
        return current->f;
    }
    if(at_goal(current) == True){
        found_goal_state = True;
        return -1;
    }

    min = infinite;

    expand_nodes(new_nodes);

    for(i=0; i<NEW_NODES_ARR_SIZE; i++){
        if(new_nodes[i] == NULL) break;;
        if(is_in_list(new_nodes[i]) == True){
            free(new_nodes[i]);
            continue;
        }

        list_append(new_nodes[i]);
        temp = search(threshold);
        if(found_goal_state == True) return -1;
        if(temp < min) min = temp;
        
        list_pop();
        free(new_nodes[i]);
    }

    return min;
}


void ida_star(nodes *current_node){
    int threshold = start_state->heuristic;
    int temp, t=0;

    if(at_goal(current_node) == True){
        found_goal_state = True;
        return;
    }
    list_append(current_node);

    while(1){
        temp = search(threshold);
        if(found_goal_state == True) return;
        else if(temp == infinite){
            printf("Not found\n");
            return;
        }
        threshold = temp;

        t++;
        total_nodes = 0;
    }
}



void report_solution(list *cur_node){
    if(cur_node->previous_node != NULL){
        report_solution(cur_node->previous_node);
        move_count++;
        switch (cur_node->puzzle_node->move){
            case up:
            printf("%2d    Move a tile down  | (= space up   )\n", move_count);
            fprintf(fp, "%2d    Move a tile down  | (= space up   )\n", move_count);
            break;
            
            case down:
            printf("%2d    Move a tile up    | (= space down )\n", move_count);
            fprintf(fp, "%2d    Move a tile up    | (= space down )\n", move_count);
            break;
            
            case left:
            printf("%2d    Move a tile right | (= space left )\n", move_count);
            fprintf(fp, "%2d    Move a tile right | (= space left )\n", move_count);
            break;
            
            case right:
            printf("%2d    Move a tile left  | (= space right)\n", move_count);
            fprintf(fp, "%2d    Move a tile left  | (= space right)\n", move_count);
            break;

            default:
            printf("      No moves required\n");
            fprintf(fp, "      No moves required\n");
            break;
        }
    }
}

int main(){
    nodes *current_node, *goal_node;
    clock_t begin, end;
    double total_time;
    fp = fopen("4by4_test.txt", "w");
    fprintf(fp, "The initial state:\n");

    set_goal_state();
    start_state = get_start_state();
    current_node = start_state;

    begin = clock();

    ida_star(current_node);

    end = clock();
    total_time = (double)(end - begin) / CLOCKS_PER_SEC;

    if(found_goal_state == True){
        printf("\n\nSolution found (%2d moves):\n", last_node->puzzle_node->depth);
        fprintf(fp, "\n\nSolution found (%2d moves):\n", last_node->puzzle_node->depth);
        report_solution(last_node);
        
    }
    else printf("not found");

    printf("\nDone!\n");
    fprintf(fp, "\nDone!\n");
    printf("\ntime used: %.3lf sec", total_time);
    fprintf(fp, "\ntime used: %.3lf sec", total_time);

    fclose(fp);

    return 0;
}
