/*
  Xmatch catalogue InCat against a reference catalogue RefCat.
  A simple implementation of the "spherematch" algorithm querying on
  HTM/HELAPix pixels indexed MySQL tables.

   Notes:
    1. HEALPix pixelization X-match not yet implemented. Use HTM.
    2. Unless a specific pixel ID (or range) is given, the full catalogue is
       processed.
    3. Use option "-I refIdField" for adding a (possibly) unique source identificator
       (integer) which must be present in RefCat. Assumed to be integer type.
       If the "-K inIdField" option is used, then also the (possibly) unique identificator
       from InCat is added. Note that at the moment it is assumed to be of type char.
       By default just use HTM IDs and coordinates (default RAmas, DECmas).
       For the first case the (optional) output catalogue will have the columns:

       | Field         | Type              |
       +---------------+-------------------+
       | htmID_6       | smallint unsigned |
       | ref_htmID_6   | smallint unsigned |
       | source_id     | bigint unsigned   |  <-- this is for "gaiadr2"
       | RAmas         | int unsigned      |
       | DECmas        | int               |
       | ref_RAmas     | int unsigned      |
       | ref_DECmas    | int               |
       | Sep           | float             |
       | origID        | tinyint           |
       +---------------+-------------------+

       whereas in the second, default case:

       | Field         | Type              |
       +---------------+-------------------+
       | htmID_6       | smallint unsigned |
       | ref_htmID_6   | smallint unsigned |
       | source_name   | char(22)          |  <-- this is for "catwise"
       | source_id     | bigint unsigned   |
       | RAmas         | int unsigned      |
       | DECmas        | int               |
       | ref_RAmas     | int unsigned      |
       | ref_DECmas    | int               |
       | Sep           | float             |
       | origID        | tinyint           |
       +---------------+-------------------+

    4. Notice the various defaults in the help text.


  Examples:

  1. match the ASCC25 catalogue HTM_6 pixel ID 32775 against the TYCHO2 reference catalogue
     with matches in 1 arcsec. Neighbors of depth 8 (about 17 arcmin) are used:
    pix_myXmatch -d MyCats -x ascc25 tycho2 -D 6 8 32775
  2. as above but with matching radius of 2 arcsec:
    pix_myXmatch -d MyCats -x ascc25 tycho2 -D 6 8 -S 2 32775
  3. as above but ask to save matched and unmatched objects in DB tables (def. DB=test):
    pix_myXmatch -d MyCats -x ascc25 tycho2 -D 6 8 -S 2 -A 32775
  4. full catalogue match saving matched and unmatched objects and using neighbors pixels
     of depth 14 (about 15 arcsec) to those of depth 6. Specify in/out DBs with table names.
    pix_myXmatch -x DBin1.ascc25 DBin2.tycho2 -t DBout.xout_tab -D 6 14 -qA
  5. use -I for Turin schema catalogues:
    pix_myXmatch -d TOCats -x ascc25 tycho2 -t DBout.xout_tab -D 8 14 -qA -I source_id 524288 1048575


  LN @ INAF-OAS, June 2013                         Last changed: 30/06/2020
*/

using namespace std;

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>

#include <vector>
#include <algorithm>

#include "my_stmt_db2.h"

#include "pix_myXmatch_def.hh"


int stoi(string s)
{
  return strtol(s.c_str(), NULL, 10);
/*
  int i;
  if (sscanf(s.c_str(), "%d", &i) == 1)
    return i;
  else
    return NULL;
*/
}

string itos(long i)
{
  char buf[20];
  sprintf(buf, "%ld", i);
  return string(buf);
}

string ltos(long long i)
{
  char buf[20];
  sprintf(buf, "%lld", i);
  return string(buf);
}

string dtos(double f)
{
  char buf[23];
  sprintf(buf, "%.16g", f);
  return string(buf);
}

string dtos12(double f)
{
  char buf[20];
  sprintf(buf, "%12.7lf", f);
  return string(buf);
}

string dtos3f(double f)
{
  char buf[20];
  sprintf(buf, "%.3lf", f);
  return string(buf);
}


string dif_sqltype(string param)  // from HTM/HEALPix depth/order to INT type
{
  int id = atoi(param.c_str());
  string sqltype;

  switch (id) {
    case 0:
    case 1:
    case 2:
      sqltype = "TINYINT UNSIGNED";
      break;
    case 3:
    case 4:
    case 5:
    case 6:
      sqltype = "SMALLINT UNSIGNED";
      break;
    case 7:
    case 8:
    case 9:
    case 10:
      sqltype = "MEDIUMINT UNSIGNED";
      break;
    case 11:
    case 12:
    case 13:
    case 14:
      sqltype = "INTEGER UNSIGNED";
      break;
    default:
      sqltype = "BIGINT";
  }

  return (sqltype);
}

int dif_htmid_maxw(string param)  // from HTM depth to max Nr of digits in ID
{
  int id = atoi(param.c_str());
  int nd = 5;  // use 5 digits as min.; OK up to depth 6

  if (id < 7) return (nd);

  switch (id) {
    case 7:
      nd = 6;
      break;
    case 8:
    case 9:
      nd = 7;
      break;
    case 10:
    case 11:
      nd = 8;
      break;
    case 12:
      nd = 9;
      break;
    case 13:
    case 14:
      nd = 10;
      break;
    case 15:
    case 16:
      nd = 11;
      break;
    case 17:
      nd = 12;
      break;
    case 18:
    case 19:
      nd = 13;
      break;
    case 20:
    case 21:
      nd = 14;
      break;
    case 22:
      nd = 15;
      break;
    case 23:
    case 24:
      nd = 16;
      break;
    default:
      nd = 17;
  }

  return(nd);
}


int dif_hpxid_maxw(string param)  // from HEALPix order to max Nr of digits in ID
{
  int id = atoi(param.c_str());
  int nd = 5;  // use 5 digits as min.; OK up to order 6

  if (id < 7) return (nd);

  switch (id) {
    case 7:
    case 8:
      nd = 6;
      break;
    case 9:
      nd = 7;
      break;
    case 10:
    case 11:
      nd = 8;
      break;
    case 12:
    case 13:
      nd = 9;
      break;
    case 14:
      nd = 10;
      break;
    case 15:
    case 16:
      nd = 11;
      break;
    case 17:
    case 18:
      nd = 12;
      break;
    case 19:
      nd = 13;
      break;
    case 20:
    case 21:
      nd = 14;
      break;
    case 22:
    case 23:
      nd = 15;
      break;
    case 24:
      nd = 16;
      break;
    case 25:
    case 26:
      nd = 17;
      break;
    case 27:
    case 28:
      nd = 18;
      break;
    default:
      nd = 19;
  }

  return(nd);
}

enum IdType {
  UNKNOWN_TYPE,
  HTM_TYPE,
  HPX_TYPE
};

enum IdOpt {
  RING,
  NESTED
};

enum IdType id_type;
enum IdOpt id_opt;

typedef struct DB_st {
  string my_host;
  string my_user;
  string my_passw;
  string my_db1;
  string my_db2;
  string cat1;
  string cat2;
} DB_st;

DB_st db;

typedef struct OTabs_st {
  string out_db;
  string x;
  string nx;
  string nxc;
  string nxcf;
  string ext;
} OTabs_st;

//OTabs_st otab;
typedef struct TabInfo_st {
  bool use_master_id1;
  bool use_master_id2;
  bool in_full;
  string order1;
  string order2;
  string id_coln1;
  string id_coln2;
  string ra_coln1;
  string ra_coln2;
  string de_coln1;
  string de_coln2;
  string rf_coln1;
  string rf_coln2;
  string deg_fac;
  string id_sqltype;
  OTabs_st otab;
} TabInfo_st;

TabInfo_st t;

typedef struct DIFtbl_st {
  string db;
  string name;
  IdType id_type;
  IdOpt id_opt;
  string param;
  string Ra_field;
  string Dec_field;
  unsigned short db_fw;
  unsigned short nm_fw;
  unsigned short pm_fw;
  unsigned short ra_fw;
  unsigned short de_fw;
} DIFtbl_st;

vector<DIFtbl_st> catinfo;



//
// -- Allocate a 2-d char array
//
inline char** alloc2d(int height, int width, char **arr)
{
  int i;
  if ( !(arr = (char**) malloc(sizeof(char*) * height)) ) {
	  cerr <<"Could not allocate "<< height <<" rows.\n";
	  exit (-1);
  }

  for (i = 0; i < height; i++)
	arr[i] = NULL;

  for (i = 0; i < height; i++) {
	if ( !(arr[i] = (char*) malloc(sizeof(char*) * width)) ) {
	  cerr <<"Could not allocate elements of "<< height <<" rows.\n";
	  exit (-1);
	}
  }

  return arr;
}

//
// -- Realloc a 2-d char array (here N elements of fixed N chars)
// 
inline char** resize2d(int oldHeight, int oldWidth, int height, int width, char **arr)
{
  int i;
  char** tmpr = NULL;
  char* tmpc = NULL;
  if ( !(tmpr = (char**) realloc(arr, sizeof(char*) * height)) ) {
	  cerr <<"Could not realloc "<< height <<" rows.\n";
	  exit (-1);
  }
  arr = tmpr;

//cout<<oldHeight<<" "<<oldWidth<<" "<<height<<" "<<width<<endl;

  for (i = oldHeight; i < height; i++)  // Assume constant width -> field with fixed Nr of chars
	arr[i] = NULL;

  for (i = oldHeight; i < height; i++) {  // Assume constant width -> field with fixed Nr of chars
	if ( !(tmpc = (char*) realloc(arr[i], sizeof(char*) * width)) ) {
	  cerr <<"Could not realloc elements of "<< height <<" rows.\n";
	  exit (-1);
	}
	arr[i] = tmpc;
  }

  return arr;
}




