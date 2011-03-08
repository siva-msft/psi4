#ifndef PW91_C_functional_h
#define PW91_C_functional_h
/**********************************************************
* PW91_C_functional.h: declarations for PW91_C_functional for KS-DFT
* Robert Parrish, robparrish@gmail.com
* Autogenerated by MATLAB Script on 27-Oct-2010
*
***********************************************************/
#include <libmints/properties.h>
#include <libciomr/libciomr.h>
#include "functional.h"
#include <stdlib.h>
#include <string>
#include <vector>

using namespace psi;
using namespace boost;
using namespace std;

namespace psi { namespace functional {

class PW91_C_Functional : public Functional {
public:
    PW91_C_Functional(int npoints, int deriv);
    virtual ~PW91_C_Functional();
    virtual void computeRKSFunctional(shared_ptr<Properties> prop);
    virtual void computeUKSFunctional(shared_ptr<Properties> prop);
};
}}
#endif
