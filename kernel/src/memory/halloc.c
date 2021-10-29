#include "halloc.h"
#include "buddy_alloc.h"
#include <vga_basic/vga.h>

#define ALLOC_OFF(a, b) ((alloc_header_t*)((uint8_t*)(a) + (b)))
#define ALLOC_OFF_PREV(a, b) ((alloc_header_t*)((uint8_t*)(a) - (b)))
#define GETOFF(a, b) ((uint32_t)((uint8_t*)(a) - (uint8_t*)b))
#define WITHIN_BLOCK(a, b, o) ((size_t)(a) + (a)->size <= (size_t)(b) + (b)->size * HALLOC_BLOCK - (o))

/*
 * the halloc haiku:
 * brave soul to look at
 * a cursed allocator,
 * in need for rewrite
 */


block_header_t *halloc_first = NULL;


static void* get_aligned_blocks(uint64_t n) {
	
}

static void unget_aligned_blocks(void* ptr, uint64_t n) {
	
}

static void init_block(block_header_t *block, size_t blocksize) {
	*block = (block_header_t) {
		.next = NULL,
		.size = blocksize,
		.freelist = sizeof(block_header_t)
	};

	alloc_header_t *first_free = ALLOC_OFF(block, block->freelist);
	*first_free = (alloc_header_t) {
		.size = blocksize * HALLOC_BLOCK - sizeof(block_header_t),
		.next = 0,
		.bef = 0,
		.prev = 0,
		.meta = 0,
	};
}

static block_header_t *create_block(size_t bytes) {
		size_t blocksize = ((bytes + sizeof(block_header_t)) / HALLOC_BLOCK) + 1;
		block_header_t *tmp = get_aligned_blocks(blocksize);
		init_block(tmp, blocksize);
		return tmp;
}

static void freelist_insert(block_header_t *block, alloc_header_t *what, uint32_t sp) {
	alloc_header_t *placement;

	if (block->freelist == 0) {
		what->bef = 0;
		block->freelist = GETOFF(what, block);
		return;
	}

	if (sp != 0)
		placement = ALLOC_OFF(block, sp);
	else
		placement = ALLOC_OFF(block, block->freelist);
	
	alloc_header_t *prev = NULL;

	// place remaining before second largest
	while (placement) {
		// if remains fit here, insert then go out
		if (what->size >= placement->size) {
			if (prev == NULL) {
				block->freelist = GETOFF(what, block);
				what->bef = 0;
			} else {
				prev->next = GETOFF(what, block);
				what->bef = GETOFF(prev, block);
			}
			what->next = GETOFF(placement, block);
			placement->bef = GETOFF(what, block);
			break;
		}

		// if placement doesn't have next, then we place it here
		if (placement->next == 0) {
			placement->next = GETOFF(what, block);
			what->bef = GETOFF(placement, block);
			break;
		}

		// otherwise let's continue
		prev = placement;
		placement = ALLOC_OFF(block, placement->next);
	}
	
}

static alloc_header_t *split(block_header_t *block, alloc_header_t *cur, size_t size) {
		if (cur->meta & 1 || cur->size < size)
			return NULL;
			// FREE AND GOOD SIZE

		// remove the suitable choice from the freelist
		if (cur->bef == 0) {
			block->freelist = cur->next;
		}
		else {
			alloc_header_t *prev = ALLOC_OFF(block, cur->bef);
			prev->next = cur->next;
		}

		if (cur->size - size <= sizeof(alloc_header_t)) {
			size = cur->size;
		}
		else {
			alloc_header_t *remains = ALLOC_OFF(cur, size);
			*remains = (alloc_header_t) {
				.prev = size,
				.size = cur->size - size,
				.meta = 0,
			};

			freelist_insert(block, remains, cur->next);
		}

		cur->size = size;
		cur->next = 0;
		cur->meta |= 1;
		block->rc++;
		return cur;
}

alloc_header_t *march(block_header_t *block, size_t size) {
	//printf("beginning march at block %p (last = %p) and seeking for %lu\n", block, last, size);
	alloc_header_t *cur = ALLOC_OFF(block, block->freelist);
	uint64_t curoff = block->freelist;
	
	while (cur) {
		//printf("attempting to split cur (%p)\n", cur);
		alloc_header_t *h = split(block, cur, size);
		if (h) {
			//printf("found suitable alloc (%p) of size (%u), returning\n", h, h->size);
			return h;
		}

		if (cur->next == 0)
			break;
		cur = ALLOC_OFF(block, cur->next);
	}

	if (block->next == NULL) {
		block->next = create_block(size);
	}
	if (block->next) {
		return march(block->next, size);
	}
	return NULL;
}

void *malloc(size_t size) {
	return realloc(NULL, size);
}

