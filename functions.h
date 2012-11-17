#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

//fixed point calculation functions exported

//root basis functions
extern float square(float x);
extern float irt(float x);
extern float inv(float x);
extern float sqrt(float x);

//logarithm group of functions
extern float log(float x);
extern float halfa(float x);
extern float atan(float x);
extern float circ(float x);

//exponential group of functions
extern float exp(float x);//exponential function
extern float qfn(float x);//my q function
extern float invw(float x);//inverse lambert W function
extern float ein(float x);//the natural origin Ei function

//other group of functions
extern float lin(float x);//the natural origin Li function (primes less than estimate)

//hidden group of functions
extern float halfs(float x);
extern float halfc(float x);
extern float asin(float x);
extern float acos(float x);
extern float sin(float x);
extern float cos(float x);
extern float tan(float x);
extern float entropy(float x);

struct token
{
	u8 *name;
	float (*ptr)(float);
};
extern struct token tokens[];

#endif                          /* FUNCTIONS_H_ */
