--TEST--
Async\Walker::walk with file_get_contents
--FILE--
<?php
require_once __DIR__."/../php_cli_server.php";
php_cli_server_start();

Async\Walker::walk([1, 2, 3], function(mixed $value) {
    echo file_get_contents("http://".PHP_CLI_SERVER_ADDRESS)."\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
Hello world
Hello world
Hello world
end
