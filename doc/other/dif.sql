#
# This file show how to install (Sect. 1) and use (Sect. 2)
# the DIF facilities.
#

#---------------------------------------------------------------------

#
# Section 1: install DIF
#
# To install DIF you need to configure the MCS package with the
# "--enable-dif" or "--enable-all" option and provide the path to
# the MySQL source directory through the "--with-mysql-source="
# option. 
#

#
# Once MCS has been correctly compiled and installed you can proceed
# installing DIF on the MySQL database server. To do so you will need a
# running MySQL server (version >= 5.1.20) and an open mysql terminal
# to execute the following SQL statement.
#

#
# Ensure the DIF shared library is accessible from MySQL, to do so
# execute the statment:
#
SHOW VARIABLES LIKE 'plugin_dir';
#
# and check that the ha_dif.so is present in that directory. To be
# sure that the link cache has been refreshed issue the command:
#
#   ldconfig
#
# from a root shell.
#

#
# Create a dedicated database.
#
CREATE DATABASE DIF;
USE DIF;

#
# Set up the DIF plugin.
#
INSTALL PLUGIN dif SONAME 'ha_dif.so';
SHOW PLUGINS;



#
# Install external functions
#
# The DIF_FineSearch function is not meant to be used by users.
#
DROP FUNCTION IF EXISTS DIF_FineSearch;
CREATE FUNCTION DIF_FineSearch RETURNS INTEGER SONAME 'ha_dif.so';

DROP FUNCTION IF EXISTS DIF_setHTMDepth;
CREATE FUNCTION DIF_setHTMDepth RETURNS INTEGER SONAME 'ha_dif.so';

DROP FUNCTION IF EXISTS DIF_setHEALPDepth;
CREATE FUNCTION DIF_setHEALPDepth RETURNS INTEGER SONAME 'ha_dif.so';



#
# Sphedist(Ra_deg1 DOUBLE, Dec_deg1 DOUBLE, Ra_deg2 DOUBLE, Dec_deg2 DOUBLE)
#
# Calculate spherical distance on a sphere given the coordinates of
# two points. All values are in degree.
# 
# Ra_deg1, Dec_deg1, Ra_deg2, Dec_deg2 are in degrees.
# Return value is in degrees.
#
DROP FUNCTION IF EXISTS Sphedist;
CREATE FUNCTION Sphedist RETURNS REAL SONAME 'ha_dif.so';



#
# DIF_HTMCircle(Ra_deg DOUBLE, Dec_deg DOUBLE, Rad_deg DOUBLE)
#
# Generate htmID for those pixel in a circle centered at Ra_deg,
# Dec_deg, with a radius of Rad_arcmin.
#
# Ra_deg, Dec_deg and Rad_deg are in degrees.
#
# Returns:
#  how many pixels falls into the circle
#
DROP FUNCTION IF EXISTS DIF_HTMCircle;
CREATE FUNCTION DIF_HTMCircle RETURNS INTEGER SONAME 'ha_dif.so';

DROP FUNCTION IF EXISTS DIF_HTMRect;
CREATE FUNCTION DIF_HTMRect RETURNS INTEGER SONAME 'ha_dif.so';

DROP FUNCTION IF EXISTS DIF_HEALPCircle;
CREATE FUNCTION DIF_HEALPCircle RETURNS INTEGER SONAME 'ha_dif.so';

#
# HTMLookup(Depth INT, Ra_deg DOUBLE, Dec_deg DOUBLE)
#
# Lookup the HTM id of the pixel which contain the point indicated
# with Ra_deg and Dec_deg. The firs parameter specify the depth of
# pixelization .
#
# Depth is an integer between 0 and 25;
# Ra_deg, Dec_deg are in degrees.
#
# Returns:
#  the HTM id of the pixel
#
DROP FUNCTION IF EXISTS HTMLookup;
CREATE FUNCTION HTMLookup RETURNS INTEGER SONAME 'ha_dif.so';

DROP FUNCTION IF EXISTS HEALPLookup;
CREATE FUNCTION HEALPLookup RETURNS INTEGER SONAME 'ha_dif.so';


#
# Create the htm table.
#
DROP TABLE IF EXISTS htm;
CREATE TABLE htm(htmID BIGINT, full BOOL) ENGINE=DIF;

#
# Test the DIF dbengine
#
#SELECT DIF_setHTMDepth(6);
#SELECT DIF_setHEALPDepth(64);
#
#SELECT * FROM htm;
#SELECT * FROM htm WHERE DIF_HTMCircle(83.625, 22.0167, 2);
#SELECT * FROM htm WHERE DIF_HEALPCircle(83.625, 22.0167, 2);
#



#---------------------------------------------------------------------

#
# Section 2: use DIF
#

#
# Suppose you want to use the HTM index on a catalog stored in the
# database DB, table CAT, in which the object coordinate are stored
# in the RA and DECL fields (values in degrees).
#
USE DB;

#
# Add a column and an index for the HTM id to the table
#
ALTER TABLE CAT ADD COLUMN htmID BIGINT, ADD INDEX(htmID);
DESCRIBE CAT;

#
# Populate new column with htmIDs
#
UPDATE CAT SET htmID = HTMLookup(6, Ra, Decl);

#
# Set up a trigger to automatically update the htmID field when a new
# record is inserted
#
delimiter //
DROP TRIGGER trig_ins_CAT//
CREATE TRIGGER trig_ins_CAR  BEFORE INSERT ON CAT FOR EACH ROW
BEGIN
  SET NEW.htmID = HTMLookup(6, NEW.RA, NEW.DECL);
END//

#
# Do the same for updated records;
#
DROP TRIGGER trig_upd_CAT//
CREATE TRIGGER trig_upd_CAT BEFORE UPDATE ON CAT FOR EACH ROW
BEGIN
  SET NEW.htmID = HTMLookup(6, NEW.RA, NEW.DECL);
END//
delimiter ;


#
# Create a view to automatically perform the join on the htm table.
#
DROP VIEW IF EXISTS CAT_htm;
CREATE ALGORITHM=MERGE VIEW CAT_htm AS
SELECT CAT.*
  FROM CAT INNER JOIN DIF.htm USING(htmID)
  WHERE DIF_setHTMDepth(6)  AND  DIF_FineSearch(Ra, Decl, DIF.htm.full);

#
# Finally perform the query
#
SELECT * FROM CAT_htm WHERE DIF_HTMCircle(83.625, 22.0167, 2);


#
# The same result can be obtained without using the DIF facilities.
#
SELECT CAT.*
  FROM CAT 
  WHERE DIF_sphedist(83.625, 22.0167, Ra, Decl) < 2; 




#
# To deinstall the DIF plugin execute the following statements:
#
DROP FUNCTION IF EXISTS DIF_FineSearch;
DROP FUNCTION IF EXISTS Sphedist;
DROP FUNCTION IF EXISTS DIF_HTMCircle;
DROP FUNCTION IF EXISTS DIF_HTMRect;
DROP FUNCTION IF EXISTS HTMLookup;
DROP FUNCTION IF EXISTS HEALPLookup;
DROP FUNCTION IF EXISTS HEALPNeighb;
DROP FUNCTION IF EXISTS HEALPNeighbC;
DROP FUNCTION IF EXISTS DIF_HEALPNeighbC;
DROP FUNCTION IF EXISTS DIF_setHTMDepth;
DROP FUNCTION IF EXISTS DIF_setHEALPDepth;

DROP DATABASE IF EXISTS DIF;
UNINSTALL PLUGIN dif;

