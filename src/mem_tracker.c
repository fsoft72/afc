#include <limits.h>
#include <stdint.h>
#include "mem_tracker.h"

static int _memtrack_realloc(MemTracker *mt);
static int _memtrack_realloc_free(MemTracker *mt);
static void _memtrack_add(MemTracker *mt, MemTrackData *hd);
static int _memtrack_find(MemTracker *mt, void *mem);
static void _memtrack_del(MemTracker *mt, int pos);
static void _free_item(MemTracker *mt, MemTrackData *d);

/* _memtrack_hash_index - compute hash bucket index from a pointer */
static unsigned int _memtrack_hash_index(void *ptr)
{
	uintptr_t v = (uintptr_t)ptr;
	v = (v >> 4) ^ (v >> 16);
	return (unsigned int)(v & (AFC_MEMTRACK_HASH_SIZE - 1));
}

/* _memtrack_hash_insert - insert entry into the hash table */
static void _memtrack_hash_insert(MemTracker *mt, MemTrackData *hd)
{
	unsigned int idx = _memtrack_hash_index(hd->mem);
	hd->hash_next = mt->hash_table[idx];
	mt->hash_table[idx] = hd;
}

/* _memtrack_hash_remove - remove entry from the hash table */
static void _memtrack_hash_remove(MemTracker *mt, MemTrackData *hd)
{
	unsigned int idx = _memtrack_hash_index(hd->mem);
	MemTrackData **pp = &mt->hash_table[idx];

	while (*pp)
	{
		if (*pp == hd)
		{
			*pp = hd->hash_next;
			hd->hash_next = NULL;
			return;
		}
		pp = &(*pp)->hash_next;
	}
}

/* _memtrack_hash_find - O(1) lookup by pointer in the hash table */
static MemTrackData *_memtrack_hash_find(MemTracker *mt, void *mem)
{
	unsigned int idx = _memtrack_hash_index(mem);
	MemTrackData *hd = mt->hash_table[idx];

	while (hd)
	{
		if (hd->mem == mem)
			return hd;
		hd = hd->hash_next;
	}

	return NULL;
}

MemTracker *afc_mem_tracker_new()
{
	MemTracker *mt = malloc(sizeof(MemTracker));
	if (mt == NULL)
		return NULL;

	mt->data_max = 200;
	mt->data_cur = 0;
	mt->data = malloc(mt->data_max * sizeof(MemTrackData *));
	if (mt->data == NULL)
	{
		free(mt);
		return NULL;
	}
	memset(mt->data, 0, mt->data_max * sizeof(MemTrackData *));

	mt->free_cur = -1;
	mt->free_max = 100;
	mt->free = malloc(mt->free_max * sizeof(unsigned int));
	if (mt->free == NULL)
	{
		free(mt->data);
		free(mt);
		return NULL;
	}

	mt->show_mallocs = FALSE;
	mt->show_frees = FALSE;
	mt->allocs = 0;
	mt->frees = 0;
	mt->alloc_bytes = 0;

	memset(mt->hash_table, 0, sizeof(mt->hash_table));

#ifndef MINGW
	pthread_mutex_init(&mt->mutex, NULL);
#endif

	return mt;
}

void _afc_mem_tracker_delete(MemTracker *mt)
{
	unsigned int t;
	MemTrackData *hd;

	_afc_dprintf("%s::%s\n", __FILE__, __FUNCTION__);

	for (t = 0; t < mt->data_cur; t++)
	{
		hd = mt->data[t];
		if (!hd)
			continue;
		printf("LEAK: file: %s - func: %s - line: %d - size: %d\n", hd->file, hd->func, hd->line, (int)hd->size);

		_free_item(mt, hd);
	}

	free(mt->data);
	free(mt->free);
#ifndef MINGW
	pthread_mutex_destroy(&mt->mutex);
#endif
	free(mt);
}

