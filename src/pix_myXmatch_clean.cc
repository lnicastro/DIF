/*
  Xmatch catalogue Cat_in against a reference catalogue Cat_ref.
  A simple implementation of the "spherematch" algorithm querying on
  HTM/HELAPix pixels indexed MySQL tables.

   Notes:
    1. HEALPix pixelization X-match not yet implemented. Use HTM.
    2. Unless a specific pixel ID (or range) is given, the full catalogue is
       processed.
    3. Use option "-I" for Turin Observatory standard format for DB catalogues,
       i.e. the columns MASTERhpx6 and runningnumber must be present in Cat_ref
       (with their combination being a unique index). If also the -K option is
       used, then these columns must be also present in Cat_in.
       By default just use HTM IDs and coordinates (default RAmas, DECmas).
       For the first case the (optional) output catalogue will have the columns:

       | Field         | Type                 |
       +---------------+----------------------+
       | ref_htmID_6   | smallint(5) unsigned |
       | htmID_6       | smallint(5) unsigned |
       | MASTERhpx6    | smallint(5) unsigned |
       | runningnumber | int(10) unsigned     |
       | RAmas         | int(10) unsigned     |
       | DECmas        | int(11)              |
       | Sep           | float                |
       | origID        | tinyint(4)           |
       +---------------+----------------------+

       whereas in the second, default case:

       | Field         | Type                 |
       +---------------+----------------------+
       | ref_htmID_6   | smallint(5) unsigned |
       | htmID_6       | smallint(5) unsigned |
       | ref_RAmas     | int(10) unsigned     |
       | ref_DECmas    | int(10)              |
       | RAmas         | int(10) unsigned     |
       | DECmas        | int(11)              |
       | Sep           | float                |
       | origID        | tinyint(4)           |
       +---------------+----------------------+

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
    pix_myXmatch -d TOCats -x ascc25 tycho2 -t DBout.xout_tab -D 6 14 -qAI


  LN @ IASF-INAF, June 2013                         Last changed: 19/03/2015
*/

using namespace std;

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string>

#include <vector>
#include <algorithm>

#include "my_stmt_db2.h"

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )

// Global variables
//
static string bl = " ";
static string co = ",";

// Progran name
const char PROGNAME[] = "pix_myXmatch";

// Version ID string
static string VERID="Ver 0.1f, 19-03-2015, LN@IASF-INAF";


/* Unused
  MYSQL_ROW record;

void row2num(char **row, double *c1, double *c2, unsigned long *c3) {
    //*c1 = atof(row[0]);
    //*c2 = atof(row[1]);
    //*c3 = atoi(row[2]);
   sscanf(row[0],"%lf", c1);
   sscanf(row[1],"%lf", c2);
   sscanf(row[2],"%d",  c3);
}
*/


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

string itos(int i)
{
  char buf[20];
  sprintf(buf, "%d", i);
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
  char buf[20];
  sprintf(buf, "%lf", f);
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

//char my_host[32]="localhost", my_user[32]="generic", my_passw[32]="password",
     //my_db1[32]="test", my_db2[32]="test";

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


typedef struct DIFtbl_st {
  string db;
  string name;
  IdType id_type;
  IdOpt id_opt;
  int param;
  string Ra_field;
  string Dec_field;
} DIFtbl_st;

vector<DIFtbl_st> catinfo;

typedef struct OTabs_st {
  string out_db;
  string x;
  string nx;
  string nxc;
  string ext;
} OTabs_st;

//OTabs_st otab;

typedef struct TabInfo_st {
  bool use_master_id1;
  bool use_master_id2;
  string orde1;
  string orde2;
  string id_coln1;
  string id_coln2;
  string ra_coln;
  string de_coln;
  string mt_coln;
  string rn_coln;
  string deg_fac;
  string id_sqltype;
  OTabs_st otab;
} TabInfo_st;

TabInfo_st t;

int set_out_tabs(string my_user, string cat1, string cat2)
{
  const char root_otab[]="pix_Xmatch_";
  if (t.otab.x.empty()) {
    t.otab.x   = root_otab + my_user +"_"+ cat1 +"_x_"+   cat2;
    t.otab.nx  = root_otab + my_user +"_"+ cat1 +"_nx_"+  cat2;
    t.otab.nxc = root_otab + my_user +"_"+ cat1 +"_nxc_"+ cat2;
    t.otab.ext = root_otab + my_user +"_"+ cat1 +"_ext_"+ cat2;
  } else {
    t.otab.nx  = t.otab.x +"_unmatched";
    t.otab.nxc = t.otab.x +"_unmatched_clean";
    t.otab.ext = t.otab.x +"_external";
  }
}

int print_out_tabs()
{
  cout << "Output matched objects DB table: "<<t.otab.out_db<<"."<<t.otab.x<<endl
       << "Output unmatched objects DB table: "<<t.otab.out_db<<"."<<t.otab.nx<<endl
       << "Output external objects DB table: "<<t.otab.out_db<<"."<<t.otab.ext << endl;
}


int crea_out_tabs(const int my_cID, bool drop_prematch, bool verbose)
{
  set_out_tabs(db.my_user, db.cat1, db.cat2);

if (verbose)
  print_out_tabs();

  int i;
  string qry_str_i, qry_str;
  std::vector<bool> tab_exists(3, false);


    qry_str_i = string("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='") + t.otab.out_db +"' AND table_name='";
    for (i=0; i<3; i++) {
      if (i == 0) 
        qry_str = qry_str_i + t.otab.x +"'";
      else if (i == 1)
        qry_str = qry_str_i + t.otab.nx +"'";
      else
        qry_str = qry_str_i + t.otab.ext +"'";

if (verbose)
cout <<qry_str<<endl;
      if (!db_query(my_cID, qry_str.c_str())) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit(1);
      }
      tab_exists[i] = atoi(db_data(my_cID, 0, 0));
    }



}



