/*
  A simple implementation of the "spherematch" algorithm querying DIF
  indexed catalogues.

  LN @ IASF-INAF, Sep. 2010                         Last changed: 22/06/2020 
*/

using namespace std;

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <string.h>

#include <vector>

#include "my_stmt_db.h"

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )

static string bl = " ";
static string co = ",";

// Progran name
const char PROGNAME[] = "myXmatch";

// Version ID string
static string VERID="Ver 1.0c, 22-06-2020, LN@INAF-OAS";


/* Unused
  MYSQL_ROW record;

void row2num(char **row, double *c1, double *c2, unsigned long *c3) {
    // *c1 = atof(row[0]);
    // *c2 = atof(row[1]);
    // *c3 = atoi(row[2]);
   sscanf(row[0],"%lf", c1);
   sscanf(row[1],"%lf", c2);
   sscanf(row[2],"%d",  c3);
}
*/

// Global variables
char my_host[32]="localhost", my_user[32]="generic", my_passw[32]="password",
     my_db1[32]="test", my_db2[32]="test";

string itos(int i)
{
  char buf[20];
  sprintf(buf, "%d", i);
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

extern "C" {
  double deg_ra(char *ra_str);
  double deg_dec(char *dec_str);
}

long spherematch2(unsigned long npoints1, double *ra1, double *dec1,
                 unsigned long npoints2, double *ra2, double *dec2,
                 double matchlength, double minchunksize,
                 vector<long> &match1, vector<long>&match2, vector<float>&distance12, unsigned long *nmatch);
long spherematch2_mm(unsigned long npoints1, double *ra1, double *dec1,
                 unsigned long npoints2, double *ra2, double *dec2,
                 double matchlength, double minchunksize,
                 vector<long> &match1, vector<long>&match2, vector<float>&distance12, unsigned long *nmatch);


 
int dif_cats_list(const int cID)
{

/* Connect to the DB */
  if (!db_init(cID)) {
    cerr << PROGNAME <<"Can't set CONNECT_TIMEOUT for MySQL connection."<< endl;
    exit(1);
  }
  if (!db_connect(cID, my_host, my_user, my_passw, my_db1)) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }

  string qry_str = "SELECT distinct db,name from DIF.tbl";
  if ( ! db_query(cID, qry_str.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    return (0);
  }

  int n_cats = db_num_rows(cID);
  db_free_result(cID);
  if (n_cats == 0) {
    cerr << PROGNAME << ": no DIF indexed catalogue found!"<< endl;
    return (0);
  }

  unsigned int i, j;

  //for (i=0; i<n_cats; i++) {

  qry_str = "SELECT db,name,id_type,id_opt,param from DIF.tbl";

  if ( ! db_query(cID, qry_str.c_str()) ) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    return (0);
  }

  if (db_num_rows(cID) == 0) {
    cerr << PROGNAME << ": no DIF indexed catalogue found!"<< endl;
    return (0);
  }

  unsigned int nf = db_num_fields(cID);
//  unsigned int fw[nf];
//  for (j=0; j<nf; j++) {
//    fw[j] = db_fieldlength(cID, j);
//    cout << db_fieldname(cID, j) <<"\t";
//  }
//  cout << endl;
    cout << "DBname\tTABname\n\t\tHTM/HPX (1/2)\tNested scheme?\tHTM/HPX depth/order"<< endl;

    cout << db_data(cID, 0, 0) <<"\t"<< db_data(cID, 0, 1) <<"\n\t\t"
         << db_data(cID, 0, 2) <<"\t"<< db_data(cID, 0, 3) <<"\t"
         << db_data(cID, 0, 4)
         << endl;
  for (i=1; i<db_num_rows(cID); i++) {
//    for (j=0; j<nf; j++)
    if ( strcmp(db_data(cID, i-1, 0),db_data(cID, i, 0)) ||
         strcmp(db_data(cID, i-1, 1),db_data(cID, i, 1)) )
      cout <<endl<< db_data(cID, i, 0) <<"\t"<< db_data(cID, i, 1) <<"\n";

      cout << "\t\t"
           << db_data(cID, i, 2) <<"\t"<< db_data(cID, i, 3) <<"\t"
           << db_data(cID, i, 4)
           << endl;
  }

  db_free_result(cID);
  db_close(cID);
  return (n_cats);
}


void
usage() {

  cout << PROGNAME << "  " << VERID << "\n" << endl
       << "Usage: " << PROGNAME << " [OPTIONS] RAcenter DECcenter Radius\n"
       << "       " << PROGNAME << " [OPTIONS] RAcenter DECcenter Side\n"
       << "       " << PROGNAME << " [OPTIONS] RAcenter DECcenter Side1 Side2\n\n"
       << "Where OPTIONS are:\n"
       << "  -h: print this help\n"
       << "  -H: print help on available DIF indexed catalogues\n"
       << "  -a: archive (append if it exists) matched objects into a DB table (see -t)\n"
       << "  -A: like -a but (if exists) first remove DB table\n"
       << "  -l: list on screen selected and matched objects\n"
       << "  -q: do not list on screen matched objects\n"
       << "  -m: input (-S) and returned separation are arcmin (def. arcsec)\n"
       << "  -M: accept multiple matches within given max separation (-S) (def. 1 match only)\n"
       << "  -r: input region is a rectangle (or square): input center and two (one for square) side length (def. circle)\n"
       << "  -d DBnane: use 'DBnane' as input database (def. test)\n"
       << "  -o OutDB: use 'OutDB' as output database (def. test, implies -a)\n"
       << "  -p Password: MySQL user password is 'Password' (def. password)\n"
       << "  -s Server: send query to DB server 'Server' (def. localhost)\n"
       << "  -t Table: matched objects table will be 'Table' (def. Xmatch_User_Cat1_Cat2)\n"
       << "  -u User: MySQL user name is 'User' (def. generic)\n"
       << "  -x Cat1 Cat2: cross match catalogue 'Cat1' and 'Cat2'\n"
       << "  -D Depth: HTM pixelization depth to use is 'Depth' (def. all : excludes -O)\n"
       << "  -O Order: HEALPix pixelization order (NESTED) to use is 'Order' (if not present use smallest avail.: excludes -D)\n"
       << "  -S Sep: max separation defining a positive match is 'Sep' arcsec (def. 1 : see -m)\n"
       << "\n Note:\n"
       << "   RA, DEC in fractional degrees or string format, Radius/Side in arcmin.\n"
       << "   Options -D and -O apply to both catalogues.\n"
       << "   If -O not given then assume both catalogues are HTM indexed.\n"
       << "\nDefault server, DB, user, password:  localhost, test, generic, password\n"
       << endl;
  exit(0);
}


