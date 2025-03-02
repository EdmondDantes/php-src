--TEST--
cURL with Http
--EXTENSIONS--
curl
--FILE--
<?php
include "../../sapi/cli/tests/php_cli_server.inc";
php_cli_server_start();

Async\async(function() {
    echo "fiber start\n";

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, "http://" . PHP_CLI_SERVER_ADDRESS);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

    $response = curl_exec($ch);
    curl_close($ch);

    var_dump($response);
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
