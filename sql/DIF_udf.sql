#@ONERR_DIE|Database DIF does not exists|
USE DIF;


#No longer used UDFs:

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_HTMCircle//
#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_HTMRect//
#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_HTMRectV//
#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_HEALPCircle//
#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_useParam//



#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS IDMatch//

#@ONERR_DIE|Cannot install function IDMatch|
#CREATE FUNCTION IDMatch RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS CrossMatch//

#@ONERR_DIE|Cannot install function CrossMatch|
#CREATE FUNCTION CrossMatch RETURNS INTEGER SONAME 'ha_dif.so'//


#Version 0.5.4 enabled UDFs

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMnameById//

#@ONERR_DIE|Cannot install function HTMnameById|
CREATE FUNCTION HTMnameById RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMidByName//

#@ONERR_DIE|Cannot install function HTMidByName|
CREATE FUNCTION HTMidByName RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMLookup//

#@ONERR_DIE|Cannot install function HTMLookup|
CREATE FUNCTION HTMLookup RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPLookup//

#@ONERR_DIE|Cannot install function HEALPLookup|
CREATE FUNCTION HEALPLookup RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS Sphedist//

#@ONERR_DIE|Cannot install function Sphedist|
CREATE FUNCTION Sphedist RETURNS REAL SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMBaryDist//

#@ONERR_DIE|Cannot install function HTMBaryDist|
CREATE FUNCTION HTMBaryDist RETURNS REAL SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPBaryDist//

#@ONERR_DIE|Cannot install function HEALPBaryDist|
CREATE FUNCTION HEALPBaryDist RETURNS REAL SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMBary//

#@ONERR_DIE|Cannot install function HTMBary|
CREATE FUNCTION HTMBary RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMBaryC//

#@ONERR_DIE|Cannot install function HTMBaryC|
CREATE FUNCTION HTMBaryC RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPBary//

#@ONERR_DIE|Cannot install function HEALPBary|
CREATE FUNCTION HEALPBary RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPBaryC//

#@ONERR_DIE|Cannot install function HEALPBaryC|
CREATE FUNCTION HEALPBaryC RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMNeighb//

#@ONERR_DIE|Cannot install function HTMNeighb|
CREATE FUNCTION HTMNeighb RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMsNeighb//

#@ONERR_DIE|Cannot install function HTMsNeighb|
CREATE FUNCTION HTMsNeighb RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HTMNeighbC//

#@ONERR_DIE|Cannot install function HTMNeighbC|
CREATE FUNCTION HTMNeighbC RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPNeighb//

#@ONERR_DIE|Cannot install function HEALPNeighb|
CREATE FUNCTION HEALPNeighb RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPNeighbC//

#@ONERR_DIE|Cannot install function HEALPNeighbC|
CREATE FUNCTION HEALPNeighbC RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPBound//

#@ONERR_DIE|Cannot install function HEALPBound|
CREATE FUNCTION HEALPBound RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPBoundC//

#@ONERR_DIE|Cannot install function HEALPBoundC|
CREATE FUNCTION HEALPBoundC RETURNS STRING SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_setHTMDepth//

#@ONERR_DIE|Cannot install function DIF_setHTMDepth|
CREATE FUNCTION DIF_setHTMDepth RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_setHEALPOrder//

#@ONERR_DIE|Cannot install function DIF_setHEALPOrder|
CREATE FUNCTION DIF_setHEALPOrder RETURNS INTEGER SONAME 'ha_dif.so'//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_clear//

#@ONERR_DIE|Cannot install function DIF_clear|
CREATE FUNCTION DIF_clear RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_cpuTime//

#@ONERR_DIE|Cannot install function DIF_cpuTime|
CREATE FUNCTION DIF_cpuTime RETURNS REAL SONAME 'ha_dif.so'//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_FineSearch//

#@ONERR_DIE|Cannot install function DIF_FineSearch|
CREATE FUNCTION DIF_FineSearch RETURNS INTEGER SONAME 'ha_dif.so'//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_Circle//

#@ONERR_DIE|Cannot install function DIF_Circle|
CREATE FUNCTION DIF_Circle RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_Rect//

#@ONERR_DIE|Cannot install function DIF_Rect|
CREATE FUNCTION DIF_Rect RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_Rectv//

#@ONERR_DIE|Cannot install function DIF_Rectv|
CREATE FUNCTION DIF_Rectv RETURNS INTEGER SONAME 'ha_dif.so'//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_NeighbC//

#@ONERR_DIE|Cannot install function DIF_NeighbC|
CREATE FUNCTION DIF_NeighbC RETURNS INTEGER SONAME 'ha_dif.so'//

#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DIF_sNeighb//

#@ONERR_DIE|Cannot install function DIF_Neighb|
CREATE FUNCTION DIF_sNeighb RETURNS INTEGER SONAME 'ha_dif.so'//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS HEALPMaxS//

#@ONERR_DIE|Cannot install function Sphedist|
CREATE FUNCTION HEALPMaxS RETURNS REAL SONAME 'ha_dif.so'//


#SQL functions below

#@ONERR_IGNORE||
DROP PROCEDURE IF EXISTS difInfo//

#@ONERR_DIE|Cannot create procedure difInfo|
CREATE PROCEDURE difInfo(IN udf VARCHAR(64))
DETERMINISTIC
BEGIN
  DECLARE w_udf VARCHAR(50) DEFAULT '%';

  IF (udf != '') THEN
    SET w_udf = udf;
  END IF;

  SELECT * FROM DIF.func WHERE name like w_udf;
END//


#@ONERR_IGNORE||
DROP PROCEDURE IF EXISTS difview_Check//

#@ONERR_DIE|Cannot create procedure difview_Check|
CREATE PROCEDURE difview_Check()
NOT DETERMINISTIC
BEGIN
  DECLARE dbdif, tabdif, tabview VARCHAR(64);
  DECLARE idtype, nviews INTEGER;
  DECLARE eof INT DEFAULT 0;
  DECLARE c CURSOR FOR SELECT id_type, db, name FROM DIF.tbl;
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET eof = 1;

  OPEN c;

  REPEAT
    FETCH c INTO idtype, dbdif,tabdif;
    IF NOT eof THEN
      IF (idtype = 1) THEN
        SET @pixt = '_htm';
      ELSE
        SET @pixt = '_healp';
      END IF;

      SELECT COUNT(table_name) INTO nviews FROM INFORMATION_SCHEMA.VIEWS WHERE TABLE_SCHEMA=dbdif AND TABLE_NAME like CONCAT(tabdif,@pixt,'%');
      IF (nviews = 0) THEN
        SELECT CONCAT(dbdif,'.',tabdif) as in_DIF_tbl_but_no_view, CONCAT('DELETE FROM DIF.tbl where db="',dbdif,'" AND name="',tabdif,'"') as to_remove_run_command;
      END IF;
    END IF;
  UNTIL eof END REPEAT;
  CLOSE c;

END//


#@ONERR_IGNORE||
DROP PROCEDURE IF EXISTS difview_htmClean//

#@ONERR_DIE|Cannot create procedure difview_htmClean|
CREATE PROCEDURE difview_htmClean(IN doclean BOOLEAN)
NOT DETERMINISTIC
BEGIN
  DECLARE viewdb, viewtab, tabdb, tabname VARCHAR(64);
  DECLARE idtype, depth INTEGER;
  DECLARE in_dif BOOLEAN;
  DECLARE eof INT DEFAULT 0;
  DECLARE c CURSOR FOR SELECT table_schema, table_name FROM INFORMATION_SCHEMA.VIEWS WHERE TABLE_NAME like '%_htm%';
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET eof = 1;
 
  OPEN c;

  REPEAT
    FETCH c INTO viewdb, viewtab; 
    IF NOT eof THEN
      SET @undl = locate("_htm", viewtab);
      SET tabname = left(viewtab,@undl-1);
      SET depth = substr(viewtab,@undl+5);
      IF (depth > 0) THEN
        SELECT COUNT(*) INTO in_dif FROM DIF.tbl WHERE db=viewdb AND name=tabname AND id_type=1 AND param=depth;
      ELSE
        SELECT COUNT(*) INTO in_dif FROM DIF.tbl WHERE db=viewdb AND name=tabname AND id_type=1;
      END IF;

      IF (NOT in_dif) THEN
        SELECT CONCAT(viewdb,'.',viewtab,' not listed in DIF.tbl') as Message;
        IF (doclean) THEN
          SET @sql = CONCAT('DROP VIEW ', viewdb,'.',viewtab);
          PREPARE s1 FROM @sql;
          EXECUTE s1;
          DEALLOCATE PREPARE s1;
        END IF;
      END IF;
    END IF;
  UNTIL eof END REPEAT;
  CLOSE c;

END//


#@ONERR_IGNORE||
DROP PROCEDURE IF EXISTS difview_healpClean//

#@ONERR_DIE|Cannot create procedure difview_healpClean|
CREATE PROCEDURE difview_healpClean(IN doclean BOOLEAN)
NOT DETERMINISTIC
BEGIN
  DECLARE viewdb, viewtab, tabdb, tabname VARCHAR(64);
  DECLARE idtype, depth INTEGER;
  DECLARE in_dif BOOLEAN;
  DECLARE eof INT DEFAULT 0;
  DECLARE c CURSOR FOR SELECT table_schema, table_name FROM INFORMATION_SCHEMA.VIEWS WHERE TABLE_NAME like '%_healp_%';
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET eof = 1;
 
  OPEN c;

  REPEAT
    FETCH c INTO viewdb, viewtab; 
    IF NOT eof THEN
      SET @undl = locate("_healp_", viewtab);
      SET tabname = left(viewtab,@undl-1);
      SET depth = substr(viewtab,@undl+12);
      IF (depth > 0) THEN
        SELECT COUNT(*) INTO in_dif FROM DIF.tbl WHERE db=viewdb AND name=tabname AND id_type=2 AND param=depth;
      ELSE
        SELECT COUNT(*) INTO in_dif FROM DIF.tbl WHERE db=viewdb AND name=tabname AND id_type=2;
      END IF;

      IF (NOT in_dif) THEN
        SELECT CONCAT(viewdb,'.',viewtab,' not listed in DIF.tbl') as Message;
        IF (doclean) THEN
          SET @sql = CONCAT('DROP VIEW ', viewdb,'.',viewtab);
          PREPARE s1 FROM @sql;
          EXECUTE s1;
          DEALLOCATE PREPARE s1;
        END IF;
      END IF;
    END IF;
  UNTIL eof END REPEAT;
  CLOSE c;

END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS getHTMDepth//

#@ONERR_DIE|Cannot create function getHTMDepth|
CREATE FUNCTION getHTMDepth(p_db CHAR(64), p_name CHAR(64))
RETURNS INTEGER
DETERMINISTIC
BEGIN
  RETURN (SELECT param FROM DIF.tbl WHERE db=p_db AND name=p_name AND id_type=1
          ORDER BY param ASC LIMIT 1);
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS getHEALPOrder//

#@ONERR_DIE|Cannot create function getHEALPOrder|
CREATE FUNCTION getHEALPOrder(p_db CHAR(64), p_name CHAR(64))
RETURNS INTEGER
DETERMINISTIC
BEGIN
  RETURN (SELECT param FROM DIF.tbl WHERE db=p_db AND name=p_name AND id_type=2 ORDER BY param ASC LIMIT 1);
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS getHEALPNested//

#@ONERR_DIE|Cannot create function getHEALPNested|
CREATE FUNCTION getHEALPNested(p_db CHAR(64), p_name CHAR(64), p_order INTEGER)
RETURNS INTEGER
DETERMINISTIC
BEGIN
  RETURN (SELECT id_opt FROM DIF.tbl WHERE db=p_db AND name=p_name AND id_type=2 AND param=p_order);
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS getRa//

#@ONERR_DIE|Cannot create function getRa|
CREATE FUNCTION getRa(p_db CHAR(64), p_name CHAR(64))
RETURNS VARCHAR(100)
DETERMINISTIC
BEGIN
  RETURN (SELECT Ra_field FROM DIF.tbl WHERE db=p_db AND name=p_name LIMIT 1);
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS getDec//

#@ONERR_DIE|Cannot create function getDec|
CREATE FUNCTION getDec(p_db CHAR(64), p_name CHAR(64))
RETURNS VARCHAR(100)
DETERMINISTIC
BEGIN
  RETURN (SELECT Dec_field FROM DIF.tbl WHERE db=p_db AND name=p_name LIMIT 1);
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS getRaDec//

#@ONERR_DIE|Cannot create function getRaDec|
CREATE FUNCTION getRaDec(p_db CHAR(64), p_name CHAR(64))
RETURNS VARCHAR(100)
DETERMINISTIC
BEGIN
  RETURN (SELECT CONCAT(Ra_field, ',', Dec_field) FROM (SELECT Ra_field, Dec_field FROM DIF.tbl WHERE db=p_db AND name=p_name LIMIT 1) as t);
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS RAcol//

