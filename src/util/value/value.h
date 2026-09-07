#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>

typedef struct Object Object;
typedef struct ObjString ObjString;

typedef enum { VAL_NUM, VAL_BOOL, VAL_OBJ, VAL_NULL } ValueType;

typedef struct {
	ValueType type;
	union {
		double num;
		bool boolean;
		Object *obj;
	} as;
} Value;

#define BOOL_VAL(val) ((Value){VAL_BOOL, {.boolean = val}})
#define NULL_VAL ((Value){VAL_NULL, {0}})
#define NUM_VAL(val) ((Value){VAL_NUM, {.num = val}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Object *)object}})

#define IS_NULL(val) ((val).type == VAL_NULL)
#define IS_BOOL(val) ((val).type == VAL_BOOL)
#define IS_NUM(val) ((val).type == VAL_NUM)
#define IS_OBJ(val) ((val).type == VAL_OBJ)

#define AS_BOOL(val) ((val).as.boolean)
#define AS_NUM(val) ((val).as.num)
#define AS_OBJ(val) ((val).as.obj)

typedef struct {
	Value *values;
	int len, cap;
} ValueList;

void initValueList(ValueList *va);
void writeValueList(ValueList *va, Value v);
void printValue(Value v);
void freeValueList(ValueList *va);

bool valuesEqual(Value a, Value b);

#endif