// Get DIF.tbl rows for a given DB and table. Optionally only return info for HTM indices.

//int dif_cat_info(const int cID, string dbname, string catname, vector <DIFtbl_st> *catinfo, bool htm_only=false)
int dif_cat_info(const int cID, string dbname, string catname, bool htm_only=false)
{
  bool do_close = false;
  int cid = cID;
  string d = " db like '%'", t = "name like '%'";

if (cID < 0) {
  cid = 0;
  if (!db_init(cid)) {
    cerr << PROGNAME <<"Can't set CONNECT_TIMEOUT for MySQL connection."<< endl;
    exit(1);
  }
  if (!db_connect(cid, db.my_host.c_str(), db.my_user.c_str(), db.my_passw.c_str(), db.my_db1.c_str())) {
    cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
    exit(1);
  }
  do_close = true;
}

  if (dbname.length() > 0) d = dbname;
  //string difqry = "SELECT id_type,id_opt,param,Ra_field,Dec_field from DIF.tbl WHERE db='"+ dbname + "' AND name='"+ catname;

  string difqry = "SELECT db,name,id_type,id_opt,param,Ra_field,Dec_field from DIF.tbl WHERE ";
  //if (dbname.length() > 0 || catname.length() > 0) difqry += "WHERE ";
  if (dbname.length() > 0) d = " db='"+ dbname +"'";
  if (catname.length() > 0) t = " name='"+ catname +"'";

  difqry += d +" AND "+ t;

  if (htm_only)
    difqry += " AND id_type=1";
  //else
    //difqry += "'";
  cout << difqry << endl;

  if (!db_query(cid, difqry.c_str())) {
    cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
    exit(1);
  }

  if (db_num_rows(cid) == 0) {
    cerr << PROGNAME <<": "<< dbname <<"."<< catname;
    if (htm_only)
      cerr << PROGNAME << ": no HTM indexing found."<< endl;
    else
      cerr << PROGNAME << ": no DIF indexing found."<< endl;
    exit(1);
  }

  DIFtbl_st e;
  for (int i=0; i<db_num_rows(cid); i++) {
     e.db        = db_data(cid, i, 0);
     e.name      = db_data(cid, i, 1);
     e.id_type   = atoi(db_data(cid, i, 2)) == 1 ? HTM_TYPE : HPX_TYPE; 
     e.id_opt    = atoi(db_data(cid, i, 3)) == 0 ? RING : NESTED;
     e.param     = atoi(db_data(cid, i, 4));
     e.Ra_field  = db_data(cid, i, 5);
     e.Dec_field = db_data(cid, i, 6);
     //catinfo->push_back(e);
     catinfo.push_back(e);
  }
  db_free_result(cid);
  if (do_close) db_close(cid);
}