int main(int argc, char *argv[]) {
  unsigned short kwds=0, use_hpx=0, do_list_match=1, do_list_all=0, do_list_cats=0,
                 is_rect=0, use_arcmin=0, save_match=0, drop_prematch=0,
                 out_tab_ex=0, db_set=0, multi_match=0;
  static const int cID = 0;  // MySQL connection ID
  int ret=0;
  double min_dist=-1.;

  string ans, db_view1, db_view2, qry_str, my_tab,
         sep_unit="arcsec", dif_func="DIF_Circle(";
  char c, db_cat1[40]="", db_cat2[40]="",
       my_depth[3]="", my_order[3]="", out_db[32]="test", root_tablen[]="Xmatch_";


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
        case 'q':
          do_list_match = 0;
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
        case 'r':
          is_rect = 1;
          dif_func = "DIF_Rect(";
          break;
        case 'd':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_db1);
          strcpy(my_db2,my_db1);
          --argc;
          db_set = 1;
          kwds=0;
          break;
        case 'o':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",out_db);
          save_match = 1;
          --argc;
          kwds=0;
          break;
        case 'p':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_passw);
          --argc;
          kwds=0;
          break;
        case 's':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_host);
          --argc;
          kwds=0;
          break;
        case 't':
          if (argc < 2) usage();
          my_tab = string(*++argv);
          --argc;
          kwds=0;
          break;
        case 'u':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_user);
          --argc;
          kwds=0;
          break;
        case 'x':
          if (argc < 3) usage();
          sscanf(*++argv,"%s",db_cat1);
          --argc;
          sscanf(*++argv,"%s",db_cat2);
          --argc;
          kwds=0;
          break;
        case 'D':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_depth);
          --argc;
          kwds=0;
          break;
        case 'O':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_order);
          --argc;
          use_hpx = 1;
          kwds=0;
          break;
        case 'S':
          if (argc < 2) usage();
          sscanf(*++argv,"%lf",&min_dist);
          --argc;
          kwds=0;
          break;
        default:
          cerr << "Illegal option `"<< c << "'.\n\n";
          usage();
      }
    }
  }

  if (do_list_cats) {
    cout <<"\nFound "<< dif_cats_list(cID) <<" catalogues."<<endl;
    exit(0);
  }

  if (argc < 3)
    usage();

  if (db_cat1[0] == '\0' || db_cat2[0] == '\0')
    usage();

  if (!db_set) {
    //size_t found = db_cat1.find(".");
    //if (found != string::npos)
    //strcpy(my_db1, db_cat1.substr(found).c_str());

    char * chr_dot = strchr(db_cat1,'.');
    if (chr_dot != NULL) {
      strcpy(my_db1, db_cat1);
      chr_dot = strchr(my_db1,'.');
      *chr_dot = 0;
chr_dot = strchr(db_cat1,'.');
strcpy(db_cat1,++chr_dot);
    }

    chr_dot = strchr(db_cat2,'.');
    if (chr_dot != NULL) {
      strcpy(my_db2, db_cat2);
      chr_dot = strchr(my_db2,'.');
      *chr_dot = 0;
chr_dot = strchr(db_cat2,'.');
strcpy(db_cat2,++chr_dot);
    }
  }


  //if (my_host[0] != '\0')
  //  strcpy(hostn,my_host);

  //if (my_db[0] != '\0')
  //  strcpy(dbname,my_db);

  //if (my_user[0] != '\0')
  //  strcpy(usern,my_user);

  //if (my_passw[0] != '\0')
  //  strcpy(passwd,my_passw);

  unsigned int nr1=0,nr2=0,i=0;
  unsigned long *id1, *id2;
  double *ra1, *de1, *ra2, *de2;
  double rac=0., decc=0.;
  char **row;
  string coords;

// Decode coordinates: RA (hh mm ss) Dec. (+/-dd mm ss) Rad [Side1 Side2]
  if (argc == 4 || argc == 8) { 
    is_rect = 1;
    dif_func = "DIF_Rect(";
  }

  if (argc > 6) {
    unsigned short is_neg;
    char dec_deg[4];
    rac = atof(*argv);
    rac += atof(*argv++)/60.;
    rac += atof(*argv++)/3600.;
    rac *= 15.;
    //rac = (atof(*argv)+atof(*argv++)/60.+atof(*argv++)/3600.)*15.;

    strcpy(dec_deg,*argv++);
    if ( strchr(dec_deg,'-') != NULL )
    {
      is_neg = 1;
      decc = - atof(dec_deg);
    } else {
      is_neg = 0;
      decc = atof(dec_deg);
    }
    decc += atof(*argv++)/60.;
    decc += atof(*argv++)/3600.;
    //decc += atof(*argv++)/60.+atof(*argv++)/3600.;
    if ( is_neg ) decc *= -1;
    coords = dtos(rac) +","+ dtos(decc);
  }

