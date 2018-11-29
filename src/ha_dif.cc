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

#include "dif.hh"

#include "ha_dif.h"

// version 5.5.x doesn't contain mysql_priv.h . We need to add the provided includes.
#if MY_VERSION_ID >= 50505

// These two are not present in 5.7.9
#if MY_VERSION_ID < 50709 && ! defined(MARIADB_BASE_VERSION)
#include <my_pthread.h>
#include <sql_priv.h>
#endif

//#include <mysql/plugin.h>

// MySQL 8 #include "sql_class.h"           // MYSQL_HANDLERTON_INTERFACE_VERSION


//#include "probes_mysql.h"
#include <my_global.h>
#include "sql_plugin.h"

//#include <sql/sql_table.h>

#if MY_VERSION_ID >= 50600

static handler *dif_create_handler(handlerton *hton,
                                       TABLE_SHARE *table,
                                       MEM_ROOT *mem_root);

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

  count= array_elements(all_dif_mutexes);
  mysql_mutex_register(category, all_dif_mutexes, count);
}
#endif


Dif_share::Dif_share()
{
  thr_lock_init(&lock);
  mysql_mutex_init(ex_key_mutex_Dif_share_mutex,
                   &mutex, MY_MUTEX_INIT_FAST);
}


Dif_share *ha_dif::get_share()
{
  Dif_share *tmp_share;

  DBUG_ENTER("ha_dif::get_share()");

  lock_shared_ha_data();
  if (!(tmp_share= static_cast<Dif_share*>(get_ha_share_ptr())))
  {
    tmp_share= new Dif_share;
    if (!tmp_share)
      goto err;

    set_ha_share_ptr(static_cast<Handler_share*>(tmp_share));
  }
err:
  unlock_shared_ha_data();
  DBUG_RETURN(tmp_share);
}

#if MY_VERSION_ID >= 50709
#include <sql/log.h>
#if ! defined(MARIADB_BASE_VERSION)
#include <sql/auth/auth_common.h>
#endif
#endif

#endif  // >= 50600

#else
#include "ha_dif.h"
#include <mysql/plugin.h>
#endif

extern ThreadSpecificData<DIF_Region> difreg;


/* Static declarations for handlerton */

static handler *dif_create_handler(handlerton *hton,
                                         TABLE_SHARE *table,
                                         MEM_ROOT *mem_root)
{
  return new (mem_root) ha_dif(hton, table);
}


/*****************************************************************************
** DIF tables
*****************************************************************************/

ha_dif::ha_dif(handlerton *hton,
	       TABLE_SHARE *table_arg)
  :handler(hton, table_arg)
{
    int id_type;
    int id_opt;
    int param;
}


static const char *ha_dif_exts[] = { NullS };
const char **ha_dif::bas_ext() const
{
  return ha_dif_exts;
}

int ha_dif::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_dif::open");

//#if defined(MYSQL5_6) || defined(MYSQL5_7) || defined(MYSQL10_0) || defined(MYSQL10_1)
#if ! defined(MYSQL5_1) && ! defined(MYSQL5_5)
  if (!(share = get_share()))
    DBUG_RETURN(1);
  thr_lock_data_init(&share->lock,&lock,NULL);

#else
  thr_lock_init(&thr_lock);
  thr_lock_data_init(&thr_lock,&lock,NULL);
#endif

  //Parse file name
  //const char* p = name;
  //const char* q;
  //while (q = strchr(p, '/')) 
  //    p = q+1;
  //
  //if (strncmp(p, "htm", 3) == 0)
  //    id_type = 1;
  //
  //if (strncmp(p, "hea", 3) == 0)
  //    id_type = 2;
  //
  //char buf[2];
  //buf[0] = *(p+4);
  //buf[1] = 0;
  //sscanf(buf, "%d", &id_opt);
  //sscanf(p+6, "%d", &param);

  DBUG_RETURN(0);
}


int ha_dif::close(void)
{
  DBUG_ENTER("ha_dif::close");

//#if ! defined(MYSQL5_5) && ! defined(MYSQL5_6) && ! defined(MYSQL5_7) && ! defined(MYSQL10_0) && ! defined(MYSQL10_1)
#if defined(MYSQL5_1) || defined(MYSQL5_5)
  thr_lock_delete(&thr_lock);
#endif

  DBUG_RETURN(0);
}


int ha_dif::create(const char *name, TABLE *table_arg,
                         HA_CREATE_INFO *create_info)
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

int ha_dif::write_row(uchar * buf)
{
  DBUG_ENTER("ha_dif::write_row");
  DBUG_RETURN(0);
}


int ha_dif::rnd_init(bool scan)
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


int ha_dif::rnd_next(uchar *buf)
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


int ha_dif::rnd_pos(uchar * buf, uchar *pos)
{
  DBUG_ENTER("ha_dif::rnd_pos");
  DBUG_ASSERT(0);
  DBUG_RETURN(0);
}


void ha_dif::position(const uchar *record)
{
  DBUG_ENTER("ha_dif::position");
  DBUG_ASSERT(0);
  DBUG_VOID_RETURN;
}


#ifdef VOID_HANDLER_INFO
  void ha_dif::info(uint flag)
#else
  int ha_dif::info(uint flag)
#endif
{
  DBUG_ENTER("ha_dif::info");

  memset((char*) &stats, 0, sizeof(stats));
  if (flag & HA_STATUS_AUTO)
    stats.auto_increment_value= 1;

#ifndef VOID_HANDLER_INFO
  DBUG_RETURN(0);
#endif
}


int ha_dif::external_lock(THD *thd, int lock_type)
{
  DBUG_ENTER("ha_dif::external_lock");
  DBUG_RETURN(0);
}

/*
uint ha_dif::lock_count(void) const
{
  DBUG_ENTER("ha_dif::lock_count");
  DBUG_RETURN(0);
}
*/

THR_LOCK_DATA **ha_dif::store_lock(THD *thd,
                                         THR_LOCK_DATA **to,
                                         enum thr_lock_type lock_type)
{
  DBUG_ENTER("ha_dif::store_lock");
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

int ha_dif::index_next(uchar * buf)
{
  DBUG_ENTER("ha_dif::index_next");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_prev(uchar * buf)
{
  DBUG_ENTER("ha_dif::index_prev");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_first(uchar * buf)
{
  DBUG_ENTER("ha_dif::index_first");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


int ha_dif::index_last(uchar * buf)
{
  DBUG_ENTER("ha_dif::index_last");
  DBUG_RETURN(HA_ERR_END_OF_FILE);
}


static int dif_init(void *p)
{
  handlerton *dif_hton;
  dif_hton= (handlerton *)p;
  dif_hton->state= SHOW_OPTION_YES;
  dif_hton->db_type= DB_TYPE_UNKNOWN;
  dif_hton->create= dif_create_handler;
  dif_hton->flags= HTON_CAN_RECREATE;
  return 0;
}


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
  dif_init, /* Plugin Init */
  NULL, /* Plugin Deinit */
  0x0100 /* 1.0 */,
  NULL,                       /* status variables                */
  NULL,                       /* system variables                */
  NULL                        /* config options                  */
}
mysql_declare_plugin_end;
