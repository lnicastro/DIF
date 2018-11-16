// ----------------------------------------------------------------------^
// Copyright (C) 2004 - 2018
// Giorgio Calderone <giorgio.calderone@inaf.it>
// Luciano Nicastro <luciano.nicastro@inaf.it>
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

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation				// gcc: Class implementation
#endif

#include <mysql/plugin.h>
#include <mysql/psi/mysql_file.h>

#include "dif.hh"
#include "ha_dif.h"

#include "my_dbug.h"
#include "mysql/plugin.h"
#include "my_psi_config.h"
#include "mysql/psi/mysql_memory.h"
#include "sql/field.h"
#include "sql/table.h"
#include "sql/sql_plugin.h"

extern ThreadSpecificData<DIF_Region> difreg;

/* Static declarations for handlerton */

static handler *dif_create_handler(handlerton *hton, TABLE_SHARE *table,
                                       bool partitioned, MEM_ROOT *mem_root);

handlerton *dif_hton;


/* Interface to mysqld, to check system tables supported by SE */
static const char* dif_system_database();
static bool dif_is_supported_system_table(const char *db,
                                      const char *table_name,
                                      bool is_sql_layer_system_table);
#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key ex_key_mutex_Dif_share_mutex;

static PSI_mutex_info all_dif_mutexes[]=
{
  { &ex_key_mutex_Dif_share_mutex, "Dif_share::mutex", 0}
};

static void init_dif_psi_keys()
{
  const char* category= "dif";
  int count;

  count = static_cast<int>(array_elements(all_dif_mutexes));
  mysql_mutex_register(category, all_dif_mutexes, count);
}
#endif

/* Interface to mysqld, to check system tables supported by SE */
static const char* dif_system_database();
static bool dif_is_supported_system_table(const char *db,
                                      const char *table_name,
                                      bool is_sql_layer_system_table);



Dif_share::Dif_share() { thr_lock_init(&lock); }


static int dif_init_func(void *p) {
  //handlerton *dif_hton;
  dif_hton= (handlerton *)p;
  dif_hton->state= SHOW_OPTION_YES;
  dif_hton->db_type= DB_TYPE_UNKNOWN;
  dif_hton->create= dif_create_handler;
  dif_hton->flags= HTON_CAN_RECREATE;

  dif_hton->is_supported_system_table = dif_is_supported_system_table;
  return 0;
}


Dif_share *ha_dif::get_share() {
  Dif_share *tmp_share;

  DBUG_ENTER("ha_dif::get_share()");

  lock_shared_ha_data();
  if (!(tmp_share= static_cast<Dif_share*>(get_ha_share_ptr())))
  {
    tmp_share = new Dif_share;
    if (!tmp_share)
      goto err;

    set_ha_share_ptr(static_cast<Handler_share*>(tmp_share));
  }
err:
  unlock_shared_ha_data();
  DBUG_RETURN(tmp_share);
}


extern ThreadSpecificData<DIF_Region> difreg;


/* Static declarations for handlerton */

static handler *dif_create_handler(handlerton *hton, TABLE_SHARE *table,
				   bool, MEM_ROOT *mem_root) {
  return new (mem_root) ha_dif(hton, table);
}


/*****************************************************************************
** DIF tables
*****************************************************************************/

ha_dif::ha_dif(handlerton *hton, TABLE_SHARE *table_arg)
  : handler(hton, table_arg)
{
    int id_type;
    int id_opt;
    int param;
}


// Commented: no extensions
/*
static const char *ha_dif_exts[] = { NullS };
const char **ha_dif::bas_ext() const
{
  return ha_dif_exts;
}
*/


// Optional
static st_handler_tablename ha_dif_system_tables[] = {
    {(const char *)NULL, (const char *)NULL}};


static bool dif_is_supported_system_table(const char *db,
                                              const char *table_name,
                                              bool is_sql_layer_system_table) {
  st_handler_tablename *systab;

  // Does this SE support "ALL" SQL layer system tables ?
  if (is_sql_layer_system_table) return false;

  // Check if this is SE layer system tables
  systab = ha_dif_system_tables;
  while (systab && systab->db) {
    if (systab->db == db && strcmp(systab->tablename, table_name) == 0)
      return true;
    systab++;
  }

  return false;
}



//int ha_dif::open(const char *name, int mode, uint test_if_locked)
int ha_dif::open(const char *, int, uint, const dd::Table *)
{
  DBUG_ENTER("ha_dif::open");

  if (!(share = get_share()))
    DBUG_RETURN(1);
  thr_lock_data_init(&share->lock, &lock, NULL);

  DBUG_RETURN(0);
}


int ha_dif::close(void)
{
  DBUG_ENTER("ha_dif::close");
  DBUG_RETURN(0);
}


int ha_dif::write_row(uchar * buf)
{
  DBUG_ENTER("ha_dif::write_row");
  DBUG_RETURN(0);
}


// No update_row
// No delete_row


// Added (needed?)
/*
int ha_dif::index_read_map(uchar *, const uchar *, key_part_map,
                           enum ha_rkey_function) {
  int rc;
  DBUG_ENTER("ha_dif::index_read_map");
  rc = HA_ERR_WRONG_COMMAND;
  DBUG_RETURN(rc);
}
*/

