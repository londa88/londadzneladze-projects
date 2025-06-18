#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn){
			assert(elemSize>0);
			assert(numBuckets>0);
			assert(hashfn!=NULL&&comparefn!=NULL);
			h->numBuckets=numBuckets;
			h->comparefn=comparefn;
			h->hashfn=hashfn;
			h->size=0;
			h->buckets=malloc(numBuckets*sizeof(vector));
			assert(h->buckets != NULL);
			for(int i=0; i<numBuckets; i++){
				VectorNew(&h->buckets[i], elemSize, freefn, 4);
			}
}

void HashSetDispose(hashset *h){
	for(int i=0; i<h->numBuckets; i++){
		VectorDispose(&h->buckets[i]);
	}
	free(h->buckets);
}

int HashSetCount(const hashset *h){ 
	return h->size;
 }

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData){
	assert(mapfn!=NULL);
	for(int i=0; i<h->numBuckets; i++){
		VectorMap(&h->buckets[i], mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr){
	assert(elemAddr!=NULL);
	int code=h->hashfn(elemAddr, h->numBuckets);
	assert(code>=0&&code<h->numBuckets);
	int pos=VectorSearch(&h->buckets[code],elemAddr, h->comparefn, 0, false);
	if(pos==-1){
		VectorAppend(&h->buckets[code], elemAddr);
		h->size++;
	}
	else{
		VectorReplace(&h->buckets[code], elemAddr, pos);
	}

}

void *HashSetLookup(const hashset *h, const void *elemAddr){
	assert(elemAddr!=NULL);
	int code=h->hashfn(elemAddr, h->numBuckets);
	assert(code>=0&&code<h->numBuckets);
	int pos=VectorSearch(&h->buckets[code],elemAddr, h->comparefn, 0, false);
	if(pos==-1) return NULL;
	return VectorNth(&h->buckets[code], pos);
} 
