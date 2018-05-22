#
# To execute these examples you should login as user "root" with:
#
# mysql --local-infile=1 -u root -p
#
# and type the password when requested.
#
# In the following examples we will assume that DIF has been properly 
# installed. Remember that comment lines start with # or -- !
# Also note that functions can only return 1 value!
#
# Use the "DIF" database
USE DIF

#
# Set geographic coordinate and time zone offset (Palermo)
#
SET @Long := 13;
SET @Lat := 38;
SET @TZ_offset := -1;

#
# Coordinate of M1 (Crab Nebula):  RA  = 5.575   (Hours)
#                                  DEC = 22.0167 (Degrees);
#
# To convert from Hours to Degrees simply multiply by 15.0
#
SET @Ra := 5.575 * 15.0;
SET @Decl := 22.0167;


#
# This variable will be used to perform circular selection
#
# Radius in degrees
#
SET @Rad := 10;


#
# From now on all angles will be in Degrees
#


# Create the table that will contain the Messier catalog
DROP TABLE IF EXISTS Messier;
CREATE TABLE Messier (
	M int NOT NULL, 
	Type CHAR(2) DEFAULT '**',
	Const CHAR(3) DEFAULT '***', 
	Mag FLOAT, 
	Ra  FLOAT, 
	Decl FLOAT, 
	Dist CHAR(20), 
	App_size CHAR(20) DEFAULT 'unknown', 
        PRIMARY KEY(M),
        INDEX(Type)
);

DESCRIBE Messier;
SELECT * FROM Messier;


# Load data into the table
LOAD DATA LOCAL INFILE
'./messier'  
INTO TABLE Messier;

SELECT * FROM Messier;


# Convert Ra to degrees.
UPDATE Messier SET Ra = Ra * 15.0;
SELECT * FROM Messier;


# Create the table that will contain the description of each object type
DROP TABLE IF EXISTS TypeDescr;
CREATE TABLE TypeDescr(
      Type CHAR(2), 
      Descr CHAR(20), 
      PRIMARY KEY(Type)
);
DESCRIBE TypeDescr;

# Load data into the table
LOAD DATA LOCAL INFILE
'./legend'  
INTO TABLE TypeDescr;

SELECT * FROM TypeDescr;



# A simple join to show the long description of each object type
SELECT Messier.*, TypeDescr.Descr 
FROM Messier LEFT JOIN TypeDescr
ON Messier.Type = TypeDescr.Type;




# From here we assume you have DIF installed!
# To index the table, from the shell give the command:
# dif --index-htm DIF Messier 6 Ra Decl
SELECT *
FROM Messier_htm
WHERE DIF_Circle(@Ra, @Decl, @Rad);






# Examples of useful functions and procedures

delimiter //
#
# Compute distance between two points on a sphere
#
#  Note that DIF already has a Sphedist function!
#  This is just for demonstration.
#
# a1/b1: long/lat of first point (DEG)
# a2/b2: long/lat of second point (DEG)
#
# Returns:
#    distance in Degrees
#
DROP FUNCTION IF EXISTS sphedist//
CREATE FUNCTION sphedist(a1 DOUBLE, b1 DOUBLE, a2 DOUBLE, b2 DOUBLE) RETURNS DOUBLE
BEGIN
  DECLARE deg2rad, a1r, b1r, a2r, b2r, radiff, cosdis, dist DOUBLE;

  SET deg2rad = 0.017453292519943;
  SET a1r = a1*deg2rad;
  SET b1r = b1*deg2rad;
  SET a2r = a2*deg2rad;
  SET b2r = b2*deg2rad;
  SET radiff = ABS(a2r-a1r);
  IF (radiff > pi()) THEN
    SET radiff = pi() * 2e0 - radiff;
  END IF;
  SET cosdis = sin(b1r)*sin(b2r) + cos(b1r)*cos(b2r)*cos(radiff);
  SET dist = acos(cosdis)/deg2rad;

  RETURN dist;
END//


-- From year, month, day, hour to Julian day
DROP FUNCTION IF EXISTS jdcnv//
CREATE FUNCTION jdcnv(yr INT, mn INT, day INT, hr DOUBLE) RETURNS DOUBLE
BEGIN
  DECLARE L DOUBLE;
  DECLARE j BIGINT;
  DECLARE julian DOUBLE;

  SET L = (mn-14) DIV 12;
  SET j = day - 32075 + 1461*(yr+4800+L) DIV 4 + 
               367*(mn - 2-L*12) DIV 12 - 3*((yr+4900+L) DIV 100) DIV 4;

  SET julian = j + hr/24.0 -0.5;
  RETURN julian;
