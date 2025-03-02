--TEST--
Channel: channel closed in cycle
--FILE--
<?php

Async\run(function() {
    $channel = new Async\Channel();

    Async\run(function() use($channel) {
        while (($data = $channel->receiveOrNull()) != null) {
            echo "receive: $data\n";
        }
    });

    $channel->send("event data");
});

echo "start\n";
Async\launchScheduler();
echo "end\n";

?>
--EXPECT--
start
receive: event data
end
