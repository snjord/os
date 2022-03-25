
#ifndef LIST_H
#define LIST_H

template <typename T>
class listentry
{
public:
    listentry(T &data) : next(NULL), prev(NULL), data(data)
    { }

    listentry *next;
    listentry *prev;

    T data;
};

template <typename T>
class forward_iterator
{
public:
    forward_iterator(void) : curr(NULL)
    { }

    forward_iterator(listentry<T> &start) : curr(&start)
    { }

    forward_iterator(forward_iterator const &copy) : curr(copy.curr)
    { }

    forward_iterator& operator++(void)
    {
        curr = curr->next;

        return *this;
    }

    forward_iterator operator++(int)
    {
        forward_iterator before(*this);

        curr = curr->next;

        return before;
    }

    T& operator*(void)
    {
        return curr->data;
    }

    T* operator->(void)
    {
        return &curr->data;
    }

    bool operator==(forward_iterator const &other)
    {
        return curr == other.curr;
    }

    bool operator!=(forward_iterator const &other)
    {
        return curr != other.curr;
    }

private:
    listentry<T> *curr;
};

template <typename T>
class list
{
public:
    list(void) : head(NULL), tail(NULL)
    { }

    virtual ~list(void)
    { }

    void push_back(T data)
    {
        listentry<T> *node = new listentry<T>(data);

        if (head == NULL)
        {
            head = tail = node;

            return;
        }

        node->prev = tail;
        tail->next = node;
        tail = node;

        return;
    }

    void push_front(T data)
    {
        listentry<T> *node = new listentry<T>(data);

        if (head == NULL)
        {
            head = tail = node;

            return;
        }

        node->next = head;
        head->prev = node;
        head = node;

        return;
    }

    T* front(void) const
    {
        if (head == NULL)
        {
            return NULL;
        }

        return &head->data;
    }

    T* back(void) const
    {
        if (head == NULL)
        {
            return NULL;
        }

        return &tail->data;
    }

    forward_iterator<T> begin(void)
    {
        if (head == NULL)
        {
            return end();
        }

        return forward_iterator<T>(*head);
    }

    forward_iterator<T>& end(void)
    {
        return end_iter;
    }

private:
    listentry<T> *head;
    listentry<T> *tail;

    forward_iterator<T> end_iter;
};

#endif

