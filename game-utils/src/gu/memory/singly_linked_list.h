#pragma once

template <typename T>
struct SinglyLinkedList {
    struct Node {
        T Data;
        Node *Next;
    };

    Node *Head = 0;

    void insert(Node *previousNode, Node *newNode) {
        if (previousNode == 0) {
            if (Head) {
                // The list has more elements
                newNode->Next = Head;
            } else {
                newNode->Next = 0;
            }
            Head = newNode;
        } else {
            if (!previousNode->Next) {
                // Is the last node
                previousNode->Next = newNode;
                newNode->Next = 0;
            } else {
                // Is a middle node
                newNode->Next = previousNode->Next;
                previousNode->Next = newNode;
            }
        }
    }

    void remove(Node *previousNode, Node *deleteNode) {
        if (!previousNode) {
            if (deleteNode->Next) {
                // List has more elements
                Head = deleteNode->Next;
            } else {
                // List only has one element
                Head = 0;
            }
        } else {
            previousNode->Next = deleteNode->Next;
        }
    }
};
