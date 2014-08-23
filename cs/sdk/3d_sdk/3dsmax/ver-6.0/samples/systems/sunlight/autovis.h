/*===========================================================================*\
	FILE: autovis.h

	DESCRIPTION: Code from Autovision.

	HISTORY: Adapted by John Hutchinson 10/08/97 
			

	Copyright (c) 1996, All Rights Reserved.
 \*==========================================================================*/

#define TWO_PI 6.283185307
#define FTWO_PI 6.283185307f
#define EPOCH 1721424.5f
#define SECS_PER_DAY 86400

///////////////////////////////////////////////////////////////////
// Sun locator code from AutoVision
//////////////////////////////////////////////////////////////////
static double dtr(double dang){return TWO_PI * dang/360.0;}
static double rtd(double rang){return 360.0 * rang/TWO_PI;}
static float fdtr(float dang){return FTWO_PI * dang/360.0f;}
static float frtd(float rang){return 360.0f * rang/FTWO_PI;}
static double kepler(double m, double ecc);
static double gregorian2julian(unsigned short month, unsigned short day, unsigned short year);
static void altaz(double *az, double *alt, double *s_time, double sunra,
                  double sundec, double longitude, double latitude,
                  double jdate, double jtime);
static void rotate(double pt[3], double matrix[3][3]);
static double distance(double pt1[3], double pt2[3]);
static void precession(double p[3][3], double jdate);
static void ecliptic(double planet[3], double v_planet[3], double jdate);
static void barycentric(double earth[3], double v_earth[3], double jdate);
static void topocentric(double earth[3], double v_earth[3], double lon,
                        double lat, double jdate, double jtime);
static void nutation(double *ra, double *dec, double jdate);
static int sunloc(void);
static void julian2gregorian(double jdate, unsigned short* month, unsigned short* day, unsigned short* year);
static BOOL isleap( long year);
/* Date and time of creation September 23, 1991 */

/* Date of epoch for the orbital elements */
double edate[2] = {2447920.5, 2448120.5};
double ee = 23.439291; /* Obliquity for date of epoch J2000.0 */

#define fixangle(x)  ((x) - 360.0 * (floor ((x) / 360.0)))
#define angle(x,y) (atan((x)/(y))+((y)<=0.0?PI:(x)<=0.0?2*PI:0.0))
#define sqr(x) ((x)*(x))
#define ER2AU (6378160.0/149600.0e6) /* A.U per earth radii */


static int mdays[2][12] = {
    {0,31,59,90,120,151,181,212,243,273,304,334},
    {0,31,60,91,121,152,182,213,244,274,305,335}
};

/***************************************************************************/
static double
gregorian2julian(long month, long day, long year)
{


   long ydays, ydays1;

   ydays1 = 365*year+year/4-year/100+year/400;
   year--;
   ydays = 365*year+year/4-year/100+year/400;
   return ydays+mdays[(ydays1-ydays)==366?1:0][month-1]+day+1721424.5f;
}

BOOL isleap( long year){
	long ydays, ydays1;

   ydays1 = 365*year+year/4-year/100+year/400;
   year--;
   ydays = 365*year+year/4-year/100+year/400;
   return (ydays1-ydays)==366?TRUE:FALSE;
}



void
julian2gregorian(double jdate, int* month, int* day, int* year){
	long ijd, a, b, c, d, e, g;
	double fjd;

	jdate += 0.5f;
	ijd = (long) jdate;	 //whole part
	fjd = jdate-ijd;	  //decimal part
	if(ijd>2299160){
		a = (long) ((ijd- 1867216.25) / 36524.25);
		b = ijd+ 1 + a - (int)(a/4);
	} else {
		b=ijd;
	}

	c=b+1524;
	d= (long) ((c-122.1)/365.25);
	e= (long) (365.25*d);
	g= (long) ((c-e)/30.6001);

	//round off to closest day
	*day = (int)(c-e+fjd-(int) (30.6001*g)+0.5f);

	
	if(g<13.5)
		*month = g-1;
	else
		*month= g-13;
	
	
	if(*month>2.5)
		*year = d-4716;
	else
		*year = d-4715;

}



