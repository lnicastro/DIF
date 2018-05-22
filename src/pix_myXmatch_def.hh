/*
   Definitions for pix_myXmatch

   LN@IASF-INAF, January 2016                   ( Last change: 09/11/2016 )
*/

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )

// Global variables
static string bl = " ";
static string co = ",";
static string dt = ".";

// Progran name
const char PROGNAME[] = "pix_myXmatch";

// Version ID string
static string VERID="Ver 0.1i, 03-02-2016, LN@IASF-INAF";

extern "C" {
  double deg_ra(char *ra_str);
  double deg_dec(char *dec_str);
  //char *enc_str_radeg(double ra);
  //char *enc_str_decdeg(double dec);
}

long spherematch2(long npoints1, double *ra1, double *dec1,
                 long npoints2, double *ra2, double *dec2,
                 double matchlength, double minchunksize,
                 vector<long> &match1, vector<long>&match2, vector<double>&distance12, long *nmatch);
long spherematch2_mm(long npoints1, double *ra1, double *dec1,
                 long npoints2, double *ra2, double *dec2,
                 double matchlength, double minchunksize,
                 vector<long> &match1, vector<long>&match2, vector<double>&distance12, long *nmatch);
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

