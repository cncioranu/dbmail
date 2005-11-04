/*
 $Id$

 Copyright (C) 1999-2004 IC & S  dbmail@ic-s.nl

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/** 
 * \file dbmailtypes.h
 *
 * all data type definitions used within the dbmail package should 
 * be declared here.
 *
 */

#ifndef _DBMAILTYPES_H
#define _DBMAILTYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "memblock.h"
#include "list.h"

/** max length of search query */
#define MAX_SEARCH_LEN 1024

#define MIME_FIELD_MAX 128
#define MIME_VALUE_MAX 4096

#define UID_SIZE 70
#define IPNUM_LEN 32
#define IPLEN 32
#define BACKLOG 16


#define DM_SOCKADDR_LEN 108
#define DM_USERNAME_LEN 100

/** string length of configuration values */
#define FIELDSIZE 1024

/** use 64-bit unsigned integers as common data type */
typedef unsigned long long u64_t;

typedef enum {
	DM_EQUERY 	= -1,
	DM_SUCCESS 	= 0,
	DM_EGENERAL 	= 1
} DbmailErrorCodes;

/** status fields for messages */
typedef enum {
	MESSAGE_STATUS_NEW     = 0,
	MESSAGE_STATUS_SEEN    = 1,
	MESSAGE_STATUS_DELETE  = 2,
	MESSAGE_STATUS_PURGE   = 3,
	MESSAGE_STATUS_UNUSED  = 4,
	MESSAGE_STATUS_INSERT  = 5,
	MESSAGE_STATUS_ERROR   = 6
} MessageStatus_t;

/** field_t is used for storing configuration values */
typedef char field_t[FIELDSIZE];

/** size of a timestring_t field */
#define TIMESTRING_SIZE 30

/** timestring_t is used for holding timestring */
typedef char timestring_t[TIMESTRING_SIZE];

/** parameters for the database connection */
typedef struct {
	field_t host;		/**< hostname or ip address of database server */
	field_t user;		/**< username to connect with */
	field_t pass;		/**< password of user */
	field_t db;		/**< name of database to connect with */
	unsigned int port;	/**< port number of database server */
	field_t sock;		/**< path to local unix socket (local connection) */
	field_t pfx;		/**< prefix for tables e.g. dbmail_ */
	unsigned int serverid;	/* unique id for dbmail instance used in clusters */
			
} db_param_t;

/** configuration items */
typedef struct {
	field_t name;		/**< name of configuration item */
	field_t value;		/**< value of configuration item */
} item_t;

typedef struct {
	FILE *tx, *rx;
	char ip_src[IPNUM_LEN];	/* client IP-number */
	field_t clientname;	/* resolved client ip */
	char *timeoutMsg;
	int timeout;		/* server timeout (seconds) */
	void *userData;
} clientinfo_t;

typedef struct {
	int maxConnect;
	int listenSocket;
	int resolveIP;
	int timeout;
	char *timeoutMsg;
	int (*ClientHandler) (clientinfo_t *);
} ChildInfo_t;


/*
 * structures used by POP mechanism
 *
 */

/** all virtual_ definitions are session specific
 *  when a RSET occurs all will be set to the real values */
struct message {
	u64_t msize;	  /**< message size */
	u64_t messageid;  /**< messageid (from database) */
	u64_t realmessageid;
			  /**< ? */
	char uidl[UID_SIZE];
			  /**< unique id */
	/* message status :
	 * 000 message is new, never touched 
	 * 001 message is read
	 * 002 message is deleted by user 
	 * ----------------------------------
	 * The server additionally uses:
	 * 003 message is deleted by sysop
	 * 004 message is ready for final deletion */

	/** message status */
	MessageStatus_t messagestatus;
	/** virtual message status */
	MessageStatus_t virtual_messagestatus;
};

/**
 * pop3 connection states */
typedef enum {
	POP3_AUTHORIZATION_STATE,
	POP3_TRANSACTION_STATE,
	POP3_UPDATE_STATE,
} Pop3State_t;

/**
 * struct for a POP3 session.
 */
typedef struct {
	int error_count;/**< number of errors that have occured */
	Pop3State_t state; /**< current POP state */
	int was_apop;	/**< 1 if session was  session was apop (no plaintext password) */

	int SessionResult; /**< what happened during the session */

	char *username;
	char *password;

	char *apop_stamp; /**< timestamp for APOP */

	u64_t useridnr;	/**< Used by timsieved */

	u64_t totalsize;/**< total size of messages */
	u64_t virtual_totalsize;
	u64_t totalmessages; /**< number of messages */
	u64_t virtual_totalmessages;

	struct dm_list messagelst; /** list of messages */
} PopSession_t;



/*
 * define some IMAP symbols
 */
#define IMAP_NFLAGS 6

enum IMAP4_CLIENT_STATES { 
	IMAPCS_INITIAL_CONNECT,
	IMAPCS_NON_AUTHENTICATED,
	IMAPCS_AUTHENTICATED, 
	IMAPCS_SELECTED, 
	IMAPCS_LOGOUT
};

enum IMAP4_FLAGS { 
	IMAPFLAG_SEEN		= 0x01, 
	IMAPFLAG_ANSWERED	= 0x02,
	IMAPFLAG_DELETED	= 0x04, 
	IMAPFLAG_FLAGGED	= 0x08,
	IMAPFLAG_DRAFT		= 0x10, 
	IMAPFLAG_RECENT		= 0x20
};