/* Check if RA and Dec are given as string with ":" */
  else if (strchr(*argv, ':'))
  {
    rac  = deg_ra(*argv);
    decc = deg_dec(*++argv);
    coords = dtos(rac) +","+ dtos(decc);
  } else {
    coords = string(*argv) +",";
    coords += string(*++argv);
  }

  coords += ","+ string(*++argv);
  if (argc == 4 || argc == 8)
    coords += ","+ string(*++argv);


  if (use_hpx && my_order[0] != '\0')
    if (is_rect) {
      cerr << PROGNAME << ": rectangular region not allowed for HEALPix" << endl;
      exit(1);
    }


  if (my_passw[0] == '\0') {
    string ans;
    cout << "Enter " << my_user << " password: ";
    getline(cin, ans);
    strcpy(my_passw,ans.c_str());
  }


//  DBConn db;
//  db.connect("generic", "password", "MyCats");
//  Query qry(&db);
//  qry.query("SELECT RAcs/3.6e5, DECcs/3.6e5 FROM GSC22_htm_6 where DIF_Circle("+
//    coords +")", true);
//  nr1 = qry.nRows();


/* Connect to the DB */
  ret = db_init(cID);
  if (!ret)
  {
    cerr << PROGNAME <<"Can't set CONNECT_TIMEOUT for MySQL connection."<< endl;
    exit(1);
  }

  ret = db_connect(cID, my_host, my_user, my_passw, my_db1);

  if (!ret) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }
  const int iwidth=6, dwidth=12;  // printed values widths
  int idw1=0, idw2=0;

  cout << setiosflags(ios::fixed);
  cout << setprecision(7);
// Check catalogues are DIF indexed ... TBD

  string difqry_ini1, difqry_ini2, id_type, orde1, orde2, ra_fld1,
         ra_fld2, de_fld1, de_fld2, id_coln1, id_coln2;

  if (use_hpx) {
    id_type = "2";
  } else {
    id_type = "1";
  }

  difqry_ini1 = "SELECT param,Ra_field,Dec_field from DIF.tbl WHERE db='";
  difqry_ini2 =  "' AND id_type="+ id_type +" AND name='";

  qry_str = difqry_ini1 + my_db1 + difqry_ini2 + db_cat1 +"'";

  ret = db_query(cID, qry_str.c_str());
  if (!ret) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }

  if (db_num_rows(cID) == 0) {
    cerr << PROGNAME <<": "<< my_db1 <<"."<< db_cat1 <<" is not ";
    if (use_hpx)
      cerr << PROGNAME << ": HEALPix DIF indexed."<< endl;
    else
      cerr << PROGNAME << ": HTM DIF indexed."<< endl;
    exit(1);
  }

  if (use_hpx) {
    for (i=0; i<db_num_rows(cID); i++)
      if ( ! strcmp(db_data(cID, i, 0), my_order) ) {
        orde1 = my_order;
        break;
      }
    if (orde1.length() == 0) {
      orde1 = db_data(cID, 0, 0);
      cout << PROGNAME <<": "<< my_db1 <<"."<< db_cat1
           <<" is not DIF indexed at HEALPix order "<< my_order << endl
           << PROGNAME <<": using order "<< orde1 << endl;
    }
    db_view1 = db_cat1 + string("_healp_nest_") + orde1;
  } else if (my_depth[0] != '\0') {
    for (i=0; i<db_num_rows(cID); i++)
      if ( ! strcmp(db_data(cID, i, 0), my_depth) ) {
        orde1 = my_depth;
        break;
      }
    if (orde1.length() == 0) {
      orde1 = db_data(cID, 0, 0);
      cout << PROGNAME <<": "<< my_db1 <<"."<< db_cat1
           <<" is not DIF indexed at HTM depth "<< my_depth << endl
           << PROGNAME <<": using depth "<< orde1 << endl;
    }
    db_view1 = db_cat1 + string("_htm_") + orde1;

  } else {
    orde1   = db_data(cID, 0, 0);
    db_view1 = db_cat1 + string("_htm");
  }

  ra_fld1 = db_data(cID, 0, 1);
  de_fld1 = db_data(cID, 0, 2);
  if (use_hpx) {
    id_coln1 = "healpID_nest_"+ orde1;
    idw1 = MAX(dif_hpxid_maxw(orde1),id_coln1.length());
  } else {
    id_coln1 = "htmID_"+ orde1;
    idw1 = MAX(dif_htmid_maxw(orde1),id_coln1.length());
  }

