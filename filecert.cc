/*
 * $Id: filecert.cc,v 1.1.2.8 2001/08/23 18:29:26 dbrumleve Exp $
 */

#define _NAPD_FILECERT_CC

#include <iostream>

extern "C" {
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
}

#include "file.hh"
#include "filecert.hh"
#include "functions.hh"

const U8 FileCert::key[] =
  "\xa4\x47\xad\xa8\x09\xfc\xb1\x5d\xd2\xfc\xfe\x06\x31\x99\x5f\xd1"
  "\x96\x27\xe2\x89\xe6\x0d\xe8\x52\xdb\x0a\x70\x70\xca\xad\x58\xea"
  "\x9e\x24\x0c\x4a\x94\xf4\xa3\x0c\x23\xeb\x80\xdd\x08\x8b\x31\x0f" 
  "\xfd\x43\xd1\x11\x1c\xb1\xf9\x65\x6a\xe3\x28\x41\x64\x22\xfa\x2a"
  ;
const U8 SongCert::key[] =
  "\xa4\x47\xad\xa8\x09\xfc\xb1\x5d\xd2\xfc\xfe\x06\x31\x99\x5f\xd1"
  "\x96\x27\xe2\x89\xe6\x0d\xe8\x52\xdb\x0a\x70\x70\xca\xad\x58\xea"
  "\x9e\x24\x0c\x4a\x94\xf4\xa3\x0c\x23\xeb\x80\xdd\x08\x8b\x31\x0f" 
  "\xfd\x43\xd1\x11\x1c\xb1\xf9\x65\x6a\xe3\x28\x41\x64\x22\xfa\x2a"
  ;

#if 0
// this crashes in napd but not as a standalone program!?
// using openssl for the time being /jdb

extern "C" {
#include <secude/ssl.h>
#include <secude/sec_glob.h>
#include <secude/crypt.h>
#include <secude/pkcs.h>
}

typedef void *HMAC_CTX;

int EVP_sha1(void) { return 0; }

void HMAC_Init(HMAC_CTX *ctx, U8 *p, U32 pn, int foo = 0) {
  OctetString ps = {pn, (char *)p};
  hmac_hash_init(ctx, &sha1_aid, &ps);
}

void HMAC_Update(HMAC_CTX *ctx, U8 *p, U32 pn) {
  OctetString ps = {pn, (char *)p};
  hmac_hash_more(ctx, &ps);
}

void HMAC_Final(HMAC_CTX *ctx, U8 *q, U32 *qn) {
  OctetString qs = {20, malloc(256)};
  hmac_hash_end(ctx, &qs);
  *qn = qs.noctets;
}
#endif

void FileCert::createSignature(U8 *s, U32 *size) {
  HMAC_CTX hmac_ctx;

  HMAC_Init(&hmac_ctx, key, sizeof(key), EVP_sha1());

  // XXX - Fixme
  HMAC_Update(&hmac_ctx, (U8 *)this, sizeof(*this) - 20);
  HMAC_Final(&hmac_ctx, s, size);
  HMAC_cleanup(&hmac_ctx);
}


// Convert a binary stream into individual fields for access
//
// @param buffer the buffer to copy from
// @param bufferSize the size of the buffer in bytes

void FileCert::getFromBytes(const char *buffer, unsigned long bufferSize) {
  if (bufferSize < sizeof(*this))
    THROW;
  memcpy(this, buffer, sizeof(*this));
}


// Check to see if the stored signature is valid

bool FileCert::validSignature() {
  U8 s[20];
  U32 size = sizeof(s);

  createSignature(s, &size);
  return !memcmp(s, signature, 20);
}


// Create a new certificate and copy it into a buffer
//
// @param buffer the buffer to copy the certificate into
// @param bufferSize the size of the buffer

void FileCert::copyToBytes(char *buffer, unsigned long bufferSize) {
  if (bufferSize < sizeof(*this))
    THROW;

  version = 0;

  U8 s[20];
  U32 size = sizeof(s);

  createSignature(s, &size);
  memcpy(signature, s, size);
  memcpy(buffer, (U8*)this, sizeof(*this));
}

const char *FileCert::getFileTypeAsString() {
  switch (ntohl(file_type)) {
  case File::TYPE_MP3:	return "mp3";
  case File::TYPE_WMA:	return "wma";
  default:		return "";
  }
}

