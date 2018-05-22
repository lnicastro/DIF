// ----------------------------------------------------------------------^
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009
// Giorgio Calderone <gcalderone@ifc.inaf.it>
// 
// This file is part of DIF.
// 
// DIF is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// DIF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with DIF; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
// 
// ----------------------------------------------------------------------$

//Argument to  *_init functions
#define INIT_ARGS UDF_INIT*, UDF_ARGS*, char*

//Argument to main functions
#define MAIN_ARGS UDF_INIT*, UDF_ARGS*, char*, char*

//Argument to main functions: char
#define MAIN_ARGS_CHAR UDF_INIT*, UDF_ARGS*, char*, unsigned long*, char*, char*

//Argument to *_deinit functions
#define DEINIT_ARGS UDF_INIT*


#define DEFINE_FUNCTION(RETTYPE, NAME) \
my_bool  NAME ## _init  (INIT_ARGS);       \
RETTYPE  NAME           (MAIN_ARGS);       \
void     NAME ## _deinit(DEINIT_ARGS);

#define DEFINE_FUNCTION_CHAR(RETTYPE, NAME) \
my_bool  NAME ## _init  (INIT_ARGS);        \
RETTYPE  NAME           (MAIN_ARGS_CHAR);   \
void     NAME ## _deinit(DEINIT_ARGS);


#define CHECK_ARG_NUM(NUM)         \
  if (args->arg_count != NUM) {    \
    strcpy(message, argerr);       \
    return 1;                      \
  }


#define CHECK_ARG_TYPE(NUM, TYPE)     \
  if (args->arg_type[NUM] != TYPE) {  \
    strcpy(message, argerr);          \
    return 1;                         \
  }


#define CHECK_ARG_NOT_TYPE(NUM, TYPE) \
  if (args->arg_type[NUM] == TYPE) {  \
    strcpy(message, argerr);          \
    return 1;                         \
  }


#define ISNULL(A)  (!((bool) args->args[A]))
#define CHECK_AND_RETURN_NULL(A) if (! args->args[A]) {	*is_null = 1; return 0;  }

#define IARGS(A) Conv_int(args->args[A], args->arg_type[A])
#define DARGS(A) Conv_double(args->args[A], args->arg_type[A])
#define CARGS(A) ((char*) args->args[A])

long long int Conv_int(void* p, enum Item_result type)
{
  long long int ret;

  switch (type) {
  case STRING_RESULT:
  case DECIMAL_RESULT:
    if (sscanf((char*) p, "%lld", &ret) != 1)
      return 0;
    break;
  case INT_RESULT:
    ret = (*((long long*) p));
    break;
  case REAL_RESULT:
    ret = (long long int) rint(*((double*) p));
    break;
  default:
    return 0;
  }

  return ret;
}


double Conv_double(void* p, enum Item_result type)
{
  double ret;

  switch (type) {
  case STRING_RESULT:
  case DECIMAL_RESULT:
    if (sscanf((char*) p, "%lf", &ret) != 1)
      return 0;
    break;
  case INT_RESULT:
    ret = (*((long long*) p));
    break;
  case REAL_RESULT:
    ret = (*((double*) p));
    break;
  default:
    return 0;
  }

  return ret;
}



#define CLEAR *is_null = 0; *error = 0;


struct myData
{
    char firstTime;
    char is_null;
    char error;
    char flag;
    long long val;
    long long tmp;
};

#define ALLOC_MYDATA                                                  \
struct myData* data = (struct myData*) malloc(sizeof(struct myData)); \
data->firstTime = 1; 						      \
data->is_null = 0; 						      \
data->error = 0; 						      \
init->ptr = (char*) data


#define MYDATA  struct myData* data = (struct myData*) init->ptr

#define RETURN_MYDATA \
data->firstTime = 0;  \
if (data->is_null)    \
    *is_null = 1;     \
                      \
if (data->error)      \
    *error = 1;       \
                      \
return data->val


#define FREE_MYDATA free(init->ptr)


static const char NULLEXCEPTION = 1;
static const char ERREXCEPTION  = 2;
