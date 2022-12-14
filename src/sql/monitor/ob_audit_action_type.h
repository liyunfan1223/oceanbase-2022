/**
 * Copyright (c) 2021 OceanBase
 * OceanBase CE is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan PubL v2.
 * You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifdef OB_AUDIT_ACTION_TYPE_DEF
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_UNKNOWN, 0, "UNKNOWN")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_TABLE, 1, "CREATE TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_INSERT, 2, "INSERT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SELECT, 3, "SELECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_CLUSTER, 4, "CREATE CLUSTER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_CLUSTER, 5, "ALTER CLUSTER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_UPDATE, 6, "UPDATE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DELETE, 7, "DELETE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_CLUSTER, 8, "DROP CLUSTER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_INDEX, 9, "CREATE INDEX")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_INDEX, 10, "DROP INDEX")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_INDEX, 11, "ALTER INDEX")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_TABLE, 12, "DROP TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_SEQUENCE, 13, "CREATE SEQUENCE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_SEQUENCE, 14, "ALTER SEQUENCE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_TABLE, 15, "ALTER TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_SEQUENCE, 16, "DROP SEQUENCE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_GRANT_OBJECT, 17, "GRANT OBJECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_REVOKE_OBJECT, 18, "REVOKE OBJECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_SYNONYM, 19, "CREATE SYNONYM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_SYNONYM, 20, "DROP SYNONYM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_VIEW, 21, "CREATE VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_VIEW, 22, "DROP VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_VALIDATE_INDEX, 23, "VALIDATE INDEX")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PROCEDURE, 24, "CREATE PROCEDURE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PROCEDURE, 25, "ALTER PROCEDURE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_LOCK, 26, "LOCK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_NO_OP, 27, "NO-OP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_RENAME, 28, "RENAME")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_COMMENT, 29, "COMMENT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_AUDIT_OBJECT, 30, "AUDIT OBJECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_NOAUDIT_OBJECT, 31, "NOAUDIT OBJECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_DATABASE_LINK, 32, "CREATE DATABASE LINK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_DATABASE_LINK, 33, "DROP DATABASE LINK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_DATABASE, 34, "CREATE DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_DATABASE, 35, "ALTER DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_ROLLBACK_SEG, 36, "CREATE ROLLBACK SEG")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_ROLLBACK_SEG, 37, "ALTER ROLLBACK SEG")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_ROLLBACK_SEG, 38, "DROP ROLLBACK SEG")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_TABLESPACE, 39, "CREATE TABLESPACE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_TABLESPACE, 40, "ALTER TABLESPACE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_TABLESPACE, 41, "DROP TABLESPACE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_SESSION, 42, "ALTER SESSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_USER, 43, "ALTER USER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_COMMIT, 44, "COMMIT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ROLLBACK, 45, "ROLLBACK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SAVEPOINT, 46, "SAVEPOINT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PL_SQL_EXECUTE, 47, "PL/SQL_EXECUTE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SET_TRANSACTION, 48, "SET TRANSACTION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_SYSTEM, 49, "ALTER SYSTEM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_EXPLAIN, 50, "EXPLAIN")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_USER, 51, "CREATE USER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_ROLE, 52, "CREATE ROLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_USER, 53, "DROP USER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_ROLE, 54, "DROP ROLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SET_ROLE, 55, "SET ROLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_SCHEMA, 56, "CREATE SCHEMA")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_CONTROL_FILE, 57, "CREATE CONTROL FILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_TRACING, 58, "ALTER TRACING")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_TRIGGER, 59, "CREATE TRIGGER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_TRIGGER, 60, "ALTER TRIGGER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_TRIGGER, 61, "DROP TRIGGER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ANALYZE_TABLE, 62, "ANALYZE TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ANALYZE_INDEX, 63, "ANALYZE INDEX")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ANALYZE_CLUSTER, 64, "ANALYZE CLUSTER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PROFILE, 65, "CREATE PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PROFILE, 66, "DROP PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PROFILE, 67, "ALTER PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PROCEDURE, 68, "DROP PROCEDURE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_RESOURCE_COST, 70, "ALTER RESOURCE COST")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_MATERIALIZED_VIEW_LOG, 71, "CREATE MATERIALIZED VIEW LOG")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_MATERIALIZED_VIEW_LOG, 72, "ALTER MATERIALIZED VIEW LOG")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_MATERIALIZED_VIEW_LOG, 73, "DROP MATERIALIZED VIEW LOG")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_MATERIALIZED_VIEW, 74, "CREATE MATERIALIZED VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_MATERIALIZED_VIEW, 75, "ALTER MATERIALIZED VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_MATERIALIZED_VIEW, 76, "DROP MATERIALIZED VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_TYPE, 77, "CREATE TYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_TYPE, 78, "DROP TYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_ROLE, 79, "ALTER ROLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_TYPE, 80, "ALTER TYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_TYPE_BODY, 81, "CREATE TYPE BODY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_TYPE_BODY, 82, "ALTER TYPE BODY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_TYPE_BODY, 83, "DROP TYPE BODY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_LIBRARY, 84, "DROP LIBRARY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_TRUNCATE_TABLE, 85, "TRUNCATE TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_TRUNCATE_CLUSTER, 86, "TRUNCATE CLUSTER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_VIEW, 88, "ALTER VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SET_CONSTRAINTS, 90, "SET CONSTRAINTS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_FUNCTION, 91, "CREATE FUNCTION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_FUNCTION, 92, "ALTER FUNCTION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_FUNCTION, 93, "DROP FUNCTION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PACKAGE, 94, "CREATE PACKAGE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PACKAGE, 95, "ALTER PACKAGE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PACKAGE, 96, "DROP PACKAGE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PACKAGE_BODY, 97, "CREATE PACKAGE BODY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PACKAGE_BODY, 98, "ALTER PACKAGE BODY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PACKAGE_BODY, 99, "DROP PACKAGE BODY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_LOGON, 100, "LOGON")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_LOGOFF, 101, "LOGOFF")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_LOGOFF_BY_CLEANUP, 102, "LOGOFF BY CLEANUP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SESSION_REC, 103, "SESSION REC")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SYSTEM_AUDIT, 104, "SYSTEM AUDIT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SYSTEM_NOAUDIT, 105, "SYSTEM NOAUDIT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_AUDIT_DEFAULT, 106, "AUDIT DEFAULT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_NOAUDIT_DEFAULT, 107, "NOAUDIT DEFAULT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SYSTEM_GRANT, 108, "SYSTEM GRANT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SYSTEM_REVOKE, 109, "SYSTEM REVOKE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PUBLIC_SYNONYM, 110, "CREATE PUBLIC SYNONYM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PUBLIC_SYNONYM, 111, "DROP PUBLIC SYNONYM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PUBLIC_DATABASE_LINK, 112, "CREATE PUBLIC DATABASE LINK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PUBLIC_DATABASE_LINK, 113, "DROP PUBLIC DATABASE LINK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_GRANT_ROLE, 114, "GRANT ROLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_REVOKE_ROLE, 115, "REVOKE ROLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_EXECUTE_PROCEDURE, 116, "EXECUTE PROCEDURE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_USER_COMMENT, 117, "USER COMMENT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ENABLE_TRIGGER, 118, "ENABLE TRIGGER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DISABLE_TRIGGER, 119, "DISABLE TRIGGER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ENABLE_ALL_TRIGGERS, 120, "ENABLE ALL TRIGGERS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DISABLE_ALL_TRIGGERS, 121, "DISABLE ALL TRIGGERS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_NETWORK_ERROR, 122, "NETWORK ERROR")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_EXECUTE_TYPE, 123, "EXECUTE TYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_READ_DIRECTORY, 125, "READ DIRECTORY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_WRITE_DIRECTORY, 126, "WRITE DIRECTORY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_FLASHBACK, 128, "FLASHBACK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_BECOME_USER, 129, "BECOME USER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_MINING_MODEL, 130, "ALTER MINING MODEL")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SELECT_MINING_MODEL, 131, "SELECT MINING MODEL")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_MINING_MODEL, 133, "CREATE MINING MODEL")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PUBLIC_SYNONYM, 134, "ALTER PUBLIC SYNONYM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_EXECUTE_DIRECTORY, 135, "EXECUTE DIRECTORY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_SQL_LOADER_DIRECT_PATH_LOAD, 136, "SQL*LOADER DIRECT PATH LOAD")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DATAPUMP_DIRECT_PATH_UNLOAD, 137, "DATAPUMP DIRECT PATH UNLOAD")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DATABASE_STARTUP, 138, "DATABASE STARTUP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DATABASE_SHUTDOWN, 139, "DATABASE SHUTDOWN")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_SQL_TXLN_PROFILE, 140, "CREATE SQL TXLN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_SQL_TXLN_PROFILE, 141, "ALTER SQL TXLN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_USE_SQL_TXLN_PROFILE, 142, "USE SQL TXLN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_SQL_TXLN_PROFILE, 143, "DROP SQL TXLN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_MEASURE_FOLDER, 144, "CREATE MEASURE FOLDER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_MEASURE_FOLDER, 145, "ALTER MEASURE FOLDER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_MEASURE_FOLDER, 146, "DROP MEASURE FOLDER")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_CUBE_BUILD_PROCESS, 147, "CREATE CUBE BUILD PROCESS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_CUBE_BUILD_PROCESS, 148, "ALTER CUBE BUILD PROCESS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_CUBE_BUILD_PROCESS, 149, "DROP CUBE BUILD PROCESS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_CUBE, 150, "CREATE CUBE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_CUBE, 151, "ALTER CUBE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_CUBE, 152, "DROP CUBE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_CUBE_DIMENSION, 153, "CREATE CUBE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_CUBE_DIMENSION, 154, "ALTER CUBE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_CUBE_DIMENSION, 155, "DROP CUBE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_DIRECTORY, 157, "CREATE DIRECTORY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_DIRECTORY, 158, "DROP DIRECTORY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_LIBRARY, 159, "CREATE LIBRARY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_JAVA, 160, "CREATE JAVA")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_JAVA, 161, "ALTER JAVA")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_JAVA, 162, "DROP JAVA")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_OPERATOR, 163, "CREATE OPERATOR")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_INDEXTYPE, 164, "CREATE INDEXTYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_INDEXTYPE, 165, "DROP INDEXTYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_INDEXTYPE, 166, "ALTER INDEXTYPE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_OPERATOR, 167, "DROP OPERATOR")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ASSOCIATE_STATISTICS, 168, "ASSOCIATE STATISTICS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DISASSOCIATE_STATISTICS, 169, "DISASSOCIATE STATISTICS")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CALL_METHOD, 170, "CALL METHOD")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_SUMMARY, 171, "CREATE SUMMARY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_SUMMARY, 172, "ALTER SUMMARY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_SUMMARY, 173, "DROP SUMMARY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_DIMENSION, 174, "CREATE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_DIMENSION, 175, "ALTER DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_DIMENSION, 176, "DROP DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_CONTEXT, 177, "CREATE CONTEXT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_CONTEXT, 178, "DROP CONTEXT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_OUTLINE, 179, "ALTER OUTLINE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_OUTLINE, 180, "CREATE OUTLINE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_OUTLINE, 181, "DROP OUTLINE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_UPDATE_INDEXES, 182, "UPDATE INDEXES")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_OPERATOR, 183, "ALTER OPERATOR")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_SPFILE, 187, "CREATE SPFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PFILE, 188, "CREATE PFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_MERGE, 189, "MERGE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PASSWORD_CHANGE, 190, "PASSWORD CHANGE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_SYNONYM, 192, "ALTER SYNONYM")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_DISKGROUP, 193, "ALTER DISKGROUP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_DISKGROUP, 194, "CREATE DISKGROUP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_DISKGROUP, 195, "DROP DISKGROUP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PURGE_USER_RECYCLEBIN, 197, "PURGE USER_RECYCLEBIN")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PURGE_DBA_RECYCLEBIN, 198, "PURGE DBA_RECYCLEBIN")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PURGE_TABLESPACE, 199, "PURGE TABLESPACE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PURGE_TABLE, 200, "PURGE TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PURGE_INDEX, 201, "PURGE INDEX")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_UNDROP_OBJECT, 202, "UNDROP OBJECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_DATABASE, 203, "DROP DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_FLASHBACK_DATABASE, 204, "FLASHBACK DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_FLASHBACK_TABLE, 205, "FLASHBACK TABLE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_RESTORE_POINT, 206, "CREATE RESTORE POINT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_RESTORE_POINT, 207, "DROP RESTORE POINT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_PROXY_AUTHENTICATION_ONLY, 208, "PROXY AUTHENTICATION ONLY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DECLARE_REWRITE_EQUIVALENCE, 209, "DECLARE REWRITE EQUIVALENCE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_REWRITE_EQUIVALENCE, 210, "ALTER REWRITE EQUIVALENCE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_REWRITE_EQUIVALENCE, 211, "DROP REWRITE EQUIVALENCE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_EDITION, 212, "CREATE EDITION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_EDITION, 213, "ALTER EDITION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_EDITION, 214, "DROP EDITION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_ASSEMBLY, 215, "DROP ASSEMBLY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_ASSEMBLY, 216, "CREATE ASSEMBLY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_ASSEMBLY, 217, "ALTER ASSEMBLY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_FLASHBACK_ARCHIVE, 218, "CREATE FLASHBACK ARCHIVE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_FLASHBACK_ARCHIVE, 219, "ALTER FLASHBACK ARCHIVE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_FLASHBACK_ARCHIVE, 220, "DROP FLASHBACK ARCHIVE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DEBUG_CONNECT, 221, "DEBUG CONNECT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DEBUG_PROCEDURE, 223, "DEBUG PROCEDURE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_DATABASE_LINK, 225, "ALTER DATABASE LINK")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_PLUGGABLE_DATABASE, 226, "CREATE PLUGGABLE DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PLUGGABLE_DATABASE, 227, "ALTER PLUGGABLE DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_PLUGGABLE_DATABASE, 228, "DROP PLUGGABLE DATABASE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_AUDIT_POLICY, 229, "CREATE AUDIT POLICY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_AUDIT_POLICY, 230, "ALTER AUDIT POLICY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_AUDIT_POLICY, 231, "DROP AUDIT POLICY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CODE_BASED_GRANT, 232, "CODE-BASED GRANT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CODE_BASED_REVOKE, 233, "CODE-BASED REVOKE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_LOCKDOWN_PROFILE, 234, "CREATE LOCKDOWN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_LOCKDOWN_PROFILE, 235, "DROP LOCKDOWN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_LOCKDOWN_PROFILE, 236, "ALTER LOCKDOWN PROFILE")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_TRANSLATE_SQL, 237, "TRANSLATE SQL")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ADMINISTER_KEY_MANAGEMENT, 238, "ADMINISTER KEY MANAGEMENT")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_MATERIALIZED_ZONEMAP, 239, "CREATE MATERIALIZED ZONEMAP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_MATERIALIZED_ZONEMAP, 240, "ALTER MATERIALIZED ZONEMAP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_MATERIALIZED_ZONEMAP, 241, "DROP MATERIALIZED ZONEMAP")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_MINING_MODEL, 242, "DROP MINING MODEL")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_ATTRIBUTE_DIMENSION, 243, "CREATE ATTRIBUTE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_ATTRIBUTE_DIMENSION, 244, "ALTER ATTRIBUTE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_ATTRIBUTE_DIMENSION, 245, "DROP ATTRIBUTE DIMENSION")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_HIERARCHY, 246, "CREATE HIERARCHY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_HIERARCHY, 247, "ALTER HIERARCHY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_HIERARCHY, 248, "DROP HIERARCHY")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_CREATE_ANALYTIC_VIEW, 249, "CREATE ANALYTIC VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_ANALYTIC_VIEW, 250, "ALTER ANALYTIC VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_DROP_ANALYTIC_VIEW, 251, "DROP ANALYTIC VIEW")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_ALTER_PUBLIC_DATABASE_LINK, 305, "ALTER PUBLIC DATABASE LINK")

// the subsequence action types will not be exposed to user
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_GUARD, 400, "UNUSED")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_OB_CREATE_SYNONYM, 401, "INTERNAL USED")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_OB_DROP_SYNONYM, 402, "INTERNAL USED")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_OB_CREATE_ROUTINE, 403, "INTERNAL USED")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_OB_DROP_ROUTINE, 404, "INTERNAL USED")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_OB_AUDIT, 405, "INTERNAL USED")
OB_AUDIT_ACTION_TYPE_DEF(ACTION_TYPE_MAX, 500, "UNUSED")
#endif

#ifndef OCEANBASE_SQL_OB_AUDIT_ACTION_TYPE_H_
#define OCEANBASE_SQL_OB_AUDIT_ACTION_TYPE_H_

namespace oceanbase {
namespace sql {
namespace audit {

enum AuditActionType : int32_t
{
#define OB_AUDIT_ACTION_TYPE_DEF(action_type, action_id, action_name) action_type = action_id,
#include "sql/monitor/ob_audit_action_type.h"
#undef OB_AUDIT_ACTION_TYPE_DEF
};

struct AuditActionTypeIndex
{
public:
  AuditActionTypeIndex()
    : audit_action_type_idx_()
  {
    int i = 0;
    for (int j = 0; j < ARRAYSIZEOF(audit_action_type_idx_); j++) {
      audit_action_type_idx_[j] = -1;
    }
    #define OB_AUDIT_ACTION_TYPE_DEF(action_type, action_id, action_name) audit_action_type_idx_[action_type] = i++;
    #include "sql/monitor/ob_audit_action_type.h"
    #undef OB_AUDIT_ACTION_TYPE_DEF
  }
  int32_t audit_action_type_idx_[ACTION_TYPE_MAX + 1];
};

inline int32_t get_audit_action_type_idx(AuditActionType type)
{
  static AuditActionTypeIndex inst;
  return inst.audit_action_type_idx_[type];
}

} // namespace audit
} // namespace sql
} // namespace oceanbase

#endif /* OCEANBASE_SQL_OB_AUDIT_ACTION_TYPE_H */
