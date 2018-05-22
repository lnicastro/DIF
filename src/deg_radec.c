/*
  double deg_ra.c

  Purpose:

   Decode a right ascension string formatted as "hh mm ss.s" or "hh:mm:ss.s"
   into a fractional double precision value.


   LN@IASF-CNR, November 2004                   ( Last change: 02/10/2010 )
*/

#include <stdio.h>

double deg_ra(char *ra_str)
{
  unsigned int rah,ram, n=0;
  float rafs;
  double ra;
//  static char str_ra[20];

  if (ra_str[2] == ':' || ra_str[2] == ' ') {
    sscanf(ra_str,"%2d", &rah);
    ra_str += 3;
    n += 3;
  } else {
    sscanf(ra_str,"%1d", &rah);
    ra_str += 2;
    n += 2;
  }
  if (ra_str[2] == ':' || ra_str[2] == ' ') {
    sscanf(ra_str,"%2d", &ram);
    ra_str += 3;
    n += 3;
  } else {
    sscanf(ra_str,"%1d", &ram);
    ra_str += 2;
    n += 2;
  }
  sscanf(ra_str,"%f", &rafs);
  ra_str -= n;
  ra  = (rah + ram/60. + rafs/3.6e3) * 15.;
  if (ra < 0.) ra -= 360;  // Should never happen
//  printf("ra=%f\n",ra);
  return (ra);
}

/*
  double deg_dec.c

  Purpose:

   Decode a declination string formatted as "+/-dd mm ss.s" or "+/-dd:mm:ss.s"
   into a fractional double precision value.


   LN@IASF-CNR, November 2004                   ( Last change: 02/10/2010 )
*/

double deg_dec(char *dec_str)
{
  unsigned int deg, dem, n=0;
  float defs;
  double de;

//  sscanf(dec_str,"%1s%2d%1s%2d%1s%f",&sign, &deg, &div, &dem, &div, &defs);
  if (dec_str[0] == '+' || dec_str[0] == '-') {
    dec_str += 1;
    n += 1;
  }

  if (dec_str[2] == ':' || dec_str[2] == ' ') {
    sscanf(dec_str,"%2d", &deg);
    dec_str += 3;
    n += 3;
  } else {
    sscanf(dec_str,"%1d", &deg);
    dec_str += 2;
    n += 2;
  }
  if (dec_str[2] == ':' || dec_str[2] == ' ') {
    sscanf(dec_str,"%2d", &dem);
    dec_str += 3;
    n += 3;
  } else {
    sscanf(dec_str,"%1d", &dem);
    dec_str += 2;
    n += 2;
  }
  sscanf(dec_str,"%f", &defs);
  de  = deg + dem/60. + defs/3.6e3;
  dec_str -= n;
  if (dec_str[0] == '-') de *= -1;
//  printf("dec=%f\n",de);
  return (de);
}
