#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    int size;
} LinkedList;

void ll_init(LinkedList *list);
void ll_insert_front(LinkedList *list, int value);
void ll_insert_back(LinkedList *list, int value);
int  ll_find(const LinkedList *list, int value);       /* index ou -1 */
int  ll_get(const LinkedList *list, int index);
void ll_remove_front(LinkedList *list);
void ll_free_collection(LinkedList *list);

#endif /* LINKED_LIST_H */
