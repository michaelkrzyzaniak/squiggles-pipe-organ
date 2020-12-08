
#import <objc/Object.h>//&#60objc/Object.h&#62
#import <stdlib.h>//&#60stdlib.h&#62

enum mkDataType
  {
    mkIDDataType,
    mkBYTEDataType,
    mkINTDataType, 
    mkFLOATDataType,
    mkBYTEARRAYDataType,
  };

typedef union
{
  char signedValue;
  unsigned char unsignedValue;
}mkByte;

typedef struct
{
  unsigned int size;
  unsigned char data[];
}mkByteArray;

typedef union
{
  id idValue;
  unsigned char byteValue;
  int intValue;
  float floatValue;
  mkByteArray* byteArrayValue;
}mkArrayObject;
  
typedef struct
{
  void* previous;
  void* next;
  enum mkDataType dataType;
  mkArrayObject object;
}mkArrayEntry;


@interface MKArray : Object
{
  mkArrayEntry* firstEntry;
  mkArrayEntry* lastEntry;
  mkArrayEntry* currentEntry;
  unsigned long int count;
  unsigned long int currentIndex;
}

//Private
-(void)cleanEntry:(mkArrayEntry*)thisEntry;
-(mkByteArray*)createByteArray:(unsigned char[])aByteArray ofSize:(unsigned int)size;
//Public
-(void)addMKArrayObject:(mkArrayObject)arrayObject ofType:(enum mkDataType)type;
-(void)addObject:(id)anObject;
-(void)addByte:(unsigned char)aByte;
-(void)addInt:(int)anInt;
-(void)addFloat:(float)aFloat;
-(void)addByteArray:(unsigned char[])aByteArray ofSize:(unsigned int)size;

-(void)setMKArrayObject:(mkArrayObject)arrayObject ofType:(enum mkDataType)type atIndex:(unsigned long int)index;
-(void)setObject:(id)anObject atIndex:(unsigned long int)index;
-(void)setByte:(unsigned char)aByte atIndex:(unsigned long int)index;
-(void)setInt:(int)anInt atIndex:(unsigned long int)index;
-(void)setFloat:(float)aFloat atIndex:(unsigned long int)index;
-(void)setByteArray:(unsigned char[])aByteArray ofSize:(unsigned int)size atIndex:(unsigned long int)index;

-(void)insertMKArrayObject:(mkArrayObject)arrayObject ofType:(enum mkDataType)type atIndex:(unsigned long int)index;
-(void)insertObject: (id)anObject atIndex:(unsigned long int)index;
-(void)insertByte: (unsigned char)aByte atIndex:(unsigned long int)index;
-(void)insertInt: (int)anInt atIndex:(unsigned long int)index;
-(void)insertFloat: (float)aFloat atIndex:(unsigned long int)index;
-(void)insertByteArray: (unsigned char[])aByteArray ofSize: (unsigned int)size atIndex:(unsigned long int)index;

-(void)removeObjectAtIndex:(unsigned long int)index;
-(void)removeByteAtIndex:(unsigned long int)index;
-(void)removeIntAtIndex:(unsigned long int)index;
-(void)removeFloatAtIndex:(unsigned long int)index;
-(void)removeByteArray:(unsigned char[])aByteArray atIndex:(unsigned long int)index;

-(id)lastObject;
-(void)removeLastObject;

-(mkArrayObject)mkArrayObjectAtIndex:(unsigned long int)index;
-(id)objectAtIndex:(unsigned long int)index;
-(int)intAtIndex:(unsigned long int)index;
-(unsigned char)byteAtIndex:(unsigned long int)index;
-(float)floatAtIndex:(unsigned long int)index;
-(unsigned char*)byteArrayAtIndex:(unsigned long int)index;

-(BOOL)resetIterator;
-(BOOL)advanceIterator:(int)distance; //negative for previous;
-(unsigned long int)currentIndex;
-(id)currentObject;
-(int)currentInt;
-(unsigned char)currentByte;
-(unsigned char*)addressOfCurrentByte;
-(float)currentFloat;
-(unsigned char*)currentByteArray;
-(unsigned int)sizeOfCurrentByteArray;

-(enum mkDataType)dataTypeAtIndex:(unsigned long int)index;
-(unsigned int)sizeOfByteArrayAtIndex:(unsigned long)index;
-(unsigned long int)count;
-(void)clear;
-(mkArrayEntry*)entryAtIndex:(unsigned long int)index;

@end
