#import "MKArray.h"

@implementation MKArray

-(id)init
{			
  if(self = [super init])
    {
      firstEntry = lastEntry = NULL;
      currentEntry = firstEntry;
      count = currentIndex = 0;
    }
  return self;
}

-(void)cleanEntry:(mkArrayEntry*)thisEntry
{
  switch (thisEntry->dataType)
    {
    case mkIDDataType: [thisEntry->object.idValue free];
      break;
    case mkBYTEARRAYDataType: free(thisEntry->object.byteArrayValue);
      break;
    default:  break;    
    }
}

-(mkByteArray*)createByteArray:(unsigned char[])aByteArray ofSize:(unsigned int)size;
{
  unsigned int i;
  mkByteArray *byteArray = (mkByteArray*)malloc(sizeof(*byteArray)+size);
  byteArray->size = size;
  for(i=0; i<size; i++) byteArray->data[i] = aByteArray[i];
  return byteArray;
}

-(void)addMKArrayObject:(mkArrayObject)arrayObject ofType:(enum mkDataType)type
{
  mkArrayEntry* thisEntry = (mkArrayEntry*) malloc(sizeof(*thisEntry));
  thisEntry->object = arrayObject; 
  thisEntry->previous = (mkArrayEntry*)lastEntry;
  thisEntry->dataType = type;
  if(lastEntry != NULL)lastEntry->next = thisEntry;
  thisEntry->next = (mkArrayEntry*)NULL;
  if(firstEntry == NULL) firstEntry = thisEntry;
  lastEntry = thisEntry;
  count++;
} 

-(void)addObject:(id)anObject 
{
  mkArrayObject arrayObject;
  arrayObject.idValue = anObject;
  [self addMKArrayObject: arrayObject ofType: mkIDDataType];
}

-(void)addByte:(unsigned char)aByte;
{
  mkArrayObject arrayObject;
  arrayObject.byteValue = aByte;
  [self addMKArrayObject: arrayObject ofType: mkBYTEDataType];
}

-(void)addInt:(int)anInt;
{
  mkArrayObject arrayObject;
  arrayObject.intValue = anInt;
  [self addMKArrayObject: arrayObject ofType: mkINTDataType];
}

-(void)addFloat:(float)aFloat;
{
  mkArrayObject arrayObject;
  arrayObject.floatValue = aFloat;
  [self addMKArrayObject: arrayObject ofType: mkFLOATDataType];
}

-(void)addByteArray:(unsigned char[])aByteArray ofSize:(unsigned int)size
{
  mkArrayObject arrayObject;
  arrayObject.byteArrayValue = [self createByteArray: aByteArray ofSize: size];
  [self addMKArrayObject: arrayObject ofType: mkBYTEARRAYDataType];
}

-(void)setMKArrayObject:(mkArrayObject)arrayObject ofType:(enum mkDataType)type atIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  [self cleanEntry: thisEntry];
  thisEntry->dataType = type;
  thisEntry->object = arrayObject; 
}

-(void)setObject:(id)anObject atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.idValue = anObject;
  [self setMKArrayObject:arrayObject ofType: mkIDDataType atIndex: index];
}

-(void)setByte:(unsigned char)aByte atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.byteValue = aByte;
  [self setMKArrayObject: arrayObject ofType: mkBYTEDataType atIndex: index];
}

-(void)setInt:(int)anInt atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.intValue = anInt;
  [self setMKArrayObject:arrayObject ofType: mkINTDataType atIndex: index];
}

-(void)setFloat:(float)aFloat atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.floatValue = aFloat;
  [self setMKArrayObject:arrayObject ofType: mkFLOATDataType atIndex: index];
}

-(void)setByteArray:(unsigned char[])aByteArray ofSize:(unsigned int)size atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.byteArrayValue = [self createByteArray: aByteArray ofSize: size];
  [self setMKArrayObject: arrayObject ofType: mkBYTEARRAYDataType atIndex: index];
}

-(void)insertMKArrayObject:(mkArrayObject)arrayObject ofType:(enum mkDataType)type atIndex:(unsigned long int)index
{
  if(index >= count)
    {        
      [self addMKArrayObject: arrayObject ofType: type];  
    }
  else
    {
      mkArrayEntry* oldEntry = [self entryAtIndex: index];
      mkArrayEntry* newEntry  = (mkArrayEntry*) malloc(sizeof(*newEntry));
      newEntry->object = arrayObject;
      newEntry->dataType = type;
      if(index == 0) firstEntry = newEntry;
      else ((mkArrayEntry*)(oldEntry->previous))->next = newEntry;
      newEntry->previous = oldEntry->previous;
      newEntry->next = oldEntry; 
      oldEntry->previous = newEntry;
      count++;
      if(currentIndex>=index)currentIndex++;
    }
}

-(void)insertObject: (id)anObject atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.idValue = anObject;
  [self insertMKArrayObject: arrayObject ofType: mkIDDataType atIndex: index];
}

-(void)insertByte: (unsigned char)aByte atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.byteValue = aByte;
  [self insertMKArrayObject: arrayObject ofType: mkBYTEDataType atIndex: index];
}

-(void)insertInt: (int)anInt atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.intValue = anInt;
  [self insertMKArrayObject: arrayObject ofType: mkINTDataType atIndex: index];
}

