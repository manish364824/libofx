/***************************************************************************
         ofx_connect.cpp 
                             -------------------
    copyright            : (C) 2005 by Ace Jones
    email                : acejones@users.sourceforge.net
***************************************************************************/
/**@file
 * \brief Code for ofxconnect utility.  C++ example code
 *
 * the purpose of the ofxconnect utility is to server as example code for
 * ALL functions of libOFX that have to do with creating OFX files.
 *
 * ofxconnect prints to stdout the created OFX file based on the options
 * you pass it
 *
 * currently it will only create the statement request file.  you can POST
 * this to an OFX server to request a statement from that financial
 * institution for that account.
 *
 * In the hopefully-not-to-distant future, ofxconnect will also make the
 * connection to the OFX server, post the data, and call ofxdump itself.
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>
#include <string>
#include "libofx.h"
#include <config.h>		/* Include config constants, e.g., VERSION TF */
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cmdline.h" /* Gengetopt generated parser */

using namespace std;

bool post(const char* request, const char* url, const char* filename)
{
  CURL *curl = curl_easy_init();
  if(! curl)
    return false;

  unlink("tmpout");  
  FILE* file = fopen(filename,"wb");
  if (! file )
  {
    curl_easy_cleanup(curl);
    return false;
  }
    
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request);

  struct curl_slist *headerlist=NULL;
  headerlist=curl_slist_append(headerlist, "Content-type: application/x-ofx");
  headerlist=curl_slist_append(headerlist, "Accept: */*, application/x-ofx");    
  
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)file);
    
  CURLcode res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl_slist_free_all (headerlist);
  
  fclose(file);
  
  return true;
}

int main (int argc, char *argv[])
{
  gengetopt_args_info args_info;
  
  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit(1) ;

  if ( argc == 1 )
  {
    cmdline_parser_print_help();
    exit(1);
  }

  if ( args_info.inputs_num > 0 )
  {
    cout << "file " << args_info.inputs[0] << endl;
  }
  else
  {
    cerr << "ERROR: You must specify an output file" << endl;
  }
  
  if ( args_info.statement_req_flag )
  {
    cerr << "Statement request" << endl;
    
    OfxFiLogin fi;
    memset(&fi,0,sizeof(OfxFiLogin));
    OfxAccountInfo account;
    memset(&account,0,sizeof(OfxAccountInfo));
    
    bool ok = true;
    if ( args_info.fid_given )
    {
      cerr << "fid " <<  args_info.fid_arg << endl;  
      strncpy(fi.fid,args_info.fid_arg,OFX_FID_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --fid is required for a statement request" << endl;
      ok = false;
    }
    
    if ( args_info.org_given )
    {
      cerr << "org " << args_info.org_arg << endl;  
      strncpy(fi.org,args_info.org_arg,OFX_ORG_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --org is required for a statement request" << endl;
      ok = false;
    }
    
    if ( args_info.bank_given )
    {
      cerr << "bank " << args_info.bank_arg << endl;  
      strncpy(fi.bankid,args_info.bank_arg,OFX_BANKID_LENGTH-1);
    }
    else    
    {
      if ( args_info.type_given && args_info.type_arg == 1 )
      {
        cerr << "ERROR: --bank is required for a bank request" << endl;
        ok = false;
      }
    }
    
    if ( args_info.broker_given )
    {
      cerr << "broker " << args_info.broker_arg << endl;  
      strncpy(fi.brokerid,args_info.broker_arg,OFX_BROKERID_LENGTH-1);
    }
    else
    {
      if ( args_info.type_given && args_info.type_arg == 2 )
      {
        cerr << "ERROR: --broker is required for an investment statement request" << endl;
        ok = false;
      }
    }
    
    if ( args_info.user_given )
    {
      cerr << "user " << args_info.user_arg << endl;  
      strncpy(fi.userid,args_info.user_arg,OFX_USERID_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --user is required for a statement request" << endl;
      ok = false;
    }
    
    if ( args_info.pass_given )
    {
      cerr << "pass " << args_info.pass_arg << endl;  
      strncpy(fi.userpass,args_info.pass_arg,OFX_USERPASS_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --pass is required for a statement request" << endl;
      ok = false;
    }
    
    if ( args_info.acct_given )
    {
      cerr << "acct " << args_info.acct_arg << endl;  
      strncpy(account.accountid,args_info.acct_arg,OFX_ACCOUNT_ID_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --acct is required for a statement request" << endl;
      ok = false;
    }
    
    if ( args_info.type_given )
    {
      cerr << "type " << args_info.type_arg << endl;  
      account.type = static_cast<AccountType>(args_info.type_arg);
    }
    else
    {
      cerr << "ERROR: --type is required for a statement request" << endl;
      ok = false;
    }
    
    if ( args_info.past_given )
    {
      cerr << "past " << args_info.past_arg << endl;  
    }
    else
    {
      cerr << "ERROR: --past is required for a statement request" << endl;
      ok = false;
    }
    
    if ( ok )
    {
      char* request = libofx_request_statement( &fi, &account, time(NULL) - args_info.past_arg * 86400L );
    
      if ( args_info.url_given )
        post(request,args_info.url_arg,args_info.inputs[0]);
      else
        cout << request;
      
      free(request);
    }
  }

  if ( args_info.accountinfo_req_flag )
  {
    OfxFiLogin fi;
    memset(&fi,0,sizeof(OfxFiLogin));

    bool ok = true;
    if ( args_info.fid_given )
    {
      cerr << "fid " <<  args_info.fid_arg << endl;  
      strncpy(fi.fid,args_info.fid_arg,OFX_FID_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --fid is required for an account info request" << endl;
      ok = false;
    }
    if ( args_info.org_given )
    {
      cerr << "org " << args_info.org_arg << endl;  
      strncpy(fi.org,args_info.org_arg,OFX_ORG_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --org is required for an account info request" << endl;
      ok = false;
    }
    if ( args_info.user_given )
    {
      cerr << "user " << args_info.user_arg << endl;  
      strncpy(fi.userid,args_info.user_arg,OFX_USERID_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --user is required for an account info request" << endl;
      ok = false;
    }
    if ( args_info.pass_given )
    {
      cerr << "pass " << args_info.pass_arg << endl;  
      strncpy(fi.userpass,args_info.pass_arg,OFX_USERPASS_LENGTH-1);
    }
    else
    {
      cerr << "ERROR: --pass is required for an account info request" << endl;
      ok = false;
    }
    
    if ( ok )
    {
      char* request = libofx_request_accountinfo( &fi );
    
      if ( args_info.url_given )
        post(request,args_info.url_arg,args_info.inputs[0]);
      else
        cout << request;
    
      free(request);
    }
  }
        
  return 0;
}


// vim:cin:si:ai:et:ts=2:sw=2:

