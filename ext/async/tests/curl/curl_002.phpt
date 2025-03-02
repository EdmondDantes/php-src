--TEST--
cURL Multi Select with Async
--EXTENSIONS--
curl
--FILE--
<?php
include "../../sapi/cli/tests/php_cli_server.inc";
php_cli_server_start();

Async\async(function() {
    echo "fiber start\n";

    $mh = curl_multi_init();

    // First cURL handle
    $ch1 = curl_init();
    curl_setopt($ch1, CURLOPT_URL, "http://" . PHP_CLI_SERVER_ADDRESS);
    curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true);
    curl_multi_add_handle($mh, $ch1);

    // Second cURL handle
    $ch2 = curl_init();
    curl_setopt($ch2, CURLOPT_URL, "http://" . PHP_CLI_SERVER_ADDRESS);
    curl_setopt($ch2, CURLOPT_RETURNTRANSFER, true);
    curl_multi_add_handle($mh, $ch2);

    $active = null;
    do {
        $status = curl_multi_exec($mh, $active);
        if ($status !== CURLM_OK) {
            echo "Error: " . curl_multi_strerror($status) . "\n";
            break;
        }

        curl_multi_select($mh, 1.0);
    } while ($active);

    // Retrieve responses
    $response1 = curl_multi_getcontent($ch1);
    $response2 = curl_multi_getcontent($ch2);

    curl_multi_remove_handle($mh, $ch1);
    curl_multi_remove_handle($mh, $ch2);
    curl_multi_close($mh);

    curl_close($ch1);
    curl_close($ch2);

    var_dump($response1);
    var_dump($response2);

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
string(11) "Hello world"
fiber end
end