END//


-- From hour angle and declination to Alt, Az coordinates
DROP PROCEDURE IF EXISTS hadec2altaz//
CREATE PROCEDURE hadec2altaz(IN ha DOUBLE, IN decl DOUBLE, IN lat DOUBLE, 
                             OUT alt DOUBLE, OUT az DOUBLE)
BEGIN
  DECLARE d2r, sh, ch, sd, cd, sl, cl DOUBLE;
  DECLARE x, y, z, r DOUBLE;

  SET d2r = pi()/180.0;
  
  SET sh = sin(ha*d2r);
  SET ch = cos(ha*d2r);
  SET sd = sin(decl*d2r);
  SET cd = cos(decl*d2r);
  SET sl = sin(lat*d2r);
  SET cl = cos(lat*d2r);

  SET x = - ch * cd * sl + sd * cl;
  SET y = - sh * cd;
  SET z = ch * cd * cl + sd * sl;
  SET r = sqrt(x*x + y*y);
  
  SET az = atan(y,x) / d2r;
  SET alt = atan(z,r) / d2r;

  IF (az < 0) THEN
    SET az = az + 360;
  END IF;
END//


-- From hour angle and declination to Altitude
DROP FUNCTION IF EXISTS hadec2alt//
CREATE FUNCTION hadec2alt(ha DOUBLE, decl DOUBLE, lat DOUBLE) RETURNS DOUBLE
BEGIN
  DECLARE alt, az DOUBLE;
  CALL hadec2altaz(ha, decl, lat, alt, az);

  RETURN alt;
END//


-- From hour angle and declination to Azimuth
DROP FUNCTION IF EXISTS hadec2az//
CREATE FUNCTION hadec2az(ha DOUBLE, decl DOUBLE, lat DOUBLE) RETURNS DOUBLE
BEGIN
  DECLARE alt, az DOUBLE;
  CALL hadec2altaz(ha, decl, lat, alt, az);

  RETURN az;
END//


-- From Julian day to local sidereal time
DROP FUNCTION IF EXISTS jd2st//
CREATE FUNCTION jd2st(jd DOUBLE, lng DOUBLE) RETURNS DOUBLE
BEGIN
  DECLARE c0, c1, c2, c3 DOUBLE;
  DECLARE jd2000, lst DOUBLE;
  DECLARE t, t0, theta DOUBLE;

  SET c0 = 280.46061837;
  SET c1 = 360.98564736629;
  SET c2 = 0.000387933;
  SET c3 = 38710000.0;
 
  SET jd2000 = 2451545.0;

  SET t0 = jd - jd2000;
  SET t = t0/36525.0;
  
  SET theta = c0 + (c1 * t0) + t*t*(c2 - t / c3 );
  
  SET lst = (theta + lng)/15.0;

  IF (lst < 0) THEN
    SET lst = 24.0 + (lst % 24.0);
  END IF;
  SET lst = lst % 24.0;

  RETURN lst;
END//


-- From local DATETIME and Time zone to Julian day
DROP FUNCTION IF EXISTS Julian//
CREATE FUNCTION Julian(Time DATETIME, TZ_offset INT) RETURNS DOUBLE
BEGIN
  DECLARE Yr, Mn, Dy, Hr, Mi, Se INT;
  DECLARE Hr_fr, jd DOUBLE;

  SET Yr = year(Time);
  SET Mn = month(Time);
  SET Dy = day(Time);
  SET Hr = hour(Time);
  SET Mi = minute(Time);
  SET Se = second(Time);
  SET Hr_fr =(Hr+TZ_offset) + Mi/60.0 + Se/3600.0;
  
  SET jd = jdcnv(Yr, Mn, Dy, Hr_fr);
  RETURN jd;
END//


-- From local DATETIME and Time zone to local sidereal time
DROP FUNCTION IF EXISTS Sidereal//
CREATE FUNCTION Sidereal(Time DATETIME, TZ_offset INT, Lng DOUBLE) RETURNS DOUBLE
BEGIN
  DECLARE jd, st DOUBLE;

  SET jd = Julian(Time, TZ_offset);
  SET st = jd2st(jd, Lng);
  RETURN st;
END//


-- From equatorial Ra, Dec to Altitude
DROP FUNCTION IF EXISTS radec2alt//
CREATE FUNCTION radec2alt(Ra DOUBLE, Decl DOUBLE, Lat DOUBLE, Lng DOUBLE, 
                          TZ_offset INT, Time DATETIME)