void set_out_tabs(string my_user, string cat1, string cat2)
{
  const char root_otab[] = "pix_Xmatch_";
  if (t.otab.x.empty()) {
    t.otab.x    = root_otab + my_user +"_"+ cat1 +"_x_"+   cat2;
    t.otab.nx   = root_otab + my_user +"_"+ cat1 +"_nx_"+  cat2;
    t.otab.nxc  = root_otab + my_user +"_"+ cat1 +"_nxc_"+ cat2;
    t.otab.nxcf = root_otab + my_user +"_"+ cat1 +"_nxc_"+ cat2 +"_full";
    t.otab.ext  = root_otab + my_user +"_"+ cat1 +"_ext_"+ cat2;
  } else {
    t.otab.nx   = t.otab.x +"_unmatched";
    t.otab.nxc  = t.otab.x +"_unmatched_clean";
    t.otab.nxcf = t.otab.x +"_unmatched_clean_full";
    t.otab.ext  = t.otab.x +"_external";
  }
}

void print_out_tabs()
{
  cout <<"Output matched objects DB table: "<< t.otab.out_db << dt << t.otab.x
       <<"\nOutput unmatched objects DB table: "<< t.otab.out_db << dt << t.otab.nx
       <<"\nOutput unmatched cleaned objects DB table: "<< t.otab.out_db << dt << t.otab.nxc
       <<"\nOutput extended unmatched objects DB table: "<< t.otab.out_db << dt << t.otab.nxcf
       <<"\nOutput external objects DB table: "<< t.otab.out_db << dt << t.otab.ext
       << endl;
}


//
// -- Create (if not there) output match tables, etc.
//
int crea_out_tabs(const int my_cID, bool drop_prematch, bool verbose)
{
  const char errmsg[] = "crea_out_tabs: DB error: ";
  set_out_tabs(db.my_user, db.cat1, db.cat2);

if (verbose)
  print_out_tabs();

  unsigned int i;
  string qry_str_i, qry_str;
  std::vector<bool> tab_exists(3, false);

// Assume connection open and do not perform connection status check

  if (drop_prematch) {
    qry_str = string("DROP TABLE IF EXISTS ")+ t.otab.out_db +dt+ t.otab.x +co+ t.otab.out_db +dt+ t.otab.nx +co+ t.otab.out_db +dt+ t.otab.ext;

if (verbose)
  cout << qry_str << endl;
    if ( !db_query(my_cID, qry_str.c_str()) ) {
       cerr << errmsg << db_error(my_cID) << endl;
       exit (1);
    }
  } else {
    qry_str_i = string("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='")+ t.otab.out_db +"' AND table_name='";
    for (i = 0; i < 3; i++) {
      if (i == 0) 
        qry_str = qry_str_i + t.otab.x +"'";
      else if (i == 1)
        qry_str = qry_str_i + t.otab.nx +"'";
      else
        qry_str = qry_str_i + t.otab.ext +"'";

if (verbose)
  cout << qry_str << endl;
      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << errmsg << db_error(my_cID) << endl;
        exit (1);
      }
      tab_exists[i] = atoi(db_data(my_cID, 0, 0));
    }
  }

// Because of ALTER TABLE cannot use "IF EXISTS" on creation
  qry_str_i = string("CREATE TABLE ")+ t.otab.out_db +dt;
  string tabname;

  for (i = 0; i < 3; i++) {
    if (!tab_exists[i]) {
      if (i == 0) {

// Matched objects table
/*

+---------+-------------+-------------+-----------+-------+--------+-----------+------------+-------+--------+
| htmID_8 | ref_htmID_8 | source_name | source_id | RAmas | DECmas | ref_RAmas | ref_DECmas | Sep   | origID |
+---------+-------------+-------------+-----------+-------+--------+-----------+------------+-------+--------+

source_name, source_id -> optional

*/

	tabname = t.otab.x;
	qry_str = qry_str_i + tabname +" SELECT i."+  t.id_coln1 +" AS "+ t.id_coln1 +",r."+ t.id_coln1 +" AS ref_"+ t.id_coln1;

	if (t.use_master_id1)
	  qry_str += ",i."+ t.rf_coln1;
	if (t.use_master_id2)
	  qry_str += ",r."+ t.rf_coln2;

	qry_str += " FROM "+ db.my_db1 +dt+ db.cat1 +" AS i,"+ db.my_db2 +dt+ db.cat2 +" AS r LIMIT 0";

      } else if (i == 1) {

// Unmatched objects table
	tabname = t.otab.nx;
	qry_str = qry_str_i + tabname +" SELECT "+ t.id_coln1;
	if (t.use_master_id1)
	  qry_str += co+ t.rf_coln1;

	qry_str += " FROM "+ db.my_db1 +dt+ db.cat1 +" LIMIT 0";

      } else {

// External objects table
	tabname = t.otab.ext;
	qry_str = qry_str_i + tabname +" SELECT "+ t.id_coln1;
	if (t.use_master_id1)
	  qry_str += co+ t.rf_coln1;

	 qry_str += " FROM "+ db.my_db1 +dt+ db.cat1 +" LIMIT 0";
      }

if (verbose)
  cout << qry_str << endl;
      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << errmsg << db_error(my_cID) << endl;
        exit (1);
      }

// What to alter for matched/unmatched/external catalogue
      if (i == 0)
        qry_str = "ALTER TABLE "+ t.otab.out_db +dt+ tabname +" ADD RAmas int unsigned not null, ADD DECmas int not null, ADD ref_RAmas int unsigned not null, ADD ref_DECmas int not null, ADD Sep FLOAT NOT NULL DEFAULT 0, ADD origID TINYINT NOT NULL DEFAULT 0, ENGINE=MyISAM, CHARSET=ASCII";

      else if (i == 1) 

        qry_str = "ALTER TABLE "+ t.otab.out_db +dt+ tabname +" ADD RAmas int unsigned not null, ADD DECmas int not null, ADD origID TINYINT NOT NULL DEFAULT 0, ENGINE=MyISAM, CHARSET=ASCII";

      else 

        qry_str = "ALTER TABLE "+ t.otab.out_db +dt+ tabname +" ADD RAmas int unsigned not null, ADD DECmas int not null, ADD Sep FLOAT NOT NULL DEFAULT 0, ENGINE=MyISAM, CHARSET=ASCII";

      if (!qry_str.empty()) {

if (verbose)
  cout << qry_str << endl;
        if ( !db_query(my_cID, qry_str.c_str()) ) {
          cerr << errmsg << db_error(my_cID) << endl;
          exit (1);
        }
      }
    }  // !tab_exists[i]
  }  // for i

  return 0;
}


//
// -- Get DIF.tbl rows for given DB and table(s). Optionally only return info for HTM indices.
//
int dif_cat_info(const int cID, string dbname, string catname, bool htm_only=false)
{
  bool do_close = false;
  unsigned int i;
  int cid = cID;
  string server_ip, d = " db like '%'", t = "name like '%'";

  if (cID < 0) {
    cid = 0;
    if (!db_init(cid)) {
	cerr <<"--> Error: cannot set CONNECT_TIMEOUT for MySQL connection.\n";
	exit (1);
    }

/* If IP address not given (< 7 chars!) then use the default one */
    if (db.my_host.size() >= 7)
      server_ip = db.my_host;
    else
      server_ip = "localhost";

    if ( !db_connect(cid, server_ip.c_str(), db.my_user.c_str(), db.my_passw.c_str(), db.my_db1.c_str()) ) {
      cerr <<"--> Error: DB error: "<< db_error(cid) << endl;
      exit (1);
    }
    do_close = true;
  }

  if (dbname.length() > 0)
    d = dbname;

  string difqry = "SELECT db,name,id_type,id_opt,param,Ra_field,Dec_field from DIF.tbl WHERE ";

  if (dbname.length() > 0)
    d = " db='"+ dbname +"'";
  if (catname.length() > 0) {
    size_t w;
    while (1) {
	w = catname.find("*");
	if (w != string::npos)
		catname.replace(w, 1, "%");
         else
		break;
    }
    w = catname.find("%");
    if (w != string::npos)
	t = "name like '"+  catname +"'";
    else
	t = "name='"+ catname +"'";
  }

  difqry += d +" AND "+ t;

  if (htm_only)
    difqry += " AND id_type=1";

  if ( !db_query(cid, difqry.c_str()) ) {
    cerr <<"--> Error: DB error: "<< db_error(cid) << endl;
    exit (1);
  }

  if (db_num_rows(cid) == 0) {
    cerr <<"--> Error: "<< dbname <<dt<< catname;
    if (htm_only)
      cerr <<": no HTM indexing found.\n";
    else
      cerr <<": no DIF indexing found.\n";
    exit (1);
  }

  DIFtbl_st e;
  for (i = 0; i < db_num_rows(cid); i++) {
     e.db        = db_data(cid, i, 0);
     e.name      = db_data(cid, i, 1);
     e.id_type   = atoi(db_data(cid, i, 2)) == 1 ? HTM_TYPE : HPX_TYPE; 
     e.id_opt    = atoi(db_data(cid, i, 3)) == 0 ? RING : NESTED;
     e.param     = db_data(cid, i, 4);
     e.Ra_field  = db_data(cid, i, 5);
     e.Dec_field = db_data(cid, i, 6);
     e.db_fw = e.db.length();
     e.nm_fw = e.name.length();
     e.pm_fw = e.param.length();
     e.ra_fw = e.Ra_field.length();
     e.de_fw = e.Dec_field.length();
     catinfo.push_back(e);
  }
  db_free_result(cid);
  if (do_close) db_close(cid);

  return 0;
}