void SongCert::createSignature(
 U8 *signature, U32 *signatureSize) {
  HMAC_CTX hmac_ctx;

  HMAC_Init(&hmac_ctx, key, sizeof(key), EVP_sha1());
  HMAC_Update(&hmac_ctx, (U8 *)&head, sizeof(struct SongCertHead));

  const U8 *empty = (const U8 *)"";

  if (_artist)
    HMAC_Update(&hmac_ctx, (U8 *)_artist, strlen(_artist) + 1);
  else HMAC_Update(&hmac_ctx, empty, 1);

  if (_title)
    HMAC_Update(&hmac_ctx, (U8 *)_title, strlen(_title) + 1);
  else HMAC_Update(&hmac_ctx, empty, 1);

  if (_album)
    HMAC_Update(&hmac_ctx, (U8 *)_album, strlen(_album) + 1);
  else HMAC_Update(&hmac_ctx, empty, 1);

  if (_composer)
    HMAC_Update(&hmac_ctx, (U8 *)_composer, strlen(_composer) + 1);
  else HMAC_Update(&hmac_ctx, empty, 1);

  if (_orchestra)
    HMAC_Update(&hmac_ctx, (U8 *)_orchestra, strlen(_orchestra) + 1);
  else HMAC_Update(&hmac_ctx, empty, 1);

  if (_soloist)
    HMAC_Update(&hmac_ctx, (U8 *)_soloist, strlen(_soloist) + 1);
  else HMAC_Update(&hmac_ctx, empty, 1);
      
  HMAC_Final(&hmac_ctx, signature, signatureSize);
  HMAC_cleanup(&hmac_ctx);
}


// Convert a binary stream into individual fields for access
//
// @param buffer the buffer to copy from
// @param bufferSize the size of the buffer in bytes

void SongCert::getFromBytes(char *buffer, unsigned long bufferSize) {
  // XXX - fixme
  if (bufferSize < sizeof(struct SongCertHead) + 20 + 6)
    THROW;

  char *ptr = buffer;
  U32 len;
  memcpy(&head, ptr, sizeof(struct SongCertHead));
   
  ptr += sizeof(struct SongCertHead);
   
  len = strlen(ptr) + 1;
  _artist = new char[len];
  memcpy(_artist, ptr, len);
  ptr += len;

  len = strlen(ptr) + 1;
  _title = new char[len];
  memcpy(_title, ptr, len);
  ptr += len;

  len = strlen(ptr) + 1;
  _album = new char[len];
  memcpy(_album, ptr, len);
  ptr += len;

  len = strlen(ptr) + 1;
  _composer = new char[len];
  memcpy(_composer, ptr, len);
  ptr += len;

  len = strlen(ptr) + 1;
  _orchestra = new char[len];
  memcpy(_orchestra, ptr, len);
  ptr += len;

  len = strlen(ptr) + 1;
  _soloist = new char[len];
  memcpy(_soloist, ptr, len);
  ptr += len;

  // XXX - Fixme
  memcpy(_signature, ptr, 20);
}


// Check to see if the stored signature is valid

bool SongCert::validSignature() {
  U8 signature[20];
  U32 signatureSize = sizeof(signature);

  createSignature(signature, &signatureSize);

  // XXX - Fixme
  return !memcmp(signature, _signature, 20);
}

// Get the size of the certificate in bytes
//
// @returns the size of the certificate in bytes

uint32_t SongCert::getByteSize() {
  // XXX - Fixme
  uint32_t size = sizeof(struct SongCertHead) + 20;

  size += 1 + (_artist ? strlen(_artist) : 0);
  size += 1 + (_title ? strlen(_title) : 0);
  size += 1 + (_album ? strlen(_album) : 0);
  size += 1 + (_composer ? strlen(_composer) : 0);
  size += 1 + (_orchestra ? strlen(_orchestra) : 0);
  size += 1 + (_soloist ? strlen(_soloist) : 0);

  return size;
}


// Create a new certificate and copy it into a buffer
//
// @param buffer the buffer to copy the certificate into
// @param bufferSize the size of the buffer

void SongCert::copyToBytes(char *buffer, unsigned long bufferSize) {
  if (bufferSize < getByteSize())
    THROW;

  head.version = 0;

  char *ptr = buffer;

  memcpy(ptr, &head, sizeof(struct SongCertHead));
  ptr += sizeof(struct SongCertHead);

  if (_artist) {
    memcpy(ptr, _artist, strlen(_artist) + 1);
    ptr += strlen(_artist) + 1;
  } else {
    memcpy(ptr, "", 1);
    ptr++;
  }

  if (_title) {
    memcpy(ptr, _title, strlen(_title) + 1);
    ptr += strlen(_title) + 1;
  } else {
    memcpy(ptr, "", 1);
    ptr++;
  }

  if (_album) {
    memcpy(ptr, _album, strlen(_album) + 1);
    ptr += strlen(_album) + 1;
  } else {
    memcpy(ptr, "", 1);
    ptr++;
  }

  if (_composer) {
    memcpy(ptr, _composer, strlen(_composer) + 1);
    ptr += strlen(_composer) + 1;
  } else {
    memcpy(ptr, "", 1);
    ptr++;
  }

  if (_orchestra) {
    memcpy(ptr, _orchestra, strlen(_orchestra) + 1);
    ptr += strlen(_orchestra) + 1;
  } else {
    memcpy(ptr, "", 1);
    ptr++;
  }

  if (_soloist) {
    memcpy(ptr, _soloist, strlen(_soloist) + 1);
    ptr += strlen(_soloist) + 1;
  } else {
    memcpy(ptr, "", 1);
    ptr++;
  }

  U8 s[20];
  U32 size = sizeof(s);

  createSignature(s, &size);
  memcpy(ptr, s, size);
}

