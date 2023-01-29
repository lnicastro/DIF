/*
  MySQL interface functions: definitions

  Note: "myXmatch" custom

  LN @ INAF-OAS, Sep. 2010                         Last changed: 28/01/2023
*/

#include <mysql.h>  /* header of the database client API library */

#if MYSQL_VERSION_ID >= 80000  &&  MYSQL_VERSION_ID < 100000
typedef bool   my_bool;
#endif

#define MYSQL_SOCK NULL

//#define PASSWORD "12345678"
#define DEFAULT_TIMEOUT "5"   /* Timeout in seconds: must be char */

/* myXmatch specific: 3 cols: 2 DOUBLE + 1 INT  */
#define NCOLS 3

MYSQL_STMT* stmt;
MYSQL_BIND bind[NCOLS];
MYSQL_RES     *metadata;
MYSQL conn[1];
MYSQL_RES *result[1];

unsigned int num_fields[1];
unsigned int num_rows[1];
short return_row[1];

int           param_count;
unsigned long length[NCOLS];
my_bool       is_null[NCOLS];
my_bool       error[NCOLS];

double dbl_data[2];
unsigned long long long_data;
