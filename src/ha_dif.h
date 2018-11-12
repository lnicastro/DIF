// ----------------------------------------------------------------------^
// Copyright (C) 2004, 2005, 2006, 2007 Giorgio Calderone <gcalderone@ifc.inaf.it>
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

#ifndef DEF_HA_DIF_H
#define DEF_HA_DIF_H

//#define VOID_HANDLER_INFO

#ifdef USE_PRAGMA_INTERFACE
#pragma interface			/* gcc class implementation */
#endif

#if MY_VERSION_ID < 80000
#include <mysql_version.h>
#else
#include <mysql.h>
#endif

#if MYSQL_VERSION_ID < 50505
#include <mysql_priv.h>
#else
#include "thr_lock.h"                    /* THR_LOCK, THR_LOCK_DATA */
#include "handler.h"                     /* handler */
#include "my_base.h"                     /* ha_rows */

#if MYSQL_VERSION_ID >= 50600

class Dif_share : public Handler_share {
public:
  mysql_mutex_t mutex;
  THR_LOCK lock;
  Dif_share();
  ~Dif_share()
  {
    thr_lock_delete(&lock);
    mysql_mutex_destroy(&mutex);
  }
};

#endif
#endif


/*
  Class definition for the dif storage engine
  "Dumbest named feature ever"
*/
class ha_dif: public handler
{
  THR_LOCK_DATA lock;      /* MySQL lock */

//#if defined(MYSQL5_6) || defined(MYSQL5_7) || defined(MYSQL10_0) || defined(MYSQL10_1)
#if MYSQL_VERSION_ID >= 50600
  Dif_share *share;    ///< Shared lock info
  Dif_share *get_share(); ///< Get the share

#else
  THR_LOCK thr_lock;
#endif

  //const COND *cond_push(const COND *cond);
  int id_type;
  int id_opt;
  int param;

public:
  ha_dif(handlerton *hton, TABLE_SHARE *table_arg);
  ~ha_dif()
  {}

  /* The name that will be used for display purposes */
  const char *table_type() const { return "DIF"; }

  /*
    The name of the index type that will be used for display
    don't implement this method unless you really have indexes
  */
  const char *index_type(uint key_number);
  const char **bas_ext() const;
  ulonglong table_flags() const
  {
    return(HA_NULL_IN_KEY | HA_CAN_FULLTEXT | HA_CAN_SQL_HANDLER |
           HA_CAN_INDEX_BLOBS | HA_AUTO_PART_KEY |
           HA_FILE_BASED | HA_CAN_GEOMETRY);
           //HA_FILE_BASED | HA_CAN_GEOMETRY | HA_CAN_INSERT_DELAYED);
  }
  ulong index_flags(uint inx, uint part, bool all_parts) const
  {
    //return ((table_share->key_info[inx].algorithm == HA_KEY_ALG_FULLTEXT) ?
            //0 : HA_READ_NEXT | HA_READ_PREV | HA_READ_RANGE |
            //HA_READ_ORDER | HA_KEYREAD_ONLY);
    return (HA_READ_NEXT | HA_READ_PREV | HA_READ_RANGE | HA_READ_ORDER | HA_KEYREAD_ONLY);
  }

  /* The following defines can be increased if necessary */
#define DIF_MAX_KEY	64		/* Max allowed keys */
#define DIF_MAX_KEY_SEG	16		/* Max segments for key */
#define DIF_MAX_KEY_LENGTH 1000
  uint max_supported_keys()          const { return DIF_MAX_KEY; }
  uint max_supported_key_length()    const { return DIF_MAX_KEY_LENGTH; }
  uint max_supported_key_part_length() const { return DIF_MAX_KEY_LENGTH; }

  int open(const char *name, int mode, uint test_if_locked);
  int close(void);
  int write_row(uchar * buf);
  int rnd_init(bool scan);
  int rnd_next(uchar *buf);
  int rnd_pos(uchar * buf, uchar *pos);
  int index_read(uchar * buf, const uchar * key,
                 uint key_len, enum ha_rkey_function find_flag);
  int index_read_idx(uchar * buf, uint idx, const uchar * key,
                     uint key_len, enum ha_rkey_function find_flag);
  int index_read_last(uchar * buf, const uchar * key, uint key_len);
  int index_next(uchar * buf);
  int index_prev(uchar * buf);
  int index_first(uchar * buf);
  int index_last(uchar * buf);
  void position(const uchar *record);


#ifdef VOID_HANDLER_INFO
  void info(uint flag);
#else
  int info(uint flag);
#endif

  int external_lock(THD *thd, int lock_type);
  uint lock_count(void) const;
  int create(const char *name, TABLE *table_arg,
             HA_CREATE_INFO *create_info);
  THR_LOCK_DATA **store_lock(THD *thd,
                             THR_LOCK_DATA **to,
                             enum thr_lock_type lock_type);
};

#endif // DEF_HA_DIF_H
