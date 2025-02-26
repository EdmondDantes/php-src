--TEST--
MySQL PDO->query() async
--EXTENSIONS--
pdo_mysql
--SKIPIF--
<?php
require_once __DIR__ . '/inc/mysql_pdo_test.inc';
MySQLPDOTest::skip();
?>
--FILE--
<?php

require_once __DIR__ . '/inc/mysql_pdo_test.inc';

$db = MySQLPDOTest::factory();
$db->query('CREATE TABLE test_mysql_async_exec (id INT, label VARCHAR(255))');
$db->query('INSERT INTO test_mysql_async_exec (id, label) VALUES (1, "foo1")');
$db->query('INSERT INTO test_mysql_async_exec (id, label) VALUES (2, "foo2")');

Async\run(function() {

    echo "start fiber1\n";

    $db = MySQLPDOTest::factory();

    echo "executing query1\n";

    $results = $db->query('SELECT `label` FROM test_mysql_async_exec WHERE id = 1');

    echo "fetching results1\n";

    foreach ($results as $row) {
        echo "fiber1 row: " . $row['label'] . "\n";
    }
});

Async\run(function() {

    echo "start fiber2\n";

    $db = MySQLPDOTest::factory();

    echo "executing query2\n";

    $results = $db->query('SELECT `label` FROM test_mysql_async_exec WHERE id = 2');

    echo "fetching results2\n";

    foreach ($results as $row) {
        echo "fiber2 row: " . $row['label'] . "\n";
    }
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--CLEAN--
<?php
require_once __DIR__ . '/inc/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->query('DROP TABLE IF EXISTS test_mysql_async_exec');
?>
--EXPECTF--
start
start fiber1
start fiber2
executing query1
executing query2
fetching results1
fiber1 row: foo1
fetching results2
fiber2 row: foo2
end