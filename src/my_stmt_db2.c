/*
  Function to access MySQL DBs. Version suitable for C or C++ calls.

  Note: here use "pix_myXmatch" custom bind function "my_difbind2"

  LN @ IASF-INAF, Sep. 2010                         Last changed: 09/05/2020
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_stmt_db2_defs.h"
#include "my_stmt_db2.h"


int db_stmt_exec(int ID, const char *query, const char *param) {

/* Execute the query */
  if (mysql_stmt_execute(stmt))
  {
    fprintf(stderr, " mysql_stmt_execute(), failed\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    return (1);
  }

  return (0);
}


int db_stmt_add(const char *query, const char *param) {

  return (0);
}


int db_stmt_prep(int ID, const char *query, const char *param) {

// "insert into tab (c1,c2,c3) values(?,?,?)"

/* Prepare a SELECT query to fetch data from table */
  stmt = mysql_stmt_init(&conn[ID]);
  if (!stmt)
  {
    fprintf(stderr, " mysql_stmt_init(), out of memory\n");
    return (1);
  }
  if (mysql_stmt_prepare(stmt, query, (uint) strlen(query)))
  {
    fprintf(stderr, " mysql_stmt_prepare() failed\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    mysql_stmt_close(stmt);
    return (1);
  }
//fprintf(stdout, " prepare successful\n");

/* Get the parameter count from the statement */
  param_count= mysql_stmt_param_count(stmt);
//fprintf(stdout, " total parameters: %d\n", param_count);

  if (param_count != 0) /* validate parameter count */
  {
    fprintf(stderr, " invalid parameter count returned by MySQL\n");
    mysql_stmt_close(stmt);
    return (1);
  }

  return (0);
}


int db_stmt_prepexe2(int ID, const char *query, const char *param, const unsigned short tid) {

  unsigned long type= CURSOR_TYPE_READ_ONLY;

/* Prepare a SELECT query to fetch data from table */
  stmt = mysql_stmt_init(&conn[ID]);
  if (!stmt)
  {
    fprintf(stderr, " mysql_stmt_init(), out of memory\n");
    return (1);
  }
  if (mysql_stmt_prepare(stmt, query, strlen(query)))
  {
    fprintf(stderr, " mysql_stmt_prepare(), SELECT failed\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    return (1);
  }
//fprintf(stdout, " prepare, SELECT successful\n");

// open read-only
  mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (const void*) &type);

/* Get the parameter count from the statement */
  param_count = mysql_stmt_param_count(stmt);
//fprintf(stdout, " total parameters in SELECT: %d\n", param_count);

  if (param_count != 0) /* validate parameter count */
  {
    fprintf(stderr, " invalid parameter count returned by MySQL\n");
    return (1);
  }

/* Fetch result set meta information */
  metadata = mysql_stmt_result_metadata(stmt);
  if (!metadata)
  {
    fprintf(stderr,
         " mysql_stmt_result_metadata(), returned no meta information\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    return (1);
  }

/* Get total columns in the query */
  num_fields[ID] = mysql_num_fields(metadata);
//fprintf(stdout, " total columns in SELECT statement: %d\n", num_fields[ID]);

  if (num_fields[ID] != NCOLS[0] && num_fields[ID] != NCOLS[1]) /* validate column count */
  {
    fprintf(stderr, " invalid column count returned by MySQL\n");
    fprintf(stderr, " total columns in SELECT statement: %d  expected: %d or %d\n",
            num_fields[ID], NCOLS[0], NCOLS[1]);
    return (1);
  }

  mysql_free_result(metadata);

/* Execute the SELECT query */
  if (mysql_stmt_execute(stmt))
  {
    fprintf(stderr, " mysql_stmt_execute(), failed\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    return (1);
  }

/* Bind the result buffers for all the columns before fetching them */
  memset(bind, 0, sizeof(bind));

  my_difbind2(num_fields[ID], param, bind, tid);

/* Now buffer all results to client (optional step) */

  if (mysql_stmt_store_result(stmt))
  {
    fprintf(stderr, " mysql_stmt_store_result() failed\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    return (1);
  }

/*
  int rc;
  bool upd = true;
  rc = mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void*) &upd);
*/

  num_rows[ID] = mysql_stmt_num_rows(stmt);

  return (0);
}


int my_difbind2(unsigned int n_fields, const char *param, MYSQL_BIND *bind, const unsigned short tid)  // bind results
{
  int id = atoi(param);

/* First 2 columns are DOUBLE -> e.g. RA, Dec */
  bind[0].buffer_type= MYSQL_TYPE_DOUBLE;
  bind[0].buffer= (char *)&dbl_data[0];
  bind[0].is_null= &is_null[0];
  bind[0].length= &length[0];
  bind[0].error= &error[0];

  bind[1].buffer_type= MYSQL_TYPE_DOUBLE;
  bind[1].buffer= (char *)&dbl_data[1];
  bind[1].is_null= &is_null[1];
  bind[1].length= &length[1];
  bind[1].error= &error[1];

/* MySQL INT type from HTM/HEALPix depth/order */
// (def. htmID_6)
  switch (id) {
    case 0:
    case 1:
    case 2:
      bind[2].buffer_type= MYSQL_TYPE_TINY;
      break;
    case 3:
    case 4:
    case 5:
    case 6:
      bind[2].buffer_type= MYSQL_TYPE_SHORT;
      break;
    case 7:
    case 8:
    case 9:
    case 10:
      bind[2].buffer_type= MYSQL_TYPE_INT24;
      break;
    case 11:
    case 12:
    case 13:
    case 14:
      bind[2].buffer_type= MYSQL_TYPE_LONG;
      break;
    default:
      bind[2].buffer_type= MYSQL_TYPE_LONGLONG;

  }

  bind[2].buffer = (char *)&long_data[0];  // always use largest INT
  bind[2].is_unsigned = 1;             // always UNSIGNED
  bind[2].is_null = &is_null[2];
  bind[2].length = &length[2];
  bind[2].error = &error[2];


/* This is temporary! Adjust for your needs! */
  if (n_fields == 4) {
	if (tid == 0) {  // catwise source_name
  //MYSQL_FIELD* aField = &metadata->fields[3];
	  bind[3].buffer_type = MYSQL_TYPE_STRING;
	  bind[3].buffer = str_data;
	  bind[3].buffer_length = STRING_SIZE;
	  //bind[3].buffer_length = aField->length;
	} else {  // GAIA DR2 source_id
	  bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
	  bind[3].buffer = (char *)&long_data[1];  // always use largest INT
	  bind[3].is_unsigned = 1;                 // always UNSIGNED
	}
	bind[3].length = &length[3];
	bind[3].is_null = &is_null[3];
	bind[3].error = &error[3];
  }

  //mysql_free_result(metadata);

/* Bind the result buffers */
  if (mysql_stmt_bind_result(stmt, bind))
  {
    fprintf(stderr, " mysql_stmt_bind_result() failed\n");
    fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
    return (1);
  }

  return (0);
}


/* Functions below are from my_stmt_db.c */

int db_init(int ID) {
  int iret;

  iret=0;
  if (mysql_init(&conn[ID])) {
    iret=mysql_options(&conn[ID], MYSQL_OPT_CONNECT_TIMEOUT, DEFAULT_TIMEOUT);
    if (iret == 0) iret=1;
    else iret=0;
  }
  return iret;
}

int db_connect(int ID, const char *host, const char *username, const char *password, const char *db) {
  MYSQL *ret;

  ret=mysql_real_connect(&conn[ID], host, username, password, db, 0, MYSQL_SOCK, 0);
  if (ret) return 1;
  else return 0;
}

const char * db_error(int ID) {
  return mysql_error(&conn[ID]);
}

int db_return_row(int ID) {
  return return_row[ID];
}

int db_select(int ID, const char *db) {
  int ret;

  ret=mysql_select_db(&conn[ID], db);
  if (ret) return 1;
  else return 0;
}





int db_query(int ID, const char *query) {
  if (mysql_query(&conn[ID], query))
    return 0;
  else {
    /* query succeeded, process any data returned by it*/
    result[ID] = mysql_store_result(&conn[ID]);
    if (result[ID]) {
      /* there are rows*/
      return_row[ID]=1;

      num_fields[ID] = mysql_num_fields(result[ID]);
      num_rows[ID] = mysql_num_rows(result[ID]);

      return 1;
    }
    else {
        return_row[ID]=0;
        num_rows[ID] = mysql_affected_rows(&conn[ID]);
        return 1;
    }
  }
}

int db_uquery(int ID, const char *query) {
  if (mysql_query(&conn[ID], query))
    return 0;
  else {
    /* query succeeded, process any data returned by it*/
//    result[ID] = mysql_store_result(&conn[ID]);
    result[ID] = mysql_use_result(&conn[ID]);
    if (result[ID]) {
      /* there are rows*/
      return_row[ID]=1;

      num_fields[ID] = mysql_num_fields(result[ID]);
      //num_rows[ID] = mysql_num_rows(result[ID]);

      return 1;
    }
    else {
        return_row[ID]=0;
        num_rows[ID] = mysql_affected_rows(&conn[ID]);
        return 1;
    }
  }
}

unsigned int db_num_fields(int ID) {
  return num_fields[ID];
}

unsigned int db_num_rows(int ID) {
  return num_rows[ID];
}

char *db_fieldname(int ID, int ord) {
  MYSQL_FIELD *fields;

  if ((unsigned int)ord < num_fields[ID]) {
    fields = mysql_fetch_fields(result[ID]);
    return fields[ord].name;
  }
  else
    return NULL;
}

enum enum_field_types db_fieldtype(int ID, int ord) {
  MYSQL_FIELD *fields;

  if ((unsigned int)ord < num_fields[ID]) {
    fields = mysql_fetch_fields(result[ID]);
    return fields[ord].type;
  }
  else
    return FIELD_TYPE_NULL;
}

unsigned int db_fieldlength(int ID, int ord) {
  MYSQL_FIELD *fields;

  if ((unsigned int)ord < num_fields[ID]) {
    fields = mysql_fetch_fields(result[ID]);
    return fields[ord].length;
  }
  else
    return 0;
}

char *db_data(int ID, unsigned int row, unsigned int field) {
  MYSQL_ROW record;

  if ((field < num_fields[ID]) && (row < num_rows[ID])) {


    mysql_data_seek(result[ID], row);
    record = mysql_fetch_row(result[ID]);
    return record[field];
  }
  else
    return NULL;
}

char **db_row(int ID, unsigned int row) {
  MYSQL_ROW record;

  if (row < num_rows[ID]) {
    mysql_data_seek(result[ID], row);
    record = mysql_fetch_row(result[ID]);
    return record;
  }
  return NULL;
}

short db_unsigned(int ID, int ord) {
  MYSQL_FIELD *fields;

  if ((unsigned int)ord < num_fields[ID]) {
    fields = mysql_fetch_fields(result[ID]);
    if  (fields[ord].flags & UNSIGNED_FLAG)
      return 1;
    else
      return 0;
  }
  else
    return 0;
}

void db_free_result(int ID) {
  mysql_free_result(result[ID]);
}

void db_close(int ID) {
  mysql_close(&conn[ID]);
}