#@ONERR_DIE|Cannot create function RAcol|
CREATE FUNCTION RAcol(p_db CHAR(64), p_name CHAR(64))
RETURNS VARCHAR(64)
DETERMINISTIC
BEGIN
  DECLARE RAfield VARCHAR(100);
  DECLARE colname VARCHAR(64);
  DECLARE ra_oper, eof INT DEFAULT 0;
  DECLARE c CURSOR FOR SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA=p_db AND TABLE_NAME=p_name;
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET eof = 1;

  SELECT Ra_field INTO RAfield FROM DIF.tbl WHERE db=p_db AND name=p_name LIMIT 1;

  SET ra_oper = ( locate('/',RAfield) OR locate('*',RAfield) );

  OPEN c;

  REPEAT
    FETCH c INTO colname;
    IF NOT eof THEN
      IF ( (colname = RAfield) OR ((locate(colname,RAfield)) > 0 AND ra_oper > 0) ) THEN
        RETURN CONCAT('`', colname, '`');
      END IF;
    END IF;
  UNTIL eof END REPEAT;
  CLOSE c;

  RETURN '';
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS DECcol//

#@ONERR_DIE|Cannot create function DECcol|
CREATE FUNCTION DECcol(p_db CHAR(64), p_name CHAR(64))
RETURNS VARCHAR(64)
DETERMINISTIC
BEGIN
  DECLARE DECfield VARCHAR(100);
  DECLARE colname VARCHAR(64);
  DECLARE dec_oper, eof INT DEFAULT 0;
  DECLARE c CURSOR FOR SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA=p_db AND TABLE_NAME=p_name;
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET eof = 1;

  SELECT Dec_field INTO DECfield FROM DIF.tbl WHERE db=p_db AND name=p_name LIMIT 1;

  SET dec_oper = ( locate('/',DECfield) OR locate('*',DECfield) );

  OPEN c;

  REPEAT
    FETCH c INTO colname;
    IF NOT eof THEN
      IF ( (colname = DECfield) OR ((locate(colname,DECfield)) > 0 AND dec_oper > 0) ) THEN
        RETURN CONCAT('`', colname, '`');
      END IF;
    END IF;
  UNTIL eof END REPEAT;
  CLOSE c;

  RETURN '';
END//


#@ONERR_IGNORE||
DROP FUNCTION IF EXISTS RADECcol//

#@ONERR_DIE|Cannot create function RADECcol|
CREATE FUNCTION RADECcol(p_db CHAR(64), p_name CHAR(64))
RETURNS VARCHAR(64)
DETERMINISTIC
BEGIN
  DECLARE RAfield VARCHAR(100);
  DECLARE DECfield VARCHAR(100);
  DECLARE colRA VARCHAR(64) DEFAULT '';
  DECLARE colDE VARCHAR(64) DEFAULT '';
  DECLARE colname VARCHAR(64);
  DECLARE ra_oper, dec_oper, eof INT DEFAULT 0;
  DECLARE c CURSOR FOR SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA=p_db AND TABLE_NAME=p_name;
  DECLARE CONTINUE HANDLER FOR SQLSTATE '02000' SET eof = 1;

  SELECT Ra_field, Dec_field INTO RAfield, DECfield FROM DIF.tbl WHERE db=p_db AND name=p_name LIMIT 1;

  SET ra_oper = ( locate('/',RAfield) OR locate('*',RAfield) );
  SET dec_oper = ( locate('/',DECfield) OR locate('*',DECfield) );

  OPEN c;

  REPEAT
    FETCH c INTO colname;
    IF NOT eof THEN
      IF ( (colname = RAfield) OR ((locate(colname,RAfield)) > 0 AND ra_oper > 0) ) THEN
	SET colRA = CONCAT('`', colname, '`');
	IF (colDE != '') THEN
	  RETURN CONCAT(colRA, ',', colDE);
	END IF;
      ELSEIF ( (colname = DECfield) OR ((locate(colname,DECfield)) > 0 AND dec_oper > 0) ) THEN
        SET colDE = CONCAT('`', colname, '`');
	IF (colRA != '') THEN
	  RETURN CONCAT(colRA, ',', colDE);
	END IF;
      END IF;
    END IF;
  UNTIL eof END REPEAT;
  CLOSE c;

  RETURN '';
END//
