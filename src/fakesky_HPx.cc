/*
  Fake sky "created" by user defined HEALPix order in "nested" scheme
  within "ring" scheme pixels of lower order (def. 6).
  Use "fakesky_HPx -h" to see options.


  LN @ IASF-INAF March 2009                         Last changed: 28/01/2023
*/

using namespace std;

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <string.h>

#include <vector>
//#include "sqlInterface.h"
#include "arr.h"
#include "healpix_base.h"

//int getHTMid(char*& saved, int depth, double ra, double dec,
//             long long int *id);
//int getHTMBary1(int depth, long long int id,
//                double *bc_ra, double *bc_dec);
//
//int getHealPid(int nested, char*& saved, int k, double ra, double dec,
//               long long int *id);

// Progran name
const char PROGNAME[] = "fakesky_HPx";

// Version ID string
static string VERID="Ver 1.0b, 16-05-2016, LN@IASF-INAF";

/* degrees to centi-arcsec */
static double DEG2MAS = 3.6E6;

/* radians to degrees */
static const double RAD2DEG = 57.2957795130823208767981548;

/* arcseconds to radians */
static const double ARCSEC_RAD = 4.848136811095359935899141;


// MySQL interface functions

#include "my_stmt_db.h"

// --


void
usage() {

  cout << PROGNAME << "  " << VERID << "\n" << endl
       << "Usage:" << endl
       << "  " << PROGNAME << " [OPTIONS]" << endl
       << "Where OPTIONS are:\n" << endl
       << "  -h: print this help" << endl
       << "  -d DBnane: use 'DBnane' as MySQL databse name (def. TEST)" << endl
       << "  -o Order: HEALPix pixelization order to use is 'Order' (def. 10)" << endl
       << "  -p Password: MySQL user password is 'Password'" << endl
       << "  -s Server: send query to DB server 'Server' (def. localhost)" << endl
       << "  -t Table: produced table name will be 'Table' (def. fakeskyHPxNN)" << endl
       << "  -u User: MySQL user name is 'User' (def. root)" << endl
       << "  -R: use RING scheme ID numbering (def. NESTED)" << endl
       << endl;
  exit(0);

}

int
main(int argc, char *argv[]){
  unsigned short kwds=0;
  char c;

  int ret;
  string ans, fld_name;
  char db_tabname[40], ins_comm[400];
  char my_db[33]="", my_tab[33]="", my_user[33]="", my_passw[33]="",
       my_host[33]="", my_order[3]="",
       hostn[33]="localhost", dbname[33]="TEST", tablen[33]="fakeskyHPx",
       usern[33]="root", order_s[3]="10";

  long long int nside_mainp, nside_finep, npix_main, npix_fine;
  int nested = 1, order_mainp = 6, order_finep;
  T_Healpix_Base<int64>* base_mainp;
  T_Healpix_Base<int64>* base_finep;


//  if (argc < 2) usage();

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
        case 'o':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_order);
          --argc;
          kwds=0;
          break;
        case 'd':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_db);
          --argc;
          kwds=0;
          break;
        case 'p':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_passw);
          --argc;
          kwds=0;
          break;
        case 'R':  // RING scheme
          nested = 0;
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
          sscanf(*++argv,"%s",my_tab);
          --argc;
          kwds=0;
          break;
        case 'u':
          if (argc < 2) usage();
          sscanf(*++argv,"%s",my_user);
          --argc;
          kwds=0;
          break;
        default:
          fprintf (stderr,"Illegal option `%c'.\n\n",c);
          usage();
      }
    }
  }
//  if (argc != 1) usage();

  if (my_host[0] != '\0')
    strcpy(hostn,my_host);

  if (my_db[0] != '\0')
    strcpy(dbname,my_db);

  if (my_user[0] != '\0')
    strcpy(usern,my_user);

  if (my_order[0] != '\0')
    strcpy(order_s,my_order);

  if (my_tab[0] != '\0')
    strcpy(tablen,my_tab);
  if (! nested)
    strcat(tablen,"R");
  strcat(tablen,order_s);

  order_finep = atoi(order_s);

  nside_mainp = 1 << order_mainp;
  npix_main = 12 * nside_mainp*nside_mainp;
  nside_finep = 1 << order_finep;
  npix_fine = 12 * nside_finep*nside_finep;
  int nside_diff = 1 << 2*(order_finep - order_mainp);

  cout << "There will be " << npix_fine << " entries in the table '"
       << tablen << "'." << endl
       << "Aggregate pixel order is 6. Given input order " << order_finep
       << " gives " << nside_diff << " entries per pix." << endl
       << "<Return> to continue. ";