void set_dif_params(bool use_hpx=false)
{
  db.my_host = "localhost";
  db.my_user = "generic";
  db.my_passw = "password";
  db.my_db1 = "test";
  db.my_db2 = "test";
  db.cat1 = "";
  db.cat2 = "";

// ID col names have additional depth/order given in input
  if (use_hpx) {
   t.id_coln1 = "healpID_nest_"; //+ t.order1;
   t.id_coln2 = "healpID_nest_"; //+ t.order2;
  } else {
    t.id_coln1 = "htmID_"; //+ t.order1;
    t.id_coln2 = "htmID_"; //+ t.order2;
  }

  t.ra_coln1 = "RAmas";
  t.de_coln1 = "DECmas";
  t.ra_coln2 = "RAmas";
  t.de_coln2 = "DECmas";
  t.deg_fac = "/3.6e6";
  t.rf_coln1 = "";
  t.rf_coln2 = "";

  t.id_sqltype = dif_sqltype(t.order1);  // Unused
}


 

void usage()
{
  cout << PROGNAME <<bl<<bl<< VERID << endl<<endl
       << "Match an input catalogue InCat against a reference one, RefCat,\n and optionally produce an output Table. All reside in DIF indexed MySQL tables.\n\n"
       << "Usage: "<< PROGNAME <<" -x InCat RefCat [OPTIONS] [Pixel_ID] [Start_Pixel_ID End_Pixel_ID]\n"
       << "Where OPTIONS are:\n"
       << "  -h: print this help\n"
       << "  -H: print help on available DIF indexed catalogues\n"
       << "  -a: archive (append if it exists) matched/unmatched objects into DB tables (see -t)\n"
       << "  -A: like -a but (if exists) first remove DB table\n"
       << "  -B: like -A but also create the unmatched table with all input catalogue columns\n"
       << "  -F: full match of InCat in one shot (no loop on pixels)\n"
       << "  -l: list on screen selected and matched objects\n"
       << "  -m: input (see -S) and returned separation are arcmin (def. arcsec)\n"
       << "  -M: accept multiple matches within given max separation (see -S) (def. 1 match only)\n"
       << "  -q: do not list on screen matched objects\n"
       << "  -Q: only list on screen matched objects outside the pixel\n"
       << "  -v: be verbose\n"
       << "  -c ID1 ID2 Ra Dec: column names for HTM/HPX IDs and Coords of InCat (def. htmID_ + Depth1/2, RAmas, DECmas)\n"
       << "  -C Ra Dec: column names for Coords of RefCat (def. RAmas, DECmas)\n"
       << "  -d DBnane: use 'DBnane' as input database (def. '"<< t.otab.out_db <<"', implies one of -aAB)\n"
       << "  -i catID: set 'catID' as user defined catalogue ID for 'RefCat' (e.g. sky2000 = 1, def. 0)\n"
       << "  -o OutDB: use 'OutDB' as output database (def. test, implies one of -aAB)\n"
       << "  -p Password: MySQL user password is 'Password' (def. '"<< db.my_passw <<"')\n"
       << "  -s Server: send query to DB server 'Server' (def. '"<< db.my_host <<"')\n"
       << "  -t Table: root matched objects table will be 'Table' (def. pix_Xmatch_User_InCat_x_RefCat)\n"
       << "  -u User: MySQL user name is 'User' (def. '"<< db.my_user <<"')\n"
       << "  -x InCat RefCat: cross match catalogue 'InCat' against reference 'RefCat'\n"
       << "  -D Depth1 Depth2: HTM pixelization depths to use are 'Depth1' and 'Depth2' (def. 8 14 : excludes -O)\n"
       << "  -I refIdField: field Id (e.g. source_id in Gaia) to read from RefCat and add to out table (integer type)\n"
       << "  -K inIdField: field Id (e.g. source_name in catwise) to read from InCat and add to out table (char type - TODO)\n"
       << "  [-O Order1 Order2]: HEALPix pixelization order (NESTED) to use are 'Order1' and 'Order2' (def. 6 14 : excludes -D)\n"
       << "  -R nRows: process 'nRows' per INSERT query to increase speed (def. 300 : require -A or -a)\n"
       << "  -S Sep: max separation defining a positive match is 'Sep' arcsec (def. 1 : see -m)\n"
       << "\nNote:\n"
       << "   Option -O is not implemented yet.\n"
       << "   Options -D and -O apply to both catalogues.\n"
       << "   If -O not given then assume both catalogues are HTM indexed.\n"
       << "\nCan join DB name with table name, e.g.: "<< db.my_db1 <<".InCat or "<< t.otab.out_db <<".InCat_xm_RefCat"
       << endl<<endl;
  exit(0);
}


