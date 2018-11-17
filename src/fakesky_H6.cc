/*
  Fake sky ordered by HTM depth 6 pixels.
  Use "fakesky_H6 -h" to see options.


  LN @ INAF-OAS March 2009                         Last changed: 17/11/2018
*/

#include <iostream>
using namespace std;

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string.h>

#include <vector>
#include "sqlInterface.h"

int getHTMid(char*& saved, int depth, double ra, double dec,
             unsigned long long int *id);
int getHTMBary1(int depth, unsigned long long int id,
                double *bc_ra, double *bc_dec);

// Progran name
const char PROGNAME[] = "fakesky_H6";

// Version ID string
static string VERID="Ver 1.0b, 25-10-2015, LN@IASF-INAF";

/* degrees to milli-arcsec */
static double DEG2MAS = 3.6E6;

/* radians to degrees */
static const double RAD2DEG = 57.2957795130823208767981548;

/* arcseconds to radians */
static const double ARCSEC_RAD = 4.848136811095359935899141;


// MySQL interface functions

#include <my_stmt_db.h>

// --


void
usage() {

  cout << PROGNAME << "  " << VERID << "\n" << endl
       << "Usage:" << endl
       << PROGNAME << " [OPTIONS]" << endl
       << "Where OPTIONS are:\n" << endl
       << "  -h: print this help" << endl
       << "  -D Dens: Nr of objects per arcmin^2 will be 'Dens' (def. 0.1)" << endl
       << "  -d DBnane: use 'DBnane' as MySQL databse name (def. TEST)" << endl
       << "  -p Password: MySQL user password is 'Password'" << endl
       << "  -s Server: send query to DB server 'Server' (def. localhost)" << endl
       << "  -t Table: produced table name will be 'Table' (def. fakeskyH6)" << endl
       << "  -u User: MySQL user name is 'User' (def. root)" << endl
       << endl;
  exit(0);

}


int
main(int argc, char *argv[]){
  unsigned short kwds=0;
  char c;

  int ret;
  string ans;
  char ins_comm[400];
  double dens=0.1;
  char my_db[33]="", my_tab[33]="", my_user[33]="", my_passw[33]="",
       my_host[33]="",
       hostn[33]="localhost", dbname[33]="test", tablen[33]="fakeskyH6",
       usern[33]="root";


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
        case 'D':
          if (argc < 2) usage();
          dens = atof(*++argv);
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

  if (my_host[0] != '\0')
    strcpy(hostn,my_host);

  if (my_db[0] != '\0')
    strcpy(dbname,my_db);

  if (my_user[0] != '\0')
    strcpy(usern,my_user);

  if (my_tab[0] != '\0')
    strcpy(tablen,my_tab);


// Density related parametrs
  double sky_area = (4*M_PI) * (RAD2DEG * RAD2DEG);
  long long int i, n_e, n_epp;

// Depth 6 HTM grid
// Mean area=2PI/4^(d+1), Min side = PI/2^(d+1), Max side -> Min_side * PI/2
  int depth = 6;
  long long int one = 1;
  unsigned long long int id, cur_id, npix = ( one << (2*depth+3) );
  double m_area = 2*M_PI/( one << (2*(depth+1)) ) * (RAD2DEG * RAD2DEG);

  n_e = (long long int) (dens*3600. * sky_area);

  cout << "There will be approx " << n_e << " entries in the table '"
       << tablen << "'." << endl
       << "<Return> to continue. ";
  getline(cin, ans);

  
//  n_epp = 2000; //-> 5,850 million entries, 175 MB (with HTM6 and HEAL8)
//                                           + 60 MB index (HEAL8)
//  n_epp = 1000000;  //-> 2.9 billion entries, 85 GB () + 30 GB index (HEAL8)

  n_epp = (long long int) (dens*3600. * m_area);

// Must account for spread of circular region
  n_epp *= 10;
cout << "Nepp*10: " << n_epp << endl;

/* Connect to the DB */
  ret = db_init(0);
  if (!ret)
  {
    cout << "Can't set CONNECT_TIMEOUT for MySQL connection." << endl;
    return(1);
  }


  if (my_passw[0] == '\0') {
    cout << "Enter " << usern << " password: ";
    getline(cin, ans);
    strcpy(my_passw,ans.c_str());
  }

  ret = db_connect(0, hostn, usern, my_passw, dbname);

  if (!ret) {
    cout << "DB error: " << db_error(0);
    return(1);
  }

// Create table in case it is not there: ignore error
  strcpy(ins_comm, "CREATE TABLE ");
  strcat(ins_comm, tablen);
  strcat(ins_comm, " (RAmas int unsigned not null default 0, DECmas int not null default 0, RAdeg double not null default 0, DECdeg double not null default 0, htmID_6 smallint unsigned not null default 0)");
  ret = db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl
         << "... continue." << endl;
  }

  cout << "Table creation start time: ";
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



  double bc_ra, bc_dec, phi, lat;
  long int RAmas, DECmas;
  double max_rad2, max_rad, max_rad2_ra, max_rad_ra, lat_fac;

  double d_rand_max = (double) RAND_MAX;

  double x, y;

  char* saved = NULL;  // dummy for HTM id calc.

  cout << "Be patient while creating the catalogue..."<<endl;