int set_dif_params(bool use_hpx=false)
{
  db.my_host = "localhost";
  db.my_user = "generic";
  db.my_passw = "password";
  db.my_db1 = "test";
  db.my_db2 = "test";
  db.cat1 = "";
  db.cat2 = "";

// ID col names have additinal depth/order given in input
  if (use_hpx) {
   t.id_coln1 = "healpID_nest_"; //+ t.orde1;
   t.id_coln2 = "healpID_nest_"; //+ t.orde2;
  } else {
    t.id_coln1 = "htmID_"; //+ t.orde1;
    t.id_coln2 = "htmID_"; //+ t.orde2;
  }

  t.ra_coln = "RAmas";
  t.de_coln = "DECmas";
  t.deg_fac = "/3.6e6";

  if (t.use_master_id1 || t.use_master_id2) {
    t.mt_coln = "MASTERhpx6";
    t.rn_coln = "runningnumber";
  } else {
    t.mt_coln = "none";
    t.rn_coln = "none";
  }
  t.id_sqltype = dif_sqltype(t.orde1);  // Unused
}



extern "C" {
  double deg_ra(char *ra_str);
  double deg_dec(char *dec_str);
}

long spherematch2(long npoints1, double *ra1, double *dec1,
                 long npoints2, double *ra2, double *dec2,
                 double matchlength, double minchunksize,
                 vector<long> &match1, vector<long>&match2, vector<double>&distance12, long *nmatch);
long spherematch2_mm(long npoints1, double *ra1, double *dec1,
                 long npoints2, double *ra2, double *dec2,
                 double matchlength, double minchunksize,
                 vector<long> &match1, vector<long>&match2, vector<double>&distance12, long *nmatch);

 

void
usage() {

  cout << PROGNAME << "  " << VERID << endl<<endl
       << "Match an input catalogue Cat_in against a reference one, Cat_ref,\n and optionally produce an output Cat_out. All reside in DIF indexed MySQL tables.\n\n"
       << "Usage: " << PROGNAME << " [OPTIONS] [Pixel_ID] [Start_Pixel_ID End_Pixel_ID]\n"
       << "Where OPTIONS are:\n"
       << "  -h: print this help\n"
       << "  -H: print help on available DIF indexed catalogues\n"
       << "  -a: archive (append if it exists) matched objects into a DB table (see -t)\n"
       << "  -A: like -a but (if exists) first remove DB table\n"
       //<< "  -F: full scan of Cat1 pixels\n"
       << "  -I: use Turin specific unique ID (= MASTERhpx6+runningnumber) of ref cat. in out table (+ Cat_in coords)\n"
       << "  -K: if -I given, then also read and display the corresponding MASTERhpx6+runningnumber of Cat_in\n"
       << "  -l: list on screen selected and matched objects\n"
       << "  -m: input (see -S) and returned separation are arcmin (def. arcsec)\n"
       << "  -M: accept multiple matches within given max separation (see -S) (def. 1 match only)\n"
       << "  -q: do not list on screen matched objects\n"
       << "  -Q: only list on screen matched objects outside the pixel\n"
       << "  -v: be verbose\n"
       << "  -c ID1 ID2 Ra Dec: column names for HTM/HPX IDs and Coords (def. htmID_ + Depth1/2, RAmas, DECmas)\n"
       << "  -d DBnane: use 'DBnane' as input database (def. test)\n"
       << "  -i catID: set 'catID' as user defined catalogue ID for 'Cat2' (e.g. sky2000 = 1, def. 0)\n"
       << "  -o OutDB: use 'OutDB' as output database (def. test, implies -a)\n"
       << "  -p Password: MySQL user password is 'Password' (def. password)\n"
       << "  -s Server: send query to DB server 'Server' (def. localhost)\n"
       << "  -t Table: matched objects table will be 'Table' (def. pix_Xmatch_User_Cat1_Cat2)\n"
       << "  -u User: MySQL user name is 'User' (def. generic)\n"
       << "  -x Cat_in Cat_ref: cross match catalogue 'Cat_in' against reference 'Cat_ref'\n"
       << "  -D Depth1 Depth2: HTM pixelization depths to use are 'Depth1' and 'Depth2' (def. 6 15 : excludes -O)\n"
       << "  [-O Order1 Order2]: HEALPix pixelization order (NESTED) to use are 'Order1' and 'Order2' (def. 6 14 : excludes -D)\n"
       << "  -R nRows: process 'nRows' per INSERT query to increase speed (def. 300 : require -A or -a)\n"
       << "  -S Sep: max separation defining a positive match is 'Sep' arcsec (def. 1 : see -m)\n"
       << "\n Note:\n"
       << "   Option -O is not implemented yet.\n"
       << "   Options -D and -O apply to both catalogues.\n"
       << "   If -O not given then assume both catalogues are HTM indexed.\n"
       << "\nDefault server, DB, user, password:  "<< db.my_host <<bl<< t.otab.out_db <<bl<< db.my_user <<bl<< db.my_passw
       << endl<<endl;
  exit(0);
}


