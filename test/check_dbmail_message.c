/*
 *   Copyright (c) 2004-2013 NFG Net Facilities Group BV support@nfg.nl
 *   Copyright (c) 2014-2019 Paul J Stevens, The Netherlands, support@nfg.nl
 *   Copyright (c) 2020-2024 Alan Hicks, Persistent Objects Ltd support@p-o.co.uk
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *   
 *
 *
 *  
 *
 *   Basic unit-test framework for dbmail (www.dbmail.org)
 *
 *   See http://check.sf.net for details and docs.
 *
 *
 *   Run 'make check' to see some action.
 *
 */ 

#include <check.h>
#include "check_dbmail.h"

extern char configFile[PATH_MAX];
extern char *multipart_message;
extern char *multipart_message_part;
extern char *raw_lmtp_data;


/*
 *
 * the test fixtures
 *
 */

	
void setup(void)
{
	config_get_file();
	config_read(configFile);
	configure_debug(NULL,511,0);
	GetDBParams();
	db_connect();
	auth_connect();
}

void teardown(void)
{
	db_disconnect();
	auth_disconnect();
	config_free();
}

//DbmailMessage * dbmail_message_new(void);
START_TEST(test_dbmail_message_new)
{
	DbmailMessage *m = dbmail_message_new(NULL);
	fail_unless(m->id==0,"dbmail_message_new failed");
	dbmail_message_free(m);
}
END_TEST
//void dbmail_message_set_class(DbmailMessage *self, int klass);
START_TEST(test_dbmail_message_set_class)
{
	int result;
	DbmailMessage *m = dbmail_message_new(NULL);
	result = dbmail_message_set_class(m,DBMAIL_MESSAGE);
	fail_unless(result==0,"dbmail_message_set_class failed");
	result = dbmail_message_set_class(m,DBMAIL_MESSAGE_PART);
	fail_unless(result==0,"dbmail_message_set_class failed");
	result = dbmail_message_set_class(m,10);
	fail_unless(result!=0,"dbmail_message_set_class failed");
	
	dbmail_message_free(m);
}
END_TEST
//int dbmail_message_get_class(DbmailMessage *self);
START_TEST(test_dbmail_message_get_class)
{
	int klass;
	DbmailMessage *m = dbmail_message_new(NULL);
	
	/* default */
	klass = dbmail_message_get_class(m);
	fail_unless(klass==DBMAIL_MESSAGE,"dbmail_message_get_class failed");
	
	/* set explicitely */
	dbmail_message_set_class(m,DBMAIL_MESSAGE_PART);
	klass = dbmail_message_get_class(m);
	fail_unless(klass==DBMAIL_MESSAGE_PART,"dbmail_message_get_class failed");
	
	dbmail_message_set_class(m,DBMAIL_MESSAGE);
	klass = dbmail_message_get_class(m);
	fail_unless(klass==DBMAIL_MESSAGE,"dbmail_message_get_class failed");

	dbmail_message_free(m);
}
END_TEST

FILE *i;

static DbmailMessage  * message_init(const char *message)
{
	DbmailMessage *m;

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, message);

	//fail_unless(m != NULL, "dbmail_message_init_with_string failed");

	return m;

}
static char *store_and_retrieve(DbmailMessage *m)
{
	uint64_t physid;
	DbmailMessage *n;
	char *t;

	dbmail_message_store(m);
	physid = dbmail_message_get_physid(m);
	fail_unless(physid != 0,"dbmail_message_store failed. physid [%" PRIu64 "]", physid);
	dbmail_message_free(m);

	n = dbmail_message_new(NULL);
	dbmail_message_set_physid(n, physid);
	n = dbmail_message_retrieve(n,physid);
	fail_unless(n != NULL, "_mime_retrieve failed");
	
	t = dbmail_message_to_string(n);
	dbmail_message_free(n);
	return t;
}