enum IMAP4_PERMISSION { 
	IMAPPERM_READ		= 0x01,
	IMAPPERM_READWRITE	= 0x02 
};

enum IMAP4_FLAG_ACTIONS { 
	IMAPFA_NONE, 
	IMAPFA_REPLACE, 
	IMAPFA_ADD,
	IMAPFA_REMOVE
};

enum BODY_FETCH_ITEM_TYPES { 
	BFIT_TEXT, 
	BFIT_HEADER, 
	BFIT_MIME,
	BFIT_HEADER_FIELDS,
	BFIT_HEADER_FIELDS_NOT, 
	BFIT_TEXT_SILENT
};

/* maximum size of a mailbox name */
#define IMAP_MAX_MAILBOX_NAMELEN 100

/* length of internaldate string */
#define IMAP_INTERNALDATE_LEN 34

/* length of database date string 
   YYYY-MM-DD HH:MM:SS
   1234567890123456789 */
#define SQL_INTERNALDATE_LEN 19

/* max length of number/dots part specifier */
#define IMAP_MAX_PARTSPEC_LEN 100


typedef struct {
	int listenSocket;
	int startChildren;
	int minSpareChildren;
	int maxSpareChildren;
	int maxChildren;
	int childMaxConnect;
	int timeout;
	char ip[IPLEN];
	int port;
	int resolveIP;
	char *timeoutMsg;
	field_t serverUser, serverGroup;
	field_t socket;
	int (*ClientHandler) (clientinfo_t *);
} serverConfig_t;

/* 
 * (imap) mailbox data type
 */
typedef struct {
	u64_t uid, msguidnext, owner_idnr;
	char *name;
	unsigned no_select, no_inferiors, exists, recent, unseen;
	unsigned flags;
	int permission;
	u64_t *seq_list;
} mailbox_t;


/*
 * search data types
 */

enum IMAP_SEARCH_TYPES { 
	IST_SET, 		/* 0 */
	IST_SET_UID, 		/* 1 */
	IST_FLAG,  		/* 2 */
	IST_SORT,  		/* 3 */
	IST_SORTHDR,  		/* 4 */
	IST_SORT_FLD,		/* 5 */
	IST_HDR,  		/* 6 */
	IST_HDRDATE_BEFORE,  	/* 7 */
	IST_HDRDATE_ON,  	/* 8 */
	IST_HDRDATE_SINCE, 	/* 9 */
	IST_IDATE,  		/* 10 */
	IST_DATA_BODY,  	/* 11 */
	IST_DATA_TEXT, 		/* 12 */
	IST_SIZE_LARGER,  	/* 13 */
	IST_SIZE_SMALLER,  	/* 14 */
	IST_SUBSEARCH_AND, 	/* 15 */
	IST_SUBSEARCH_OR,  	/* 16 */
	IST_SUBSEARCH_NOT 	/* 17 */
};

typedef struct {
	int type;
	u64_t size;
	char table[MAX_SEARCH_LEN];
	char order[MAX_SEARCH_LEN];
	char field[MAX_SEARCH_LEN];
	char search[MAX_SEARCH_LEN];
	
	char hdrfld[MIME_FIELD_MAX];
	struct dm_list sub_search;
	int match;
	GTree *found;
	gboolean reverse;
} search_key_t;

/**
 * remembering database positions for mail
 */
typedef struct {
	u64_t block, pos;
} db_pos_t;

/* 
 * simple cache mechanism
 */
typedef struct {
	struct DbmailMessage *dmsg;
	MEM *memdump, *tmpdump;
	u64_t num;
	int file_dumped, msg_parsed;
	u64_t dumpsize;
} cache_t;


/*
 * structure for basic message info 
 * so it can be retrieved at once
 */
typedef struct {
	int flags[IMAP_NFLAGS];
	char internaldate[IMAP_INTERNALDATE_LEN];
	u64_t rfcsize, uid;
} msginfo_t;

/*
 * A struct to hold info about a Sieve script
 */
typedef struct ssinfo {
	char *name;
	int active;
} sievescript_info_t;

/* messageblk types */
typedef enum {
	BODY_BLOCK = 0,
	HEAD_BLOCK = 1
} blocktype_t;

/*
 * A struct to say which Sieve allocations
 * will need an associated free.
 */
typedef struct {
	int free_interp  : 1; // t
	int free_script  : 1; // s
	int free_support : 1; // p
	int free_error   : 1; // e
	int free_message : 1; // m
	int free_action  : 1; // a
} sievefree_t;


#define SA_KEEP		1
#define SA_DISCARD	2
#define SA_REDIRECT	3
#define SA_REJECT	4
#define SA_FILEINTO	5

typedef struct sort_action {
	int method;
	char *destination;
	char *message;
} sort_action_t;

typedef enum {
	ACL_RIGHT_LOOKUP,
	ACL_RIGHT_READ,
	ACL_RIGHT_SEEN,
	ACL_RIGHT_WRITE,
	ACL_RIGHT_INSERT,
	ACL_RIGHT_POST,
	ACL_RIGHT_CREATE,
	ACL_RIGHT_DELETE,
	ACL_RIGHT_ADMINISTER,
	ACL_RIGHT_NONE
} ACLRight_t;


#endif