int main(int argc, char *argv[]) {
  unsigned short kwds=0, use_hpx=0, do_list_match=1, do_list_external=1, do_list_all=0, do_list_cats=0,
                 use_arcmin=0, save_match=0, drop_prematch=0, out_tab_ex=0, idb_set=0, odb_set=0,
                 multi_match=0, one_pix=0, range_pix=0, full_scan=1, verbose=0, input_col_names=0;
  static const int my_cID = 0;  // MySQL connection ID
  int ret=0, insert_Nrows=300;
  double minchunksize, min_dist=-1.,
         matchlength=1./3600;  // def. match dist.= 1''
  unsigned long nmatchret, nmatchext; // nmatchmax
  long nmatch, totals_read=0, totals_match=0, totals_ext=0, totals_unmatch=0;

  string ans, db_view1, db_view2, qry_str,
         sep_unit="arcsec", origID="0";
  char c;

// Set default params
set_dif_params();

t.otab.out_db = "test";
t.otab.x = "";
t.use_master_id1 = false;
t.use_master_id2 = false;
t.orde1 = "6";
t.orde2 = "15";

/* Keywords section */
  while (--argc > 0 && (*++argv)[0] == '-')
  {
    kwds=1;
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
        case 'I':
          t.use_master_id2 = true;
          t.mt_coln = "MASTERhpx6";
          t.rn_coln = "runningnumber";
          break;
        case 'K':
          t.use_master_id1 = true;
          break;
        case 'l':
          do_list_all = 1;
          break;
        case 'm':
          use_arcmin = 1;
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
        case 'c':  // Pass column names for 2 IDs, Ra and Dec
          if (argc < 5) usage();
          t.id_coln1 = string(*++argv);
          --argc;
          t.id_coln2 = string(*++argv);
          --argc;
          t.ra_coln = string(*++argv);
          --argc;
          t.de_coln = string(*++argv);
          --argc;
          input_col_names = 1;
          kwds=0;
          break;
        case 'd':
          if (argc < 2) usage();
          db.my_db1 = string(*++argv);
          db.my_db2 = db.my_db1;
          --argc;
          idb_set = 1;
          kwds=0;
          break;
        case 'D':
          if (argc < 3) usage();
          t.orde1 = string(*++argv);
          --argc;
          t.orde2 = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'i':
          if (argc < 2) usage();
          origID = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'o':
          if (argc < 2) usage();
          t.otab.out_db = string(*++argv);
          --argc;
          save_match = 1;
          odb_set = 1;
          kwds=0;
          break;
        case 'O':
          if (argc < 3) usage();
          t.orde1 = string(*++argv);
          --argc;
          t.orde2 = string(*++argv);
          --argc;
          use_hpx = 1;
          kwds=0;
          break;
        case 'p':
          if (argc < 2) usage();
          db.my_passw = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'R':
          if (argc < 2) usage();
          insert_Nrows = atoi(*++argv);
          --argc;
          kwds=0;
          break;
        case 's':
          if (argc < 2) usage();
          db.my_host = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'S':
          if (argc < 2) usage();
          sscanf(*++argv,"%lf",&min_dist);
          --argc;
          kwds=0;
          break;
        case 't':
          if (argc < 2) usage();
          t.otab.x = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'u':
          if (argc < 2) usage();
          db.my_user = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'x':
          if (argc < 3) usage();
          db.cat1 = string(*++argv);
          --argc;
          db.cat2 = string(*++argv);
          --argc;
          kwds=0;
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
    dif_cat_info(-1, db.my_db1, db.cat1, true);
    cout<< db.my_db1 <<"."<< (db.cat1.length()>0 ? db.cat1 : "*") <<" info in DIF.tbl: \n";
    for (int i=0; i<catinfo.size(); i++)
      //cout <<" id_type: "<< catinfo[i].id_type <<" id_opt: "<< catinfo[i].id_opt <<" Depth/order: "<< catinfo[i].param
      cout <<" db: "<< catinfo[i].db <<" name: "<< catinfo[i].name <<"  Depth/order: "<< catinfo[i].param
           <<"  Ra_field: '"<< catinfo[i].Ra_field <<"'  Dec_field: '"<< catinfo[i].Dec_field <<"'\n";
    exit (0);
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



  unsigned long in_entries1, inr1, nr1=0,nr2=0,i,j,ij, iin_id, n, n_unmatched, npix=1,
                *id_list, *id1=NULL, *mt1=NULL, *mt2=NULL, *rn1=NULL, *rn2=NULL;
  long long l_ra=0, l_de=0;
  double *ra1=NULL, *de1=NULL, *ra2=NULL, *de2=NULL, rac=0., decc=0.;
  char **row;
  string in_id;
  bool tab_swapped;

// If an argument passed, it is the pixel ID: only process it
  if (argc == 1) { 
    one_pix = 1;
    full_scan = 0;  // disable full table scan
    in_id = string(*argv);
  }
// If two arguments passed, then build list in that range
  else if (argc == 2) { 
    range_pix = 1;
    full_scan = 0;  // disable full table scan
    iin_id = atoi(*argv++);
    in_id = itos(iin_id);
    unsigned long iin_id2 = atoi(*argv);
    npix = iin_id2 - iin_id + 1;
    if (!(id_list = (unsigned long *) malloc(npix * sizeof(unsigned long)))) {
      cerr << PROGNAME <<": id_list: error allocating memory.\n";
      exit(-1);
    }
    for (i=0; i<npix; i++)
      id_list[i] = iin_id + i;
  }


  if (db.my_passw.empty()) {
    cout << "Enter " << db.my_user << " password: ";
    getline(cin, db.my_passw);
  }


//  DBConn db;
//  db.connect("generic", "password", "MyCats");
//  Query qry(&db);
//  qry.query("SELECT RAcs/3.6e5, DECcs/3.6e5 FROM GSC22_htm_6 where DIF_Circle("+
//    coords +")", true);
//  nr1 = qry.nRows();


/* Connect to the DB */
  if (!db_init(my_cID))
  {
    cerr << PROGNAME <<"Can't set CONNECT_TIMEOUT for MySQL connection.\n";
    exit(1);
  }

  if (!db_connect(my_cID, db.my_host.c_str(), db.my_user.c_str(), db.my_passw.c_str(), db.my_db1.c_str())) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit(1);
  }

// First get the total number of entries in the table to be matched
  qry_str = string("SELECT count(*) FROM ")+ db.cat1;

if (verbose)
cout<<"Query: "<< qry_str<<endl;

  if (!db_query(my_cID, qry_str.c_str())) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit(1);
  }

  in_entries1 = atoi(db_data(my_cID, 0, 0));

  const int iwidth=6, dwidth=12;  // printed values widths
  int idw1=0, idw2=0;

  cout << setiosflags(ios::fixed);
  cout << setprecision(7);

  string qry_ini, difqry_ini1,
         ra_fld1=t.ra_coln + t.deg_fac, ra_fld2=t.ra_coln + t.deg_fac,
         de_fld1=t.de_coln + t.deg_fac, de_fld2=t.de_coln + t.deg_fac; 