/***************************************************************************/
static void
altaz(double *az, double *alt, double *s_time, double sunra, double sundec,
      double longitude, double latitude, double jdate, double jtime)
{
   double gmt,h, a,b,c;
   double hours, minutes;

   /* Calculate gmt sidereal time */
   gmt = 6.6265313+0.0657098243*(jdate-jtime-2447891.5)+1.00273791*24.0*jtime;
   gmt = dtr(gmt * 360.0 / 24.0);
   h = gmt - longitude - sunra;

   /* Calculate azimuth and altitude */
   a = -cos(sundec)*sin(h);
   b = sin(sundec)*cos(latitude)-cos(sundec)*cos(h)*sin(latitude);
   c = sin(sundec)*sin(latitude)+cos(sundec)*cos(h)*cos(latitude);

   *alt = asin(c);
   *az = angle(a,b);

   h = rtd(h);  /* now in degrees */
   h = h * (24.0 / 360.0); /* convert to hours */
   hours = (12 + (int)floor(h)) % 24;  /* nuke the extra days */
   minutes = h - floor(h);  /* get the hour fractions back */
   *s_time = hours + minutes;
}

/***************************************************************************/
static void
rotate(double pt[3], double matrix[3][3])
{
   double newpt[3];
   int i,j;

   for(i=0; i<3; i++) {
      newpt[i] = 0;
      for(j=0;j<3;j++)
         newpt[i] = newpt[i] + pt[i]*matrix[j][i];
   }
   for(i=0;i<3;i++)
      pt[i] = newpt[i];
}

/**************************************************************************/
static double
distance(double pt1[3], double pt2[3])
{
   return sqrt(sqr(pt1[0]-pt2[0]) + sqr(pt1[1]-pt2[1]) + sqr(pt1[2]-pt2[2]));
}

/***************************************************************************/
static void
precession(double p[3][3], double jdate)
{
   double t,zeta,z,theta;

   t = (jdate-2451545.0)/36525.0;
   zeta = ((0.0000050*t+0.0000839)*t+0.6406161)*t;
   z = ((0.0000051*t+0.0003041)*t+0.6406161)*t;
   theta = ((-0.0000116*t-0.0001185)*t+0.5567530)*t;

   zeta = dtr(zeta); z = dtr(z); theta = dtr(theta);
   
   p[0][0] = cos(zeta)*cos(theta)*cos(z) - sin(zeta)*sin(z);
   p[0][1] = cos(zeta)*cos(theta)*sin(z) + sin(zeta)*cos(z);
   p[0][2] = cos(zeta)*sin(theta);
   p[1][0] = -sin(zeta)*cos(theta)*cos(z) - cos(zeta)*sin(z);
   p[1][1] = -sin(zeta)*cos(theta)*sin(z) + cos(zeta)*cos(z);
   p[1][2] = -sin(zeta)*sin(theta);
   p[2][0] = -sin(theta)*cos(z);
   p[2][1] = -sin(theta)*sin(z);
   p[2][2] = cos(theta);
}

