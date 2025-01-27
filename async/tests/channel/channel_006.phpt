--TEST--
Channel: two receivers and two producers
--FILE--
<?php

$channel = new Async\Channel();

Async\run(function() use($channel) {
    while (($data = $channel->receive()) != null) {
        echo "receive: $data\n";
    }
});

Async\run(function() use($channel) {
    while (($data = $channel->receive()) != null) {
        echo "receive: $data\n";
    }
});

Async\run(function() use($channel) {
    for ($i = 1; $i <= 3; $i++) {
        $channel->send("event data $i");
    }
});

Async\run(function() use($channel) {
    for ($i = 10; $i <= 13; $i++) {
        $channel->send("event data $i");
    }

    $channel->close();
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
receive: event data 1
receive: event data 10
receive: event data 2
receive: event data 11
receive: event data 3
receive: event data 12
receive: event data 13
end