RETURNS DOUBLE
BEGIN
  DECLARE st, ha, alt DOUBLE;

  SET st = Sidereal(Time, TZ_offset, Lng) * 15.0;
  SET ha = st - Ra;
  SET alt = hadec2alt(ha, Decl, Lat);
  RETURN alt;
END//


-- From equatorial Ra, Dec to Azimuth
DROP FUNCTION IF EXISTS radec2az//
CREATE FUNCTION radec2az(Ra DOUBLE, Decl DOUBLE, Lat DOUBLE, Lng DOUBLE, 
                         TZ_offset INT, Time DATETIME)
RETURNS DOUBLE
BEGIN
  DECLARE st, ha, az DOUBLE;

  SET st = Sidereal(Time, TZ_offset, Lng) * 15.0;
  SET ha = st - Ra;
  SET az = hadec2az(ha, Decl, Lat);
  RETURN az;
END//
delimiter ;



#
# Example of usage of jdcnv and jd2st
#
SELECT  @Time:= now() AS Time, 
	@Yr:=year(@Time) AS Year, 
	@Mn:=month(@Time) AS Month, 
	@Dy:=day(@Time) AS Day,
	@Hr:=hour(@Time) AS Hour,
	@Mi:=minute(@Time) AS Minute,
	@Se:=second(@Time) AS Second,
	@Hr_fr:=(@Hr+@TZ_offset) + @Mi/60.0 + @Se/3600.0 AS Hr_fr,
	@Jd:=jdcnv(@Yr, @Mn, @Dy, @Hr_fr) AS Julian,
	@Si:=jd2st(@Jd, @Long) AS Sidereal;

# Do the same with
SELECT Julian(now(), @TZ_offset), Sidereal(now(), @TZ_offset, @Long);

#
# Example of usage of hadec2alt, hadec2az
# (coordinate conversion from Ra/Dec to Alt/Az)
#
# Coompute sidereal time and convert to Degrees
#
SET @Si := Sidereal(now(), @TZ_offset, @Long) * 15.0;

#
# Compute hour angle
#
SET @Hour_angle := @Si - @Ra;

SELECT @Ra, @Decl, @Hour_angle, 
       hadec2alt(@Hour_angle, @Decl, @Lat) AS Alt, 
       hadec2az( @Hour_angle, @Decl, @Lat) AS Az;

# Do the same with
SELECT @Ra, @Decl, @Hour_angle,
       radec2alt(@Ra, @Decl, @Lat, @Long, @TZ_offset, now()) AS Alt,
       radec2az( @Ra, @Decl, @Lat, @Long, @TZ_offset, now()) AS Az;


#
# Note: variables (like @Lat, @Long, @TZ_offset) cannot be used inside
# views.
#
DROP VIEW IF EXISTS Messier_view;
CREATE VIEW Messier_view AS
  SELECT Messier.M, 
         TypeDescr.Descr AS Descr,
         Messier.Const,
         Messier.Mag,
         Messier.Ra,
         Messier.Decl,
         radec2alt(Messier.Ra, Messier.Decl, 38, 13, -1, now()) AS Alt,
         radec2az( Messier.Ra, Messier.Decl, 38, 13, -1, now()) AS Az,
         Messier.Dist,
         Messier.App_size
  FROM Messier LEFT JOIN TypeDescr
  ON Messier.Type = TypeDescr.Type
  ORDER BY Descr, M;

DESCRIBE Messier_view;
SELECT * FROM Messier_view;



-- This part deals with the "manual" management of DIF to index tables.
-- You should use the script "dif" to do this, but if you want to practice
-- with DB managment you can try this examples.
#
# Set up the DIF_Rect, DIF_Circle, and HTMLookup functions.
# 
# NOTE: this require that DIF has been installed and that the
# 'plugin_dir' variable of MySQL has been set to the library path of
#  MyRO (default is /usr/local/lib).
# 
# NOTE 2: after DIF has been installed issue 'ldconfig' from a root
# shell if you have troubles seeing the functions.
# 

#
# DIF_Circle(Ra_deg DOUBLE, Dec_deg DOUBLE, Rad_arcmin DOUBLE)
#
# Generate htmIDs for those pixel in a circle centered at Ra_deg, Dec_deg
# (deg) with a radius of Rad_arcmin (arcmin).
#
# Returns:
#  how many pixels fall into the circle
#
DROP FUNCTION IF EXISTS DIF_Circle;
CREATE FUNCTION DIF_Circle RETURNS INTEGER SONAME 'ha_dif.so';