-(void)insertFloat: (float)aFloat atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.floatValue = aFloat;
  [self insertMKArrayObject: arrayObject ofType: mkFLOATDataType atIndex: index];
}

-(void)insertByteArray: (unsigned char[])aByteArray ofSize: (unsigned int)size atIndex:(unsigned long int)index
{
  mkArrayObject arrayObject;
  arrayObject.byteArrayValue = [self createByteArray: aByteArray ofSize: size];
  [self insertMKArrayObject: arrayObject ofType: mkBYTEARRAYDataType atIndex: index];
}

-(mkArrayObject)mkArrayObjectAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  return thisEntry->object;
}

-(id)objectAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  if(thisEntry->dataType != mkIDDataType) return nil;
  return thisEntry->object.idValue;
}

-(unsigned char)byteAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  return thisEntry->object.byteValue;
}

-(int)intAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  return thisEntry->object.intValue;
}

-(float)floatAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  return thisEntry->object.floatValue;
}

-(unsigned char*)byteArrayAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  if(thisEntry->dataType != mkBYTEARRAYDataType) return NULL;
  return thisEntry->object.byteArrayValue->data;
}

-(void)removeObjectAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  if(thisEntry == NULL) return;
  [self cleanEntry: thisEntry];
  
  if(thisEntry->previous != NULL)
    {  
      ((mkArrayEntry*)thisEntry->previous)->next = thisEntry->next;  
    } 
  else 
    {
      firstEntry = (mkArrayEntry*)thisEntry->next; 
      if(firstEntry != NULL) firstEntry->previous = NULL;
    } 
  if(thisEntry->next != NULL)  
    {
      ((mkArrayEntry*)thisEntry->next)->previous = thisEntry->previous;
    }
  else 
    {
      lastEntry = (mkArrayEntry*)thisEntry->previous;  
      if(lastEntry != NULL) lastEntry->next = NULL;
    }
  
  free(thisEntry);
  count--;
}

-(void)removeByteAtIndex:(unsigned long int)index
{
  [self removeObjectAtIndex: index];
}

-(void)removeIntAtIndex:(unsigned long int)index
{
  [self removeObjectAtIndex: index];
}

-(void)removeFloatAtIndex:(unsigned long int)index;
{
  [self removeObjectAtIndex: index];
}

-(void)removeByteArray:(unsigned char[])aByteArray atIndex:(unsigned long int)index
{
  [self removeObjectAtIndex: index];
}

-(id)lastObject
{
  return lastEntry->object.idValue;
}

-(void)removeLastObject
{
  if(lastEntry != NULL)
    {
      mkArrayEntry* thisEntry = lastEntry;
      lastEntry = thisEntry->previous;
      if(lastEntry != NULL) lastEntry->next = NULL;
      [self cleanEntry: thisEntry];
      free(thisEntry);
      count--;
    }
}


-(enum mkDataType)dataTypeAtIndex:(unsigned long int)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  return thisEntry->dataType;
}

-(BOOL)resetIterator
{
  if(firstEntry != NULL)
    {
      currentEntry = firstEntry;
      currentIndex = 0;
      return YES;
    }
  else return NO;
}

-(BOOL)advanceIterator:(int)distance
{
  if(distance > 0)
    {
      for(distance; distance>0; distance--)
	{
	  if(currentEntry->next == NULL) return NO;
	  currentEntry = currentEntry->next;
	  currentIndex++;
	}
    }
  else if(distance < 0)
    {
      for(distance; distance<0; distance++)
	{
	  if(currentEntry->previous == NULL) return NO;
	  currentEntry = currentEntry->previous;
	  currentIndex--;
	}
    }
  return YES;
}

-(unsigned long int)currentIndex
{
  return currentIndex;
}

-(id)currentObject
{
  if(currentEntry->dataType == mkIDDataType) 
    {
      return currentEntry->object.idValue;
    }
  return nil;
}

-(unsigned char)currentByte  { return currentEntry->object.byteValue; }
-(unsigned char*)addressOfCurrentByte{ return &(currentEntry->object.byteValue); }
-(int)currentInt  { return currentEntry->object.intValue; }
-(float)currentFloat  { return currentEntry->object.floatValue; }
-(unsigned char*)currentByteArray  { return currentEntry->object.byteArrayValue->data; }
-(unsigned int)sizeOfCurrentByteArray  { return currentEntry->object.byteArrayValue->size; }


-(unsigned int)sizeOfByteArrayAtIndex:(unsigned long)index
{
  mkArrayEntry* thisEntry = [self entryAtIndex: index];
  if(thisEntry->dataType != mkBYTEARRAYDataType) return 0;
  return thisEntry->object.byteArrayValue->size;
}

-(mkArrayEntry*)entryAtIndex:(unsigned long int)index
{
  if(index >= count) return lastEntry;
  mkArrayEntry* thisEntry = firstEntry;
  while(index>0)
    {
      thisEntry = thisEntry->next;
      index--;
    }
  return thisEntry;
}

-(unsigned long int)count
{
  return count;
}

-(void)clear
{
  while(count > 0)  [self removeObjectAtIndex: 0];
}

-(id)free
{
  [self clear];
  return [super free];
}

@end
