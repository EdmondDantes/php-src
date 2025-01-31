--TEST--
Get HTTP response from PHP CLI server using socket functions
--FILE--
<?php
Async\async(function() {
    echo "fiber start\n";

    echo gethostbyname("localhost") . "\n";

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
127.0.0.1
fiber end
end
