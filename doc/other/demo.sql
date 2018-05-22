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
# Use the "DIF" database if you have DIF installed, otherwise can
# use the "test" database.

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




# To use the sky pixelization facilities, you must have DIF installed!
# If not, you cannot use the DIF* functions.
#
# To index the Messier table, from the shell give the command:
# dif --index-htm DIF Messier 6 Ra Decl
# Then you can select objects in a circular region, e.g.:
SELECT *
FROM Messier_htm
WHERE DIF_Circle(@Ra, @Decl, @Rad);

# or a square region of side 10 times @Rad:
SELECT *
FROM Messier_htm
WHERE DIF_Rect(@Ra, @Decl, @Rad*10);


-- DIF makes sense for very large catalogues. See the DIF manual and other
-- reference material on ross.iasfbo.inaf.it/MCS/.
--
-- Can have a look to "demo_manual_dif.sql" (in the doc/other subdir of the
-- DIF dsitribution) for a manual implementation of DIF functionalities.
-- If you are not interested in the development of DB related tools you
-- should only use the automatic implementation of DIF using the "dif" script. 
