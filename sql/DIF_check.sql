#@ONERR_DIE|Database DIF does not exists|
USE DIF//

#@ONERR_DIE|Table tbl does not exists|
SELECT * FROM tbl LIMIT 0//

#@ONERR_DIE|Function Sphedist does not exists|
SELECT Sphedist(0, 0, 0, 0)//