START_TEST(test_g_mime_object_get_body)
{
	char * result;
	DbmailMessage *m;

	m = message_init(multipart_message);
	
	result = g_mime_object_get_body(GMIME_OBJECT(m->content));
	fail_unless(strlen(result)==1057,"g_mime_object_get_body failed [%lu:%s]\n", strlen(result), result);
	g_free(result);
	dbmail_message_free(m);
	
	m = message_init(rfc822);
	result = g_mime_object_get_body(GMIME_OBJECT(m->content));
	ck_assert_str_eq("\n    this is a test message\n\n", result);
	g_free(result);
	dbmail_message_free(m);
}
END_TEST


START_TEST(test_dbmail_message_store)
{
	DbmailMessage *m;
	char *t, *e;
	//-----------------------------------------
	m = message_init("From: paul\n");
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);

	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(simple);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(rfc822);
	e = dbmail_message_to_string(m);

	t = store_and_retrieve(m);
	// trim junk from rfc822
	strncpy(e, rfc822+37, strlen(rfc822)-37);
	m = message_init(e);
	e = dbmail_message_to_string(m);
	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message2);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	// trim junk from multipart_message2
	strncpy(e, multipart_message2+56, strlen(multipart_message2)-56);
	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(message_rfc822);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(message_rfc822, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message3);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message3, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message4);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message4, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message5);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message5, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message6);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message6, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message7);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message7, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message8);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(multipart_message8, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message9);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message9, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_mixed);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_mixed, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(broken_message);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	// remove non message preamble
	strncpy(e, broken_message+42, strlen(broken_message)-42);
	// add closing boundary
	strcat(e, "--=-i5BOOWGh5HearcweMC39--\n");
	ck_assert_str_eq(e, t);
	// broken message is fixed so not equal
	// ck_assert_str_eq(broken_message, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(encoded_message_latin_1);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(encoded_message_latin_1, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(encoded_message_latin_2);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(encoded_message_latin_2, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(encoded_message_utf8);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(encoded_message_utf8, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(encoded_message_utf8_1);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(encoded_message_utf8_1, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(encoded_message_utf8_2);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(encoded_message_utf8_2, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(encoded_message_koi);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(encoded_message_koi, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(raw_message_koi);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(raw_message_koi, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_alternative);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	// remove non message preamble
	strncpy(e, multipart_alternative+42, strlen(multipart_alternative)-42);
	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(outlook_multipart);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	strncpy(e, outlook_multipart+65, strlen(outlook_multipart)-65);
	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_alternative2);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_alternative2, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_apple);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_apple, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(long_quopri_subject);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(long_quopri_subject, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message_big);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message_big, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_digest);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	// TODO
	// https://github.com/dbmail/dbmail/issues/324
	// ck_assert_str_eq(e, t);
	// ck_assert_str_eq(multipart_digest, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_alternative3);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_alternative3, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_signed);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	g_free(e);
	g_free(t);

	//-----------------------------------------
	m = message_init(multipart_message_submessage);
	e = dbmail_message_to_string(m);
	t = store_and_retrieve(m);
	ck_assert_str_eq(e, t);
	ck_assert_str_eq(multipart_message_submessage, t);
	g_free(e);
	g_free(t);
}
END_TEST

START_TEST(test_dbmail_message_store2)
{
	DbmailMessage *m, *n;
	uint64_t physid;
	char *t;
	char *expect;

	m = message_init(broken_message2);
	
	dbmail_message_set_header(m, "Return-Path", dbmail_message_get_header(m, "From"));
	
	expect = dbmail_message_to_string(m);
	fail_unless(m != NULL, "dbmail_message_store2 failed");

	dbmail_message_store(m);
	physid = dbmail_message_get_physid(m);
	dbmail_message_free(m);

	n = dbmail_message_new(NULL);
	dbmail_message_set_physid(n, physid);
	n = dbmail_message_retrieve(n,physid);
	fail_unless(n != NULL, "_mime_retrieve failed");
	
	t = dbmail_message_to_string(n);
	
	ck_assert_str_eq(expect,t);
	
	dbmail_message_free(n);
	g_free(expect);
	g_free(t);
	
}
END_TEST


