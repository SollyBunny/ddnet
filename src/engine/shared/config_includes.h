// This file can be included several times.

#ifndef SET_CONFIG_DOMAIN
#error "SET_CONFIG_DOMAIN macro not defined"
#define SET_CONFIG_DOMAIN(CONFIGDOMAIN) ;
#endif

SET_CONFIG_DOMAIN(ConfigDomain::DDNET)
#include "config_variables.h"

SET_CONFIG_DOMAIN(ConfigDomain::TCLIENT)
#include "config_variables_tclient.h"

SET_CONFIG_DOMAIN(ConfigDomain::SOLLYS)
#include "config_variables_sollys.h"

SET_CONFIG_DOMAIN(ConfigDomain::INSTA)
#include "config_variables_insta.h"
