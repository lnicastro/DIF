/*
  Fake sky with fully random generated points.

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

// Progran name
//const char PROGNAME[] = "fakesky_RND";

// Version ID string
static string VERID="Ver 1.0, 22-04-2009, LN@IASF-INAF";

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
usage(char *name) {

  cout << name << "  " << VERID << "\n" << endl
       << "Usage:" << endl
       << name << " Sky_density (entries/arcmin^2) [ DB_table_name ]" << endl
       << endl;
  exit(0);

}


int
main(int argc, char *argv[]){
  int narg = 1;                 // number of required arguments
  int ret;
  string password;
  char db_tabname[40], ins_comm[200];
  double dens;

  if (argc != narg+1) usage(argv[0]);

  char host[] = "localhost", user[] = "root", db[] = "test";

  dens = atof(*++argv);

/* Connect to the DB */
  ret = db_init(0);
  if (!ret)
  {
    cout << "Can't set CONNECT_TIMEOUT for MySQL connection." << endl;
    return(1);
  }

  if (argc > 2)
    strcpy(db_tabname,*++argv);
  else
    strcpy(db_tabname,"fakeskyRND");


  cout << "Enter root password: ";
  getline(cin, password);
//password="baff0lO";
//password="R0zz02";

  ret = db_connect(0, host, user, password.c_str(), db);

  if (!ret) {
    cout << "DB error: " << db_error(0);
    return(1);
  }

// Create table in case it is not there: ignore error
  strcpy(ins_comm, "CREATE TABLE ");
  strcat(ins_comm, db_tabname);
  strcat(ins_comm, " (RAmas int unsigned not null default 0, DECmas int not null default 0, RAdeg double not null default 0, DECdeg double not null default 0, htmID_6 smallint unsigned not null default 0)");
  ret = db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl
         << "... continue." << endl;
  }

  ret = system("date");
/* First lock the table */
  strcpy(ins_comm, "LOCK TABLES ");
  strcat(ins_comm, db_tabname);
  strcat(ins_comm, " WRITE");
  ret = db_query(0, ins_comm);
  if (!ret) {
    cout << "DB error: " << db_error(0) << endl;
    return(1);
  }


// Use the RA=[0, 360], Dec=[-90, 90] range (could be parametrized)
  double sky_area = (4*M_PI) * (RAD2DEG * RAD2DEG);
  double phi, lat;
  long int RAmas, DECmas;
  long long int i, n_e;

  n_e = (long long int) (dens*3600. * sky_area);
cout << "Sky area (deg^2): " << sky_area <<"  N_entries=" << n_e << endl;

// Depth 6 HTM grid
  int depth = 6;
  long long int one = 1;
  unsigned long long int cur_id, npix = ( one << (2*depth+3) );
  
//  n_e = 500000000;
/* Random No. generator init. */
  srand(184537612);  // Initialization: any large integer would be OK
//cout << RAND_MAX << endl;
  double d_rand_max = (double) RAND_MAX + 1.;

  char* saved = NULL;  // dummy for HTM id calc.

  cout << "Be patient while creating the catalogue..."<<endl;

  for (i=0; i<n_e; i++) {
/* Random Theta and Phi */
    lat = asin(2 * (double) rand()/d_rand_max - 1.) * RAD2DEG; // -90, 90 deg
    phi = (double) rand() / d_rand_max * 360.; // 0-360 deg

    RAmas  = (long int)(phi*DEG2MAS);
    DECmas = (long int)(lat*DEG2MAS);

   getHTMid(saved, depth, phi, lat, &cur_id);

    sprintf(ins_comm, "INSERT INTO %s VALUES (%ld, %ld, %lf, %lf, %lld)",
        db_tabname, RAmas, DECmas, phi, lat, cur_id);
//cout << ins_comm << endl;

       ret = db_query(0, ins_comm);
        if (!ret)
          cout << "DB error: " << db_error(0) << endl;

  }

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