//DbmailMessage * dbmail_message_retrieve(DbmailMessage *self, uint64_t physid, int filter);
START_TEST(test_dbmail_message_retrieve)
{
	DbmailMessage *m, *n;
	uint64_t physid;

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, multipart_message);
	fail_unless(m != NULL, "dbmail_message_init_with_string failed");

	dbmail_message_set_header(m, 
			"References", 
			"<20050326155326.1afb0377@ibook.linuks.mine.nu> <20050326181954.GB17389@khazad-dum.debian.net> <20050326193756.77747928@ibook.linuks.mine.nu> ");
	dbmail_message_store(m);

	physid = dbmail_message_get_physid(m);
	fail_unless(physid > 0, "dbmail_message_get_physid failed");
	
	n = dbmail_message_new(NULL);
	n = dbmail_message_retrieve(n,physid);	
	fail_unless(n != NULL, "dbmail_message_retrieve failed");
	fail_unless(n->content != NULL, "dbmail_message_retrieve failed");

	dbmail_message_free(m);
	dbmail_message_free(n);

}
END_TEST
//DbmailMessage * dbmail_message_init_with_string(DbmailMessage *self, const GString *content);
START_TEST(test_dbmail_message_init_with_string)
{
	DbmailMessage *m;
	GList *t;
	
	m = message_init(multipart_message);
	
	t = dbmail_message_get_header_repeated(m, "Received");
	fail_unless(g_list_length(t)==3,"Too few or too many headers in tuple [%d]\n", 
			g_list_length(t));
	dbmail_message_free(m);
}
END_TEST

START_TEST(test_dbmail_message_get_internal_date)
{
	DbmailMessage *m;

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, rfc822);
	// From_ contains: Wed Sep 14 16:47:48 2005
	const char *expect = "2005-09-14 16:47:48";
	const char *expect03 = "2003-09-14 16:47:48";
	const char *expect75 = "1975-09-14 16:47:48";
	const char *expect10 = "2010-05-28 18:10:18";
	char *result;

	/* baseline */
	result = dbmail_message_get_internal_date(m, 0);
	fail_unless(MATCH(expect,result),"dbmail_message_get_internal_date failed exp [%s] got [%s]", expect, result);
	g_free(result);

	/* should be the same */
	result = dbmail_message_get_internal_date(m, 2007);
	fail_unless(MATCH(expect,result),"dbmail_message_get_internal_date failed exp [%s] got [%s]", expect, result);
	g_free(result);

	/* capped to 2004, which should also be the same  */
	result = dbmail_message_get_internal_date(m, 2004);
	fail_unless(MATCH(expect,result),"dbmail_message_get_internal_date failed exp [%s] got [%s]", expect, result);
	g_free(result);

	/* capped to 2003, should be different */
	result = dbmail_message_get_internal_date(m, 2003);
	fail_unless(MATCH(expect03,result),"dbmail_message_get_internal_date failed exp [%s] got [%s]", expect03, result);
	g_free(result);

	/* capped to 1975, should be way different */
	result = dbmail_message_get_internal_date(m, 1975);
	fail_unless(MATCH(expect75,result),"dbmail_message_get_internal_date failed exp [%s] got [%s]", expect75, result);
	g_free(result);

	dbmail_message_free(m);

	// test work-around for broken envelope header
	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, simple_broken_envelope);

	result = dbmail_message_get_internal_date(m, 0);
	fail_unless(MATCH(expect10,result),"dbmail_message_get_internal_date failed exp [%s] got [%s]", expect10, result);

	char *before = dbmail_message_to_string(m);
	char *after = store_and_retrieve(m);
	ck_assert_str_eq(before, after);
	g_free(before);
	g_free(after);
}
END_TEST

START_TEST(test_dbmail_message_to_string)
{
	char *result;
	char *expect;
	DbmailMessage *m;

	m = message_init(multipart_message);
	result = dbmail_message_to_string(m);
	ck_assert_str_eq(multipart_message, result);
	g_free(result);
	dbmail_message_free(m);

	// remove non message preamble
	expect = malloc(sizeof(char) * (strlen(simple_with_from)-52));
	strncpy(expect, simple_with_from+52, strlen(simple_with_from)-52);

	m = message_init(simple_with_from);
	result = dbmail_message_to_string(m);
	ck_assert_str_eq(expect, result);
	g_free(result);
	g_free(expect);
	dbmail_message_free(m);
}
END_TEST
    
START_TEST(test_dbmail_message_hdrs_to_string)
{
	char *result;
	DbmailMessage *m;
	
	m = dbmail_message_new(NULL);
        m = dbmail_message_init_with_string(m, multipart_message);

	result = dbmail_message_hdrs_to_string(m);
	fail_unless(strlen(result)==676, "dbmail_message_hdrs_to_string failed [%lu] != [634]\n[%s]\n", strlen(result), result);
	
        dbmail_message_free(m);
	g_free(result);
}
END_TEST

//gchar * dbmail_message_body_to_string(DbmailMessage *self);

START_TEST(test_dbmail_message_body_to_string)
{
	char *result;
	DbmailMessage *m;
	
	m = dbmail_message_new(NULL);
        m = dbmail_message_init_with_string(m,multipart_message);
	result = dbmail_message_body_to_string(m);
	fail_unless(strlen(result)==1057, "dbmail_message_body_to_string failed [%lu] != [1057]\n[%s]\n", strlen(result),result);
	
        dbmail_message_free(m);
	g_free(result);

	m = dbmail_message_new(NULL);
        m = dbmail_message_init_with_string(m,outlook_multipart);
	result = dbmail_message_body_to_string(m);
	fail_unless(strlen(result)==330, "dbmail_message_body_to_string failed [330 != %lu:%s]", strlen(result), result);
	
        dbmail_message_free(m);
	g_free(result);

	
}
END_TEST

//void dbmail_message_free(DbmailMessage *self);
START_TEST(test_dbmail_message_free)
{
	DbmailMessage *m = dbmail_message_new(NULL);
	dbmail_message_free(m);
}
END_TEST

START_TEST(test_dbmail_message_set_header)
{
	char *res = NULL;
	DbmailMessage *m;
	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m,multipart_message);
	dbmail_message_set_header(m, "X-Foobar","Foo Bar");
	fail_unless(dbmail_message_get_header(m, "X-Foobar")!=NULL, "set_header failed");
	dbmail_message_free(m);
	//
	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, encoded_message_utf8);
	res = dbmail_message_to_string(m);
	fail_unless(MATCH(encoded_message_utf8, res), "to_string failed");
	g_free(res);

	dbmail_message_set_header(m, "X-Foobar", "Test");
	g_mime_object_remove_header(GMIME_OBJECT(m->content), "X-Foobar");
	res = dbmail_message_to_string(m);
	fail_unless(MATCH(encoded_message_utf8, res), "to_string failed \n[%s] != \n[%s]\n", encoded_message_utf8, res);
	g_free(res);
	dbmail_message_free(m);
	//

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, long_quopri_subject);
	res = dbmail_message_to_string(m);
	fail_unless(MATCH(long_quopri_subject, res), "to_string failed");
	g_free(res);
	
	dbmail_message_set_header(m, "X-Foo", "Test");

	res = dbmail_message_to_string(m);
	fail_unless(! MATCH(long_quopri_subject, res), "to_string failed");
	g_free(res);

	g_mime_object_remove_header(GMIME_OBJECT(m->content), "X-Foo");
	res = dbmail_message_to_string(m);
	//FIXME: fail_unless(MATCH(long_quopri_subject, res), "to_string failed \n[%s] != \n[%s]\n", long_quopri_subject, res);
	g_free(res);

	dbmail_message_free(m);
	//

}
END_TEST

START_TEST(test_dbmail_message_get_header)
{
	char *t;
	DbmailMessage *h = dbmail_message_new(NULL);
	DbmailMessage *m = dbmail_message_new(NULL);
	
	m = dbmail_message_init_with_string(m, multipart_message);
	t = dbmail_message_hdrs_to_string(m);
	h = dbmail_message_init_with_string(h, t);
	g_free(t);
	
	fail_unless(dbmail_message_get_header(m, "X-Foobar")==NULL, "get_header failed on full message");
	fail_unless(strcmp(dbmail_message_get_header(m,"Subject"),"multipart/mixed")==0,"get_header failed on full message");

	fail_unless(dbmail_message_get_header(h, "X-Foobar")==NULL, "get_header failed on headers-only message");
	fail_unless(strcmp(dbmail_message_get_header(h,"Subject"),"multipart/mixed")==0,"get_header failed on headers-only message");
	
	dbmail_message_free(m);
	dbmail_message_free(h);
}
END_TEST

