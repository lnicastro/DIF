#@ONERR_DIE|Database DIF does not exists|
USE DIF//



#@ONERR_IGNORE||
DROP TABLE IF EXISTS func//

#@ONERR_DIE|Cannot create table func|
CREATE TABLE func (
	name CHAR(64) DEFAULT 'unknown', 
	params VARCHAR(128) DEFAULT 'undefined', 
	ret  CHAR(16) DEFAULT 'string',
        type enum('function','procedure','aggregate'),
	plugin_lib CHAR(64) DEFAULT '', 
	description VARCHAR(256) DEFAULT ''
)
CHARACTER SET ASCII COLLATE ascii_bin COMMENT='DIF functions and procedures description'//



#@ONERR_DIE|Cannot load data into table|
LOAD DATA INFILE
'/usr/local/share/dif/sql/funcdesc'
INTO TABLE func FIELDS TERMINATED BY ' & ' OPTIONALLY ENCLOSED BY '"'//
