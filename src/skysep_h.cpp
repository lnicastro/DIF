/*
  (double) skysep_h

  Purpose: gets the distance (in "arcmin") of 2 objects on a sphere.

  Usage: dist = skysep_h(phi1, theta1, phi2, theta2, radians)

  Note:
      All values double, radians short.
      If radians=0 then input values are in degrees.
      phi1/2 cannot be negative.

      The Haversine formula (R.W Sinnot, 1984) is used to get rid of singularities.


  LN@IASF-INAF, March 2007                    ( Last change: 07/07/2016 )
*/

#include <cmath>

/* PI, 2*PI */
//static const double PI2 = M_PI * 2.;
static const double PI  = 3.14159265358979323846264338327950288;
static const double PI2 = 6.28318530717958647692528676655900577;
/* radians to degrees */
//#define RAD_DEG 57.2957795130823209

/* degrees to radians */
static const double DEG2RAD = 1.74532925199432957692369E-2;


double skysep_h(double phi1, double theta1, double phi2, double theta2,
                short coord_type)
{
  double a1r, b1r, a2r, b2r, radif, sin2a, sin2d;

  if (phi1 < 0. || phi2 < 0.) return -1.;

  if (! coord_type)
  {
    a1r = phi1*DEG2RAD;
    b1r = theta1*DEG2RAD;
    a2r = phi2*DEG2RAD;
    b2r = theta2*DEG2RAD;
  } else {
    a1r = phi1;
    b1r = theta1;
    a2r = phi2;
    b2r = theta2;
  }

  radif = fabs(a2r-a1r);
  if (radif > PI) radif = PI2 - radif;
  sin2a = sin(radif/2.);
  sin2a *= sin2a;
  sin2d = sin((b2r-b1r)/2.);
  sin2d *= sin2d;
  return (2 * asin( sqrt(sin2d + cos(b1r)*cos(b2r)*sin2a) )/DEG2RAD * 60.);
}