//  qry_str = string("SELECT count(*) FROM ") + db_view1 +
//            " WHERE "+ dif_func + coords + ")";
//  ret = db_uquery(cID, qry_str.c_str());
//  record = mysql_fetch_row(result[0]);
//  sscanf(record[0],"%d",  &nr1);

  db_free_result(cID);

  qry_str = string("SELECT ") + ra_fld1 +co+ de_fld1 +co+ id_coln1 +" FROM "+ db_view1 +
            " WHERE "+ dif_func + coords +")";
cout<<"Query: "<< qry_str<<endl;
  ret = db_stmt_prepexe(cID, qry_str.c_str(), orde1.c_str());
  if (ret) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }

  nr1 = db_num_rows(cID);
cout << db_cat1 <<": N_entries="<< nr1 << endl;
  ra1 = (double *) malloc(nr1 * sizeof(double));
  de1 = (double *) malloc(nr1 * sizeof(double));
  id1 = (unsigned long *) malloc(nr1 * sizeof(unsigned long));

i=0;
//  while ((record = mysql_fetch_row(result[0])) != NULL)
  while (!mysql_stmt_fetch(stmt))
  {
   ra1[i] = dbl_data[0];
   de1[i] = dbl_data[1];
   id1[i] = long_data;
//row2num(record, &ra1[i], &de1[i], &id1[i]);
   //mysql_data_seek(result[0], i);
   //record = mysql_fetch_row(result[0]);
   //sscanf(record[0],"%lf", &ra1[i]);
   //sscanf(record[1],"%lf", &de1[i]);
   //sscanf(record[2],"%d",  &id1[i]);
i++;
  }

  //db_free_result(cID);
  mysql_stmt_close(stmt);

  if (do_list_all)
    for (i=0; i<nr1; i++)
      cout <<setw(iwidth)<< i << bl <<setw(idw1)<<id1[i]
           << bl <<setw(dwidth)<< ra1[i]
           << bl <<setw(dwidth)<< de1[i] << endl;


  ret = db_select(cID, my_db2);

  if (ret) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }


  qry_str = difqry_ini1 + my_db2 + difqry_ini2 + db_cat2 +"'";

  ret = db_query(cID, qry_str.c_str());
  if (!ret) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }

  if (db_num_rows(cID) == 0) {
    cerr << PROGNAME <<": "<< my_db2 <<"."<< db_cat2 <<" is not ";
    if (use_hpx)
      cerr << PROGNAME << ": HEALPix DIF indexed."<< endl;
    else
      cerr << ": HTM DIF indexed."<< endl;
    exit(1);
  }

  if (use_hpx) {
    for (i=0; i<db_num_rows(cID); i++)
      if ( ! strcmp(db_data(cID, i, 0), my_order) ) {
        orde2 = my_order;
        break;
      }
    if (orde2.length() == 0) {
      orde2 = db_data(cID, 0, 0);
      cout << PROGNAME <<": "<< my_db2 <<"."<< db_cat2
           <<" is not DIF indexed at HEALPix order "<< my_order << endl
           << PROGNAME <<": using order "<< orde2 << endl;
    }
    db_view2 = db_cat2 + string("_healp_nest_") + orde2;
  } else if (my_depth[0] != '\0') {
    for (i=0; i<db_num_rows(cID); i++)
      if ( ! strcmp(db_data(cID, i, 0), my_depth) ) {
        orde2 = my_depth;
        break;
      }
    if (orde2.length() == 0) {
      orde2 = db_data(cID, 0, 0);
      cout << PROGNAME <<": "<< my_db2 <<"."<< db_cat2
           <<" is not DIF indexed at HTM depth "<< my_depth << endl
           << PROGNAME <<": using depth "<< orde2 << endl;
    }
    db_view2 = db_cat2 + string("_htm_") + orde2;

  } else {
    orde2   = db_data(cID, 0, 0);
    db_view2 = db_cat2 + string("_htm");
  }

  ra_fld2 = db_data(cID, 0, 1);
  de_fld2 = db_data(cID, 0, 2);

  if (use_hpx) {
    id_coln2 = "healpID_nest_"+ orde2;
    idw2 = MAX(dif_hpxid_maxw(orde2),id_coln2.length());
  } else {
    id_coln2 = "htmID_"+ orde2;
    idw2 = MAX(dif_htmid_maxw(orde2),id_coln2.length());
  }