// NOTE: HEALPix pixelization X-match not yet implemented
  if (use_hpx) {
    if (! input_col_names) {
      t.id_coln1 += t.orde1;
      t.id_coln2 += t.orde2;
    }
    idw1 = dif_hpxid_maxw(t.orde1);

    db_view1 = db.cat1 +"_healp_nest_"+ t.orde2;
  } else {
    if (! input_col_names) {
// HTM IDs column name
      t.id_coln1 += t.orde1;
      t.id_coln2 += t.orde2;
    }
    idw1 = dif_htmid_maxw(t.orde1);

    db_view1 = db.cat1 +"_htm_"+ t.orde2;
  }

  idw1 = MAX(idw1, t.id_coln1.length());

// Preliminary check in DIF.tbl to see if the requested indices are available.
// Should also check for reference catalogue...
  //dif_cat_info(my_cID, my_db1, cat1, &catinfo);
  dif_cat_info(my_cID, db.my_db1, db.cat1);

  int nd=0;
if (verbose)
  cout<< db.my_db1 <<"."<< db.cat1 <<" info in DIF.tbl: \n";
  for (i=0; i<catinfo.size(); i++) {
    if (catinfo[i].param == stoi(t.orde1) || catinfo[i].param == stoi(t.orde2))
      nd++;
if (verbose)
  cout <<" id_type: "<< catinfo[i].id_type <<" id_opt: "<< catinfo[i].id_opt <<" Depth/order: "<< catinfo[i].param
       <<"  Ra_field: '"<< catinfo[i].Ra_field <<"'  Dec_field: '"<< catinfo[i].Dec_field <<"'\n";
  }

  if (nd != 2)
    cout << PROGNAME <<": Warning: the 2 requested depths/orders where not found in DIF.tbl for "<< db.my_db1 <<"."<< db.cat1 << endl;


    if (save_match)
      crea_out_tabs(my_cID, drop_prematch, verbose);


