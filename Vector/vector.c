#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation){
        assert(elemSize>0);
        assert(initialAllocation>=0);
        if(initialAllocation==0){
            initialAllocation=4;
        }
        v->elemSize=elemSize;
        v->allocLength=initialAllocation;
        v->freeFn=freeFn;
        v->curLength=0;
        v->ptr=malloc(v->allocLength*v->elemSize);
        assert(v->ptr!=NULL);
}

void VectorDispose(vector *v){
       if(v->freeFn!=NULL) {
    for(int i=0; i<v->curLength; i++){
       v->freeFn((char*)v->ptr+v->elemSize*i);
    }
       }
       free(v->ptr);
}

int VectorLength(const vector *v){ 
    return v->curLength;
}

void *VectorNth(const vector *v, int position){ 
    assert(position>=0&&position<v->curLength);
    return ((char*)v->ptr+v->elemSize*position); 
}

void VectorReplace(vector *v, const void *elemAddr, int position){
    assert(position>=0&&position<v->curLength);
    memcpy(((char*)v->ptr+v->elemSize*position), elemAddr, v->elemSize);

}

void VectorInsert(vector *v, const void *elemAddr, int position){
    assert(position>=0&&position<=v->curLength);
    if(v->curLength==v->allocLength){
      v->allocLength*=2;
      v->ptr=realloc(v->ptr, v->allocLength*v->elemSize);
      assert(v->ptr!=NULL);
    }

    for(int i=v->curLength; i>position; i--){
     memcpy(((char*)v->ptr+v->elemSize*i),((char*)v->ptr+v->elemSize*(i-1)), v->elemSize);
     }
     memcpy((char*)v->ptr+position*v->elemSize, elemAddr, v->elemSize);
     v->curLength++;
}

void VectorAppend(vector *v, const void *elemAddr){ 
    if(v->curLength==v->allocLength){
        v->allocLength*=2;
        v->ptr=realloc(v->ptr, v->allocLength*v->elemSize);
        assert(v->ptr!=NULL);
    }
    memcpy(((char*)v->ptr+v->elemSize*v->curLength),elemAddr, v->elemSize);
    v->curLength++;
}

void VectorDelete(vector *v, int position){
    assert(position>=0&&position<v->curLength);
    if(v->freeFn!=NULL){
        v->freeFn(((char*)v->ptr+position*v->elemSize));
    }
    if(position!=v->curLength-1){
        memmove(((char*)v->ptr+position*v->elemSize),(char*)((char*)v->ptr+position*v->elemSize)+v->elemSize,(v->curLength-position-1)*v->elemSize);
    }
    v->curLength--;
}

void VectorSort(vector *v, VectorCompareFunction compare){
    assert(compare!=NULL);
    qsort(v->ptr, v->curLength, v->elemSize, compare);

}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData){
    assert(mapFn!=NULL);
    for(int i=0; i<v->curLength; i++){
        mapFn((char*)v->ptr+i*v->elemSize, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted){
     assert(startIndex>=0&&startIndex<=v->curLength);
     assert(key!=NULL);
     assert(searchFn!=NULL);
     void* found=NULL;
     if(isSorted){
      found=bsearch(key, ((char*)v->ptr)+v->elemSize*startIndex, v->curLength-startIndex,v->elemSize, searchFn);
     }
     else{
        void* stAdd=((char*)v->ptr)+v->elemSize*startIndex;
        for(char* cur=(char*)stAdd; cur<((char*)v->ptr+v->elemSize*v->curLength); cur+=v->elemSize){
            if(searchFn(key, cur)==0){
                found=(void*)cur;
                break;
            }
        }
     }
     if(found==NULL) return kNotFound;
     return ((char*)found-(char*)v->ptr)/v->elemSize;
 } 
