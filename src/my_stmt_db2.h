/*
  MySQL interface functions

  Note: "pix_myXmatch" custom

  LN @ INAF-OAS, Sep. 2010                         Last changed: 29/11/2018
*/

#ifndef MY_STMT_DB_H
#define MY_STMT_DB_H

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS /* empty */
# define __END_DECLS /* empty */
#endif

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

int           param_count;
unsigned long length[4];
my_bool       is_null[4];
my_bool       error[4];

double dbl_data[2];
unsigned long long long_data[2];
//unsigned long long long_data2;
//unsigned long long long_data3;
#define STRING_SIZE 21  // catwise source_name
char   str_data[STRING_SIZE]; 

MYSQL conn[2];
MYSQL_RES *result[2];
unsigned int num_fields[2];
unsigned int num_rows[2];
short return_row[2];

__BEGIN_DECLS

int db_init(int ID);
int db_connect(int ID, const char *host, const char *username, const char *password, const char *db);
int db_select(int ID, const char *db);
const char * db_error(int ID);
int db_return_row(int ID);
int db_query(int ID, const char *query);
unsigned int db_num_fields(int ID);
unsigned int db_num_rows(int ID);
char *db_fieldname(int ID, int ord);
enum enum_field_types db_fieldtype(int ID, int ord);
unsigned int db_fieldlength(int ID, int ord);
char *db_data(int ID, unsigned int row, unsigned int field);
char **db_row(int ID, unsigned int row);
short db_unsigned(int ID, int ord);
void db_free_result(int ID);
void db_close(int ID);

int db_stmt_prepexe2(int ID, const char *query, const char *param, const unsigned short tid);
int my_difbind2(unsigned int n_fields, const char *param, MYSQL_BIND bind[4], const unsigned short tid);

__END_DECLS

#endif
