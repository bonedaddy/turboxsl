#include "external_cache.h"

#include <string.h>
#include <libmemcached/memcached.h>

#include "logger.h"

typedef struct external_cache_entry {
    pthread_t thread;
    memcached_st *object;
    struct external_cache_entry *next_entry;
} external_cache_entry;

struct external_cache_ {
    char *server_list;
    struct external_cache_entry *entry;
};

external_cache *external_cache_create(char *server_list)
{
    external_cache *cache = malloc(sizeof(external_cache));
    if (cache == NULL)
    {
        error("external_cache_create:: create");
        return NULL;
    }
    memset(cache, 0, sizeof(external_cache));
    cache->server_list = server_list;

    return cache;
}

void external_cache_add_client(external_cache *cache, pthread_t thread)
{
    external_cache_entry *entry = malloc(sizeof(external_cache_entry));
    if (entry == NULL)
    {
        error("external_cache_add_client:: create");
        return;
    }
    memset(entry, 0, sizeof(external_cache_entry));

    entry->object = memcached(cache->server_list, strlen(cache->server_list));
    if (entry->object == NULL)
    {
        error("external_cache_add_client:: memcached");
        free(entry);
        return;
    }

    memcached_return_t r = memcached_behavior_set(entry->object, MEMCACHED_BEHAVIOR_USE_UDP, 0);
    if (r != MEMCACHED_SUCCESS)
    {
        error("external_cache_add_client:: UDP: %s", memcached_strerror(entry->object, r));
        memcached_free(entry->object);
        free(entry);
        return;
    }

    if (cache->entry == NULL)
    {
        cache->entry = entry;
    }
    else
    {
        external_cache_entry *t = cache->entry;
        while (t->next_entry != NULL) t = t->next_entry;
        t->next_entry = entry;
    }
}

void external_cache_release(external_cache *cache)
{
    external_cache_entry *current = cache->entry;
    while (current != NULL)
    {
        external_cache_entry *next = current->next_entry;
        memcached_free(current->object);
        free(current);
        current = next;
    }
    free(cache);
}

int external_cache_set(external_cache *cache, char *key, char *value)
{
    if (cache == NULL) return 0;

    pthread_t self = pthread_self();
    external_cache_entry *t = cache->entry;
    while (t != NULL && t->thread != self) t = t->next_entry;
    if (t == NULL || t->thread != self)
    {
        error("external_cache_set:: unknown thread");
        return NULL;
    }

    size_t key_length = strlen(key);
    size_t value_length = strlen(value);
    memcached_return_t r = memcached_set(t->object, key, key_length, value, value_length, (time_t)0, 0);
    if (r != MEMCACHED_SUCCESS)
    {
        error("external_cache_set:: %s", memcached_strerror(t->object, r));
        return 0;
    }
    return 1;
}

char *external_cache_get(external_cache *cache, char *key)
{
    if (cache == NULL) return NULL;

    pthread_t self = pthread_self();
    external_cache_entry *t = cache->entry;
    while (t != NULL && t->thread != self) t = t->next_entry;
    if (t == NULL || t->thread != self)
    {
        error("memory_cache_allocate:: unknown thread");
        return NULL;
    }

    size_t key_length = strlen(key);
    size_t value_length = 0;
    uint32_t flags = 0;
    memcached_return_t r;
    char *value = memcached_get(t->object, key, key_length, &value_length, &flags, &r);
    if (value == NULL)
    {
        error("memory_cache_allocate:: %s", memcached_strerror(t->object, r));
        return NULL;
    }
    return value;
}