/***************************************************************************/
static void
ecliptic(double planet[3], double v_planet[3], double jdate)
{
   double i, ohm, nw, a, n, e, l, m,
            u, f, r, dm, du, df, dr,
            ldate, aa, bb, cc, dd, ee, ff;

   if (jdate <= edate[0]+100) {
/* Orbital elements for JD 2447920.5 and JD 2448120.5 referred to the
    mean ecliptic and equinox of J2000.0 */
      i = dtr(0.00132);  /* Inclination */
      ohm = dtr(352.4);  /* Longitude of ascending node */
      nw = dtr(102.90260);  /* Longitude of perhelion */
      n = dtr(0.9856049);  /* Daily motion */
      l = dtr(128.12283);  /* Mean longitude */
      a = 1.0000029;   /* Eccentricity */
      e = 0.0167006;   /* Mean distance */
      ldate = edate[0];
   }
   else {  /* See above for descriptions */
      i = dtr(0.00126);
      ohm = dtr(353.7);
      nw = dtr(103.0005);
      n = dtr(0.9856094);
      l = dtr(325.24544);
      a = 0.9999998;
      e = 0.0166744;
      ldate = edate[1];
   }

/* Calculate mean anamoly and its derivative */
   m = (jdate - ldate)*n+l-nw; dm = n;

/* Calculate eccentric anamoly and its derivative */
   u = m;
   u = (sin(u)*e+m-u)/(1-e*cos(u)) + u;
   u = (sin(u)*e+m-u)/(1-e*cos(u)) + u;
   u = (sin(u)*e+m-u)/(1-e*cos(u)) + u;
   u = (sin(u)*e+m-u)/(1-e*cos(u)) + u;
   du = dm/(1-e*cos(u));
/* Calculate tru anamoly and its derivative */
   f = 2*atan(sqrt((1+e)/(1-e))*tan(u/2));
   df =sqrt((1+e)/(1-e))/(1+sqr(sqrt((1+e)/(1-e))*tan(u/2)))/sqr(cos(u/2))*du;

/* Calculate radius vector and its derivative */
   r = a*(1-e*e)/(1+e*cos(f));
   dr = a*(1-e*e)*e*sin(f)/sqr(1+e*cos(f))*df;

/* Calculate ecliptic coordinates and velocity */
   aa = cos(f+nw-ohm);
   bb = sin(f+nw-ohm);
   cc = cos(ohm);
   dd = sin(ohm);
   ee = cos(i);
   ff = sin(i);

   planet[0] = r*(aa*cc-bb*ee*dd);
   planet[1] = r*(aa*dd+bb*ee*cc);
   planet[2] = r*bb*ff;

   v_planet[0] = dr*(aa*cc-bb*ee*dd)-r*(bb*cc+aa*ee*dd)*df;
   v_planet[1] = dr*(aa*dd+bb*ee*cc)-r*(bb*dd-aa*ee*cc)*df;
   v_planet[2] = dr*bb*ff+r*aa*ff*df;
}

/***************************************************************************/
static void
barycentric(double earth[3], double v_earth[3], double jdate)
{
   double t,l,dt,dl;

   t = (jdate-2451545.0)/36525.0;
   dt = 1/36525.0;
   l = dtr(218.0 + 481268.0*t);
   dl = dtr(481268.0*dt);

   earth[0] -= 0.0000312*cos(l);
   earth[1] -= 0.0000312*sin(l);

   v_earth[0] += 0.0000312*sin(l)*dl;
   v_earth[1] -= 0.0000312*cos(l)*dl;
}

/****************************************************************************/
static void
topocentric(double earth[3], double v_earth[3], double lon, double lat,
            double jdate, double jtime)
{
   double gmt, dgmt, lst, dlst, x, y, z, dx, dy, dz;

/* Compute gmt and its derivative */
   gmt = 6.6265313+0.0657098243*(jdate-jtime-2447891.5)+
         1.00273791*24.0*jtime;
   gmt = dtr(gmt*15.0);
   dgmt = 1.00273791*24.0;
   dgmt = dtr(dgmt*15.0);

/* Calculate local sidereal time and its derivative */
   lst = gmt-lon;
   dlst = dgmt;

/* Calculate geocentric coordinates and velocity */
   x = ER2AU*cos(lat)*cos(lst);
   y = ER2AU*cos(lat)*sin(lst);
   z = ER2AU*sin(lat);

   dx = -ER2AU*cos(lat)*sin(lst)*dlst;
   dy = ER2AU*cos(lat)*cos(lst)*dlst;
   dz = 0;

/* Calculate new ecliptic coordinates and velocity */
   earth[0] += x;
   earth[1] += y*cos(dtr(ee))+z*sin(dtr(ee));
   earth[2] += z*cos(dtr(ee))-y*sin(dtr(ee));

   v_earth[0] += dx;
   v_earth[1] += dy*cos(dtr(ee))+dz*sin(dtr(ee));
   v_earth[2] += dz*cos(dtr(ee))-dy*sin(dtr(ee));
}

/****************************************************************************/
static void
nutation(double *ra, double *dec, double jdate)
{
   double a,b,d,gamma,epsilon,e;

   d = jdate-2447891.5;
   gamma = -0.0048*sin(dtr(318.5-0.053*d))
           -0.0004*sin(dtr(198.8+1.971*d));
   epsilon = 0.0026*cos(dtr(318.5-0.053*d))
             +0.0002*cos(dtr(198.8+1.971*d));

   e = dtr(ee);
   a = (cos(e)+sin(e)*sin(*ra)*tan(*dec))*gamma-cos(*ra)*tan(*dec)*epsilon;
   b = sin(e)*cos(*ra)*gamma+sin(*ra)*epsilon;

   *ra = *ra + dtr(a);
   *ra = angle(sin(*ra), cos(*ra));
   *dec = *dec + dtr(b);
}


