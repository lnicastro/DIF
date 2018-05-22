#@ONERR_WARN|Cannot create database DIF|
CREATE DATABASE DIF//
USE DIF//

#@ONERR_WARN|Cannot install plugin DIF|
INSTALL PLUGIN dif SONAME 'ha_dif.so'//

#@ONERR_DIE|Cannot create table tbl|
CREATE TABLE tbl(db VARCHAR(64),
                 name VARCHAR(128),
		 id_type INTEGER NOT NULL,
                 id_opt INTEGER NOT NULL DEFAULT 0,
		 param INTEGER NOT NULL,
                 Ra_field VARCHAR(128),
                 Dec_field VARCHAR(128),
                 UNIQUE KEY(db, name, id_type, id_opt, param))//

CREATE TABLE dif(param INTEGER, id BIGINT, full BOOL) ENGINE=DIF//