//  qry_str = string("SELECT count(*) FROM ") + db_view2 +
//            " WHERE "+ dif_func + coords + ")";
//  ret = db_uquery(cID, qry_str.c_str());
//  record = mysql_fetch_row(result[0]);
//  sscanf(record[0],"%d",  &nr2);

  db_free_result(cID);

  qry_str = string("SELECT ") + ra_fld2 +co+ de_fld2 +co+ id_coln2 +" FROM "+ db_view2 +
            " WHERE "+ dif_func + coords + ")";
cout<<"Query: "<< qry_str<<endl;

  ret = db_stmt_prepexe(cID, qry_str.c_str(), orde2.c_str());

  if (ret) {
    cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
    exit(1);
  }

  nr2 = db_num_rows(cID);
cout << db_cat2 <<": N_entries="<< nr2 << endl;
  ra2 = (double *) malloc(nr2 * sizeof(double));
  de2 = (double *) malloc(nr2 * sizeof(double));
  id2 = (unsigned long *) malloc(nr2 * sizeof(unsigned long));
i=0;
//  while ((record = mysql_fetch_row(result[0])) != NULL)
  while (! mysql_stmt_fetch(stmt))
  {
   ra2[i] = dbl_data[0];
   de2[i] = dbl_data[1];
   id2[i] = long_data;
//row2num(record, &ra2[i], &de2[i], &id2[i]);
   //sscanf(record[0],"%lf", &ra2[i]);
   //sscanf(record[1],"%lf", &de2[i]);
   //sscanf(record[2],"%d",  &id2[i]);
i++;
  }

  mysql_stmt_close(stmt);

  if (do_list_all)
    for (i=0; i<nr2; i++)
      cout <<setw(iwidth)<< i << bl <<setw(idw2)<<id2[i]
           << bl <<setw(dwidth)<< ra2[i]
           << bl <<setw(dwidth)<< de2[i] << endl;


  if (nr1 == 0 || nr2 == 0)
    return (1);

// The matching

  double matchlength=1./3600,  // def. match dist.= 1''
         minchunksize;
  vector<float> distance12;
  vector<long> match1, match2;
  unsigned long nmatch, nmatchmax, nmatchret;

  if (min_dist >= 0.)  {
    matchlength = min_dist/3600;
    if (use_arcmin)
      matchlength *= 60;
  } else
    min_dist = 1.;

  nmatchmax = MAX(nr1,nr2);
  nmatch = nmatchmax;

  minchunksize = matchlength * 10;
//cout << matchlength << " " << minchunksize << endl;


// TODO: Make sure first list is the longest or the other way around?
  if (nr1 > nr2) {
    if (multi_match)
	spherematch2_mm(nr2, ra2, de2, nr1, ra1, de1, matchlength, minchunksize,
			match2, match1, distance12, &nmatch);
    else
	spherematch2(nr2, ra2, de2, nr1, ra1, de1, matchlength, minchunksize,
			match2, match1, distance12, &nmatch);
  } else {
    if (multi_match)
	spherematch2_mm(nr1, ra1, de1, nr2, ra2, de2, matchlength, minchunksize,
			match1, match2, distance12, &nmatch);
    else
	spherematch2(nr1, ra1, de1, nr2, ra2, de2, matchlength, minchunksize,
			match1, match2, distance12, &nmatch);
  }

  if (nmatch == 0) {
    cout << PROGNAME <<": no match at a separation of "<<
       min_dist << bl << sep_unit << endl;
    return (0);
  }

  nmatchret = MIN(nmatch,nmatchmax);