// Loop on trixels
  for (id = npix; id < 2*npix; id++) {
//  for (id = 2*npix-1; id >= npix; id--) {
//  for (id = npix; id < (npix+100); id++) {

/* Random No. generator init. */
    srand(284537612 + id);  // Initialization: any large integer would be OK

    getHTMBary1(depth, id, &bc_ra, &bc_dec);
//- cout << "ID: " << id << endl;
//- cout << "Center RA, Dec: " << bc_ra <<"  "<< bc_dec << endl;
    lat_fac = 1./cos(bc_dec/RAD2DEG);
    max_rad2 = 1.7 * M_PI/2. * M_PI/(one << (depth+1)) * RAD2DEG;
    max_rad = max_rad2 / 2.;
    max_rad2_ra = max_rad2 * lat_fac;
    max_rad_ra = max_rad * lat_fac;

//- cout << "Lat fac.: " << lat_fac << "  Max radius used: " << max_rad << endl;

    int n_in=0, n_out=0;
// Loop on events/trixel
    for (i=0; i< n_epp; i++) {
/* Random Theta and Phi */
//    lat = asin( (2* (double) rand() / (double) RAND_MAX - 1.) ) * RAD2DEG
//    phi = (double) rand() / (double) RAND_MAX * 360.;

      x = ( (double) rand() / d_rand_max );
      y = ( (double) rand() / d_rand_max );
      if (sqrt (x*x + y*y) <= 1.) {
        lat = bc_dec - max_rad + max_rad2 *  x;
        phi = bc_ra  - max_rad_ra + max_rad2_ra * y; 

        if ((phi >= 0.) && (phi < 360.) && (lat > -90.) && lat < 90.) {
          RAmas  = (long int)(phi*DEG2MAS);
          DECmas = (long int)(lat*DEG2MAS);

          getHTMid(saved, depth, phi, lat, &cur_id);

          if (cur_id == id) {
            n_in++;
            sprintf(ins_comm, "INSERT INTO %s VALUES (%ld, %ld, %lf, %lf, %lld)",
                    tablen, RAmas, DECmas, phi, lat, cur_id);
//cout << ins_comm << endl;

            ret = db_query(0, ins_comm);
            if (!ret)
              cout << "DB error: " << db_error(0) << endl;

          } else
            n_out++;

        }
      }
    }

//- cout << "N_objs IN: " << n_in  << "  N_objs OUT: " << n_out << endl;

  }  // End loop on trixels

  strcpy(ins_comm, "UNLOCK TABLES");
  ret = db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl;
    return(1);
  }

  db_close(0);


  cout << "Table creation end time: ";
  ret = system("date");
  cout << "done!" << endl;

}
