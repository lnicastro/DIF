#@ONERR_DIE|Database DIF does not exists|
USE DIF//



#@ONERR_IGNORE||
DROP TABLE IF EXISTS Messier//

#@ONERR_DIE|Cannot create table Messier|
CREATE TABLE Messier (
	M int NOT NULL, 
	Type CHAR(2) DEFAULT '**',
	Const CHAR(3) DEFAULT '***', 
	Mag FLOAT, 
	Ra  FLOAT, 
	Decl FLOAT, 
	Dist CHAR(20), 
	App_size CHAR(20) DEFAULT 'unknown'
) COMMENT='DIF test catalogue /usr/local/share/dif/sql/messier'//



#@ONERR_DIE|Cannot load data into table|
LOAD DATA INFILE
'/usr/local/share/dif/sql/messier'
INTO TABLE Messier//


# Convert Ra to degrees.
UPDATE Messier SET Ra = Ra * 15.0;
