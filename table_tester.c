// #include <stdio.h>
// #include <assert.h>
//
//
// #include "table_tester.h"
// #include "table.h"
// #include "object.h"
// #include "value.h"
//
//
// static ObjString *str(const char *s) {
//     int len = 0;
//     while (s[len] != '\0') len++;
//
//     return NULL; /*internString(s, len, &strings);*/
// }
//
// static void expectNumber(Table *table, const char *key, double expected) {
//     Value value;
//
//     assert(tableGet(table, str(key), &value));
//     assert(IS_NUMBER(value));
//     assert(AS_NUMBER(value) == expected);
// }
//
// static void expectMissing(Table *table, const char *key) {
//     Value value;
//
//     assert(!tableGet(table, str(key), &value));
// }
//
//
// static void testEmptyTable(void) {
//     Table table;
//     initTable(&table);
//
//     Value value;
//
//     assert(!tableGet(&table, str("x"), &value));
//     assert(!tableRemove(&table, str("x")));
//
//     freeTable(&table);
// }
//
//
// static void testSingleInsert(void) {
//     Table table;
//     initTable(&table);
//
//     assert(
//         tableSet(
//             &table,
//             str("x"),
//                  NUMBER_VALUE(42)
//         )
//     );
//
//     expectNumber(&table, "x", 42);
//
//     freeTable(&table);
// }
//
//
// static void testMultipleInsert(void) {
//     Table table;
//     initTable(&table);
//
//     tableSet(&table, str("a"), NUMBER_VALUE(1));
//     tableSet(&table, str("b"), NUMBER_VALUE(2));
//     tableSet(&table, str("c"), NUMBER_VALUE(3));
//     tableSet(&table, str("d"), NUMBER_VALUE(4));
//
//     expectNumber(&table, "a", 1);
//     expectNumber(&table, "b", 2);
//     expectNumber(&table, "c", 3);
//     expectNumber(&table, "d", 4);
//
//     freeTable(&table);
// }
//
//
// static void testOverwrite(void) {
//     Table table;
//     initTable(&table);
//
//     tableSet(&table, str("answer"), NUMBER_VALUE(42));
//     tableSet(&table, str("answer"), NUMBER_VALUE(99));
//
//     expectNumber(&table, "answer", 99);
//
//     freeTable(&table);
// }
//
//
// static void testRemove(void) {
//     Table table;
//     initTable(&table);
//
//     tableSet(&table, str("a"), NUMBER_VALUE(1));
//
//     assert(tableRemove(&table, str("a")));
//
//     expectMissing(&table, "a");
//
//     freeTable(&table);
// }
//
//
// static void testRemoveTwice(void) {
//     Table table;
//     initTable(&table);
//
//     tableSet(&table, str("a"), NUMBER_VALUE(1));
//
//     assert(tableRemove(&table, str("a")));
//     assert(!tableRemove(&table, str("a")));
//
//     freeTable(&table);
// }
//
//
// static void testTombstoneReuse(void) {
//     Table table;
//     initTable(&table);
//
//     tableSet(&table, str("a"), NUMBER_VALUE(1));
//
//     assert(tableRemove(&table, str("a")));
//
//     tableSet(&table, str("b"), NUMBER_VALUE(2));
//
//     expectNumber(&table, "b", 2);
//
//     freeTable(&table);
// }
//
//
// static void testResize(void) {
//     Table table;
//     initTable(&table);
//
//     char buffer[64];
//
//     for (int i = 0; i < 100; i++) {
//         sprintf(buffer, "key_%d", i);
//
//         tableSet(
//             &table,
//             str(buffer),
//                  NUMBER_VALUE(i)
//         );
//     }
//
//     for (int i = 0; i < 100; i++) {
//         sprintf(buffer, "key_%d", i);
//
//         expectNumber(
//             &table,
//             buffer,
//             i
//         );
//     }
//
//     freeTable(&table);
// }
//
//
// static void testLargeResize(void) {
//     Table table;
//     initTable(&table);
//
//     char buffer[64];
//
//     for (int i = 0; i < 5000; i++) {
//         sprintf(buffer, "sym_%d", i);
//
//         tableSet(
//             &table,
//             str(buffer),
//                  NUMBER_VALUE(i)
//         );
//     }
//
//     for (int i = 0; i < 5000; i++) {
//         sprintf(buffer, "sym_%d", i);
//
//         expectNumber(
//             &table,
//             buffer,
//             i
//         );
//     }
//
//     freeTable(&table);
// }
//
//
// static void testRandomDeletion(void) {
//     Table table;
//     initTable(&table);
//
//     char buffer[64];
//
//     for (int i = 0; i < 1000; i++) {
//         sprintf(buffer, "key_%d", i);
//
//         tableSet(
//             &table,
//             str(buffer),
//                  NUMBER_VALUE(i)
//         );
//     }
//
//     for (int i = 0; i < 1000; i += 2) {
//         sprintf(buffer, "key_%d", i);
//
//         tableRemove(
//             &table,
//             str(buffer)
//         );
//     }
//
//     for (int i = 0; i < 1000; i++) {
//         sprintf(buffer, "key_%d", i);
//
//         if (i % 2 == 0) {
//             expectMissing(&table, buffer);
//         } else {
//             expectNumber(&table, buffer, i);
//         }
//     }
//
//     freeTable(&table);
// }
//
//
// static void testHugeSymbolTable(void) {
//     Table table;
//     initTable(&table);
//
//     char buffer[128];
//
//     for (int i = 0; i < 50000; i++) {
//         sprintf(buffer, "identifier_%d", i);
//
//         tableSet(
//             &table,
//             str(buffer),
//                  NUMBER_VALUE(i)
//         );
//     }
//
//     for (int i = 0; i < 50000; i++) {
//         sprintf(buffer, "identifier_%d", i);
//
//         expectNumber(
//             &table,
//             buffer,
//             i
//         );
//     }
//
//     freeTable(&table);
// }
//
//
// void testTable() {
//     testEmptyTable();
//     testSingleInsert();
//     testMultipleInsert();
//     testOverwrite();
//     testRemove();
//     testRemoveTwice();
//     testTombstoneReuse();
//     testResize();
//     testLargeResize();
//     testRandomDeletion();
//     testHugeSymbolTable();
//
//     printf("ALL TABLE TESTS PASSED\n");
// }