/***************************************************************************/
void
sunLocator(double latitude, double longitude, long month, long day,
           long year, long hour, long mins, long sec, int zone,
           double *altitude, double *azimuth, double *solar_time)
{
   double jdate, jtime;
   double sunra, sundec, sunrv;
   double p[3][3]; /* Precession matrix */
   double earth[3]; /* Earthly ecliptic coordinates */
   double v_earth[3]; /* Earthly ecliptic velocity */
   double h_earth[3]; /* Earthly Heliocentric coordinates */
   double object[3];  /* Solar ecliptic coordinates */
   double v_object[3]; /* Solar ecliptic velocity */
   double h_object[3]; /* Solar heliocentric coordinates */
   double tau;

   jtime = (hour+mins/60.0+sec/3600.0)/24.0;
   jdate = gregorian2julian(month,day,year) + jtime;
   ecliptic(earth,v_earth,jdate);
   barycentric(earth,v_earth,jdate);
   topocentric(earth, v_earth, longitude, latitude, jdate, jtime);

 /* Compute the heliocentric coordinates */
   h_earth[0] = earth[0];
   h_earth[1] = earth[1]*cos(dtr(ee))-earth[2]*sin(dtr(ee));
   h_earth[2] = earth[2]*cos(dtr(ee))+earth[1]*sin(dtr(ee));
   precession(p, jdate);
   rotate(h_earth,p);
   object[0] = object[1] = object[2] = 0.0;
   v_object[0] = v_object[1] = v_object[2] = 0.0;
   sunrv = distance(earth,object); /* distance between earth and Sun */

  /* Correction for aberration */
   tau = sunrv / 173.1421288; /* A.U per julian day (speed of light) */
   object[0] = (v_earth[0] - v_object[0])*tau + object[0];
   object[1] = (v_earth[1] - v_object[1])*tau + object[1];
   object[2] = (v_earth[2] - v_object[2])*tau + object[2];

 /* Compute heliocentric coordinates for the sun */
   h_object[0] = object[0];
   h_object[1] = object[1]*cos(dtr(ee))-object[2]*sin(dtr(ee));
   h_object[2] = object[2]*cos(dtr(ee))+object[1]*sin(dtr(ee));
   rotate(h_object, p);

  /* Compute right ascension and declination */
   sundec = asin((h_object[2]-h_earth[2])/distance(h_object,h_earth));
   sunra = angle(h_object[1]-h_earth[1],h_object[0]-h_earth[0]);
   nutation(&sunra, &sundec, jdate);
   altaz(azimuth, altitude, solar_time, sunra, sundec, longitude, latitude,
         jdate,jtime);
}

//////////////////////////////////////////////////////////////////////////////
//JH additions for sun locator system
//////////////////////////////////////////////////////////////////////////////

static interpJulianStruct fusetime(SYSTEMTIME& tstruct);
static double date_to_days(SYSTEMTIME& t);
static double time_to_days(SYSTEMTIME& t);
static void fracturetime(interpJulianStruct time, SYSTEMTIME& t);
static void splittime( double fod, SYSTEMTIME& t);
static int time_to_secs(SYSTEMTIME& t);
static uTimevect secs_to_hms( int secs);
static uTimevect decday_to_hms( double fod);
static uTimevect addvect(uTimevect v1, uTimevect v2);

interpJulianStruct fusetime(SYSTEMTIME& tstruct){
	interpJulianStruct result;
	result.days =  date_to_days(tstruct) ;
	result.subday =  time_to_secs(tstruct);
	return result;
}

uTimevect addvect(uTimevect v1 ,uTimevect v2){
	uTimevect result;
	result.i = v1.i + v2.i;
	result.j = v1.j + v2.j;
	result.k = v1.k + v2.k;
	return result;
}