#
# HTMLookup(depth INT, Ra_deg DOUBLE, Dec_deg DOUBLE)

# Lookup the htmID, at a given depth, of the pixel which contains the point
# with with coordinates Ra_deg and Dec_deg.
#
# Returns:
#  the HTM id of the pixel at the requeste depth
#
DROP FUNCTION IF EXISTS HTMLookup;
CREATE FUNCTION HTMLookup RETURNS INTEGER SONAME 'ha_dif.so';


-- This is to query a rectangular region - see DIF manual.
DROP FUNCTION IF EXISTS DIF_Rect;
CREATE FUNCTION DIF_Rect RETURNS INTEGER SONAME 'ha_dif.so';





#
# Add a column and an index for the HTM id (depth 6) to the Messier table
#
ALTER TABLE Messier ADD COLUMN htmID_6 SMALLINT UNSIGNED, ADD INDEX(htmID_6);

DESCRIBE Messier;

# Populate the new column with htmIDs
UPDATE Messier SET htmID_6 = HTMLookup(6, Ra, Decl);
SELECT * FROM Messier;


#
# Set up a trigger to automatically update the htmID_6 field when a new
# record is inserted
#
delimiter //
DROP TRIGGER trig_ins_Messier//
CREATE TRIGGER trig_ins_Messier BEFORE INSERT ON Messier FOR EACH ROW
BEGIN
  SET NEW.htmID_6 = HTMLookup(6, NEW.Ra, NEW.Decl);
END//

#
# Do the same for updated records;
#
DROP TRIGGER trig_upd_Messier//
CREATE TRIGGER trig_upd_Messier BEFORE UPDATE ON Messier FOR EACH ROW
BEGIN
  SET NEW.htmID_6 = HTMLookup(6, NEW.Ra, NEW.Decl);
END//
delimiter ;



-- Again, this is automatically done using "dif --install", but can practice with
-- plugins and UDFs

DROP TABLE IF EXISTS dif;


# Now set up the database DIF DBengine
INSTALL PLUGIN dif SONAME 'ha_dif.so';

#
# This is for debug
#
#SET engine_condition_pushdown = On;

#
# NOTE: If you stop MySQL server while the DIF engine is installed it
# once happened that it did not restart again. Now it should not happen
# anymore, however before stopping the server you can issue the command:
#
# UNINSTALL PLUGIN dif;
#
#
#
# If you experience problems restarting MySQL then follow these steps
# (do not worry about errors, assume Myro has been installed in
# /usr/local):
#
# rm /usr/local/lib/ha_dif.*
# start MySQL server
#
# 
# DROP TABLE IF EXISTS dif;
# UNINSTALL PLUGIN dif;
# DROP FUNCTION DIF_Rect;
# DROP FUNCTION DIF_Circle;
# DROP FUNCTION HTMLookup;
# DELETE FROM mysql.plugin WHERE name = 'dif';
# DELETE FROM mysql.func WHERE name = 'DIF_Rect'  
#                          OR  name = 'DIF_Circle'  
#                          OR  name = 'HTMLookup';
#
# restart from the beginning of this file
#

SHOW PLUGINS;


# Create the (fictitious) table dif 
DROP TABLE IF EXISTS dif;
CREATE TABLE dif(field INT, id BIGINT, full BOOL) ENGINE=DIF;


#
# A select without calling DIF_Circle neither DIF_Rect will result in
# an empty result set.
#
SELECT * FROM dif;

#
# A select with a WHERE cluase using DIF_Circle will return all HTM
# ids of the pixels falling into that circle. The "full" field tells
# if a pixel is completely or partially covered by the circle.
#
SELECT * FROM dif WHERE DIF_Circle(@Ra, @Decl, 2);


#
# Select all objects in a circle centered at @Ra, @Decl, with a radius
# of @Rad.
#
# Remember: @Ra, @Decl and @Rad MUST be in DEGREES.
#
SELECT @Ra, @Decl, @Rad;

SELECT Messier.*
  FROM Messier INNER JOIN dif USING(htmID_6) 
  WHERE DIF_Circle(@Ra, @Decl, @Rad);

#
# Note that the result set is roughly sorted by htmID_6, this is because
# the index on htmID_6 has been used.
#
# Verify that we are using the index on htmID_6 when reading from the
# Messier table:
#
EXPLAIN
SELECT Messier.* 
  FROM Messier INNER JOIN dif USING(htmID_6) 
  WHERE DIF_Circle(@Ra, @Decl, @Rad);