// 23/06/2020: separation returned in arcsec
/*
  if (use_arcmin) {
    for (i=0; i<nmatchret; i++)
      distance12[i] *= 60;
  } else {
    for (i=0; i<nmatchret; i++)
      distance12[i] *= 3600;
  }
*/

  if (save_match) {
    if (my_tab.empty()) {
      my_tab = root_tablen + string(my_user) +"_"+
//      if (nr1 < nr2)
//        string(db_cat2) +"_"+ string(db_cat1);
//      else
        string(db_cat1) +"_"+ string(db_cat2);
    }

cout << "Output DB table: "<<out_db<<"."<<my_tab << endl;
// Create (if not there) output match table ... TBD
    if (drop_prematch) {
      qry_str = string("DROP TABLE IF EXISTS ") + out_db +"."+ my_tab;
      ret = db_query(cID, qry_str.c_str());
      if (!ret) {
        cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
        exit(1);
      }
    }

    qry_str = string("CREATE TABLE IF NOT EXISTS ") + out_db +"."+ my_tab +
              " (SeqID1 int unsigned, "+ db_cat1 +"_"+ id_coln1 +bl+
              dif_sqltype(orde1) +", RAdeg1 double, DECdeg1 double, SeqID2 int unsigned, "+
              db_cat2 +"_"+ id_coln2 +bl+ dif_sqltype(orde2) +", RAdeg2 double, DECdeg2 double, Sep double) ENGINE=MyISAM, CHARSET=ASCII";
//cout <<qry_str<<endl;
    ret = db_query(cID, qry_str.c_str());
    if (!ret) {
      cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
      exit(1);
    }
// Insert to be optimized
    string qry_ini =
           string("INSERT INTO ") + out_db +"."+ my_tab +" VALUES(";
    for (i=0; i<nmatchret; i++){
      qry_str = qry_ini +
                itos(match1[i]) +co+ itos(id1[i]) +co+
                dtos12(ra1[match1[i]]) +co+ dtos12(de1[match1[i]]) +co+
                itos(match2[i]) +co+ itos(id2[i]) +co+
                dtos12(ra2[match2[i]]) +co+ dtos12(de2[match2[i]]) +co+
                dtos12(distance12[i]) +")";
      ret = db_query(cID, qry_str.c_str());
      if (!ret) {
        cerr << PROGNAME <<": DB error: "<< db_error(cID) << endl;
        exit(1);
      }
    }

  }

  db_close(cID);

  cout << endl <<"Found "<< nmatch <<" matches at max sep. of "
         << min_dist << bl << sep_unit <<" (returned "<< nmatchret <<" entries)"<< endl;
  if (do_list_match) {
// Print out the matched objects
    cout << endl<<"Matched list:"<< endl

         << setw(iwidth)<<"SeqID1"<< bl <<setw(idw1)<<id_coln1<< bl
         <<setw(dwidth)<<"RAdeg1"<< bl <<setw(dwidth)<<"DECdeg1"<< bl
         << setw(iwidth)<<"SeqID2"<< bl <<setw(idw2)<<id_coln2<< bl
         <<setw(dwidth)<<"RAdeg2"<< bl <<setw(dwidth)<<"DECdeg2"<< bl
         <<"  Sep ("<< sep_unit <<")"<< endl;

    for (i=0; i<nmatchret; i++) {
      cout << setw(iwidth) << match1[i]<< bl << setw(idw1)<<id1[match1[i]]<< bl
           <<setw(dwidth)<<ra1[match1[i]]<< bl <<setw(dwidth)<<de1[match1[i]]<< bl
           << setw(iwidth) << match2[i]<< bl << setw(idw2)<<id2[match2[i]]<< bl
           <<setw(dwidth)<<ra2[match2[i]]<< bl <<setw(dwidth)<<de2[match2[i]]<< bl
           <<setw(dwidth)<< distance12[i] << endl;
    }
  }

  return (0);
}