double date_to_days(SYSTEMTIME& t){
	long month, day, year;
	month = t.wMonth;
	day = t.wDay;
	year = t.wYear;
	double d = gregorian2julian(month,day,year) - EPOCH;
	assert(floor(d)==ceil(d));// no decimal part
	return d;
}

double time_to_days(SYSTEMTIME& t){
	double dec = (t.wHour + t.wMinute/60.0 + t.wSecond/3600.0)/24.0;
	assert(dec>=0.0 && dec<1.0); // 0.0<= dec <1.0
	return dec;
}

int time_to_secs(SYSTEMTIME& t){
	int secs = (t.wHour * 3600 + t.wMinute*60 + t.wSecond);
	assert(secs>=0 && secs<86401); 
	return secs;
}


void fracturetime(interpJulianStruct time, SYSTEMTIME& t){
	int month,day,year,xtradays,xtramin,xtrahrs;
	// Here we round the time to the nearest minute, because
	// the seconds are truncated below (hms.k = 0). If you
	// decide to keep the seconds, change the rounding value
	// to 0.5 / (24.0 * 60.0 * 60.0).
	double days = time.days + 0.5 / (24.0 * 60.0);
	long wholepart ;
	double decpart = 0.0;

	wholepart = (long) floor(days);
	decpart = days - wholepart;
    // Assert that the parts are within .001 seconds of the original.
	assert(fabs(wholepart + decpart - days) < (.001 / (24.0 * 60.0 * 60.0)));
	
	double jdate = wholepart + EPOCH + time.epoch;
	
	julian2gregorian(jdate, &month, &day, &year);
	
	uTimevect hms = decday_to_hms(decpart);
	// Change the rounding if you change the next line.
	hms.k = 0; //don't trust the low order bits.
	uTimevect hms2 = addvect(hms, secs_to_hms(time.subday));

	xtramin = hms2.k/60;
	if(xtramin > 0) {
		hms2.k -= xtramin*60;
		hms2.j += xtramin;
	}

	xtrahrs = hms2.j/60;
	if(xtrahrs > 0) {
		hms2.j -= xtrahrs*60;
		hms2.i += xtrahrs;
	}

	xtradays = hms2.i/24;
	if(xtradays > 0) hms2.i -= xtradays*24;
		
	t.wHour = hms2.i;
	t.wMinute = hms2.j;
	t.wSecond = hms2.k;

	// bug fix
	day += xtradays;

		//how many days in this month
	int modays;
	int leap = isleap(year);
	if (month == 12) modays = 31;
	else modays = mdays[leap][month]-mdays[leap][month-1];

	if (day> modays) {
		day = day - modays;
		month ++;
	}
	if (month> 12) {
		month = month - 12;
		year ++;
	}
	t.wDay = (unsigned short) day;
	//end fix

	t.wMonth = (unsigned short) month;
//	t.wDay = (unsigned short) day + xtradays; uncomment to back out fix
	t.wYear	= (unsigned short) year;

}

uTimevect decday_to_hms( double fod){
//convert decimal days to hours minutes seconds
	uTimevect result;

	assert(fod>=0.0 && fod<1.0); // 0.0<= fod <1.0 fraction of day
	result.i = (unsigned short) floor(24.0 * fod);

	fod -= result.i / 24.0f; //fraction of hour remains
	result.j = (unsigned short) floor(24.0f * 60.0f * fod);

	fod -= result.j / (24.0f * 60.0f); //fraction of minute
	result.k = (unsigned short) floor(24.0f * 3600.0f * fod);


#ifdef DEBUG
	FILE *stream;
	stream=fopen("round.log","a");
  	fprintf(stream, "fod: %f\n", fod );
	fprintf(stream, "Hour: %d\tMin: %d\tSec: %d\n\n\n", result.i,result.j, result.k );
	fclose(stream);
#endif


	return result;
}

uTimevect secs_to_hms( int secs){
//convert seconds to hours minutes seconds
	uTimevect result;

//	assert(secs >= 0 && secs <= SECS_PER_DAY); 
	result.i =  secs / 3600;

	secs -= result.i*3600; //hours removed
	result.j = secs/60;

	secs -= result.j*60; //mins removed
	result.k = secs;

	return result;
}