START_TEST(test_dbmail_message_encoded)
{
	DbmailMessage *m = dbmail_message_new(NULL);
	uint64_t id = 0;

	m = dbmail_message_init_with_string(m, encoded_message_koi);

	ck_assert_str_eq(dbmail_message_get_header(m,"From"),"Антон Нехороших <bad@foo.ru>");

	dbmail_message_free(m);

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, utf7_header);

	dbmail_message_store(m);
	id = dbmail_message_get_physid(m);
	dbmail_message_free(m);

	m = dbmail_message_new(NULL);
	m = dbmail_message_retrieve(m, id);
	dbmail_message_free(m);
}
END_TEST

START_TEST(test_dbmail_message_8bit)
{
	DbmailMessage *m = dbmail_message_new(NULL);

	m = dbmail_message_init_with_string(m, raw_message_koi);

	dbmail_message_store(m);
	dbmail_message_free(m);

	/* */
	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, encoded_message_utf8);

	dbmail_message_store(m);
	dbmail_message_free(m);
}
END_TEST

START_TEST(test_dbmail_message_cache_headers)
{
	DbmailMessage *m = dbmail_message_new(NULL);
	char *s = g_new0(char,20);
	m = dbmail_message_init_with_string(m,multipart_message);
	dbmail_message_set_header(m, 
			"References", 
			"<20050326155326.1afb0377@ibook.linuks.mine.nu> <20050326181954.GB17389@khazad-dum.debian.net> <20050326193756.77747928@ibook.linuks.mine.nu> ");
	dbmail_message_store(m);
	dbmail_message_free(m);

	sprintf(s,"%.*s",10,"abcdefghijklmnopqrstuvwxyz");
	fail_unless(MATCH(s,"abcdefghij"),"string truncate failed");
	g_free(s);

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m,multipart_message);
	dbmail_message_set_header(m,
			"Subject",
			"=?utf-8?Q?[xxxxxxxxxxxxxxxxxx.xx_0000747]:_=C3=84nderungen_an_der_Artikel?= =?utf-8?Q?-Detailseite?="
			);
	dbmail_message_store(m);
	dbmail_message_free(m);
}
END_TEST

START_TEST(test_dbmail_message_get_header_addresses)
{
	GList * result;
	DbmailMessage *m;

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m,multipart_message);
	
	result = dbmail_message_get_header_addresses(m, "Cc");
	result = g_list_first(result);

	fail_unless(result != NULL, "dbmail_message_get_header_addresses failed");
	fail_unless(g_list_length(result)==2,"dbmail_message_get_header_addresses failed");
	fail_unless(strcmp((char *)result->data,"vol@inter7.com")==0, "dbmail_message_get_header_addresses failed");
	g_list_destroy(result);

	dbmail_message_free(m);
}
END_TEST

START_TEST(test_dbmail_message_get_header_repeated)
{
	GList *headers;
	DbmailMessage *m;

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m,multipart_message);
	
	headers = dbmail_message_get_header_repeated(m, "Received");

	fail_unless(headers != NULL, "dbmail_message_get_header_repeated failed");
	fail_unless(g_list_length(headers)==3, "dbmail_message_get_header_repeated failed");
	
	headers = dbmail_message_get_header_repeated(m, "received");

	fail_unless(headers != NULL, "dbmail_message_get_header_repeated failed");
	fail_unless(g_list_length(headers)==3, "dbmail_message_get_header_repeated failed");
	
	dbmail_message_free(m);
}
END_TEST