//  getline(cin, ans);


  if (my_passw[0] == '\0') {
    cout << "Enter " << usern << " password: ";
    getline(cin, ans);
    strcpy(my_passw,ans.c_str());
  }


/* Connect to the DB */
  ret = db_init(0);
  if (!ret)
  {
    cout << "Can't set CONNECT_TIMEOUT for MySQL connection." << endl;
    return(1);
  }

  ret = db_connect(0, hostn, usern, my_passw, dbname);

  if (!ret) {
    cout << "DB error: " << db_error(0);
    return(1);
  }

// Create table in case it is not there: ignore error
  fld_name = "healpID_";
  if (nested)
    fld_name += "1_";
  else
    fld_name += "0_";
  fld_name += order_s;

  strcpy(ins_comm, "CREATE TABLE ");
  strcat(ins_comm, tablen);
  strcat(ins_comm, " (RAmas int unsigned not null default 0, DECmas int not null default 0, RAdeg double not null default 0, DECdeg double not null default 0, ");
  strcat(ins_comm, fld_name.c_str());

  if (order_finep > 14)
    strcat(ins_comm, " bigint unsigned not null default 0)");
  else
    strcat(ins_comm, " int unsigned not null default 0)");

  ret = db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl
         << "... continue." << endl;
  }

  ret = system("date");
/* First lock the table */
  strcpy(ins_comm, "LOCK TABLES ");
  strcat(ins_comm, tablen);
  strcat(ins_comm, " WRITE");
  ret=db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl;
    return(1);
  }


  base_mainp = new T_Healpix_Base<int64>(nside_mainp, RING, SET_NSIDE);

  if (nested)
    base_finep = new T_Healpix_Base<int64>(nside_finep, NEST, SET_NSIDE);
  else
    base_finep = new T_Healpix_Base<int64>(nside_finep, RING, SET_NSIDE);

int64 startpix, ringpix;
double costheta, sintheta;
bool shifted;

//base_mainp->get_ring_info(0, startpix, ringpix, costheta, sintheta, shifted);
//cout << startpix << " " << ringpix << " " << costheta << " " << sintheta << endl;
//
//base_mainp->get_ring_info(1, startpix, ringpix, costheta, sintheta, shifted);
//cout << startpix << " " << ringpix << " " << costheta << " " << sintheta << endl;

//  double sky_area = (4*M_PI) * (RAD2DEG * RAD2DEG);
  double bc_ra, bc_dec;
  long int RAmas, DECmas;

  long long int id, id_main, id_nest, id_min, id_max;
  double m_area = M_PI/(3.*nside_finep*nside_finep) * (RAD2DEG * RAD2DEG) * 3600; //arcmin^2

cout << "Pixel area (arcmin^2): " << m_area << endl;

  cout << "Be patient while creating the table '" << tablen << "'..."<<endl;

// Loop on Main pixels (ring)
  for (id_main = 0; id_main < npix_main; id_main++) {
//  for (id_main = 0; id_main < 10; id_main++) {

    
    id_nest = base_mainp->ring2nest(id_main);

    id_min = id_nest * nside_diff;
    id_max = id_min + nside_diff - 1;

// Loop on Fine pixels
//  for (id = 0; id < npix_fine; id++) {
  for (id = id_min; id <= id_max; id++) {

    double sth;
    bool have_sth;
    base_finep->pix2loc(id, bc_dec, bc_ra, sth, have_sth);
    bc_ra *= RAD2DEG;
    have_sth ? bc_dec = 90. - atan2(sth,bc_dec)*RAD2DEG :
               bc_dec = asin(bc_dec) * RAD2DEG;


//- cout << "ID: " << id << endl;
//- cout << "Center RA, Dec: " << bc_ra <<"  "<< bc_dec << endl;

    RAmas  = (long int)(bc_ra*DEG2MAS);
    DECmas = (long int)(bc_dec*DEG2MAS);

      sprintf(ins_comm, "INSERT INTO %s VALUES (%ld, %ld, %lf, %lf, %lld)",
              tablen, RAmas, DECmas, bc_ra, bc_dec, id);
//cout << ins_comm << endl;

      ret = db_query(0, ins_comm);
      if (!ret)
        cout << "DB error: " << db_error(0) << endl;

  }  // End loop on Fine pixels
  }  // End loop on Main pixels

  strcpy(ins_comm, "UNLOCK TABLES");
  ret = db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl;
    return(1);
  }

  db_close(0);


  ret = system("date");
  cout << "done!" << endl;

}
