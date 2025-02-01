--TEST--
Client-server communication within fibers
--EXTENSIONS--
sockets
--FILE--
<?php

define("HOSTNAME", "localhost");
define("PORT", 80801);

Async\async(function() {
    echo "Server fiber start\n";

    $serverSocket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if (!$serverSocket) {
        echo "Server socket creation failed\n";
        return;
    }

    if (!socket_bind($serverSocket, HOSTNAME, PORT)) {
        echo "Server socket bind failed\n";
        socket_close($serverSocket);
        return;
    }

    if (!socket_listen($serverSocket, 1)) {
        echo "Server socket listen failed\n";
        socket_close($serverSocket);
        return;
    }

    echo "Server is listening...\n";

    $clientSocket = socket_accept($serverSocket);
    if (!$clientSocket) {
        echo "Server socket accept failed\n";
        socket_close($serverSocket);
        return;
    }

    $request = socket_read($clientSocket, 2048);
    echo "Server received request: $request\n";

    $response = "HTTP/1.1 200 OK\r\nContent-Length: 11\r\n\r\nHello world";
    socket_write($clientSocket, $response, strlen($response));

    socket_close($clientSocket);
    socket_close($serverSocket);

    echo "Server fiber end\n";
});

Async\async(function() {
    echo "Client fiber start\n";

    usleep(1);

    $socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if (!$socket) {
        echo "Client socket creation failed\n";
        return;
    }

    if (!socket_connect($socket, HOSTNAME, PORT)) {
        echo "Client socket connection failed\n";
        socket_close($socket);
        return;
    }

    $request = "GET / HTTP/1.1\r\nHost: " . HOSTNAME . "\r\nConnection: Close\r\n\r\n";
    socket_write($socket, $request, strlen($request));

    $response = '';
    while ($buffer = socket_read($socket, 2048)) {
        $response .= $buffer;
    }
    socket_close($socket);

    $responseBody = preg_replace("/.*\r\n\r\n/s", "", $response);
    var_dump($responseBody);

    echo "Client fiber end\n";
});

echo "start\n";
Async\launchScheduler();
echo "end\n";
?>
--EXPECT--
start
Server fiber start
Client fiber start
Server is listening...
Server received request: GET / HTTP/1.1
Host: localhost
Connection: Close


Server fiber end
string(11) "Hello world"
Client fiber end
end