// First get the total number of external entries
  qry_str = string("SELECT count(*) FROM ")+ t.otab.out_db +"."+ t.otab.ext;

if (verbose)
cout<<"Query: "<< qry_str<<endl;

  if (!db_query(my_cID, qry_str.c_str())) {
    cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
    exit(1);
  }

  totals_ext = atoi(db_data(my_cID, 0, 0));



// Now we can clean up the unmatched table (only for full scan)
// Always first drop the table
  if (save_match) {
    if (full_scan) {
      qry_str = string("DROP TABLE IF EXISTS ") + t.otab.out_db +"."+ t.otab.nxc;
cout<<qry_str<<endl;
      ret = db_query(my_cID, qry_str.c_str());
      if (!ret) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit(1);
      }
    }

    string qry_str_x, qry_str_xe, qry_str_nx, qry_str_nxc, qry_str_e;
    qry_str_x  = string("ALTER TABLE ")+ t.otab.out_db +"."+ t.otab.x +" ADD KEY ("+ t.id_coln1 +co+ t.ra_coln +")";
    qry_str_nx = string("ALTER TABLE ")+ t.otab.out_db +"."+ t.otab.nx +" ADD KEY ("+ t.id_coln1 +co+ t.ra_coln +")";

// Now create the clean table from a join of the unmatched and external tables, or simply rename the unmatched one
    if (totals_ext > 0) {
      qry_str_e = string("ALTER TABLE ")+ t.otab.out_db +"."+ t.otab.ext +" ADD KEY ("+ t.id_coln1 +co+ t.ra_coln +")";

      qry_str = string("CREATE TABLE ")+ t.otab.out_db +"."+ t.otab.nxc +" SELECT u.* FROM "+
                t.otab.out_db +"."+ t.otab.nx +" AS u LEFT JOIN "+
                t.otab.out_db +"."+ t.otab.ext +" AS e ON u."+ t.id_coln1 +"=e."+t.id_coln1 +
                " AND u."+ t.ra_coln +"=e."+ t.ra_coln +" AND u."+ t.de_coln +"=e."+ t.de_coln +" WHERE e."+t.id_coln1 +" IS NULL";
//select count(htmID_6) as n, ref_htmID_6,htmID_6,RAmas,DECmas,Sep from (SELECT u.ref_htmID_6,u.htmID_6,u.RAmas,u.DECmas,u.Sep FROM test.p1 AS u INNER JOIN test.pe AS e ON u.htmID_6=e.htmID_6 AND u.RAmas=e.RAmas AND u.DECmas=e.DECmas group by u.ref_htmID_6,u.htmID_6 order by u.htmID_6,u.Sep asc) as t group by htmID_6 having n>1;
/*
      qry_str_xe = string("SELECT COUNT(")+ t.id_coln1 +") AS n, ref_"+ t.id_coln1 +co+ t.id_coln1 +co+ t.ra_coln +co+ t.de_coln +
                ", Sep FROM(SELECT u.ref_"+ t.id_coln1 +",u."+ t.id_coln1 +",u."+ t.ra_coln +",u."+ t.de_coln +",u.Sep FROM "+
                t.otab.out_db +"."+ t.otab.x +" AS u INNER JOIN "+ t.otab.out_db +"."+ t.otab.ext +" AS e ON u."+
                t.id_coln1 +"=e."+t.id_coln1 +
                " AND u."+ t.ra_coln +"=e."+ t.ra_coln +" AND u."+ t.de_coln +"=e."+ t.de_coln +
                " GROUP BY ref_"+ t.id_coln1 +",u."+ t.id_coln1 +" ORDER BY u."+ t.id_coln1 +",u.Sep ASC) AS t GROUP BY "+
                t.id_coln1 +co+ t.ra_coln +" HAVING n>1";
*/
//SELECT count(e.htmID_6) as n, e.* FROM test.temptable2 AS u LEFT JOIN test.temptable2_external AS e ON u.htmID_6=e.htmID_6 AND u.RAmas=e.RAmas AND u.DECmas=e.DECmas WHERE e.htmID_6 IS not NULL group by e.htmID_6,e.ramas,e.decmas having n>1;
//SELECT count(u.htmID_6) as n, u.ref_htmID_6,u.htmID_6,u.RAmas,u.DECmas,u.Sep FROM test.temptable2 AS u LEFT JOIN test.temptable2_external AS e ON u.htmID_6=e.htmID_6 AND u.RAmas=e.RAmas AND u.DECmas=e.DECmas WHERE e.htmID_6 IS not NULL group by e.htmID_6,e.ramas,e.decmas having n>1;

      qry_str_xe = string("SELECT COUNT(u.")+ t.id_coln1 +") AS n, u.ref_"+ t.id_coln1 +",u."+ t.id_coln1 +",u."+ t.ra_coln +",u."+ t.de_coln +
                ", u.Sep FROM "+ t.otab.out_db +"."+ t.otab.x +" AS u LEFT JOIN "+ t.otab.out_db +"."+ t.otab.ext +" AS e ON u."+
                t.id_coln1 +"=e."+ t.id_coln1 +
                " AND u."+ t.ra_coln +"=e."+ t.ra_coln +" AND u."+ t.de_coln +"=e."+ t.de_coln +
                " WHERE e."+ t.id_coln1 +" IS NOT NULL GROUP BY e."+ t.id_coln1 +",e."+ t.ra_coln +",e."+ t.de_coln +
                " HAVING n>1";

    } else {
      if (drop_prematch) {
        qry_str_e = string("DROP TABLE IF EXISTS ") + t.otab.out_db +"."+ t.otab.ext;
        ret = db_query(my_cID, qry_str_e.c_str());
        if (!ret) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit(1);
        }
      }
      qry_str_e = string("-- No object in the external pixels table");
      qry_str_xe = string("-- No cleaning required for matched and unmatched tables");
      qry_str = string("RENAME TABLE ")+ t.otab.out_db +"."+ t.otab.nx +" TO "+ t.otab.out_db +"."+ t.otab.nxc;
    }