int ha_dif::index_next(uchar *)
{
  DBUG_ENTER("ha_dif::index_next");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_prev(uchar *)
{
  DBUG_ENTER("ha_dif::index_prev");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_first(uchar *)
{
  DBUG_ENTER("ha_dif::index_first");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_last(uchar *)
{
  DBUG_ENTER("ha_dif::index_last");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}



int ha_dif::rnd_init(bool)
{
  DBUG_ENTER("ha_dif::rnd_init");

  if (! difreg.getp()) difreg.constructor();
  difreg->subStart();
  difreg->go();
  difreg->subStop();

  DBUG_RETURN(0);
}


//Added

int ha_dif::rnd_end() {
  DBUG_ENTER("ha_dif::rnd_end");
  DBUG_RETURN(0);
}



int ha_dif::rnd_next(uchar *)
{
  DBUG_ENTER("ha_dif::rnd_next");

  int param, full;
  long long int val;

  difreg->subStart();
  int eof = difreg->read_next(param, val, full);

  if (! eof) {
      Field **field=table->field;
      (*field)->set_notnull();  field++;
      (*field)->set_notnull();  field++;
      (*field)->set_notnull();
      
      field=table->field;
      (*field)->store(param);   field++;
      (*field)->store(val);     field++;
      (*field)->store(full);
      difreg->subStop();
      DBUG_RETURN(0);
  }

  difreg->read_reset();

  //if (eof == 1) 
  //  difreg->clear();  //Final EOF
  difreg->subStop();

  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


void ha_dif::position(const uchar *)
{
  DBUG_ENTER("ha_dif::position");
  DBUG_ASSERT(0);
  DBUG_VOID_RETURN;
}


int ha_dif::rnd_pos(uchar *, uchar *)
{
  int rc;
  DBUG_ENTER("ha_dif::rnd_pos");
  //rc = HA_ERR_WRONG_COMMAND;
  //DBUG_RETURN(rc);
  DBUG_ASSERT(0);
  DBUG_RETURN(0);
}


int ha_dif::info(uint flag)
{
  DBUG_ENTER("ha_dif::info");

  memset((char*) &stats, 0, sizeof(stats));
  if (flag & HA_STATUS_AUTO)
    stats.auto_increment_value= 1;

  DBUG_RETURN(0);
}


// Added (needed?)

int ha_dif::external_lock(THD *, int)
{
  DBUG_ENTER("ha_dif::external_lock");
  DBUG_RETURN(0);
}


/*
int ha_dif::extra(enum ha_extra_function) {
  DBUG_ENTER("ha_dif::extra");
  DBUG_RETURN(0);
}

int ha_dif::delete_all_rows() {
  DBUG_ENTER("ha_dif::delete_all_rows");
  DBUG_RETURN(HA_ERR_WRONG_COMMAND);
}

ha_rows ha_dif::records_in_range(uint, key_range *, key_range *) {
  DBUG_ENTER("ha_f::records_in_range");
  DBUG_RETURN(10);  // low number to force index usage
}

static MYSQL_THDVAR_STR(last_create_thdvar, PLUGIN_VAR_MEMALLOC, NULL, NULL,
                        NULL, NULL);

static MYSQL_THDVAR_UINT(create_count_thdvar, 0, NULL, NULL, NULL, 0, 0, 1000,
                         0);
*/
//--

int ha_dif::create(const char *name, TABLE *, HA_CREATE_INFO *,
		   dd::Table *table_def)
{
  DBUG_ENTER("ha_dif::create");
  DBUG_RETURN(0);
}


/*
const char *ha_dif::index_type(uint key_number)
{
  DBUG_ENTER("ha_dif::index_type");
  DBUG_RETURN((table_share->key_info[key_number].flags & HA_FULLTEXT) ?
              "FULLTEXT" :
              (table_share->key_info[key_number].flags & HA_SPATIAL) ?
              "SPATIAL" :
              (table_share->key_info[key_number].algorithm ==
               HA_KEY_ALG_RTREE) ? "RTREE" : "BTREE");
}
*/

/*
uint ha_dif::lock_count(void) const
{
  DBUG_ENTER("ha_dif::lock_count");
  DBUG_RETURN(0);
}
*/


// Changed

THR_LOCK_DATA **ha_dif::store_lock(THD *thd, THR_LOCK_DATA **to,
                                   enum thr_lock_type lock_type)
{
  DBUG_ENTER("ha_dif::store_lock");

//if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK) lock.type = lock_type;
  //*to++ = &lock;

  DBUG_RETURN(to);
}



/*
int ha_dif::index_read(uchar * buf, const uchar * key,
                             uint key_len, enum ha_rkey_function find_flag)
{
  DBUG_ENTER("ha_dif::index_read");
  DBUG_RETURN(0);
}


int ha_dif::index_read_idx(uchar * buf, uint idx, const uchar * key,
                                 uint key_len, enum ha_rkey_function find_flag)
{
  DBUG_ENTER("ha_dif::index_read_idx");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_read_last(uchar * buf, const uchar * key, uint key_len)
{
  DBUG_ENTER("ha_dif::index_read_last");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}
*/

struct st_mysql_storage_engine dif_storage_engine=
{ MYSQL_HANDLERTON_INTERFACE_VERSION };


mysql_declare_plugin(dif)
{
    MYSQL_STORAGE_ENGINE_PLUGIN,
    &dif_storage_engine,
    "DIF",
    "Giorgio Calderone & Luciano Nicastro, INAF (Italy)",
    "Dynamic Index Facility",
    PLUGIN_LICENSE_GPL,
    dif_init_func, /* Plugin Init */
// Added
    NULL,          /* Plugin check uninstall */

    NULL,          /* Plugin Deinit */
    0x0100 /* 1.0 */,

    NULL       ,          /* status variables */
    NULL                , /* system variables */
    NULL,                 /* config options */
    0                     /* flags */
}
mysql_declare_plugin_end;