void free(void *ptr) {
	block_header_t *block = halloc_first;
	block_header_t *pblock = NULL;
	while (block) {
		if ((size_t)block < (size_t)ptr && (size_t)ptr < (size_t)block + block->size * HALLOC_BLOCK) {
			break;
		}
		pblock = block;
		block = block->next;
	}

	if (block == NULL)
		return;

	alloc_header_t *subject = ALLOC_OFF_PREV(ptr, sizeof(alloc_header_t));

	if (block->freelist == 0) {
		block->freelist = GETOFF(subject, block);
		subject->meta = 0;
		return;
	}

	// attempt to back collate
	if (subject->prev) {
		alloc_header_t *prev = ALLOC_OFF_PREV(subject, subject->prev);

		if (!(prev->meta & 1)) {
			// PREV FREE -> COLLATE
			prev->size += subject->size;
			subject = prev;
			//printf("back collate\n");

			uint32_t befofprev = prev->bef;
			alloc_header_t *befofp = ALLOC_OFF(block, befofprev);
			uint32_t nextofprev = prev->next;
			alloc_header_t *nextofp = ALLOC_OFF(block, nextofprev);
			
			if (befofprev)
				befofp->next = nextofprev;
			if (nextofprev)
				nextofp->bef = befofprev;
		}
	}

	// and now attempt to front collate
	if (WITHIN_BLOCK(subject, block, sizeof(alloc_header_t))) {
		alloc_header_t *next = ALLOC_OFF(subject, subject->size);
		next->prev = subject->size;
		if (!(next->meta & 1)) {
			//printf("front collate\n");
			subject->size += next->size;
			if (WITHIN_BLOCK(next, block, sizeof(alloc_header_t))) {
				alloc_header_t *nextnext = ALLOC_OFF(next, next->size);
				nextnext->prev = subject->size;
			}

			uint32_t befofnext = next->bef;
			alloc_header_t *prevofn = ALLOC_OFF(block, befofnext);
			uint32_t nextofnext = next->next;
			alloc_header_t *nextofn = ALLOC_OFF(block, nextofnext);
			if (befofnext)
				prevofn->next = nextofnext;
			if (nextofnext)
				nextofn->bef = befofnext;
		}
	}

	subject->meta = 0;

	freelist_insert(block, subject, 0);

	block->rc--;
	if (block->rc == 0) {
		if (pblock)
			pblock->next = block->next;
		else
			halloc_first = NULL;
		unget_aligned_blocks(block, block->size);
	}
}

void *realloc(void *ptr, size_t size) {
	//printf("allocating %p, %lu\n", ptr, size);
	if (size == 0)
		return NULL;
	if (size > (uint32_t)-(sizeof(alloc_header_t) + sizeof(block_header_t)))
		return NULL;

	size_t realsize = size + sizeof(alloc_header_t);
	if (halloc_first == NULL) {
		halloc_first = create_block(realsize);
	}


	alloc_header_t *header = NULL;
	if (ptr == NULL) {
		//printf("ptr is null, marching\n");
		header = march(halloc_first, realsize);
	}
	else {
		/* UNIMPLEMENTED */
		header = ALLOC_OFF_PREV(ptr, sizeof(alloc_header_t));
		
		block_header_t *block = halloc_first;
		while (block) {
			if ((size_t)block < (size_t)ptr && (size_t)ptr < (size_t)block + block->size * HALLOC_BLOCK) {
				break;
			}
			block = block->next;
		}		

		if (block == NULL)
			return NULL;

		// printf("header->size(%u) size(%u)\n", header->size, size);
		if (header->size < realsize) {
			//printf("yes, it is smaller\n");
			if (WITHIN_BLOCK(header, block, sizeof(alloc_header_t))) {
				//printf("yes, it is within block\n");
				alloc_header_t *next = ALLOC_OFF(header, header->size);
				alloc_header_t *h = split(block, next, realsize - header->size);
				if (h) {
					header->size += h->size;
				}
				else {
					// printf("hallocing again\n");
					h = malloc(size);
					for (uint64_t i = 0; i < header->size - sizeof(alloc_header_t); i++)
						((uint8_t*)h)[i] = ((uint8_t*)(header+1))[i];
					free(header + 1);
					header = ALLOC_OFF_PREV(h, sizeof(alloc_header_t));
				}
			}
		}
		else if (header->size > realsize) {
			// shrink
			if (WITHIN_BLOCK(header, block, sizeof(alloc_header_t))) {
				// look for next to merge with
				alloc_header_t *next = ALLOC_OFF(header, header->size);
				if (!(next->meta & 1)) {
					// OK, next free. good.
					// now i need to make
					alloc_header_t newnexttmp = *next;
					alloc_header_t *newnext = ALLOC_OFF(header, realsize);
					*newnext = newnexttmp;
					newnext->size += header->size - realsize;
					if (WITHIN_BLOCK(newnext, block, sizeof(alloc_header_t))) {
						alloc_header_t *nextnext = ALLOC_OFF(newnext, newnext->size);
						nextnext->prev = newnext->size;
					}

					// set the new size
					header->size = realsize;
					goto success;
				}

			}
			// 2 cases remaining:
			// - when the new space created is too small for a header and no free space after: do nothing [OK]
			// - when the space creates enough space for a header create an alloc

			if (header->size - realsize > sizeof(alloc_header_t)) {
				alloc_header_t *new = ALLOC_OFF(header, realsize);
				*new = (alloc_header_t) {
					.size = header->size - realsize,
					.prev = realsize,
					.meta = 0,
				};

				freelist_insert(block, new, 0);
				header->size = realsize;
			}
		}
	}

success:
	//printf("header info: next(%u) size(%u) prev(%u) meta(%u)\n", header->next, header->size, header->prev, header->meta);
	//alloc_header_t *rest = ALLOC_OFF(header, header->size);
	//printf("rest info: next(%u) size(%u) prev(%u) meta(%u)\n", rest->next, rest->size, rest->prev, rest->meta);

	if (header == NULL)
		return NULL;

	return header + 1;
}