//if (verbose)
cout <<qry_str_x<<endl
     <<qry_str_nx<<endl
     <<qry_str<<endl
     <<qry_str_e<<endl
     <<qry_str_xe<<endl;

    if (full_scan) {
      cout <<"\nAdding position key to matched objects table...\n";
      if (!db_query(my_cID, qry_str_x.c_str())) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit(1);
      }
      cout <<"Adding position key to unmatched objects table...\n";
      if (!db_query(my_cID, qry_str_nx.c_str())) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit(1);
      }

      if (totals_ext > 0) {

        cout <<"Adding position key to externally matched objects table...\n";
        if (!db_query(my_cID, qry_str_e.c_str())) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit(1);
        }


        cout <<"Cleaning matched objects table for multiple externally matched objects...\n";
        if (!db_query(my_cID, qry_str_xe.c_str())) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit(1);
        }
// Loop on multiple entries
        long ndup = db_num_rows(my_cID);
cout<<"Number of duplicates to clean: "<< ndup <<endl;
        int cid = 1;

        if (ndup > 0) {
          if (!db_init(cid)) {
            cerr << PROGNAME <<"Can't set CONNECT_TIMEOUT for MySQL connection."<< endl;
            exit(1);
          }
          if (!db_connect(cid, db.my_host.c_str(), db.my_user.c_str(), db.my_passw.c_str(), db.my_db1.c_str())) {
            cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
            exit(1);
          }
cout<<"P2"<<endl;

          string re_id, ras, des;
          for (i=0; i<ndup; i++) {
//cout<<i<<"  "<<db_data(my_cID, i, 0)<<" "<<db_data(my_cID, i, 1)<<endl;
            re_id = db_data(my_cID, i, 1);
cout<<re_id<<endl;
            in_id = db_data(my_cID, i, 2);
cout<<in_id<<endl;
            ras = db_data(my_cID, i, 3);
cout<<ras<<endl;
            des = db_data(my_cID, i, 4);
cout<<des<<endl;
            qry_str_x = string("SELECT ref_")+ t.id_coln1 +co+ t.id_coln1 +co+ t.ra_coln +co+ t.de_coln +
                     " FROM "+ t.otab.out_db +"."+ t.otab.x +" WHERE "+ t.id_coln1 +"="+ in_id +" AND "+
                     t.ra_coln +"="+ ras +" AND "+ t.de_coln +"="+ des +" order by Sep ASC limit 1";
cout<<qry_str_x<<endl;
            if (!db_query(cid, qry_str_x.c_str())) {
              cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
              exit(1);
            }
            re_id = db_data(cid, 0, 0);
            in_id = db_data(cid, 0, 1);

            qry_str_x = string("DELETE FROM ")+ t.otab.out_db +"."+ t.otab.x +" WHERE "+ t.id_coln1 +"="+ in_id +
                      " AND "+ t.ra_coln +"="+ ras +" AND "+ t.de_coln +"="+ des +" AND ref_"+ t.id_coln1 +"!= "+ re_id;
// DELETE FROM temptable WHERE htmID_6=53993 and RAmas=850976111 and DECmas=14497050 and ref_htmID_6 != 53993;
//if (verbose)
cout<<qry_str_x<<endl;

            if (!db_query(cid, qry_str_x.c_str())) {
              cerr << PROGNAME <<": DB error: "<< db_error(cid) << endl;
              exit(1);
            }
cout<<"done."<<endl;
          }
          db_free_result(cid);
          db_close(cid);

        } else
          cout <<"No duplicated entry found in matched objects table\n";

        db_free_result(my_cID);

        cout <<"Merging unmatched and externally matched objects tables...\n";

      } else {
        cout <<"Renaming unmatched objects table...\n";
      }

      ret = db_query(my_cID, qry_str.c_str());
      if (!ret) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit(1);
      }

      cout <<"Adding position key to cleaned unmatched objects table...\n";
      qry_str_nxc = string("ALTER TABLE ")+ t.otab.out_db +"."+ t.otab.nxc +" ADD KEY ("+ t.id_coln1 +co+ t.ra_coln +")";
      if (!db_query(my_cID, qry_str_nxc.c_str())) {
        cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
        exit(1);
      }

    } else {
      cout <<"\nYou did not use a single full catalogues match.\n"
             "At your convenience you need to merge manually unmatched and externally matched objects tables.\n"
             "For the current run the queries could be:\n\n"
           << qry_str_nx <<";\n" 
           << qry_str_e <<";\n" 
           << qry_str <<";\n"; 
      cout <<"\nYou can also consider indexing the matched table:\n"
           << qry_str_x <<";\n"; 
    }
  }  // save_match

  if (save_match) {
    cout <<"\nYou can now use 'dif' on the output tables to create views and perform sky queries.\n";
    if (full_scan)
      cout <<"Because all the tables are already indexed, you should use the option '--views-only'.\n";
    cout <<"\nMatched objects table: "<< t.otab.out_db <<"."<< t.otab.x<<endl;

    if (full_scan) {
      cout <<"Unmatched objects table: "<< t.otab.out_db <<"."<< t.otab.nxc << endl;
      if (totals_ext > 0) {
        qry_str = string("SELECT COUNT(*) FROM ")+ t.otab.out_db +"."+ t.otab.nxc;
        if (!db_query(my_cID, qry_str.c_str())) {
          cerr << PROGNAME <<": DB error: "<< db_error(my_cID) << endl;
          exit(1);
        }
        long nxc = atoi(db_data(my_cID, 0, 0));
        cout <<"\nNumber of unmatched objects after table clean: "<< nxc << endl;
        cout <<"\nExtra tables (you can remove):\n"
             <<"Uncleaned unmatched objects tab: "<< t.otab.out_db <<"."<< t.otab.nx
             <<"\nNeighbours matched objects tab: "<< t.otab.out_db <<"."<< t.otab.ext;
      }
    }
    cout << endl;
  }

  db_close(my_cID);
  return (0);
}
