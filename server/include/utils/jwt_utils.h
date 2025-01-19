#ifndef JWT_UTILS_H
#define JWT_UTILS_H

int generate_jwt(const char* id, const char* secret_key, char** token, PGconn* conn);
int verify_jwt(const char* token, const char* secret_key, PGconn* conn);
int delete_jwt(const char* token, PGconn* conn);

int hash_password(const char *password, char *hashed_password);
int verify_password(const char *password, const char *stored_hash);

#endif 
