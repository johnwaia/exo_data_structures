#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

static Node *ll_new_node(int value) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "LinkedList: echec de malloc\n");
        exit(EXIT_FAILURE);
    }
    node->value = value;
    node->next = NULL;
    return node;
}

void ll_init(LinkedList *list) {
    list->head = NULL;
    list->size = 0;
}

/* O(1) : le nouveau noeud devient la tete. */
void ll_insert_front(LinkedList *list, int value) {
    Node *node = ll_new_node(value);
    node->next = list->head;
    list->head = node;
    list->size++;
}

/*
 * O(n) : la structure imposee par l'enonce (head, size) ne conserve pas de
 * pointeur de queue. Il faut donc parcourir toute la liste pour atteindre
 * le dernier noeud. C'est precisement ce cout que le benchmark met en
 * evidence face au tableau dynamique (O(1) amorti).
 */
void ll_insert_back(LinkedList *list, int value) {
    Node *node = ll_new_node(value);
    if (list->head == NULL) {
        list->head = node;
    } else {
        Node *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = node;
    }
    list->size++;
}

/* O(n) : parcours lineaire. */
int ll_find(const LinkedList *list, int value) {
    Node *current = list->head;
    int index = 0;
    while (current != NULL) {
        if (current->value == value) {
            return index;
        }
        current = current->next;
        index++;
    }
    return -1;
}

/* O(n) : pas d'acces direct, il faut parcourir depuis la tete. */
int ll_get(const LinkedList *list, int index) {
    Node *current = list->head;
    int i;
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "LinkedList: index hors bornes\n");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < index; i++) {
        current = current->next;
    }
    return current->value;
}

/* O(1) : il suffit de decrocher la tete. */
void ll_remove_front(LinkedList *list) {
    Node *old_head;
    if (list->head == NULL) {
        return;
    }
    old_head = list->head;
    list->head = old_head->next;
    free(old_head);
    list->size--;
}

void ll_free_collection(LinkedList *list) {
    Node *current = list->head;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
}