START_TEST(test_dbmail_message_construct)
{
	const gchar *sender = "foo@bar.org";
	const gchar *subject = "Some test";
	const gchar *recipient = "<bar@foo.org> Bar";
	gchar *body = g_strdup("testing\nține un gând");
	gchar *expect = g_strdup("Subject: Some test\n"
	"Sender: foo@bar.org\n"
	"To: <bar@foo.org> Bar\n"
	"MIME-Version: 1.0\n"
	"Content-Type: text/plain; charset=utf-8\n"
	"Content-Transfer-Encoding: base64\n"
	"\n"
	"dGVzdGluZwrIm2luZSB1biBnw6Ju\n");
	gchar *result;

	DbmailMessage *message = dbmail_message_new(NULL);
	message = dbmail_message_construct(message,recipient,sender,subject,body);
	result = dbmail_message_to_string(message);

	ck_assert_str_eq(expect, result);

	dbmail_message_free(message);
	g_free(body);
	g_free(result);
	g_free(expect);

	body = g_strdup("Mit freundlichen Gr=C3=BC=C3=9Fen");
	message = dbmail_message_new(NULL);
	message = dbmail_message_construct(message,recipient,sender,subject,body);
	//printf ("%s\n", dbmail_message_to_string(message));
	dbmail_message_free(message);
	g_free(body);
}
END_TEST

START_TEST(test_encoding)
{
	const char *raw;
	char *enc, *dec;

	raw = g_strdup( "Kristoffer Bronemyr");
	enc = g_mime_utils_header_encode_phrase(NULL, raw, NULL);
	dec = g_mime_utils_header_decode_phrase(NULL, enc);
	fail_unless(MATCH(raw,dec),"decode/encode failed");
	g_free((char *)raw);
	g_free(dec);
	g_free(enc);
}
END_TEST

START_TEST(test_dbmail_message_get_size)
{
	DbmailMessage *m;
	size_t i, j;

	/* */
	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, rfc822);

	i = dbmail_message_get_size(m, FALSE);
	ck_assert_uint_eq (i, 277);
	//fail_unless(i==251, "dbmail_message_get_size failed [%zu]", i);
	j = dbmail_message_get_size(m, TRUE);
	ck_assert_uint_eq (j, 251);
	//fail_unless(j==289, "dbmail_message_get_size failed [%zu]", j);

	dbmail_message_free(m);

	/* */
	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, "From: paul\n\n");

	i = dbmail_message_get_size(m, FALSE);
	fail_unless(i==12, "dbmail_message_get_size failed [%zu]", i);
	j = dbmail_message_get_size(m, TRUE);
	fail_unless(j==14, "dbmail_message_get_size failed [%zu]", j);

	dbmail_message_free(m);

}
END_TEST


START_TEST(test_db_get_message_lines)
{
	DbmailMessage *m;
	char *result;
	char *raw;
	const char *header = "From: foo@bar.org\r\n"
	"Subject: Some test\r\n"
	"To: bar@foo.org\r\n"
	"MIME-Version: 1.0\r\n"
	"Content-Type: text/plain; charset=utf-8\r\n"
	"Content-Transfer-Encoding: base64\r\n"
	"\r\n";
	const char *body = "CnRlc3RpbmcKCuHh4eHk\r\n";
	raw = g_strconcat(header, body, NULL);

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m, raw);
	dbmail_message_store(m);


	result = db_get_message_lines(m->msg_idnr, 0);
	fail_unless(MATCH(result, header));
	g_free(result);

	result = db_get_message_lines(m->msg_idnr, 1);
	fail_unless(MATCH(result, raw));
	g_free(result);

	result = db_get_message_lines(m->msg_idnr, -2);
	fail_unless(MATCH(result, raw));
	g_free(result);
	
	g_free(raw);
	dbmail_message_free(m);
}
END_TEST

/* Fetching subject from database */
extern DBParam_T db_params;
#define DBPFX db_params.pfx

