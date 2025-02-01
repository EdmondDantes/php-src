--TEST--
Get HTTP response with host resolution
--EXTENSIONS--
sockets
--FILE--
<?php
// For IPv4
//define("PHP_CLI_SERVER_HOSTNAME", "127.0.0.1");

require_once __DIR__."/../php_cli_server.php";

php_cli_server_start();

Async\async(function() {
    echo "fiber start\n";

    $hostPort = explode(":", PHP_CLI_SERVER_ADDRESS);
    $host = $hostPort[0];
    $port = $hostPort[1];
    $request = "GET / HTTP/1.1\r\nHost: $host\r\nConnection: Close\r\n\r\n";

    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if (!$socket) {
        echo "Socket creation failed\n";
        return;
    }

    if (!socket_connect($socket, $host, $port)) {
        echo "Socket connection failed\n";
        socket_close($socket);
        return;
    }

    socket_write($socket, $request, strlen($request));
    $response = '';
    while ($buffer = socket_read($socket, 2048)) {
        $response .= $buffer;
    }
    socket_close($socket);

    $responseBody = preg_replace("/.*\r\n\r\n/s", "", $response);
    var_dump($responseBody);
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
