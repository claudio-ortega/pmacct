/*  
    pmacct (Promiscuous mode IP Accounting package)
    pmacct is Copyright (C) 2003-2018 by Paolo Lucente
*/

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* defines */
#define __RPKI_C

/* includes */
#include "pmacct.h"
#include "bgp/bgp.h"
#include "rpki.h"

int rpki_roas_map_load(char *file, int type)
{
  struct bgp_misc_structs *bms;

  bms = bgp_select_misc_db(type);
  if (!bms) return ERR;

#if defined WITH_JANSSON
  {
    json_t *roas_obj, *roa_json, *roas_json;
    json_error_t file_err;
    int roas_idx;

    roas_obj = json_load_file(file, 0, &file_err);

    if (roas_obj) {
      if (!json_is_object(roas_obj)) {
        Log(LOG_ERR, "ERROR ( %s/%s ): [%s] json_is_object() failed for results: %s\n", config.name, bms->log_str, file, file_err.text);
        exit_gracefully(1);
      }
      else {
        roas_json = json_object_get(roas_obj, "roas");
        if (roas_json == NULL || !json_is_array(roas_json)) {
          Log(LOG_ERR, "ERROR ( %s/%s ): [%s] no 'roas' element or not an array.\n", config.name, bms->log_str, file);
          exit_gracefully(1);
        }
        else {
	  for (roas_idx = 0; roa_json = json_array_get(roas_json, roas_idx); roas_idx++) {
	    json_t *prefix_json, *maxlen_json, *asn_json;
	    struct prefix p;
	    u_int8_t maxlen;
	    as_t asn;
	    
	    prefix_json = json_object_get(roa_json, "prefix");
	    if (prefix_json == NULL || !json_is_string(prefix_json)) {
	      Log(LOG_WARNING, "WARN ( %s/%s ): [%s] no 'prefix' element in ROA #%u.\n", config.name, bms->log_str, file, (roas_idx + 1));
	      goto exit_lane;
	    }
	    else {
	      str2prefix(json_string_value(prefix_json), &p);
	      json_decref(prefix_json);
	    }

	    asn_json = json_object_get(roa_json, "asn");
	    if (asn_json == NULL || !json_is_string(asn_json)) {
	      Log(LOG_WARNING, "WARN ( %s/%s ): [%s] no 'asn' element in ROA #%u.\n", config.name, bms->log_str, file, (roas_idx + 1));
	      goto exit_lane;
	    }
	    else {
	      asn = str2asn((char *)json_string_value(asn_json));
	      json_decref(asn_json);
	    }

	    maxlen_json = json_object_get(roa_json, "maxLength");
	    if (maxlen_json == NULL || !json_is_integer(maxlen_json)) {
	      Log(LOG_WARNING, "WARN ( %s/%s ): [%s] no 'maxLength' element in ROA #%u.\n", config.name, bms->log_str, file, (roas_idx + 1));
	      goto exit_lane;
	    }
	    else {
	      maxlen = json_integer_value(maxlen_json);
	      json_decref(maxlen_json);
	    }

	    exit_lane:
	    json_decref(roa_json);
	  }
	}

	json_decref(roas_json);
	json_decref(roas_obj);
      }
    }
    else {
      Log(LOG_ERR, "ERROR ( %s/%s ): [%s] json_loads() failed: %s.\n", config.name, bms->log_str, file, file_err.text);
      exit_gracefully(1);
    }
  }
#else
  Log(LOG_WARNING, "WARN ( %s/%s ): rpki_roas_map will not load (missing --enable-jansson).\n", config.name, bms->log_str);
#endif

  return SUCCESS;
}
