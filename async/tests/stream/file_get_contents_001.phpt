--TEST--
file_get_contents with Http
--INI--
allow_url_fopen=1
--FILE--
<?php
include "../sapi/cli/tests/php_cli_server.inc";
php_cli_server_start();

Async\async(function() {
    echo "fiber start\n";
    var_dump(file_get_contents("http://" . PHP_CLI_SERVER_ADDRESS));
    echo "fiber end\n";
});

Async\async(function() {
    echo "fiber 2\n";
});


echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
fiber start
fiber 2
string(11) "Hello world"
fiber end
end

