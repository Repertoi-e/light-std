#pragma once

template <typename T>
struct DoublyLinkedList {
    struct Node {
        T Data;
        Node *Previous;
        Node *Next;
    };

    Node *Head = null;

    template <typename T>
    inline void insert(Node *previousNode, Node *newNode) {
        if (!previousNode) {
            if (head) {
                // The list has more elements
                newNode->Next = Head;
                newNode->Next->Previous = newNode;
            } else {
                newNode->Next = null;
            }
            Head = newNode;
            Head->Previous = null;
        } else {
            if (previousNode->Next) {
                // Middle node
                newNode->Next = previousNode->Next;
                if (newNode->Next) {
                    newNode->Next->Previous = newNode;
                }
                previousNode->Next = newNode;
                newNode->Previous = previousNode;
            } else {
                // Last node
                previousNode->Next = newNode;
                newNode->Next = null;
            }
        }
    }

    template <typename T>
    void remove(Node *deleteNode) {
        if (deleteNode->Previous == 0) {
            if (deleteNode->Next == 0) {
                // List only has one element
                Head = 0;
            } else {
                // List has more elements
                head = deleteNode->Next;
                head->Previous = 0;
            }
        } else {
            if (deleteNode->Next) {
                // Middle node
                deleteNode->Previous->Next = deleteNode->Next;
                deleteNode->Next->Previous = deleteNode->Previous;
            } else {
                // Is the last node
                deleteNode->Previous->Next = 0;
            }
        }
    }
};
