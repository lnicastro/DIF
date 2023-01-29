/*
  MySQL interface functions: definitions

  Note: "pix_myXmatch" custom

  LN @ INAF-OAS, Sep. 2010                         Last changed: 28/01/2023
*/

#include <mysql.h>  /* header of the database client API library */

#if MYSQL_VERSION_ID >= 80000  &&  MYSQL_VERSION_ID < 100000
typedef bool   my_bool;
#endif

#define MYSQL_SOCK NULL

//#define PASSWORD "12345678"
#define DEFAULT_TIMEOUT "5"   /* Timeout in seconds: must be char */


/* Customize these definitions for your query! */

/* pix_myXmatch specific: 3 or 4 cols: 2 DOUBLE + 1 INT (RA, Dec, HTM/HPX id) + 1 optional INT (gaiadr2 source_id) or STRING (catwise source_name) */

static int NCOLS[] = {3, 4};
MYSQL_STMT *stmt;
MYSQL_BIND bind[4];
//MYSQL_BIND bind5[5];
//MYSQL_BIND bind3[3];
MYSQL_RES  *metadata;
MYSQL conn[2];
MYSQL_RES *result[2];

unsigned int num_fields[2];
unsigned int num_rows[2];
short return_row[2];

int           param_count;
unsigned long length[4];
my_bool       is_null[4];
my_bool       error[4];

double dbl_data[2];
unsigned long long long_data[2];
//unsigned long long long_data2;
//unsigned long long long_data3;
#define STRING_SIZE 21  // catwise source_name
char str_data[STRING_SIZE]; 
