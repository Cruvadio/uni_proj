#include "rational.h"
#include "exception.h"
#include "vector.h"

template<class T> unsigned short Node<T>::return_height (Node<T>* p)
{
    if (!p) return 0;
    unsigned short lh = return_height(p->left);
    unsigned short rh = return_height(p->right);
    
    return ((lh > rh) ? lh : rh) + 1;
}

template<class T> int Node<T>::balance_factor(Node<T>* p)
{
    return (return_height(p->right) - return_height(p->left));
}

template <class T> Node<T>* Node<T>::rotate_right (Node<T>* p)
{
    Node<T>* q = p->left;
    p->left = q->right;
    q->right = p;

    return q;
}

template <class T> Node<T>* Node<T>::rotate_left(Node *q)
{
    Node<T>* p = q->right;
    q->right = p->left;
    p->left = q;

    return p;
}

template <class T> Node<T>* Node<T>::balance_tree(Node<T>* p)
{
    if (balance_factor(p) == 2)
    {
        if (balance_factor(p->right) < 0)
            p->right = rotate_right(p->right);
        return rotate_left(p);
    }

    if (balance_factor(p) == -2)
    {
        if (balance_factor(p->left) > 0)
            p->left = rotate_left(p->left);
        return rotate_right(p);
    }
    return p;
}

template <class T> Node<T>* Node<T>::insert(unsigned int k,Node<T>* p, const T& value)
{
    if (!p) return new Node<T>(k, value);
    if ( k < p->key )
        p->left = insert(p->left, k, value);
    else if (k > p->key)
        p->right = insert(p->right, k, value);
    else
        p->value = value;

    return balance_tree(p);
}

template <class T> Node<T>* Node<T>::find_min (Node<T>* p)
{
    return p->left ? find_min(p->left) : p->right;
}

template <class T> Node<T>* Node<T>::remove_min (Node<T>* p)
{
    if (!p->left) return p->right;
    p->left = remove_min(p->left);
    return balance_tree(p);
}

template <class T> Node<T>* Node<T>::copy(Node<T>* p, Node<T>* q)
{
    p = insert(p, q->key, q->value);
    p = copy(p, q->left);
    p = copy(p, q->right);
    return p;
}

template <class T> Node<T>* Node<T>::remove(unsigned int k, Node<T>* p)
{
    if (!p) return 0;
    if (k < p->key)
        p->left = remove(k, p->left);
    else if (k > p->key)
        p->right = remove(k, p->right);
    else
    {
        Node<T>* q = p->left;
        Node<T>* r = p->right;

        delete p;

        if (!r) return q;
        Node<T>* min = find_min(r);
        min->right = remove_min(r);
        min->left = q;

        return balance_tree(min);
    }
    return balance_tree(p);
}


