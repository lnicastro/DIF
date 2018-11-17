# DIF
DIF is a collection of tools aimed at implementing a powerful indexing system for astronomical catalogs and other data with spherical coordinates, stored into MySQL / MariaDB databases.
DIF is able to use both [HTM](http://www.skyserver.org/htm/) and [HEALPix](http://healpix.jpl.nasa.gov/) pixelization schemas and it allows very fast query execution even on billion-row tables. 
Because it requires to create and enable a storage engine, you need to install it using the MySQL source code. See this [web page](http://ross.iasfbo.inaf.it/dif/) or the [documentation](doc/dif.pdf) and the [reference paper](http://www.hindawi.com/journals/aa/2010/524534.html).

If you want to use most of the DIF capabilities but avoid to compile MySQL / MariaDB source code, please use [SID](https://github.com/lnicastro/SID) instead.

This is the **Version 0.5.5** development tree.

## Requirements

1. MySQL / MariaDB source code configure cia `cmake` (same version as the system version OR installed)
2. make or gmake
3. Perl `DBI/DBD-MySQL` modules

## Compile and install
It depends on how you installed MySQL on your machine.

**Note:** as of version 0.5.5 you do not need to compile and install the MySQL source version.

**Note:** if you download DIF via `git clone`, to avoid autotools requirements with a message like this:
```
...
DIF/config/missing: line 81: aclocal-1.16: command not found
WARNING: 'aclocal-1.16' is missing on your system.
...
```

give this command before running `configure` (see below):

```
touch configure aclocal.m4 Makefile.in src/config.h.in
```

## Case 1: MySQL installed via prebuild package
This is the typical installation on any system, that is when
you have installed MySQL using a precompiled package (e.g. a `.dmg` file on Mac OS
or issuing `sudo apt install mysql-server libmysqlclient-dev` on Debian/Ubuntu).
In this case you only need to be sure that you have all the necessary include file and `mysql_config`. Check your installed version:
```
shell> mysql_config --version
5.7.24
```
DIF should work on MySQL 5.1, 5.5, 5.6, 5.7 (and the corresponding MariaDB varsions) and 8.0.

Now we need to prepare some additional include file via `cmake`.
The easiest way is to download the source code. Assuming the installed version is 5.7.24 (in a temporary directory):
```
wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-boost-5.7.24.tar.gz
tar zxvf mysql-boost-5.7.24.tar.gz
cd mysql-5.7.24
cmake . -DWITH_BOOST=boost
```

## Case 2: MySQL installed via source code
Assuming that you have downloaded, compiled and installed MySQL 5.7.24:

If downloaded via git:
```
git clone https://github.com/lnicastro/DIF.git
cd DIF
touch configure aclocal.m4 Makefile.in src/config.h.in
./configure --with-mysql-source=/path_to/mysql-5.7.24
make
sudo make install
```

If you downloaded a tar archive (similarly for other comoressed formats):
```
tar zxvf dif-0.5.5.tar.gz

cd dif-0.5.5
./configure --with-mysql-source=/path_to/mysql-5.7.24
make
sudo make install
```

**Note:** if you compiled MySQL in a build directory rather than in the root source directory, then you have to pass this to ``configure``, e.g. if you used the subdirectory `Build`:
```
./configure --with-mysql-source=/path_to/mysql-5.7.24/Build
```

## Installing DIF facilities in MySQL
`dif` is a Perl script used to perform various DIF-related tasks.
It uses the Perl `DBI/DBD-MySQL` modules to communicate with the MySQL server.
First of all be sure you have these modules installed. You can install them using `cpan DBD::mysql` or the OS specific command.

On Mac OS using MacPorts: 
```
sudo port install p5-dbd-mysql
```
On Debian, Ubuntu and variants:
```
sudo apt-get install libdbd-mysql-perl
```
On Red Hat, Fedora, centOS, and variants:
```
sudo yum install "perl(DBD::mysql)"
```
On openSUSE
```
sudo zypper install perl-DBD-mysql
```

Once the Perl modules are installed, to actually enable the DIF facilities, you need to run the installation command:
```
dif --install
```

You'll be asked the MySQL root password to complete this task.
See the manual for a full description or run:
```
dif --help
```

**Note:** If you use `tcsh` you might need to run `rehash` to have the command visible in an existing terminal.

Assuming the command executes successfully, you now need to restart the MySQL server to make the new DIF storage engine working.
Depending on your OS and/or how you started the server, you could need to use one of these commands:
```
sudo service mysql restart
```
or
```
sudo /etc/init.d/mysql.server restart
```
or locate your MySQL `bin` dir and use `mysqladmin` and `mysqld_safe`, e.g.:
```
which mysqladmin
...
sudo /usr/local/mysql/bin/mysqladmin -u root -p shutdown
sudo /usr/local/mysql/bin/mysqld_safe --user=mysql &
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
shell> dif --index-htm Catalogs ascc25_mini 6 "RAmas/3.6e6" "DECmas/3.6e6"
```

Among other things, this will create the "table view" `ascc25_mini_htm_6`. This is the table that you must use in place of `ascc25_mini` when DIF specific functions are used in the WHERE clause of the query.

Let's index the table also with an order 10 HEALPix index.
```
shell> dif --index-healpix-nested Catalogs ascc25_mini 10 "RAmas/3.6e6" "DECmas/3.6e6"
```

**Some queries on circular and rectangular regions:**

Enter the MySQL client terminal, e.g. `mysql -u root -p Catalogs`, then:

```
-- all the info for objects in a circle of radius 18 arcmin around RA=30, Dec=30
  SELECT * FROM ascc25_mini_htm_6 WHERE dif_Circle(30,30,18);

-- as above, but returning coordinates and magnitudes only in standard format
  SELECT RAmas/3.6e6 as RA, DECmas/3.6e6 as Decl, Bmm/1000 as B, Vmm/1000 as V
    FROM ascc25_mini_htm_6
    WHERE dif_Circle(30,30,18);

-- only magnitudes V less than 11 for objects in a square with side 33 arcmin
  SELECT Bmm/1000 as B, Vmm/1000 as V
     FROM ascc25_mini_htm_6
     WHERE dif_Rect(100,-20,33) and Vmm < 11000;

-- only magnitudes V less than 12 for objects in a preudo-rectangle with sides
-- 30, 15 arcmin (along RA, Dec)
  SELECT Bmm/1000 as B, Vmm/1000 as V
     FROM ascc25_mini_htm_6
     WHERE dif_Rect(100,-20, 30, 15) and Vmm < 12000;

-- a query using the HEALPix indexing
  SELECT ramas/3.6e6 as radeg, decmas/3.6e6 as decdeg, Vmm/1000 as Vmag, (Bmm-Vmm)/1000 as color
     FROM ascc25_mini_healp_nest_10
     WHERE dif_Circle(30,-20,30);
```
