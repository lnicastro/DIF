/*
  MySQL interface functions

  Note: "pix_myXmatch" custom

  LN @ INAF-OAS, Sep. 2010                         Last changed: 28/01/2023
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
