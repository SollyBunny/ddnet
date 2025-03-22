// This file can be included several times.

#ifndef SET_CONFIG_DOMAIN
#error "SET_CONFIG_DOMAIN macro not defined"
#define SET_CONFIG_DOMAIN(ConfigDomain) ;
#endif

SET_CONFIG_DOMAIN(CONFIGDOMAIN::DDNET)
#include "config_variables.h"

SET_CONFIG_DOMAIN(CONFIGDOMAIN::SOLLY)
#include "config_variables_solly.h"

SET_CONFIG_DOMAIN(CONFIGDOMAIN::INF)
#include "config_variables_inf.h"

SET_CONFIG_DOMAIN(CONFIGDOMAIN::TATER)
#include "config_variables_tater.h"

SET_CONFIG_DOMAIN(CONFIGDOMAIN::INSTA)
#include "config_variables_insta.h"