int test_db_get_subject(uint64_t physid, char **subject)
{
        Connection_T c; ResultSet_T r;
        volatile int t = DM_EGENERAL;

        c = db_con_get();
        TRY
		r = db_query(c, "SELECT subjectfield "
				"FROM %ssubjectfield WHERE physmessage_id = %" PRIu64 "",
				DBPFX, physid);
		if (db_result_next(r)) {
				int l;	
				const void *blob;
				blob = db_result_get_blob(r, 0, &l);
				*subject = g_new0(char, l + 1);
				strncpy(*subject, blob, l);
		}
		t = DM_SUCCESS;
        CATCH(SQLException)
                LOG_SQLERROR;
        FINALLY
                db_con_close(c);
        END_TRY;

        return t;
}

START_TEST(test_dbmail_message_utf8_headers)
{
	DbmailMessage *m;
	uint64_t physid = 0;
	const char *s;
	char *s_dec,*t = NULL;
	const char *utf8_invalid_fixed = "=?UTF-8?B?0J/RgNC40LPQu9Cw0YjQsNC10Lwg0L3QsCDRgdC10YA/IA==?= =?UTF-8?B?0LrQvtC90LXRhiDRgdGC0YDQvtC60Lg=?=";

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m,utf8_long_header);
	dbmail_message_store(m);
	physid = dbmail_message_get_physid(m);

	s = dbmail_message_get_header(m,"Subject");
	s_dec = g_mime_utils_header_decode_phrase(NULL, s);
	test_db_get_subject(physid,&t);

	ck_assert_str_eq (s_dec,t);

	dbmail_message_free(m);
	g_free(s_dec);

	m = dbmail_message_new(NULL);
	m = dbmail_message_init_with_string(m,utf8_invalid);
	dbmail_message_store(m);
	physid = dbmail_message_get_physid(m);

	s_dec = g_mime_utils_header_decode_phrase(NULL, utf8_invalid_fixed);
	test_db_get_subject(physid,&t);
        fail_unless(MATCH(s_dec,t), "utf8 invalid failed:\n[%s] !=\n[%s]\n", s_dec, t);

	dbmail_message_free(m);
}
END_TEST

Suite *dbmail_message_suite(void)
{
	Suite *s = suite_create("Dbmail Message");
	TCase *tc_message = tcase_create("Message");
	/* setting timeot for all tests */
	tcase_set_timeout(tc_message,600);
	suite_add_tcase(s, tc_message);
	tcase_add_checked_fixture(tc_message, setup, teardown);
	tcase_add_test(tc_message, test_dbmail_message_new);
	tcase_add_test(tc_message, test_dbmail_message_set_class);
	tcase_add_test(tc_message, test_dbmail_message_get_class);
	tcase_add_test(tc_message, test_dbmail_message_get_internal_date);
	tcase_add_test(tc_message, test_g_mime_object_get_body);
	tcase_add_test(tc_message, test_dbmail_message_store);
	tcase_add_test(tc_message, test_dbmail_message_store2);
	tcase_add_test(tc_message, test_dbmail_message_retrieve);
	tcase_add_test(tc_message, test_dbmail_message_init_with_string);
	tcase_add_test(tc_message, test_dbmail_message_to_string);
	tcase_add_test(tc_message, test_dbmail_message_hdrs_to_string);
	tcase_add_test(tc_message, test_dbmail_message_body_to_string);
	tcase_add_test(tc_message, test_dbmail_message_set_header);
	tcase_add_test(tc_message, test_dbmail_message_get_header);
	tcase_add_test(tc_message, test_dbmail_message_cache_headers);
	tcase_add_test(tc_message, test_dbmail_message_free);
	tcase_add_test(tc_message, test_dbmail_message_encoded);
	tcase_add_test(tc_message, test_dbmail_message_8bit);
	tcase_add_test(tc_message, test_dbmail_message_get_header_addresses);
	tcase_add_test(tc_message, test_dbmail_message_get_header_repeated);
	tcase_add_test(tc_message, test_dbmail_message_construct);
	tcase_add_test(tc_message, test_dbmail_message_get_size);
	tcase_add_test(tc_message, test_encoding);
	tcase_add_test(tc_message, test_db_get_message_lines);
	tcase_add_test(tc_message, test_dbmail_message_utf8_headers);
	return s;
}

int main(void)
{
	int nf;
	g_mime_init();
	Suite *s = dbmail_message_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	g_mime_shutdown();
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
	

