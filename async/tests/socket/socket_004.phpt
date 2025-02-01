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

    if (@!socket_bind($serverSocket, HOSTNAME, PORT)) {
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
});

Async\async(function() {
    echo "Client fiber start\n";
    // Error here: PHP Fatal error:  Uncaught Error: Call to undefined function
    some_function();
});

echo "start\n";
try {
    Async\launchScheduler();
} catch (\Error $e) {
    echo "Error occurred\n";
}
echo "end\n";
?>
--EXPECT--
start
Server fiber start
Client fiber start
Server socket bind failed
Error occurred
end