#
# Create a view that automatically perform the join with dif table
#
DROP VIEW IF EXISTS Messier_dif;
CREATE ALGORITHM = MERGE  VIEW Messier_dif AS
SELECT Messier.*, dif.full AS htmFull
  FROM Messier INNER JOIN dif USING(htmID_6);


#
# Now we can obtain the same result as before with
#
SELECT *
FROM Messier_dif
WHERE DIF_Circle(@Ra, @Decl, @Rad);

#
# Do the same query as above adding a field which tells the angular
# distance from the center of the circle
#
SELECT *, sphedist(@Ra, @Decl, Ra, Decl) AS ang_dist 
FROM Messier_dif
WHERE DIF_Circle(@Ra, @Decl, @Rad);


#
# You will notice that in the result set there is an object with a
# distance of about 11 degrees, which is greater than the radius of
# the circle (10 degrees). That is because the HTM pixels are roughly
# triangular so also the border of our circle is "roughly" circular.
#
# In other words the circle covers completely some HTM pixels, while
# others are partially covered. Objects lying in those partially
# covered pixels are selected as well, even if they are at a greater
# distance than the radius.
#
# We can overcome this problem adding a condition on those partially
# covered pixels, as follows:
#
SELECT *, sphedist(@Ra, @Decl, Ra, Decl) AS ang_dist
FROM Messier_dif
WHERE (DIF_Circle(@Ra, @Decl, @Rad)   AND
      ((htmFull = 1)   OR   (sphedist(@Ra, @Decl, Ra, Decl) < @Rad)));







#
# Some auxiliary function...  
# (needed because an User Defined Variable, such as @Ra, @Decl,
# etc..., cannot be used inside a view definition)
# 
delimiter //
DROP FUNCTION IF EXISTS DIF_Ra//
CREATE FUNCTION DIF_Ra() RETURNS DOUBLE
BEGIN
  RETURN @Ra;
END//

DROP FUNCTION IF EXISTS DIF_Dec//
CREATE FUNCTION DIF_Dec() RETURNS DOUBLE
BEGIN
  RETURN @Decl;
END//

DROP FUNCTION IF EXISTS DIF_Rad//
CREATE FUNCTION DIF_Rad() RETURNS DOUBLE
BEGIN
  RETURN @Rad;
END//

DROP FUNCTION IF EXISTS DIF_getPar//
CREATE FUNCTION DIF_getPar(id INT) RETURNS DOUBLE
BEGIN
  RETURN CASE id
           WHEN 1 THEN DIF_Ra()
           WHEN 2 THEN DIF_Dec()
           WHEN 3 THEN DIF_Rad()
           ELSE NULL
         END;
END//

DROP FUNCTION IF EXISTS DIF_setPars//
CREATE FUNCTION DIF_setPars(Ra DOUBLE, Decl DOUBLE, Rad DOUBLE) RETURNS INT
BEGIN
  SET @Ra = Ra;
  SET @Decl = Decl;
  SET @Rad = Rad;
  RETURN 1;
END//
delimiter ;




#
# Now create the view that performs the circular selection in the
# Messier table.
#
DROP VIEW IF EXISTS Messier_Circle;
CREATE ALGORITHM = MERGE VIEW Messier_Circle AS
SELECT *
FROM Messier INNER JOIN dif USING(htmID_6)
WHERE (DIF_Circle(DIF_Ra(), DIF_Dec(), DIF_Rad())   AND
        ((dif.Full = 1)   OR   
        (sphedist(DIF_Ra(), DIF_Dec(), Ra, Decl) < DIF_Rad()))
      );


#
# Circular selection (finally !)
#
SELECT *
FROM Messier_Circle WHERE DIF_setPars(30, 30, 20);

#
# If you want to add the spherical distance to the field list...
#
SELECT *, sphedist(DIF_Ra(), DIF_Dec(), Ra, Decl) AS ang_dist
FROM Messier_Circle WHERE DIF_setPars(30, 30, 20);




# Can try with a bigger table.
#
#SELECT count(*) FROM mmm;
#+----------+
#| count(*) |
#+----------+
#|  1897832 | 
#+----------+
#
#SELECT * FROM mmm INNER JOIN dif USING(htmID_6) WHERE DIF_Circle(0, 0, 0);
#SELECT * FROM mmm WHERE M = 111;
#