void *afc_mem_tracker_malloc(MemTracker *mt, size_t size, const char *file, const char *func, const unsigned int line)
{
	MemTrackData *hd;
	void *mem;

	if ((mem = malloc(size)) == NULL)
		return NULL;

#ifndef MINGW
	pthread_mutex_lock(&mt->mutex);
#endif

	if ((__internal_afc_base->start_log_level >= AFC_LOG_NOTICE) && (mt->show_mallocs))
		_afc_dprintf("NOTICE: MemTracker: alloc %p (%d)\n", mem, (int)size);

	hd = (MemTrackData *)malloc(sizeof(MemTrackData));

	if (hd == NULL)
	{
		free(mem);
#ifndef MINGW
		pthread_mutex_unlock(&mt->mutex);
#endif
		return NULL;
	}

	hd->mem = mem;
	hd->size = size;
	hd->file = file;  /* Store pointer directly — file/func are compile-time literals */
	hd->func = func;
	hd->line = line;
	hd->hash_next = NULL;

	mt->allocs++;
	mt->alloc_bytes += size;

	_memtrack_add(mt, hd);
	_memtrack_hash_insert(mt, hd);

#ifndef MINGW
	pthread_mutex_unlock(&mt->mutex);
#endif

	return mem;
}

// {{{ int afc_mem_tracker_free ( MemTracker * hm, void * mem )
/*
@node afc_mem_tracker_free

			 NAME: afc_mem_tracker_free ( mem_tracker, mem )  - Frees a memory block

		 SYNOPSIS: int afc_mem_tracker_add ( MemTracker * mem_tracker, void * mem )

	  DESCRIPTION: This function frees a memory block releasing it to the system.

			INPUT: - mem_tracker  - Pointer to a valid afc_mem_tracker class.
			- mem		- Memory block to be freed

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_mem_tracker_malloc()
@endnode
*/
void _afc_mem_tracker_free(MemTracker *mt, void *mem, const char *file, const char *func, const unsigned int line)
{
	MemTrackData *hd;

	if (mem == NULL)
		return;

#ifndef MINGW
	pthread_mutex_lock(&mt->mutex);
#endif

	if ((__internal_afc_base->start_log_level >= AFC_LOG_NOTICE) && (mt->show_frees))
		_afc_dprintf("NOTICE: MemTracker: free %p\n", mem);

	hd = _memtrack_hash_find(mt, mem);
	if (hd)
	{
		int pos = _memtrack_find(mt, mem);
		mt->alloc_bytes -= hd->size;
		mt->frees++;

		_memtrack_hash_remove(mt, hd);
		_free_item(mt, hd);
		if (pos != -1)
			_memtrack_del(mt, pos);
	}
	else
	{
		_afc_dprintf("%s::%s invalid memory pointer: %p at: %s::%s (%d)\n", __FILE__, __FUNCTION__, mem, file, func, line);
	}

#ifndef MINGW
	pthread_mutex_unlock(&mt->mutex);
#endif
}
// }}}

// {{{ int afc_mem_tracker_update_size ( MemTracker * hm, void * mem, size_t size )
/*
@node afc_mem_tracker_update_size

			 NAME: afc_mem_tracker_update_size ( mem_tracker, mem, new_mem, size )  - Updates a tracked allocation

		 SYNOPSIS: int afc_mem_tracker_update_size ( MemTracker * mem_tracker, void * mem, size_t size )

	  DESCRIPTION: This function updates the tracked pointer and size after a realloc.

			INPUT: - mem_tracker  - Pointer to a valid afc_mem_tracker class.
			- mem		- Original memory block pointer
			- new_mem	- New memory block pointer after realloc
			- size		- New size of memory

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_mem_tracker_malloc()
@endnode
*/
void _afc_mem_tracker_update_size(MemTracker *mt, void *mem, void *new_mem, size_t size, const char *file, const char *func, const unsigned int line)
{
	MemTrackData *hd;

	if (mem == NULL)
		return;

#ifndef MINGW
	pthread_mutex_lock(&mt->mutex);
#endif

	hd = _memtrack_hash_find(mt, mem);
	if (hd)
	{
		_memtrack_hash_remove(mt, hd);
		mt->alloc_bytes -= hd->size;

		hd->size = size;
		if (new_mem != NULL)
			hd->mem = new_mem;

		mt->alloc_bytes += hd->size;
		_memtrack_hash_insert(mt, hd);
	}
	else
	{
		_afc_dprintf("%s::%s invalid memory pointer: %p at: %s::%s (%d)\n", __FILE__, __FUNCTION__, mem, file, func, line);
	}

#ifndef MINGW
	pthread_mutex_unlock(&mt->mutex);
#endif
}
// }}}