int main(int argc, char *argv[])
{
  unsigned short full_scan = 1, do_list_match = 1, do_list_external = 1, do_list_all = 0, do_list_cats = 0,
                 use_arcmin = 0, save_match = 0, drop_prematch = 0, out_unmatched_full = 0, idb_set = 0, odb_set = 0,
                 input_col_names1 = 0, input_col_names2 = 0, kwds = 0, use_hpx = 0, multi_match = 0, verbose = 0;
  static const int my_cID = 0;  // MySQL connection ID
  int insert_Nrows = 300, sep_scale = 3600;
  unsigned long i, nmatch, nmatchret, nmatchext; // nmatchmax
  long totals_read = 0, totals_readext = 0, totals_match = 0, totals_matchext = 0, totals_unmatch = 0;

  double minchunksize, min_dist = -1.,
         matchlength = 1./3600;  // def. match dist.= 1''

  string sep_unit = "arcsec", origID = "0",
	 qry_str, db_view_order2;
  char c;

// Set default params
  set_dif_params();

  t.otab.out_db = "test";
  t.otab.x = "";
  t.use_master_id1 = false;
  t.use_master_id2 = false;
  t.order1 = "8";
  t.order2 = "14";
  t.in_full = false;

/* Keywords section */
  while (--argc > 0 && (*++argv)[0] == '-')
  {
    kwds = 1;
    while (kwds && (c = *++argv[0]))
    {
      switch (c)
      {
        case 'h':
          usage();
          break;
        case 'H':
          do_list_cats = 1;
          break;
        case 'a':
          save_match = 1;
          break;
        case 'A':
          save_match = 1;
          drop_prematch = 1;
          break;
        case 'B':
          save_match = 1;
          drop_prematch = 1;
          out_unmatched_full = 1;
          break;
        case 'F':
          t.in_full = true;
          break;
        case 'l':
          do_list_all = 1;
          break;
        case 'm':
          use_arcmin = 1;
	  sep_scale = 60;
          sep_unit = "arcmin";
          break;
        case 'M':
          multi_match = 1;
          break;
        case 'q':
          do_list_match = 0;
          do_list_external = 0;
          break;
        case 'Q':
          do_list_match = 0;
          break;
        case 'v':
          verbose = 1;
          break;
        case 'c':  // Pass column names for IDs, RA and Dec of first catalogue
          if (argc < 5) usage();
          t.id_coln1 = string(*++argv);
          --argc;
          t.id_coln2 = string(*++argv);
          --argc;
          t.ra_coln1 =  string(*++argv);
          //t.ra_coln =  "`"+ string(*++argv) + "`";
          --argc;
          t.de_coln1 = string(*++argv);
          //t.de_coln = "`"+ string(*++argv) + "`";
          --argc;
          input_col_names1 = 1;
          kwds=0;
          break;
        case 'C':  // Pass RA and Dec column names of reference catalogue
          if (argc < 3) usage();
          t.ra_coln2 =  string(*++argv);
          --argc;
          t.de_coln2 = string(*++argv);
          --argc;
          input_col_names2 = 1;
          kwds=0;
          break;
        case 'd':
          if (argc < 2) usage();
          db.my_db1 = string(*++argv);
          db.my_db2 = db.my_db1;
          --argc;
          idb_set = 1;
          kwds = 0;
          break;
        case 'D':
          if (argc < 3) usage();
          t.order1 = string(*++argv);
          --argc;
          t.order2 = string(*++argv);
          --argc;
          kwds = 0;
          break;
        case 'i':
          if (argc < 2) usage();
          origID = string(*++argv);
          --argc;
          kwds = 0;
          break;
        case 'I':
          if (argc < 2) usage();
          t.rf_coln2 = string(*++argv);
          --argc;
          t.use_master_id2 = true;
          //t.rf_coln2 = "source_id";
          kwds = 0;
          break;
        case 'K':
          if (argc < 2) usage();
          t.rf_coln1 = string(*++argv);
          --argc;
          t.use_master_id1 = true;
          //t.use_master_id2 = true;
          //t.rf_coln1 = "source_name";
          //t.rf_coln2 = "source_id";
          kwds = 0;
          break;
        case 'o':
          if (argc < 2) usage();
          t.otab.out_db = string(*++argv);
          --argc;
          save_match = 1;
          odb_set = 1;
          kwds = 0;
          break;
        case 'O':
          if (argc < 3) usage();
          t.order1 = string(*++argv);
          --argc;
          t.order2 = string(*++argv);
          --argc;
          use_hpx = 1;
          kwds=0;
          break;
        case 'p':
          if (argc < 2) usage();
          db.my_passw = string(*++argv);
          --argc;
          kwds = 0;
          break;
        case 'R':
          if (argc < 2) usage();
          insert_Nrows = atoi(*++argv);
          --argc;
          kwds = 0;
          break;
        case 's':
          if (argc < 2) usage();
          db.my_host = string(*++argv);
          --argc;
          kwds = 0;
          break;
        case 'S':
          if (argc < 2) usage();
          sscanf(*++argv, "%lf", &min_dist);
          --argc;
          kwds = 0;
          break;
        case 't':
          if (argc < 2) usage();
          t.otab.x = string(*++argv);
          --argc;
          kwds = 0;
          break;
        case 'u':
          if (argc < 2) usage();
          db.my_user = string(*++argv);
          --argc;
          kwds = 0;
          break;
        case 'x':
          if (argc < 3) usage();
          db.cat1 = string(*++argv);
          --argc;
          db.cat2 = string(*++argv);
          --argc;
          kwds = 0;
          break;
        default:
          cerr << "Illegal option `"<< c << "'.\n\n";
          usage();
      }
    }
  }


  if (argc > 2)  // max 2 params
    usage();

  if (do_list_cats) {
    if (argc > 0) db.cat1 = string(*argv);
    if ( !dif_cat_info(-1, db.my_db1, db.cat1, true) ) {
      unsigned short w[5] = {2,4,5,8,9};
      for (i = 0; i < catinfo.size(); i++) {
        if (catinfo[i].db_fw > w[0]) w[0] = catinfo[i].db_fw;
        if (catinfo[i].nm_fw > w[1]) w[1] = catinfo[i].nm_fw;
        if (catinfo[i].pm_fw > w[2]) w[2] = catinfo[i].pm_fw;
        if (catinfo[i].ra_fw > w[3]) w[3] = catinfo[i].ra_fw;
        if (catinfo[i].de_fw > w[4]) w[4] = catinfo[i].de_fw;
      }

      cout<< db.my_db1 <<dt<< (db.cat1.length()>0 ? db.cat1 : "*") <<" info in DIF.tbl:\n";
      cout<<left<<setw(w[0])<<"DB"<<bl<<setw(w[1])<<"Name"<<bl<<right
          <<setw(w[2])<<"Depth"<<bl
          <<setw(w[3])<<"Ra_field"<<bl<<setw(w[4])<<"Dec_field"<<bl<<
          setw(7)<<"id_type"<<bl<<setw(6)<<"id_opt" << endl;

      cout << left << setw(w[0]+w[1]+w[2]+w[3]+w[4] + 6 + 13) << setfill('-') <<""<<endl;
      cout << setfill(' ');
      for (i = 0; i < catinfo.size(); i++)
        cout <<left<<setw(w[0])<< catinfo[i].db <<bl<<setw(w[1])<< catinfo[i].name <<bl<<right
             <<setw(w[2])<< catinfo[i].param <<bl
             <<setw(w[3])<< catinfo[i].Ra_field <<bl<<setw(w[4])<< catinfo[i].Dec_field <<bl
             <<setw(7)<<catinfo[i].id_type <<bl<<setw(6)<< catinfo[i].id_opt <<endl;
      exit (0);
    } else
      exit (1);
  }


  if (db.cat1.empty() || db.cat2.empty())
    usage();

// Check if input DB given as part of input table
  if (!idb_set) {

    std::size_t dot = db.cat1.find('.');
    if (dot != std::string::npos) {
      db.my_db1 = db.cat1.substr(0, dot);
      db.cat1 = db.cat1.substr(dot+1);
    }

    dot = db.cat2.find('.');
    if (dot != std::string::npos) {
      db.my_db2 = db.cat2.substr(0, dot);
      db.cat2 = db.cat2.substr(dot+1);
    }
  }

// Check if output DB given as part of output table
  if (!odb_set) {

    std::size_t dot = t.otab.x.find('.');
    if (dot != std::string::npos) { 
      t.otab.out_db = t.otab.x.substr(0, dot);
      t.otab.x = t.otab.x.substr(dot+1);
    }
  }



  unsigned long inr1, nr1_old, nr2_old, nr1 = 0, nr2 = 0, j, ij, iin_id, n, n_unmatched, ndup = 0, npix = 1,
                *id_list = NULL, *id1 = NULL, *mt1 = NULL, *mt2 = NULL, *rn1 = NULL, *rn2 = NULL;

  char **refid1 = NULL;  // catwise
  if (t.use_master_id1)
    refid1 = (char **) malloc(sizeof(char *));

  unsigned long long *refid2 = NULL;

  long long l_ra = 0, l_de = 0;
  double *ra1 = NULL, *de1 = NULL, *ra2 = NULL, *de2 = NULL, rac = 0., decc = 0.;
  char **row;
  string in_id;

// If an argument passed, it is the pixel ID: only process it
  if (argc == 1) { 
    full_scan = 0;  // disable full table scan
    t.in_full = false;  // disable one shot table matching
    in_id = string(*argv);
  }
// If two arguments passed, then build list in that range
  else if (argc == 2) { 
    full_scan = 0;  // disable full table scan
    t.in_full = false;  // disable one shot table matching
    iin_id = atoi(*argv++);
    in_id = itos(iin_id);
    unsigned long iin_id2 = atoi(*argv);
    npix = iin_id2 - iin_id + 1;
    if ( !(id_list = (unsigned long *) malloc(npix * sizeof(unsigned long))) ) {
      cerr << "--> Error: id_list: error allocating memory.\n";
      exit (-1);
    }
    for (i = 0; i < npix; i++)
      id_list[i] = iin_id + i;
  }


  if (db.my_passw.empty()) {
    cout <<"Enter "<< db.my_user <<" password: ";
    getline(cin, db.my_passw);
  }

  cout <<"     "<< PROGNAME << VERID << endl<<endl;

  cout <<"===> '"<< db.cat1 <<"' vs '"<< db.cat2 <<"': matches at max sep. of "<< dtos3f(min_dist)<< bl << sep_unit <<" <===\n\n";

  cout <<"---> InCat fields: "<< t.id_coln1 <<", "<< t.id_coln2 <<", "<< t.ra_coln1 <<", "<< t.de_coln1;

  if (t.use_master_id1)
	cout <<"; added reference field: "<< t.rf_coln1;
   cout << endl;

  cout <<"---> RefCat fields: "<< t.id_coln1 <<", "<< t.ra_coln2 <<", "<< t.de_coln2;

  if (t.use_master_id2)
	cout <<"; added reference field: "<< t.rf_coln2;
   cout << endl << endl;

//  DBConn db;
//  db.connect("generic", "password", "MyCats");
//  Query qry(&db);
//  qry.query("SELECT RAmas/3.6e6, DECmas/3.6e6 FROM GSC23_htm_6 where DIF_Circle("+
//    coords +")", true);
//  nr1 = qry.nRows();


/* Connect to the DB */
  if (!db_init(my_cID))
  {
    cerr << PROGNAME <<"Can't set CONNECT_TIMEOUT for MySQL connection.\n";
    exit (1);
  }

  if ( !db_connect(my_cID, db.my_host.c_str(), db.my_user.c_str(), db.my_passw.c_str(), db.my_db1.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

// First get the total number of entries in the table to be matched
  qry_str = "SELECT count(*) FROM "+ db.cat1;

if (verbose)
  cout <<"Query: "<< qry_str << endl;

  if ( !db_query(my_cID, qry_str.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

  if (atol(db_data(my_cID, 0, 0)) < 1) {
    cerr << PROGNAME <<": DB error or empty table '"<< db.cat1 <<"'\n";
    exit(1);
  }
  db_free_result(my_cID);


// Display settings
  const int iwidth = 6, dwidth = 12;  // printed values widths
  unsigned short refidw1 = 20, refidw2 = 22, idw1 = 0, idw2 = 0;  // ID width (TBD)
  cout << std::setiosflags(ios::fixed);
  cout << std::setprecision(7);

// NOTE: HEALPix pixelization X-match not yet implemented
  if (use_hpx) {
    if (!input_col_names1) {
      t.id_coln1 = "healpID_nest_"+ t.order1;
      t.id_coln2 = "healpID_nest_"+ t.order2;
      //t.id_coln1 += t.order1;
      //t.id_coln2 += t.order2;
    }
    idw1 = dif_hpxid_maxw(t.order1);

    db_view_order2 = db.cat1 +"_healp_nest_"+ t.order2;
  } else {
    if (!input_col_names1) {
// HTM IDs column name
      t.id_coln1 += t.order1;
      t.id_coln2 += t.order2;
    }
    idw1 = dif_htmid_maxw(t.order1);

    db_view_order2 = db.cat1 +"_htm_"+ t.order2;
  }

  idw1 = MAX(idw1, t.id_coln1.length());

// Preliminary check in DIF.tbl to see if the requested indices are available.
// Should also check for reference catalogue...
  //dif_cat_info(my_cID, my_db1, cat1, &catinfo);
  dif_cat_info(my_cID, db.my_db1, db.cat1);


if (verbose)
  cout << db.my_db1 <<dt<< db.cat1 <<" info in DIF.tbl: \n";

  int nd = 0;
  for (i = 0; i < catinfo.size(); i++) {
    if (catinfo[i].param == t.order1 || catinfo[i].param == t.order2)
      nd++;
if (verbose)
  cout <<" id_type: "<< catinfo[i].id_type <<" id_opt: "<< catinfo[i].id_opt <<" Depth/order: "<< catinfo[i].param
       <<"  Ra_field: '"<< catinfo[i].Ra_field <<"'  Dec_field: '"<< catinfo[i].Dec_field <<"'\n";
  }

  if (nd != 2 && !full_scan)
    cout << PROGNAME <<": Warning: the 2 requested depths/orders where not found in DIF.tbl for "<< db.my_db1 <<dt<< db.cat1 << endl;

  string qry_ini, difqry_ini1, ra_fld1, de_fld1, ra_fld2, de_fld2;
  bool fld1_type_mas = true, fld2_type_mas = true;

  if (input_col_names1) {
	ra_fld1 = t.ra_coln1;
	de_fld1 = bt+ t.de_coln1 +bt;
// This is TODO.
	if (ra_fld1.find("mas") == string::npos) 
		fld1_type_mas = false; 
	else {
		ra_fld1 += t.deg_fac;
		de_fld1 += t.deg_fac;
	}
  } else {
	ra_fld1 = t.ra_coln1 + t.deg_fac;
	de_fld1 = t.de_coln1 + t.deg_fac;
  }

  if (input_col_names2) {
	ra_fld2 = t.ra_coln2;
	de_fld2 = bt+ t.de_coln2 +bt; 
// This is TODO.
	if (ra_fld2.find("mas") == string::npos) 
		fld2_type_mas = false; 
	else {
		ra_fld2 += t.deg_fac;
		de_fld2 += t.deg_fac;
	}
  } else {
	ra_fld2 = t.ra_coln2 + t.deg_fac;
	de_fld2 = t.de_coln2 + t.deg_fac; 
  }

// For full catalogue scan build the list of pixels (could read from the query...)
  if (full_scan) {
    qry_str = "SELECT DISTINCT "+ t.id_coln1 +" FROM "+ db.cat1;
cout<<"Query: "<< qry_str<<endl;

    if ( !db_query(my_cID, qry_str.c_str()) ) {
	cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
	exit (1);
    }

    npix = db_num_rows(my_cID);
    cout << db.cat1 <<": N_pixels to process: "<< npix << endl;

    if (npix == 0) {
	cerr << PROGNAME <<": no pixel IDs found in "<< db.cat1 << endl;
	exit(1);
    }
    if ( !(id_list = (unsigned long *) malloc(npix * sizeof(unsigned long))) ) {
	cerr << PROGNAME <<": error allocating memory.\n";
	exit (-1);
    }

    in_id = db_data(my_cID, 0, 0);

    for (i = 0; i < npix; i++)
      id_list[i] = atoi(db_data(my_cID, i, 0));

    db_free_result(my_cID);
  }  // full_scan

  vector<float> distance12;  // In arcsec
  vector<long> match1, match2;
  bool tab_swapped;

// Matching distance in arcsec
  if (min_dist >= 0.) {
    matchlength = min_dist / sep_scale;
  } else
    min_dist = 1.;

// This could be parametrized
  minchunksize = matchlength * 10;


//
// -- Main loop for each pixel
//
for (n = 0; n < npix; n++) {

  if (n == 0) {

    if (save_match)
      crea_out_tabs(my_cID, drop_prematch, verbose);

  } else {
    in_id = itos(id_list[n]);
    db_select(my_cID, db.my_db1.c_str());
  }

/*
cout<<in_id<<endl;
char c;
printf("Enter character: ");
c = getchar();
*/

  if (!t.in_full)
    cout <<"--> "<< t.id_coln1 <<": "<< in_id << endl;

  qry_str = "SELECT count(*) FROM "+ db.cat1;
  if (!t.in_full)
// First get only objects in given pixel
    qry_str += " WHERE "+ t.id_coln1 +"="+ in_id;

if (verbose) {
  if (!t.in_full)
    cout << n <<" ("<< in_id <<"): Query: "<< qry_str << endl;
  else
    cout <<"Query: "<< qry_str << endl;
}
    if ( !db_query(my_cID, qry_str.c_str()) ) {
      cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
      exit (1);
    }

    inr1 = atoi(db_data(my_cID, 0, 0));
    db_free_result(my_cID);

// To maintain the same nr of cols use dummy input id

  difqry_ini1 = "SELECT DISTINCT "+ ra_fld1 +co+ de_fld1 +co+ t.id_coln1;

  if (t.use_master_id1)
    difqry_ini1 += co+ t.rf_coln1;


  difqry_ini1 += " FROM ";

  string tmp_tab = "tmpx_"+ t.id_coln1;

  qry_str = "DROP TABLE if exists "+ tmp_tab;
if (verbose)
  cout <<"Query: "<< qry_str << endl;

  if ( !db_query(my_cID, qry_str.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

  qry_str = difqry_ini1 + db.cat1;

// Note: the DIF_sNeighb can be time consuming.
  if (!t.in_full)
    qry_str += " WHERE "+ t.id_coln1 +"="+ in_id +
               " UNION ALL "+ difqry_ini1 + db_view_order2 +" WHERE DIF_sNeighb("+ t.order1 +co+ in_id +co+ t.order2 + ")";

//qry_str += " order by "+ t.ra_fld1;

 qry_str = "CREATE temporary TABLE "+ tmp_tab +bl+ qry_str;


if (verbose)
  cout <<"Query: "<< qry_str << endl;

// Note: temporary use only order1 to bind dynamically the query buffer.
// This means we use MASTERhpx6 (smallint -> MYSQL_TYPE_SHORT) and runningnumber (long int -> MYSQL_TYPE_LONG)
  //ret = db_stmt_prepexe2(my_cID, qry_str.c_str(), t.order1.c_str(), 0);

  if ( !db_query(my_cID, qry_str.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }
if (verbose)
  cout <<"TMP table created"<< endl;

  db_free_result(my_cID);

// Select from the temp table
  qry_str = "select * from "+ tmp_tab;

if (verbose)
  cout <<"Query: "<< qry_str << endl;


  if ( db_stmt_prepexe2(my_cID, qry_str.c_str(), t.order1.c_str(), 0) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

  nr1_old = nr1;
  nr1 = db_num_rows(my_cID);

if (verbose) {
  if (!t.in_full)
    cout << db.cat1 <<": "<< t.id_coln1 <<"="<< in_id <<": distinct N_entries="<< nr1 <<" ("<< inr1<<" within pixel)\n";
  else
    cout << db.cat1 <<": distinct N_entries="<< nr1 <<" ("<< inr1<<" total)\n";
}

  if (nr1 == 0) {
    mysql_stmt_close(stmt);
    db_free_result(my_cID);
    continue;
  }


  if ( !(id1 = (unsigned long *) realloc(id1, nr1 * sizeof(unsigned long))) ) {
	cerr << PROGNAME <<": error re-allocating memory.\n";
	exit (-1);
  }
  if ( !(ra1 = (double *) realloc(ra1, nr1 * sizeof(double))) ) {
	cerr << PROGNAME <<": error re-allocating memory.\n";
	exit (-1);
  }
  if ( !(de1 = (double *) realloc(de1, nr1 * sizeof(double))) ) {
	cerr << PROGNAME <<": error re-allocating memory.\n";
	exit (-1);
  }

  if (t.use_master_id1 && nr1 > nr1_old)
	refid1 = resize2d(nr1_old, (STRING_SIZE+1), nr1, (STRING_SIZE+1), refid1);

  i = 0;
  while (!mysql_stmt_fetch(stmt))
  {
    ra1[i] = dbl_data[0];
    de1[i] = dbl_data[1];
    id1[i] = long_data[0];
 
    if (t.use_master_id1) {
	//refid1[i] = long_data[1];
	memcpy(refid1[i], str_data, STRING_SIZE);
	refid1[i][STRING_SIZE] = '\0';
    }
    i++;
  }


  if (do_list_all)
    for (i = 0; i < nr1; i++) {
      cout <<setw(iwidth)<< i << bl <<setw(idw1)<<id1[i];
      if (t.use_master_id1)
        cout << bl <<setw(refidw1)<<refid1[i];
        //cout << bl <<setw(idw1)<<mt1[i] << bl <<setw(iwidth)<<rn1[i];
      cout << bl <<setw(dwidth)<< ra1[i]
           << bl <<setw(dwidth)<< de1[i] << endl;
    }

  mysql_stmt_close(stmt);

  qry_str = "DROP TABLE "+ tmp_tab;

if (verbose)
  cout <<"Query: "<< qry_str << endl;

  if ( !db_query(my_cID, qry_str.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

if (verbose)
  cout <<"TMP table removed"<< endl;

  db_free_result(my_cID);

// ---  end selection from table 1  ---


  if ( db_select(my_cID, db.my_db2.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

//  difqry_ini1 = string("SELECT ") + ra_fld2 +" as RA"+co+ de_fld2 +co+ in_id;
  difqry_ini1 = "SELECT "+ ra_fld2 +co+ de_fld2 +co+ in_id;
  if (t.use_master_id2)
    difqry_ini1 += co+ t.rf_coln2;
    //difqry_ini1 += co+ t.mt_coln +co+ t.rn_coln;

  qry_str = difqry_ini1 +" FROM "+ db.cat2 +" WHERE "+ t.id_coln1;
  if (!t.in_full)
    qry_str += "="+ in_id;
  else {
    qry_str += " IN (";
    int in;
    for (in = 0; in < npix - 1; in++)  // for each pixel
      qry_str += itos(id_list[in]) +co;
   qry_str += itos(id_list[in]) +")";

  }

//qry_str += " order by RA";

if (verbose)
  cout <<"Query: "<< qry_str << endl;

  if ( db_stmt_prepexe2(my_cID, qry_str.c_str(), t.order1.c_str(), 1) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit (1);
  }

  nr2_old = nr2;
  nr2 = db_num_rows(my_cID);

if (verbose) {
  if (!t.in_full)
    cout << t.id_coln1 <<"="<< in_id <<", "<< db.cat2 <<": N_entries="<< nr2 << endl;
  else
    cout << db.cat2 <<": all N_entries="<< nr2 << endl;
}
  if (nr2 > 0) {

    if ( !(ra2 = (double *) realloc(ra2, nr2 * sizeof(double))) ) {
	cerr << PROGNAME <<": error re-allocating memory.\n";
	exit (-1);
    }
    if ( !(de2 = (double *) realloc(de2, nr2 * sizeof(double))) ) {
	cerr << PROGNAME <<": error re-allocating memory.\n";
	exit (-1);
    }

    if (t.use_master_id2)
	if ( !(refid2 = (unsigned long long *) realloc(refid2, nr2 * sizeof(unsigned long))) ) {
	  cerr << PROGNAME <<": error re-allocating memory.\n";
	  exit (-1);
	}

    i = 0;
//  while ((record = mysql_fetch_row(result[0])) != NULL)
    while (!mysql_stmt_fetch(stmt))
    {
      ra2[i] = dbl_data[0];
      de2[i] = dbl_data[1];

    if (t.use_master_id2) {
	refid2[i] = long_data[1];
      //memcpy(refid2[i], str_data, STRING_SIZE);
      //refid2[i][STRING_SIZE] = '\0';
    }

//cout<< dbl_data[0]<<" "<< dbl_data[1]<<" "<<long_data1<<" "<<long_data2<<endl; 
//row2num(record, &ra2[i], &de2[i], &mt2[i]);
   //sscanf(record[0],"%lf", &ra2[i]);
   //sscanf(record[1],"%lf", &de2[i]);
   //sscanf(record[2],"%d",  &mt2[i]);
      i++;
    }


    if (do_list_all)
      for (i = 0; i < nr2; i++) {
        cout << setw(iwidth) << i;
        if (t.use_master_id2)
          cout << bl <<setw(refidw2)<<refid2[i];
          //cout << bl <<setw(idw2)<<mt2[i] << bl <<setw(iwidth)<<rn2[i];
        //cout <<setw(iwidth)<< i << bl <<setw(iwidth)<<rn2[i]
        cout << bl << setw(dwidth) << ra2[i]
             << bl << setw(dwidth) << de2[i] << endl;
      }
  }

  mysql_stmt_close(stmt);
  db_free_result(my_cID);

// ---  end selection from table 2  ---

  cout <<"--> In: "<< nr1;
  if (!full_scan)
	cout <<" ("<< nr1 - inr1 <<" of which ext.)";
	
  cout <<", Ref: "<< nr2 << endl;

  if (inr1 > nr2)  // within pixel, more objects in catalogue to be matched than in ref. cat.
    cout <<"--> Warning: within pixel, RefCat "<< db.cat2 <<" has less objects than "<< db.cat1 << endl;

  iin_id = atoi(in_id.c_str());
  nmatch = 0;

  if (nr1 != 0 && nr2 != 0) {
    //continue;
    //return (1);

// The matching

// In case of 1 to 1 match, this is managed in spherematch2
//nmatchmax = MIN(nr1,nr2);
//nmatch = nmatchmax;
 
    nmatch = nr1*nr2;

//
// TODO: Make sure first list is the longest or the other way around?
//
  if (nr1 > nr2) {
    tab_swapped = true;
    if (multi_match)
	spherematch2_mm(nr2, ra2, de2, nr1, ra1, de1, matchlength, minchunksize,
			match2, match1, distance12, &nmatch);
    else
	spherematch2(nr2, ra2, de2, nr1, ra1, de1, matchlength, minchunksize,
			match2, match1, distance12, &nmatch);
  } else {
    tab_swapped = false;
    if (multi_match)
	spherematch2_mm(nr1, ra1, de1, nr2, ra2, de2, matchlength, minchunksize,
			match1, match2, distance12, &nmatch);
    else
	spherematch2(nr1, ra1, de1, nr2, ra2, de2, matchlength, minchunksize,
			match1, match2, distance12, &nmatch);
  }
//cout<<"Match done."<<endl;

  }  // nr1 and nr2 !=0

  if (nmatch == 0) {
    cout << "--> Warning: no match at a separation of "<< dtos3f(min_dist) <<bl<< sep_unit << endl;
    if (npix == 1 && !save_match)
      return (0);
    //else
      //continue;
  }

// Limit max matches to that of input list
  //nmatchret = MIN(nmatch,nmatchmax);
  nmatchret = nmatch;


// Matches external to input pixel
  nmatchext = 0;
  if (!t.in_full)
    for (i = 0; i < nmatchret; i++)
      if (id1[match1[i]] != iin_id)
        nmatchext++;

// Are considered unmatched only those objects within the input pixel
  if (!multi_match)
    n_unmatched = inr1 - nmatchret + nmatchext;
  else {
    n_unmatched = 0;
    for (i = 0; i < nr1; i++) {
      if (t.in_full || id1[i] == iin_id) {
        for (j = 0; j < nmatchret; j++)
          if (match1[j] == i)
            break;
        if (j == nmatchret) n_unmatched++;
      }
    }
  }

  cout <<"--> X: "<< nmatch <<" (";
  if (inr1 > 0) {
    cout << std::setprecision(1);
    cout << std::setw(5) << (nmatch*1000/inr1) / 10.;
  } else
    cout <<"0";
  cout << std::setprecision(7);
  cout << "%, ret: "<< nmatchret <<")";

  if (!t.in_full)
    cout << ", Xext: "<< nmatchext;

  cout <<", notX: "<< n_unmatched << endl;

  totals_read += inr1;
  totals_readext += nr1 - inr1;
  totals_match += nmatchret;
  totals_unmatch += n_unmatched;
  totals_matchext += nmatchext;

  cout <<"TOTAL: read: "<< totals_read + totals_readext;
  if (!t.in_full)
    cout <<" ("<< totals_readext <<" of which external)";

  cout <<", matched: "<< totals_match <<" (external: "<< totals_matchext <<"), unmatched: "<< totals_unmatch << endl;
// 23/06/2020: separation returned in arcsec
/*
  if (nmatch > 0) {
	for (i = 0; i < nmatchret; i++)
	  distance12[i] *= sep_scale;  // To arcsec
  }
*/


// Insert to be optimized
  if (save_match && nmatch > 0) {
    qry_ini = "INSERT INTO "+ t.otab.out_db +dt+ t.otab.x +" VALUES";
    qry_str = "";

// For matched objects in InCat and pixel ID, save the corresponding coords and ID of RefCat
    for (i = 0; i < nmatchret; i++) {
      if (!t.in_full)
        qry_str += "("+ itos(id1[match1[i]]) +co+ in_id +co;
      else
        qry_str += "("+ itos(id1[match1[i]]) +",HTMLookup("+ t.order1 +co+ dtos(ra2[match2[i]]) +co+ dtos(de2[match2[i]]) +"),";

      if (t.use_master_id1)
        qry_str += sq+ refid1[match1[i]] +sq+co;
      if (t.use_master_id2)
        qry_str += ltos(refid2[match2[i]]) +co;
 
// First coords of reference cat
     //if ( fld1_type_mas ) {
	l_ra = (long long) (ra1[match1[i]] * 3.6e6 + 0.5);
	l_de = de1[match1[i]]>0 ? (long long) (de1[match1[i]]*3.6e6 + 0.5) : (long long) (de1[match1[i]]*3.6e6 - 0.5);
	qry_str += ltos(l_ra) +co+ ltos(l_de);
      //} //else {
	//qry_str += dtos(ra2[match2[i]]) +co+ dtos(de2[match2[i]]);
      //}

     //if ( fld2_type_mas ) {
        l_ra = (long long) (ra2[match2[i]] * 3.6e6 + 0.5);
        l_de = de2[match2[i]]>0 ? (long long) (de2[match2[i]]*3.6e6 + 0.5) : (long long) (de2[match2[i]]*3.6e6 - 0.5);
        qry_str += co+ ltos(l_ra) +co+ ltos(l_de);
      //} //else {
	//qry_str += co+ dtos(ra1[match1[i]]) +co+ dtos(de1[match1[i]]);
      //}

      qry_str += co+ dtos3f(distance12[i]) +co+ origID +")";
       
      if (((i+1) % insert_Nrows) == 0) {
        qry_str = qry_ini + qry_str;
if (verbose)
  cout <<"i: "<< i <<" "<< qry_str << endl;
        if ( !db_query(my_cID, qry_str.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit (1);
        }
        qry_str = "";
      } else if (i != nmatchret - 1)
	qry_str += co;

    }  // end for i

// The remaining rows
    if ((i % insert_Nrows) != 0) {
      qry_str = qry_ini + qry_str;
if (verbose)
  cout <<"i: "<< i <<" "<< qry_str << endl;
      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }
    }

  }  // end if save_match



// Print out the matched objects
  if (do_list_match && nmatch > 0) {
    cout << "\nMatched list:\n"

         << setw(iwidth)<<"SeqID1"<< bl << setw(idw1) << t.id_coln1 << bl;
      if (t.use_master_id1)
        cout << setw(refidw1) << t.rf_coln1 << bl;

      cout  << setw(dwidth) <<"RAdeg1"<< bl << setw(dwidth) <<"DECdeg1"<< bl
         << setw(iwidth) <<"SeqID2"<< bl;
      if (t.use_master_id2)
        cout << setw(refidw2) << t.rf_coln2 << bl;

      cout << setw(dwidth)<<"RAdeg2"<< bl << setw(dwidth) <<"DECdeg2"<< bl
         <<"  Sep ("<< sep_unit <<")"<< endl;

    for (i = 0; i < nmatchret; i++) {
      cout << setw(iwidth) << match1[i] << bl <<setw(idw1) << id1[match1[i]] << bl;
      if (t.use_master_id1)
        cout << setw(refidw1) << refid1[match1[i]] << bl;

      cout <<setw(dwidth)<< ra1[match1[i]] << bl << setw(dwidth) << de1[match1[i]]<< bl
           << setw(iwidth) << match2[i] << bl;
      if (t.use_master_id2)
        cout << setw(refidw2) << refid2[match2[i]] << bl;

      cout << setw(dwidth) << ra2[match2[i]] << bl << setw(dwidth) << de2[match2[i]] << bl
           << setw(dwidth) << distance12[i] << endl;
    }

  }

// Just the objects matched outside the pixel
  if (do_list_external && !do_list_match) {
    bool first = true;
    for (i = 0; i < nmatchret; i++) {
      if (id1[match1[i]] != iin_id) {
        if (first) {
          first = false;
          cout <<"\nExternal to pixel matched list:"<< endl
               << setw(iwidth) <<"SeqID1"<< bl << setw(idw1) << t.id_coln1 << bl;

          if (t.use_master_id1)
            cout << setw(refidw1) << t.rf_coln1 << bl;
            //cout <<setw(idw1)<< t.mt_coln << bl <<setw(idw1)<< t.rn_coln << bl;

          cout << setw(dwidth) <<"RAdeg1"<< bl << setw(dwidth) <<"DECdeg1"<< bl
               << setw(iwidth) <<"SeqID2"<< bl;

          if (t.use_master_id2)
            cout << setw(refidw2) << t.rf_coln2 << bl;
            //cout <<setw(idw2)<< t.mt_coln << bl <<setw(idw2)<< t.rn_coln << bl;

          cout << setw(dwidth) <<"RAdeg2"<< bl << setw(dwidth) <<"DECdeg2"<< bl
               <<"  Sep (" << sep_unit <<")"<< endl;
        }

        cout << setw(iwidth) << match1[i] << bl << setw(idw1) << id1[match1[i]] << bl;
        if (t.use_master_id1)
          cout << setw(refidw1) << refid1[match1[i]] << bl;
          //cout <<setw(idw1)<< mt1[match1[i]] << bl << setw(idw1)<< rn1[match1[i]] << bl;

        cout << setw(dwidth) << ra1[match1[i]] << bl << setw(dwidth) << de1[match1[i]] << bl
             << setw(iwidth) << match2[i] << bl;
        if (t.use_master_id2)
          cout << setw(refidw2) << refid2[match2[i]] << bl;
          //cout <<setw(idw2)<< mt2[match2[i]] << bl << setw(idw2)<< rn2[match2[i]] << bl;

        cout << setw(dwidth) << ra2[match2[i]] << bl << setw(dwidth) << de2[match2[i]] << bl
             << setw(dwidth) << distance12[i] << endl;

      }
    }
    //if (first)
      //cout <<"No match outside pixel "<< in_id << endl<<endl;
  }


// Save external matches: for one shot match it is nmatchext=0
  if (save_match && nmatchext > 0) {
    qry_ini = string("INSERT INTO ") + t.otab.out_db +dt+ t.otab.ext +" VALUES";
    qry_str = "";
    ij = 0;
    bool end_ext = false;

    for (i = 0; i < nmatchret; i++) {
      if (id1[match1[i]] != iin_id) {

        qry_str += "("+ itos(id1[match1[i]]) +co;
        if (t.use_master_id1)
          qry_str += sq+ refid1[match1[i]] +sq +co;

         l_ra = lrint(ra1[match1[i]]*3.6e6);
         l_de = de1[match1[i]]>0 ? lrint(de1[match1[i]]*3.6e6) : lrint(de1[match1[i]]*3.6e6);
         qry_str += ltos(l_ra) +co+ ltos(l_de) +co+ dtos3f(distance12[i]) +")";

        ij++;
// Insert query every insert_Nrows
        if ((ij % insert_Nrows) == 0) {
          qry_str = qry_ini + qry_str;
if (verbose)
  cout <<"ij: "<< ij <<" "<< qry_str << endl << endl;
          if ( !db_query(my_cID, qry_str.c_str()) ) {
            cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
            exit (1);
          }
          qry_str = "";
          ij = 0;
        } else qry_str += co;

	if (ij == nmatchext)
		break;

      }  // end if != iin_id
    }  // end for i


// The remaining rows
    if (ij) {
      qry_str.erase(qry_str.end() - 1);
      qry_str = qry_ini + qry_str;
if (verbose)
  cout <<"ij: "<< ij <<" "<< qry_str << endl;
      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }
    }

  }  // end if save_match


// Returned lists follow the first passed catalogue order, which can be swapped (see above).
// Note: if all the objects in the reference catalogue are matched, just issue a warning!

  //if ((do_list_match || save_match) && inr1 > nmatchret) {
    //sort(match1.begin(), match1.end());
  //}


// Unmatched objects
  if (save_match && n_unmatched > 0) {
    qry_ini = string("INSERT INTO ") + t.otab.out_db +dt+ t.otab.nx +" VALUES";
    qry_str = "";
    ij = 0;

    unsigned long j0 = 0;
    std::vector<long> *vr;

    if (tab_swapped)
	vr = &match2;
    else
	vr = &match1;

    for (i = 0; i < nr1; i++) {
      if (t.in_full || id1[i] == iin_id) {
        for (j = j0; j < nmatchret; j++)
          if ((*vr)[j] == i) {
          //if (match1[j] == i) {
            j0 = j + 1;
            break;
          }

        if (j == nmatchret) {  // not there
          if (!t.in_full)
            qry_str += "("+ in_id +co;
          else
            qry_str += "("+ itos(id1[i]) +co;

          if (t.use_master_id1)
            //qry_str += itos(refid1[i]) +co;
            qry_str += sq+ refid1[i] +sq+co;
            //qry_str += itos(mt1[i]) +co+ itos(rn1[i]) +co;

          l_ra = (long long) (ra1[i] * 3.6e6 + 0.5);
          l_de = de1[i] > 0 ? (long long) (de1[i] * 3.6e6 + 0.5) : (long long) (de1[i] * 3.6e6 - 0.5);
          qry_str += ltos(l_ra) +co+ ltos(l_de) +co+ origID +")";

          ij++;
// Insert query every insert_Nrows
          if ((ij % insert_Nrows) == 0) {
            qry_str = qry_ini + qry_str;
if (verbose)
  cout <<"ij: "<< ij <<" "<< qry_str << endl;
            if ( !db_query(my_cID, qry_str.c_str()) ) {
              cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
              exit (1);
            }
            qry_str = "";
            ij = 0;
          } else qry_str += co;

        }  // end if j == nmatchret
      }  // end if iin_id
    }  // end for i

// The remaining rows
    if (ij) {
      qry_str.erase(qry_str.end() - 1);
      qry_str = qry_ini + qry_str;
if (verbose)
  cout <<"ij: "<< ij <<" "<< qry_str << endl;
      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }
    }

  }  // end save_match


  if (do_list_match && n_unmatched > 0) {
// Unmatched list for Cat1
    cout <<"Unmatched in "<< db.cat1 <<":\n";
    for (i = 0; i < nr1; i++) {
      if (t.in_full || id1[i] == iin_id) {
        for (j = 0; j < nmatchret; j++)
          if (match1[j] == i)
            break;
        if (j == nmatchret) {  // not there
          cout << setw(iwidth)<< i << bl << setw(idw1) << id1[i] << bl;
          if (t.use_master_id1)
            cout << setw(refidw1)<<refid1[i] << bl;
          cout << setw(dwidth) << ra1[i] << bl << setw(dwidth) << de1[i] << endl;
        }
      }
    }
    cout << endl;
  }

// Unmatched list for ref. Cat2 (this is not too meaningful)
  if (do_list_match && nr2 > nmatchret) {
    cout << nr2 - nmatchret <<" unmatched in "<< db.cat2 <<" ("<< t.id_coln1 <<"="<< in_id <<"):\n";

if (verbose) {
//... need to sort
  sort(match2.begin(), match2.end());
  j = 0;
  for (i = 0; i < nr2; i++) {
    if (j < nmatchret)
      ij = match2[j];
    else
      ij = 0;
    if (i != ij) {
      cout << setw(iwidth) << i << bl;
      if (t.use_master_id2)
        cout << setw(refidw2) << refid2[i] << bl;
        //cout << setw(idw1)<< mt2[i] << bl <<setw(iwidth)<<rn2[i] << bl;
      cout << setw(dwidth)<< ra2[i] << bl << setw(dwidth) << de2[i] << endl;
    } else
      j++;
  }
}

  }

  if (t.in_full)
    break;

}  // end for each pixel

  free(id1);
  free(ra1);
  free(de1);
  free(ra2);
  free(de2);
  //id1 = NULL;
  //ra1 = NULL;
  //de1 = NULL;
  //ra2 = NULL;
  //de2 = NULL;
  if (t.use_master_id1) {
    for (i = 0; i < nr1; i++)
	free(refid1[i]);
    free(refid1);
    //refid1 = NULL;
  }
  if (t.use_master_id2) {  // catwise
    free(refid2);
    //refid2 = NULL;
  }

  if (id_list)
	free(id_list);

// Now we can clean up the unmatched tables (only for full scan or match in one shot)
// Always first drop the tables
  if (save_match) {
    if (full_scan || t.in_full) {
      qry_str = "DROP TABLE IF EXISTS "+ t.otab.out_db +dt+ t.otab.nxc +co+ t.otab.out_db +dt+ t.otab.nxcf;
      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }
    }

    string qry_str_x, qry_str_xe, qry_str_nx, qry_str_nxc, qry_str_e, qry_str_crea = "";
    qry_str_x  = "ALTER TABLE "+ t.otab.out_db +dt+ t.otab.x +" ADD KEY ("+ t.id_coln1 +co+ "RAmas)";
    qry_str_nx = "ALTER TABLE "+ t.otab.out_db +dt+ t.otab.nx +" ADD KEY ("+ t.id_coln1 +co+ "RAmas)";

// Now create the clean table from a join of the unmatched and external tables, or simply rename the unmatched one
    if (totals_matchext > 0) {
      qry_str_e = "ALTER TABLE "+ t.otab.out_db +dt+ t.otab.ext +" ADD KEY ("+ t.id_coln1 +co+ "RAmas)";

      qry_str_crea = "CREATE TABLE "+ t.otab.out_db +dt+ t.otab.nxc +" LIKE "+ t.otab.out_db +dt+ t.otab.nx;
      qry_str = "INSERT INTO "+ t.otab.out_db +dt+ t.otab.nxc +" SELECT u.* FROM "+
                t.otab.out_db +dt+ t.otab.nx +" AS u LEFT JOIN "+
                t.otab.out_db +dt+ t.otab.ext +" AS e ON u."+ t.id_coln1 +"=e."+t.id_coln1 +
                " AND u.RAmas=e.RAmas AND u.DECmas=e.DECmas WHERE e."+t.id_coln1 +" IS NULL";
                //" AND u."+ t.ra_coln2 +"=e."+ t.ra_coln2 +" AND u."+ t.de_coln2 +"=e."+ t.de_coln2 +" WHERE e."+t.id_coln1 +" IS NULL";

      qry_str_xe = "SELECT COUNT(u."+ t.id_coln1 +") AS n, u.ref_"+ t.id_coln1 +",u."+ t.id_coln1 +",u.RAmas,u.DECmas"+
                ", u.Sep FROM "+ t.otab.out_db +dt+ t.otab.x +" AS u LEFT JOIN "+ t.otab.out_db +dt+ t.otab.ext +" AS e ON u."+
                t.id_coln1 +"=e."+ t.id_coln1 +
                " AND u.RAmas=e.RAmas AND u.DECmas=e.DECmas"+
                " WHERE e."+ t.id_coln1 +" IS NOT NULL GROUP BY e."+ t.id_coln1 +",e.RAmas,e.DECmas" +
                " HAVING n>1";


    } else {
      if (drop_prematch) {
        qry_str_e = "DROP TABLE IF EXISTS "+ t.otab.out_db +dt+ t.otab.ext;
        if ( !db_query(my_cID, qry_str_e.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit (1);
        }
      }
      qry_str_e = "-- No object in the external pixels table";
      qry_str_xe = "-- No cleaning required for matched and unmatched tables";
      qry_str = "RENAME TABLE "+ t.otab.out_db +dt+ t.otab.nx +" TO "+ t.otab.out_db +dt+ t.otab.nxc;
    }  // totals_imatchext > 0

if (verbose)
  cout << qry_str_x << endl
       << qry_str_nx << endl
       << qry_str_crea << endl
       << qry_str << endl
       << qry_str_e << endl
       << qry_str_xe << endl;

    if (full_scan || t.in_full) {
      cout <<"\nAdding position key to matched objects table...\n";
      if ( !db_query(my_cID, qry_str_x.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }
      cout <<"Adding position key to unmatched objects table...\n";
      if ( !db_query(my_cID, qry_str_nx.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }

      if (totals_matchext > 0) {

        cout <<"Adding position key to externally matched objects table...\n";
        if ( !db_query(my_cID, qry_str_e.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit (1);
        }

        cout <<"Cleaning matched objects table for multiple externally matched objects...\n";
        if ( !db_query(my_cID, qry_str_xe.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit (1);
        }

// Loop on multiple entries
        ndup = db_num_rows(my_cID);

        if (ndup > 0) {
          cout<<"Number of duplicates to clean: "<< ndup <<endl;
          int cid = 1;

          if (!db_init(cid)) {
            cerr << PROGNAME <<": cannot set CONNECT_TIMEOUT for MySQL connection.\n";
            exit(1);
          }
          if ( !db_connect(cid, db.my_host.c_str(), db.my_user.c_str(), db.my_passw.c_str(), db.my_db1.c_str()) ) {
            cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
            exit (1);
          }


          string re_id, ras, des;
          for (i = 0; i < ndup; i++) {
            re_id = db_data(my_cID, i, 1);
            in_id = db_data(my_cID, i, 2);
            ras = db_data(my_cID, i, 3);
            des = db_data(my_cID, i, 4);

            qry_str_x = "SELECT ref_"+ t.id_coln1 +co+ t.id_coln1 +co+ "RAmas, DECmas, "+
			" FROM "+ t.otab.out_db +dt+ t.otab.x +" WHERE "+ t.id_coln1 +"="+ in_id +" AND "+
			t.ra_coln2 +"RAmas="+ ras +" AND DECmas="+ des +" ORDER BY Sep ASC limit 1";

if (verbose)
  cout << qry_str_x << endl;
            if ( !db_query(cid, qry_str_x.c_str()) ) {
              cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
              exit (1);
            }
            re_id = db_data(cid, 0, 0);
            in_id = db_data(cid, 0, 1);

            qry_str_x = "DELETE FROM "+ t.otab.out_db +dt+ t.otab.x +" WHERE "+ t.id_coln1 +"="+ in_id +
			" AND RAmas="+ ras +" AND DECmas="+ des +" AND ref_"+ t.id_coln1 +"!= "+ re_id;
if (verbose)
  cout << qry_str_x << endl;
            if ( !db_query(cid, qry_str_x.c_str()) ) {
              cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
              exit (1);
            }
          }

          db_free_result(cid);
          db_close(cid);

// Matched/unmatched excluding duplicates
          totals_match -= ndup;
          totals_unmatch = totals_read - totals_match;

        } else
          cout <<"No duplicated entry found in matched objects table\n";

	db_free_result(my_cID);

	cout <<"Merging unmatched and externally matched objects tables...\n";
// First create the unmatched clean table (to use same conf as _unmatched tab) 
	if ( !db_query(my_cID, qry_str_crea.c_str()) ) {
	  cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
	  exit(1);
	}

      } else
        cout <<"Renaming unmatched objects table...\n";

      if ( !db_query(my_cID, qry_str.c_str()) ) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit (1);
      }

      if (totals_matchext > 0) {
        cout <<"Adding position key to cleaned unmatched objects table...\n";
        qry_str_nxc = "ALTER TABLE "+ t.otab.out_db +dt+ t.otab.nxc +" ADD KEY ("+ t.id_coln1 +co+ t.ra_coln2 +")";
        if ( !db_query(my_cID, qry_str_nxc.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit (1);
        }
      }

// If a full input catalogue columns must be saved for unmatched objetcs
      if (out_unmatched_full && totals_unmatch > 0) {
        cout <<"Creating extended unmatched objects table...\n";
        qry_str_nxc = "CREATE TABLE "+ t.otab.out_db +dt+ t.otab.nxcf +" SELECT t1.* FROM "+ 
                      db.my_db1 +dt+ db.cat1 + " AS t1, "+  t.otab.out_db +dt+ t.otab.nxc +" AS t2 WHERE t1."+
                      t.id_coln1 +"=t2."+ t.id_coln1 +" AND t1.RAmas=t2.RAmas AND t1.DECmas=t2.DECmas";
if (verbose)
  cout << qry_str_nxc << endl;
        if ( !db_query(my_cID, qry_str_nxc.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit(1);
        }
        cout <<"Adding position key to extended unmatched objects table...\n";
        qry_str_nxc = "ALTER TABLE "+ t.otab.out_db +dt+ t.otab.nxcf +" ADD KEY ("+ t.id_coln1 +co+ t.ra_coln2 +")";
        if ( !db_query(my_cID, qry_str_nxc.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit (1);
        }

      }

    } else {  // full_scan || t.in_full
      cout <<"\nYou did not use a single full catalogues match.\n"
	"At your convenience you need to merge manually unmatched and externally matched objects tables.\n"
	"For the current run the queries could be:\n\n"
	<< qry_str_nx <<";\n"
	<< qry_str_e <<";\n"
	<< qry_str_crea <<";\n"
	<< qry_str <<";\n"
	<<"\nYou can also consider indexing the matched table:\n"
	<< qry_str_x <<";\n";
    }

    cout <<"\nYou can now use 'dif' on the output tables to create views and perform sky queries.\n";
    if (full_scan)
      cout <<"Because all the tables are already indexed, you should use the option '--views-only'.\n";
    cout <<"\nMatched objects table: "<< t.otab.out_db <<dt<< t.otab.x<<endl;

    if (full_scan || t.in_full) {
      cout <<"Unmatched objects table: "<< t.otab.out_db <<dt<< t.otab.nxc << endl;
      if (out_unmatched_full)
        cout <<"Unmatched extended objects table: "<< t.otab.out_db <<dt<< t.otab.nxcf << endl;
      if (totals_matchext > 0) {  // this is a cross-check
        qry_str = "SELECT COUNT(*) FROM "+ t.otab.out_db +dt+ t.otab.nxc;
        if ( !db_query(my_cID, qry_str.c_str()) ) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit(1);
        }

        cout <<"\nNumber of unmatched objects after table clean: "<< db_data(my_cID, 0, 0)
             <<"\n\nExtra tables (you can remove):\n"
             <<"Uncleaned unmatched objects tab: "<< t.otab.out_db <<dt<< t.otab.nx
             <<"\nNeighbours matched objects tab: "<< t.otab.out_db <<dt<< t.otab.ext << endl;
        if (totals_unmatch != atol(db_data(my_cID, 0, 0)))
          cerr << PROGNAME <<": Error: "<< db_data(my_cID, 0, 0)
               <<", number of unmatched objects in the cleaned table, is not equal to the expected "
               << totals_unmatch <<"! Please check.\n";
      }
    }
    cout << endl;
  }

  db_close(my_cID);

  cout << totals_match <<" matched, "<< totals_unmatch <<" unmatched.\n";
  return (0);
}
