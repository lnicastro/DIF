#DIF
DIF is a collection of tools aimed at implementing a powerful indexing system for astronomical catalogs and other data with spherical coordinates, stored into MySQL / MariaDB databases.
DIF is able to use both HTM and HEALPix pixelization schemas and it allows very fast query execution even on billion-row tables. 
Because it requires to create and enable a storage engine, you need to install it using the MySQL source code. See the documentation.

## Requirements

1. MySQL / MariaDB source code compiled and installed
2. GNU Autotools + make or gmake
3. Perl `DBI/DBD-mysql` modules

## Compile and install
Assuming that you have downloaded and compiled + installed MySQL 5.7.22:
```
tar zxvf dif-0.5.4.tar.gz

cd dif-0.5.4
./configure --with-mysql-source=/path_to/mysql-5.7.22
make
sudo make install
```


## Installing DIF facilities in MySQL
To actually enable the DIF facilities, you need to run the installation command:
```
dif --instal```
```

You'll be asked the MySQL root password to complete this task.
**Note:** `dif` is a Perl script used to perform various DIF-related tasks. If you use `tcsh` you might need to run `rehash` to have the command visible in an existing terminal. See the manual for a full description or run:
```
dif --help
```

Assuming the command executes successfully, you now need to restart the MySQL server to make the new DIF storage engine working.
Depending on how you started the server, you could need to use one of these commands:
```
sudo /etc/init.d/mysql.server restart
```
or
```
sudo /usr/local/mysql/bin/mysqladmin -u root -p shutdown
sudo /usr/local/mysql/bin/mysqld_safe --user=mysql &amp;
```

## Test installation
Download the reduced version of the [ASCC 2.5](http://ross2.iasfbo.inaf.it/dif/data/ascc25_mini.sql.gz) star catalogue in a working directory, say `dif_data`. Can also download the file manually:
```
shell> mkdir ~/dif_data
shell> cd ~/dif_data
shell> wget http://ross2.iasfbo.inaf.it/dif/data/ascc25_mini.sql.gz
```

Uncompress and load the data into a database of your choice, e.g. `Catalogs`;
```
shell> cd ~/dif_data
shell> gunzip ascc25_mini.sql.gz

mysql> create database Catalogs;
mysql> use Catalogs;
mysql> source ~/dif_data/ascc25_mini.sql
mysql> describe ascc25_mini;
+-----------+------------------+------+-----+---------+-------+
| Field     | Type             | Null | Key | Default | Extra |
+-----------+------------------+------+-----+---------+-------+
| RAmas     | int(10) unsigned | NO   |     | 0       |       |
| DECmas    | int(11)          | NO   |     | 0       |       |
| RAPMdmas  | smallint(6)      | NO   |     | 0       |       |
| DECPMdmas | smallint(6)      | NO   |     | 0       |       |
| Bmm       | mediumint(9)     | NO   |     | 0       |       |
| Vmm       | mediumint(9)     | NO   |     | 0       |       |
| FLAGvar   | smallint(6)      | NO   |     | 0       |       |
+-----------+------------------+------+-----+---------+-------+
7 rows in set (0.00 sec)

```

Let's index the table with a depth 6 HTM index:
```
shell> dif --index-htm Catalogs ascc25 6 "RAmas/3.6e6" "DECmas/3.6e6"
```

Among other things, this will create the "table view" `ascc25_mini_htm_6`. This is the table that we must use in place of `ascc25_mini` when DIF specific functions are used in the WHERE clause of the query.

Let's index the table also with a n order 10 HEALPix index. The first parameter of `HEALPLookup` set to 1 selects the *nested* schema.
```
shell> dif --index-healpix-nested Catalogs ascc25_mini 10 "RAmas/3.6e6" "DECmas/3.6e6"
```

**Some queries on circular and rectangular regions:**

Enter the MySQL client terminal, e.g. `mysql -u root -p Catalogs`, then:

```
-- all the info for objects in a circle or radius 18 arcmin around RA=30, Dec=30
  SELECT * FROM ascc25_mini_htm_6 WHERE dif_Circle(30,30,18);

-- as above, but returning coordinates and magnitudes only in standard format
  SELECT RAmas/3.6e6 as RA, DECmas/3.6e6 as Decl, Bmm/1000 as B, Vmm/1000 as V
    FROM ascc25_mini_htm_6
    WHERE dif_Circle(30,30,18);

-- only magnitudes V less than 11 for objects in a square with side 33 arcmin
  SELECT Bmm/1000 as B, Vmm/1000 as V
     FROM ascc25_mini_htm_6
     WHERE dif_Rect(100,-20,33) and Vmm &lt; 11000;

-- a query using the HEALPix indexing
  SELECT ramas/3.6e6 as radeg, decmas/3.6e6 as decdeg, Vmm/1000 as Vmag, (Bmm-Vmm)/1000 as color
     FROM ascc25_mini_healp_10
     WHERE dif_Circle(30,-20,30);
```
