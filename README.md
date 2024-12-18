# DIF
DIF is a collection of tools aimed at implementing a powerful indexing system
for astronomical catalogues and other data with spherical coordinates, stored
into MySQL / MariaDB databases.
DIF is able to use both [HTM](http://www.skyserver.org/htm/) and
[HEALPix](https://healpix.sourceforge.io/) pixelization schemas simultaneously
and it allows very fast query execution even on billion-row tables. 

**Note:** as of DIF version 0.5.5, you do not need to compile and install the
  MySQL source version anymore. It's enough to get the MySQL / MariaDB source
  code and execute a `cmake` configuration. Please see below.

See also the [documentation](doc/) or the [web page](http://ross.oas.inaf.it/dif/)
and the [reference paper](https://onlinelibrary.wiley.com/doi/10.1155/2010/524534).

If you want to use almost all of the DIF capabilities but avoid to use MySQL /
MariaDB source code, please use [SID](https://github.com/lnicastro/SID) instead.


[![DOI](https://zenodo.org/badge/134399282.svg)](https://zenodo.org/badge/latestdoi/134399282)


This is the **Version 0.5.5** development tree.

## Requirements

1. MySQL or MariaDB configured and working
2. mysql_config / mariadb_config
3. MySQL / MariaDB source code configured via `cmake` (same version as the
   system version OR installed)
4. make or gmake
5. Perl `DBI/DBD-MySQL` modules

## Compile and install
Compiling DIF depends on how you installed MySQL / MariaDB on your machine.
Typically.
```shell
./configure --with-mysql-source=/path_to/mysql_source_directory
make
sudo make install
```
`/path_to/mysql_source_directory` is the full path to the MySQL source directory (see below).

DIF v. 0.5.5 should work on MySQL 5.1, 5.5, 5.6, 5.7 and 8.0. Implementation
for MariaDB is at the moment limited to version 10.3.

**Note:** if you retrieved DIF from GitHub and running `make` you get a message like this
```
...
DIF/config/missing: line 81: aclocal-1.16: command not found
WARNING: 'aclocal-1.16' is missing on your system.
...
```
then give a `touch` command before running `configure`. So, from the code main directory:

```shell
touch configure aclocal.m4 Makefile.in src/config.h.in

./configure --with-mysql-source=/path_to/mysql_source_directory
make
sudo make install
```
Let's now consider the two possible MySQL installations.

**Case 1: MySQL / MariaDB installed via prebuilt package**

This is the typical installation on any system, that is when
you have installed MySQL using a precompiled package (e.g. a `.dmg` file on Mac
OS or issuing `sudo apt install mysql-server libmysqlclient-dev` on
Debian/Ubuntu). In this case you only need to be sure that you have:

1. all the necessary include files (typically provided by packages with
  extension `-dev` or `-devel`), e.g. in `/usr/include/mysql`,
2. `mysql_config` or `mariadb_config`.

Check your installed version:
```
mysql_config --version
5.7.35
```

Now we need to prepare some additional MySQL include files via `cmake`.

> **Note:** if you have **MySQL 5.6 or 5.7** installed, you should be able to install DIF
>   without running `cmake`.

The easiest way is to download the source code.

Assuming the installed version is 5.7.35 (in a temporary directory):
```shell
wget https://dev.mysql.com/get/Downloads/MySQL-5.7/mysql-boost-5.7.35.tar.gz
tar zxvf mysql-boost-5.7.35.tar.gz
cd mysql-5.7.35

pwd
/directory/full/path
```
Annotate the directory name and [go to install section](#dif_compile)!
If instead the installed version is e.g. 8.0.27 (in a temporary directory):
```shell
wget https://dev.mysql.com/get/Downloads/MySQL-8.0/mysql-boost-8.0.27.tar.gz
tar zxvf mysql-boost-8.0.27.tar.gz
cd mysql-8.0.27

mkdir bld
cd bld
cmake .. -DWITH_BOOST=../boost

pwd
/directory/full/path
```
Note that we run `cmake` in a scratch `bld` sub-directory.
Annotate the directory name and then you are ready to [install DIF](#dif_compile).

Similarly for **MariaDB 10.3**. However here we use a configuration command that avoids
unnecessary plugins (a similar approach could have been used for MySQL too):
```shell
wget https://downloads.mariadb.org/interstitial/mariadb-10.3.20/source/mariadb-10.3.20.tar.gz
tar zxvf mariadb-10.3.20.tar.gz
cd mariadb-10.3.20

mkdir bld
cd bld
cmake .. -DENABLE_DTRACE=0 -DDEBUG_ON=0 -DWITH_DEBUG=0 -DPLUGIN_EXAMPLE=YES -DPLUGIN_TOKUDB=NO -DPLUGIN_TOKUDB=NO \
   -DPLUGIN_ROCKSDB=NO -DPLUGIN_MROONGA=NO -DPLUGIN_FEDERATED=NO -DPLUGIN_CASSANDRA=NO \
   -DPLUGIN_SPHINX=NO -DPLUGIN_CONNECT=NO -DPLUGIN_SPIDER=NO

pwd
/directory/full/path
```
The `-DENABLE_DTRACE=0` on some systems is necessary to avoid the compilation error.
```
... error: probes_mysql_dtrace.h: No such file or directory
```
Annotate the directory name and then you are ready to [install DIF](#dif_compile).

**Case 2: MySQL / MariaDB installed via source code**

Assuming that you have downloaded, configured, compiled and **installed**
MySQL / MariaDB from source, then you only need to know the directory name with
the code and be sure that you have not cleaned the required include files after
compilation.
Eventually rerun the `cmake` command.

<a name="dif_compile"></a>**Compile and install**

Below we assume you installed a prebuilt MySQL 5.7.35. In any case, remember that the
`--with-mysql-source` argument must point to the dir where you eventually run `cmake`
(so e.g. add `/bld` for the examples above).

- If you download DIF via `git`:
```
git clone https://github.com/lnicastro/DIF.git
cd DIF
./configure --with-mysql-source=/path_to/mysql-5.7.35
make
sudo make install
```

- If you downloaded the ZIP archive from GitHub:
```
unzip DIF-master.zip
cd DIF-master
./configure --with-mysql-source=/path_to/mysql-5.7.35
make
sudo make install
```

- If you instead downloaded a tar archive (similarly for other compressed formats):
```
tar zxvf dif-0.5.5.tar.gz
cd dif-0.5.5
./configure --with-mysql-source=/path_to/mysql-5.7.35
make
sudo make install
```

Again, if you compiled MySQL in a build sub-directory `bld` rather than in its
root source directory, then the configure command would be
```
  ./configure --with-mysql-source=/path_to/mysql-5.7.35/bld
```

## Installing DIF facilities in MySQL
`dif` is a DIF provided Perl script used to perform various DIF-related tasks.
It uses the Perl `DBI/DBD-MySQL` modules to communicate with the MySQL server.
First of all be sure you have these modules installed. You can install them
using `cpan DBD::mysql` or the OS specific command.

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

Once the Perl modules are installed, to actually enable the DIF facilities,
you need to run the installation command:
```
dif --install
```

You'll be asked the MySQL root password to complete this task.
See the manual for a full description or run:
```
dif --help
```

> **Note:** If you use `tcsh` you might need to run `rehash` to have the command
>  visible in an existing terminal.

Assuming the command executes successfully, you now need to restart the MySQL
server to make the new DIF storage engine working. Depending on your OS and/or
how you started the server, you could need to use one of these commands:
```
sudo service mysql restart
```
or
```
sudo systemctl restart mysql
```
or
```
sudo /etc/init.d/mysql restart
```
or locate your MySQL `bin` dir and use `mysqladmin` and `mysqld_safe`, e.g.:
```
which mysqladmin
...
sudo /usr/local/mysql/bin/mysqladmin -u root -p shutdown
sudo /usr/local/mysql/bin/mysqld_safe --user=mysql &
```

## Test installation
Enter MySQL from a terminal and check the plugin DIF and its functions are
available:
```sql
shell> mysql -u root -p
Enter password:
...
mysql> show plugins;
...
+----------------------------+----------+--------------------+-----------+---------+
| Name                       | Status   | Type               | Library   | License |
+----------------------------+----------+--------------------+-----------+---------+
...
| DIF                        | ACTIVE   | STORAGE ENGINE     | ha_dif.so | GPL     |
+----------------------------+----------+--------------------+-----------+---------+

mysql> select * from mysql.func;
+-------------------+-----+---------------+----------+
| name              | ret | dl            | type     |
+-------------------+-----+---------------+----------+
| HTMLookup         |   2 | ha_dif.so     | function |
| HTMidByName       |   2 | ha_dif.so     | function |
| HTMnameById       |   0 | ha_dif.so     | function |
...
| DIF_setHTMDepth   |   2 | ha_dif.so     | function |
...
+-------------------+-----+---------------+----------+
```

**Test with an astro-cat**

Download the reduced version of the
[ASCC 2.5](http://ross2.oas.inaf.it/dif/data/ascc25_mini.sql.gz) star
catalogue in a working directory, say `dif_data`. From a terminal:
```
shell> mkdir ~/dif_data
shell> cd ~/dif_data
shell> wget http://ross2.oas.inaf.it/dif/data/ascc25_mini.sql.gz
shell> gunzip ascc25_mini.sql.gz
```

Load the data into a database of your choice, e.g. `Catalogs`;
```sql
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

Among other things, this will create the "table view" `ascc25_mini_htm_6`.
This is the table that you must use in place of `ascc25_mini` when DIF specific
functions are used in the WHERE clause of the query.

Let's index the table also with an order 10 HEALPix index.
```
shell> dif --index-healpix-nested Catalogs ascc25_mini 10 "RAmas/3.6e6" "DECmas/3.6e6"
```

**Some queries on circular and rectangular regions:**

Enter the MySQL client terminal, e.g. `mysql -u root -p Catalogs`, then:

```sql
-- all the info for objects in a circle of radius 18 arcmin around RA=30, Dec=30
  SELECT * FROM ascc25_mini_htm_6 WHERE dif_Circle(30, 30, 18);

-- as above, but returning coordinates and magnitudes in standard format
  SELECT RAmas/3.6e6 as RA, DECmas/3.6e6 as Decl, Bmm/1000 as B, Vmm/1000 as V
    FROM ascc25_mini_htm_6
    WHERE dif_Circle(30, 30, 18);

-- only magnitudes V less than 11 for objects in a square with side 33 arcmin
  SELECT Bmm/1000 as B, Vmm/1000 as V
     FROM ascc25_mini_htm_6
     WHERE dif_Rect(100, -20, 33) and Vmm < 11000;

-- only magnitudes V less than 12 for objects in a pseudo-rectangle with sides
-- 30, 15 arcmin (along RA, Dec)
  SELECT Bmm/1000 as B, Vmm/1000 as V
     FROM ascc25_mini_htm_6
     WHERE dif_Rect(100, -20, 30, 15) and Vmm < 12000;

-- a query using the HEALPix indexing
  SELECT ramas/3.6e6 as radeg, decmas/3.6e6 as decdeg, Vmm/1000 as Vmag, (Bmm-Vmm)/1000 as color
     FROM ascc25_mini_healp_nest_10
     WHERE dif_Circle(30, -20, 30);
```
