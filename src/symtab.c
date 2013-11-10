/*
 * Functions of Symbolic Table
 * Author: Yu Zhang (yuzhang@ustc.edu.cn)
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <common.h>

extern Type Int,Float,Bool,Void;
/**
 * Creates a symbolic table
 */
Table
newTable()
{
	Table new;
	NEW0(new);
	new->top=1;
	return new;
}

static void
destroyBucket(Entry *list)
{
	Entry node = *list, temp;
	while ( node != NULL ) {
		temp = node;
		node = temp->next;
		free(temp);
	}
	*list = NULL;
}

/**
 * Destroys the specified table
 */
void
destroyTable(Table *tab)
{
	int i=0;
	Entry *bucket = (*tab)->buckets, *bucket_end = (*tab)->buckets+256;
	while (bucket < bucket_end ) {
		destroyBucket(bucket);
		bucket++;
	}
	free(*tab);
	*tab = NULL;
}

Symbol
checkSym(Table ptab,const char *name)
{
	Entry pent;
	unsigned hashkey = (unsigned long)name[0] & (HASHSIZE-1);
	int lim=ptab->base[ptab->top-1];
//	printf("CheckSymBase:%d\n",lim);
	for (pent = ptab->buckets[hashkey]; (pent!=NULL)&&((pent->sym.id)>lim); pent = pent->next)
		if ( strcmp(name, pent->sym.name) == 0 ) 
		{
			printf("Re-Define Detail: '%s' (%ld)\n",pent->sym.name,pent->sym.id);
			return &pent->sym;
		}
	return NULL;
}

// Look up the symbolic table to get the symbol with specified name
Symbol
lookup(Table ptab, const char *name)
{
//	printf("look for ID : '%s'\n",name);
	Entry pent;
	unsigned hashkey = (unsigned long)name[0] & (HASHSIZE-1);
	for (pent = ptab->buckets[hashkey]; pent!=NULL; pent = pent->next)
	{
//		printf("looked ID : '%s' id =%ld ",pent->sym.name,pent->sym.id);
		if ( strcmp(name, pent->sym.name) == 0 ) 
//		{
//			printf(", match!\n");
			return &pent->sym;
//		}
//		else printf(", but not match\n");
		
	}
//	printf("Never match.Lookup failed.\n");
	return NULL;
}

Symbol
getSym(Table ptab, const char *name)
{
	Entry pent;
	unsigned hashkey = (unsigned long)name[0] & (HASHSIZE-1);
	NEW0(pent);
	pent->sym.name = (char *)name;
//	pent->sym.val = 0;
	pent->sym.isInitial = FALSE;
	pent->next = ptab->buckets[hashkey];
	pent->sym.id=++(ptab->base[ptab->top]);
	pent->sym.level=ptab->top-1;
	ptab->buckets[hashkey] = pent;
	Fakentry sp;
	NEW0(sp);
	sp->enode=pent;
	sp->next=ptab->index;
	ptab->index=sp;
	return &pent->sym;
}

void
PushTable(Table ptab)
{
//	printf("PushTable\n");
	ptab->top++;
	ptab->base[ptab->top]=ptab->base[ptab->top-1];
}

void
PopTable(Table ptab)
{
//	printf("PopTable\n");
	Fakentry sp=ptab->index;
	Entry pent;
	unsigned hashkey;	
	int N,i;
	i=ptab->base[ptab->top];
	ptab->top--;
	N=ptab->base[ptab->top];
	while (i>N){
		pent=sp->enode;
		hashkey = (unsigned long)((pent->sym.name)[0]) & (HASHSIZE-1);
		ptab->buckets[hashkey]=pent->next;
//		free(pent);
		ptab->index=sp->next;
		free(sp);
		sp=ptab->index;
		i--;
	}
}

