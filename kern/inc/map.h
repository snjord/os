
#ifndef MAP_H
#define MAP_H

#define MAP_ELEMENTS    200

#include "defs.h"

class AddressHash
{
public:
    AddressHash(void) : hashConst(0x4ad45571758e27d3)
    { }

    unsigned int operator()(addr_t address)
    {
        // multiply by a 64-bit odd number with roughly half its bits set,
        // byte swap, multiply by the odd number again, byte swap again,
        // truncate

        unsigned long long temp = address * hashConst;

        asm volatile (
            "bswap  %0\n\t"
            : "=r" (temp)
            : "r"  (temp)
            : 
        );

        temp *= hashConst;

        asm volatile (
            "bswap  %0\n\t"
            : "=r" (temp)
            : "r"  (temp)
            : 
        );

        return static_cast<unsigned int>(temp);
    }

private:
    unsigned long long const hashConst;
};

template <typename K, typename V>
struct mapentry
{
    mapentry(K key, V value) : key(key), value(value), next(NULL)
    { }

    K key;
    V value;

    struct mapentry *next;
};

template <typename K, typename V, typename HashFunction>
class map
{
public:
    map(void)
    {
        zero(m_Table, sizeof(mapentry<K, V> *) * MAP_ELEMENTS);
    }

    virtual ~map(void)
    {
    }

    bool contains(K const &key)
    {
        unsigned int idxTable = m_Hasher(key) % MAP_ELEMENTS;
        bool found = false;

        mapentry<K, V> *curr = m_Table[idxTable];

        while (curr != NULL)
        {
            if (curr->key == key)
            {
                found = true;

                break;
            }

            curr = curr->next;
        }

        return found;
    }

    V& operator[](K const &key)
    {
        unsigned int idxTable = m_Hasher(key) % MAP_ELEMENTS;
        V *found;

        mapentry<K, V> *curr = m_Table[idxTable];

        if (curr == NULL)
        {
            // unknown key, no collision

            m_Table[idxTable] = new mapentry<K, V>(key, V());

            found = &m_Table[idxTable]->value;
        }
        else
        {
            while (curr->next != NULL && curr->key != key)
            {
                curr = curr->next;
            }

            if (curr->key == key)
            {
                // found key

                found = &curr->value;
            }
            else
            {
                // unknown key, collision

                curr->next = new mapentry<K, V>(key, V());

                found = &curr->next->value;
            }
        }

        return *found;
    }

private:
    mapentry<K, V> *m_Table[MAP_ELEMENTS];
    HashFunction    m_Hasher;
};

#endif

