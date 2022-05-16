#include "mem_tracker.h"

static void _memtrack_realloc(MemTracker *mt);
static void _memtrack_realloc_free(MemTracker *mt);
static void _memtrack_add(MemTracker *mt, MemTrackData *hd);
static int _memtrack_find(MemTracker *mt, void *mem);
static void _memtrack_del(MemTracker *mt, int pos);
static void _free_item(MemTracker *mt, MemTrackData *d);

MemTracker *afc_mem_tracker_new()
{
	MemTracker *mt = malloc(sizeof(MemTracker));

	mt->data_max = 200;
	mt->data_cur = 0;
	mt->data = malloc(mt->data_max * sizeof(MemTrackData));
	memset(mt->data, 0, mt->data_max);

	mt->free_cur = -1;
	mt->free_max = 100;
	mt->free = malloc(mt->free_max * sizeof(unsigned int));

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
	free(mt);
}

void *afc_mem_tracker_malloc(MemTracker *mt, size_t size, const char *file, const char *func, const unsigned int line)
{
	MemTrackData *hd;
	void *mem;
	char *_file = NULL, *_func = NULL;

	if ((mem = malloc(size)) == NULL)
		return NULL;

	if ((__internal_afc_base->start_log_level >= AFC_LOG_NOTICE) && (mt->show_mallocs))
		_afc_dprintf("NOTICE: MemTracker: alloc %p (%d)\n", mem, (int)size);

	hd = (MemTrackData *)malloc(sizeof(MemTrackData));

	if (hd == NULL)
	{
		free(mem);
		return NULL;
	}

	if (file)
		_file = strdup(file);
	if (func)
		_func = strdup(func);

	hd->mem = mem;
	hd->size = size;
	hd->file = _file; // ( file ? strdup ( file ) : NULL );
	hd->func = _func; // ( func ? strdup ( func ) : NULL );
	hd->line = line;

	mt->allocs++;
	mt->alloc_bytes += size;

	_memtrack_add(mt, hd);

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
	int pos;

	if (mem == NULL)
		return;

	if ((__internal_afc_base->start_log_level >= AFC_LOG_NOTICE) && (mt->show_frees))
		_afc_dprintf("NOTICE: MemTracker: free %p\n", mem);

	if ((pos = _memtrack_find(mt, mem)) != -1)
	{
		hd = mt->data[pos];
		mt->alloc_bytes -= hd->size;
		mt->frees++;

		_free_item(mt, hd);
		_memtrack_del(mt, pos);
	}
	else
	{
		_afc_dprintf("%s::%s invalid memory pointer: %p at: %s::%s (%d)\n", __FILE__, __FUNCTION__, mem, file, func, line);
	}
}
// }}}

// {{{ int afc_mem_tracker_update_size ( MemTracker * hm, void * mem, size_t size )
/*
@node afc_mem_tracker_update_size

			 NAME: afc_mem_tracker_update_size ( mem_tracker, mem, new_mem, size )  - Frees a memory block

		 SYNOPSIS: int afc_mem_tracker_update_size ( MemTracker * mem_tracker, void * mem, size_t size )

	  DESCRIPTION: This function frees a memory block releasing it to the system.

			INPUT: - mem_tracker  - Pointer to a valid afc_mem_tracker class.
			- mem		- Memory block to be freed
			- size		- New size of memory

		  RESULTS: should be AFC_ERR_NO_ERROR

		 SEE ALSO: - afc_mem_tracker_malloc()
@endnode
*/
void _afc_mem_tracker_update_size(MemTracker *mt, void *mem, void *new_mem, size_t size, const char *file, const char *func, const unsigned int line)
{
	MemTrackData *hd;
	int pos;

	if (mem == NULL)
		return;

	if ((pos = _memtrack_find(mt, mem)) != -1)
	{
		hd = mt->data[pos];
		mt->alloc_bytes -= hd->size;

		hd->size = size;
		hd->mem = new_mem;

		mt->alloc_bytes += hd->size;
	}
	else
	{
		_afc_dprintf("%s::%s invalid memory pointer: %p at: %s::%s (%d)\n", __FILE__, __FUNCTION__, mem, file, func, line);
	}
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
		_memtrack_realloc(mt);

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
		_memtrack_realloc_free(mt);

	mt->data[pos] = NULL;

	mt->free[cur] = pos;
}
// }}}

// {{{ _memtrack_realloc ( mt )
static void _memtrack_realloc(MemTracker *mt)
{
	mt->data_max *= 2;
	mt->data = realloc(mt->data, mt->data_max * sizeof(MemTrackData));
}
// }}}
// {{{ _memtrack_realloc_free ( mt )
static void _memtrack_realloc_free(MemTracker *mt)
{
	mt->free_max *= 2;
	mt->free = realloc(mt->free, mt->free_max * sizeof(int));
}
// }}}
// {{{ _free_item ( mt, d )
static void _free_item(MemTracker *mt, MemTrackData *d)
{
	if (!d)
		return;

	if (d->func)
		free(d->func);
	if (d->file)
		free(d->file);
	if (d->mem)
		free(d->mem);
	d->mem = NULL;
	free(d);
}
// }}}

#ifdef TEST_CLASS
int main()
{
	int t;
	MemTracker *mt = memtrack_new();

	for (t = 0; t < 10; t++)
		memtrack_add(mt, t);

	for (t = 5; t > 0; t--)
		memtrack_del(mt, t);

	for (t = 5; t > 0; t--)
		memtrack_add(mt, 100 + t);

	return 0;
}
#endif