// {{{ _memtrack_add ( mt, hd )
static void _memtrack_add(MemTracker *mt, MemTrackData *hd)
{
	unsigned int pos;

	if (mt->free_cur != -1)
		pos = mt->free[mt->free_cur--];
	else
		pos = mt->data_cur++;

	if (pos >= mt->data_max)
	{
		if (_memtrack_realloc(mt) != 0)
		{
			// Realloc failed - cannot track this allocation
			_afc_dprintf("WARNING: MemTracker: realloc failed, allocation not tracked\n");
			return;
		}
	}

	mt->data[pos] = hd;
}
// }}}
// {{{ _memtrack_find ( mt, mem )
static int _memtrack_find(MemTracker *mt, void *mem)
{
	unsigned int t;
	MemTrackData *hd;

	for (t = 0; t < mt->data_cur; t++)
	{
		hd = mt->data[t];
		if (!hd)
			continue;

		if (hd->mem == mem)
			return t;
	}

	return -1;
}
// }}}
// {{{ _memtrack_del ( mt, pos )
static void _memtrack_del(MemTracker *mt, int pos)
{
	int cur;

	cur = ++mt->free_cur;
	if (cur >= mt->free_max)
	{
		if (_memtrack_realloc_free(mt) != 0)
		{
			// Realloc failed - cannot track free slot
			_afc_dprintf("WARNING: MemTracker: realloc failed for free list\n");
			mt->free_cur--; // Undo increment
			// Still clear the data slot
			mt->data[pos] = NULL;
			return;
		}
	}

	mt->data[pos] = NULL;

	mt->free[cur] = pos;
}
// }}}

// {{{ _memtrack_realloc ( mt )
static int _memtrack_realloc(MemTracker *mt)
{
	unsigned int new_max;

	/* Check for integer overflow before doubling */
	if (mt->data_max > UINT_MAX / 2)
		return -1;

	new_max = mt->data_max * 2;

	MemTrackData **new_data = realloc(mt->data, (size_t)new_max * sizeof(MemTrackData *));
	if (new_data == NULL)
		return -1; // Realloc failed, original pointer still valid
	mt->data = new_data;
	mt->data_max = new_max;
	return 0;
}
// }}}
// {{{ _memtrack_realloc_free ( mt )
static int _memtrack_realloc_free(MemTracker *mt)
{
	int new_max;

	/* Check for integer overflow before doubling */
	if (mt->free_max > INT_MAX / 2)
		return -1;

	new_max = mt->free_max * 2;

	unsigned int *new_free = realloc(mt->free, (size_t)new_max * sizeof(unsigned int));
	if (new_free == NULL)
		return -1; // Realloc failed, original pointer still valid
	mt->free = new_free;
	mt->free_max = new_max;
	return 0;
}
// }}}
// {{{ _free_item ( mt, d )
static void _free_item(MemTracker *mt, MemTrackData *d)
{
	if (!d)
		return;

	/* file and func point to compile-time string literals — do not free them */
	if (d->mem)
		free(d->mem);
	d->mem = NULL;
	free(d);
}
// }}}

// {{{ _afc_mem_tracker_update_pointer ( mt, old_mem, new_mem )
void _afc_mem_tracker_update_pointer(MemTracker *mt, void *old_mem, void *new_mem)
{
	MemTrackData *hd;
	int pos;

	if (old_mem == NULL)
		return;

#ifndef MINGW
	pthread_mutex_lock(&mt->mutex);
#endif

	if ((pos = _memtrack_find(mt, old_mem)) != -1)
	{
		hd = mt->data[pos];
		hd->mem = new_mem;
	}

#ifndef MINGW
	pthread_mutex_unlock(&mt->mutex);
#endif
}
// }}}
